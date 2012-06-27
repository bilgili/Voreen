/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/vis/processors/networkevaluator.h"

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/geometry/geometrycontainer.h"
#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/exception.h"
#include "voreen/core/vis/idmanager.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/processors/portmapping.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/volumeselectionprocessor.h"
#include "voreen/core/vis/processors/volumesetsourceprocessor.h"
#include "voreen/core/vis/processors/image/cacherenderer.h"
#include "voreen/core/vis/processors/image/canvasrenderer.h"
#include "voreen/core/vis/processors/image/coarsenessrenderer.h"
#include "voreen/core/vis/processors/image/geometryprocessor.h"
#include "voreen/core/vis/processors/image/nullrenderer.h"
#include "voreen/core/vis/processors/entryexitpoints/entryexitpoints.h"

#include <queue>
#include <numeric>

namespace voreen {

const Identifier NetworkEvaluator::setReuseTextureContainerTargets_("set.reuseTextureContainerTargets");
const Identifier NetworkEvaluator::processorInvalidated_("processor.invalidated");
const Identifier NetworkEvaluator::addForbiddenTargets_("add.forbidden.targets");
const Identifier NetworkEvaluator::removeForbiddenTargets_("remove.forbidden.targets");

const std::string NetworkEvaluator::loggerCat_("voreen.NetworkEvaluator");

NetworkEvaluator::NetworkEvaluator()
    : readyToEvaluate_(false)
    , tc_(0)
    , geoContainer_(0)
    , reuseTextureContainerTargets_(false)
    , currentViewportSize_(0)
    , coarsenessActive_(false)
    , canvas_(0)
    , canvasCaches_(true)
    , portMapping_(0)
    , camera_(0)
{
    setTag("evaluator");

#ifdef DEBUG
    // add process wrapper for checking OpenGL state
    addProcessWrapper(new CheckOpenGLStateProcessWrapper());
#endif
#ifndef VRN_WITH_VOLUMECACHING
    volumeCache_.setEnabled(false);
#endif
}

NetworkEvaluator::NetworkEvaluator(std::vector<Processor*> processors)
    : readyToEvaluate_(false),
      tc_(0),
      geoContainer_(0),
      processors_(processors),
      reuseTextureContainerTargets_(false),
      portMapping_(0),
      camera_(0)
{
    setTag("evaluator");

#ifdef DEBUG
    // add process wrapper for checking OpenGL state
    addProcessWrapper(new CheckOpenGLStateProcessWrapper());
#endif
#ifdef VRN_WITH_VOLUMECACHING
    volumeCache_.setEnabled(false);
#endif
}

NetworkEvaluator::~NetworkEvaluator() {
    removeProcessWrappers();
    clearLocalPortMap();
    delete portMapping_;
}

//FIXME: either return an enum or throw an exception, but don't return numbers like -1 or -4!
int NetworkEvaluator::analyze() {
    priorityMap_.clear();
    stateMap_.clear();
    if (geoContainer_)
        geoContainer_->clear();
    readyToEvaluate_ = false;

    std::vector<Processor*> endProcessors;
    //go through all processors and propagate tc and camera to them
    for (size_t i = 0; i < processors_.size(); ++i) {
        processors_[i]->setGeometryContainer(geoContainer_);
        processors_[i]->setTextureContainer(tc_);

        if (processors_[i]->isEndProcessor())
            endProcessors.push_back(processors_[i]);

        //now check if there are any loops in the network
        //Every visited processor is put into this vector, and if it comes up again, there is a loop. This
        //could be done only for the last processor in the network, but in the future there might be more than
        //one, and then we would have to do this anyway.
        std::vector<Processor*> processorVector;
        int isThereLoop = checkForLoops(processors_.at(i),processorVector);
        if (isThereLoop != 0)
            return -2;

        // give every processor the priority -1, meaning that is shouldn't be rendered at all.
        // The real priority is set in the function assignPriority()
        priorityMap_.insert(std::pair<Processor*, int>(processors_[i], -1));

        // initialize the StateMap for the processors with an empty state.
        // this will ensure, that the cache is queried on first evaluation
        // of the network.
        //
        if (processors_[i]->getCacheable() == true)
            stateMap_.insert(std::make_pair(processors_[i], ""));
    }

    // return when there are no endprocessors in the network
    if (endProcessors.size() == 0)
        return -4;

    for (size_t i = 0; i < endProcessors.size(); ++i) {
        // assign a priority to a processor based on their (maximal)
        // distance(amount of processors) to the given endprocessor
        assignPriority(endProcessors[i], 0);
    }

    // sort the processors by that priority
    sortProcessorsByPriority();

    // assign appropriate PortData to every port of every processor
    doPortMapping();

    // analyze which render results might be cached during the render process
    initializeCaching();

    // after caching maps are created we can check which rendertargets are not allowed
    // to be overwritten by other evaluators
    addRenderTargetsUsedInCaching();

    // remove duplicates from vector
    std::sort(tempForbiddenTargets_.begin(), tempForbiddenTargets_.end());
    std::vector<int>::iterator new_end_pos = std::unique(tempForbiddenTargets_.begin(), tempForbiddenTargets_.end());
    if (new_end_pos != tempForbiddenTargets_.end())
        tempForbiddenTargets_.erase(new_end_pos);

    ForbiddenTextureTargets forbiddenTargets;
    forbiddenTargets.tc = tc_;
    forbiddenTargets.forbiddenTargets = tempForbiddenTargets_;
    MsgDistr.postMessage(new ForbiddenTargetsMsg(addForbiddenTargets_, forbiddenTargets));

     //everything set up and ready
    readyToEvaluate_ = true;

    return 0;
}

TextureContainer* NetworkEvaluator::getTextureContainer() {
    return tc_;
}

GeometryContainer* NetworkEvaluator::getGeometryContainer() {
    return geoContainer_;
}

std::vector<Processor*>& NetworkEvaluator::getProcessors() {
    return processors_;
}

bool NetworkEvaluator::isValid() {
    return readyToEvaluate_;
}

std::map<Processor*,int> NetworkEvaluator::getPriorityMap() {
    return priorityMap_;
}

//TODO: return void (can use getter instead)
TextureContainer* NetworkEvaluator::initTextureContainer(int finalTarget) {
    tc_ = TextureContainer::createTextureContainer(finalTarget + 1);
    tc_->setFinalTarget(finalTarget);

    if (!tc_->initializeGL()) {
        delete tc_;
        tc_ = 0;
        return tc_;
    }

    int renderTargetType =
        TextureContainer::VRN_RGBA_FLOAT16 |
        TextureContainer::VRN_DEPTH |
        TextureContainer::VRN_DEPTH_TEX;

    for (int i = 0 ; i < finalTarget ; ++i)
        tc_->initializeTarget(i, renderTargetType);

    tc_->initializeTarget(finalTarget, TextureContainer::VRN_FRAMEBUFFER);
    tc_->setFinalTarget(finalTarget);
    MsgDistr.postMessage(new BoolMsg(NetworkEvaluator::setReuseTextureContainerTargets_, false),"evaluator");

    return tc_;
}

void NetworkEvaluator::clearLocalPortMap() {
    typedef std::map<Processor*, LocalPortMapping*> LocalPortMap;

    for (LocalPortMap::iterator it = localPortMap_.begin(); it != localPortMap_.end(); ++it)
        delete it->second;
    localPortMap_.clear();
}

void NetworkEvaluator::doPortMapping() {
    if (geoContainer_)
        geoContainer_->clear();

    portMap_.clear();
    portEntries_.clear();
    tempForbiddenTargets_.clear();

    // this queue contains all available targets in the TC whereas the target with the smallest
    // ID has the highest priority!
    std::priority_queue<int, std::vector<int>, std::greater<std::vector<int>::value_type> > targetList;
    int numTargets = tc_->getNumAvailable();

    // At first all targets are free.
    std::vector<int> freeTargets;
    for (int i = (numTargets - 1); i >= 0; --i)
        freeTargets.push_back(i);

    // However other evaluators could have used those targets already,
    // so remove them.
    for (size_t i = 0; i < forbiddenTargets_.size(); ++i) {
        std::vector<int>::iterator it = std::find(freeTargets.begin(), freeTargets.end(), forbiddenTargets_[i]);
        if (it != freeTargets.end()) {
            freeTargets.erase(it);
        }
    }

    // Only targets not yet used remain, and are inserted into the queue
    for (size_t i=0; i<freeTargets.size(); ++i)
        targetList.push(freeTargets.at(i) );

    // Prevent the IDManger to choose a target, which is the Framebuffer.
    // This would happen if e.g. target #0 is of type VRN_FRAMEBUFFER.
    // Thererfore the list of free targets (priority in ascendeing order!) is
    // popped until a non-framebuffer target is found!
    int idManTarget = -1;
    do {
        idManTarget = targetList.top(); // should be 0 on first pass.
        targetList.pop();
    }
    while ((tc_->getAttr(idManTarget) & TextureContainer::VRN_FRAMEBUFFER) != 0);

    //give the id Manager its target in the texturecontainer
    IDManager id1;
    id1.getContent()->textureTarget_ = idManTarget;

    PortDataTexture* pdt = 0;

    int target = -1;
    //go through all outports of all processors to determine their targets in the TC ,
    //their dataset inputs, and their coprocessors
    for (size_t i = 0; i < processors_.size(); ++i) {
        Processor* processor = processors_[i];
        std::vector<Port*> outports = processor->getOutports();
        for (size_t j = 0; j < outports.size(); ++j) {
            Port* port = outports[j];

            // Ports now know their PortData which can be queried by calling
            // getPortData(). If the outport was created "traditionally" by
            // not using createGenericOutport<T>, the return value will be 0.
            // Then the "old" method is applied to check, what PortData are required
            // for the port.
            // In all cases, the port data are
            //
            // FIXME: this fallback mechanism produces memory leaks:
            // PortData objects are created using new, but they are never deleted!
            // If the PortData can be obtained from the port directly, as they were
            // created by using PortDataGeneric (i.e. via createGenericOutport<T>) 
            // which are known by they ports themselves, then this problem does not 
            // occur because the Port's dtor will delete the PortData. (dirk)
            //
            PortData* portData = port->getPortData();
            if (portData == 0) {
                if ((port->getType().getSubString(0) == "geometry") && (geoContainer_ != 0)) {
                    const int geometryContainerNumber = geoContainer_->getNextID();
                    portData = new PortDataGeometry(geometryContainerNumber);
                }
                else if (port->getType().getSubString(0) == "volumehandle") {
                    // the processor knows the pointer to the volume because
                    // the current port is the outport of the current processor!
                    if (typeid(*processor) == typeid(VolumeSelectionProcessor)) {
                        VolumeSelectionProcessor* vsp = dynamic_cast<VolumeSelectionProcessor*>(processor);
                        portData = new PortDataGeneric<VolumeHandle**>(vsp->getVolumeHandleAddress(), "portdata.volumehandle");

                    }
                    LWARNING("processor '" << processor->getName() <<"' created a VolumeHandle outport "
                        << "probably not using createGenericOutport()!");
                } else {
                    // Do PortMapping for "normal" ports
                    // newPortEntry() creates a new entry in the portEntries_ map for the current port, saving
                    // the port together with its connected ports. This is needed to determine when to free
                    // the TC target this port renders to. If the port is set to persistent, no entry is made
                    // and as a result the target is never freed.

                    // If the port isn't connected to another port, it is unused and doesn't need target. If
                    // you want a target even though the port isn't connected, get a private port.
                    if (port->getConnected().size() > 0) {
                        bool isPortForbiddenForOtherEvaluators = false;
                        if (!port->getIsPersistent())
                            newPortEntry(port);
                        else
                            isPortForbiddenForOtherEvaluators = true;

                        if (processor->getOutportToInportMap()[port] != 0) {
                            Port* tempPort = processor->getOutportToInportMap()[port];
                            std::vector<PortData*> portDataTemp = portMap_[tempPort];
                            if (portDataTemp.size() > 0) {
                                target = dynamic_cast<PortDataTexture*>(portDataTemp.at(0))->getData();
                                if (isPortForbiddenForOtherEvaluators)
                                    tempForbiddenTargets_.push_back(target);
                            } else {
                                target = targetList.top();
                                if (isPortForbiddenForOtherEvaluators)
                                    tempForbiddenTargets_.push_back(target);
                                //remove the target from the queue, thereby indicating that it is used
                                targetList.pop();
                            }
                        } else {
                            if (targetList.empty())
                                throw VoreenException("No more free targets available. Try to enable the target reusing.");
                            target = targetList.top();
                            if (isPortForbiddenForOtherEvaluators)
                                tempForbiddenTargets_.push_back(target);
                            //remove the target from the queue, thereby indicating that it is used
                            targetList.pop();
                        }

                         portData = new PortDataTexture(target);
                    }   // if (port->getConnected() > 0)
                }   // else (port type identification)
            }   // if (portData == 0)

            // No part data was mapped to this port - ignore
            if (portData == 0)
                continue;

            std::vector<PortData*> tempVec;
            tempVec.push_back(portData);
            portMap_.insert(std::pair<Port*,std::vector<PortData*> >(port, tempVec));

            std::vector<Port*> connectedPorts = port->getConnected();
            //every port connected to this one must read its input from the same target
            for (size_t k = 0; k < connectedPorts.size(); ++k) {
                std::map<Port*, std::vector<PortData*> >::iterator finder =
                    portMap_.find(connectedPorts[k]);
                if (finder != portMap_.end())
                    portMap_[connectedPorts[k]].push_back(portData);
                else
                    portMap_.insert(std::pair<Port*,std::vector<PortData*> >(connectedPorts[k], tempVec));
            }
        }   // for (j) (ports)

        // Every processor can reserve TextureContainer targets that are not part of the network,
        // the Combine postprocessor for example needs two temporary targets to function. These
        // are mapped here
        std::vector<Port*> privatePorts = processor->getPrivatePorts();
        for (size_t j = 0; j < privatePorts.size(); ++j) {
            Port* port = privatePorts[j];
            target = targetList.top();
            tempForbiddenTargets_.push_back(target);
            //remove the target from the queue, thereby indicating that it is used
            targetList.pop();

            //map this port to the target we just got
            std::vector<PortData*> tempVec;
            pdt = new PortDataTexture(target);
            tempVec.push_back(pdt);
            portMap_.insert(std::pair<Port*,std::vector<PortData*> >(port,tempVec));
        }

        // Now we go through the CoProcessorOutports. Luckily we don't have to free any targets etc
        // as in the "normal" outports
        std::vector<Port*> coProcPorts = processor->getCoProcessorOutports();
        for (size_t j = 0; j < coProcPorts.size(); ++j) {
            Port* currentPort = coProcPorts[j];
            PortDataCoProcessor* pdcp = new PortDataCoProcessor(processor,currentPort->getFunctionPointer());
            std::vector<PortData*> tempVec;
            tempVec.push_back(pdcp);
            portMap_.insert(std::pair<Port*,std::vector<PortData*> >(currentPort,tempVec));

            std::vector<Port*> connectedPorts = currentPort->getConnected();
            std::map<Port*,std::vector<PortData*> >::iterator finder;
            for (size_t k = 0; k < connectedPorts.size(); ++k) {
                finder = portMap_.find(connectedPorts[k]);
                if (finder != portMap_.end())
                    portMap_[connectedPorts.at(k)].push_back(pdcp);
                else
                    portMap_.insert(std::pair<Port*,std::vector<PortData*> >(connectedPorts.at(k),tempVec));
            }
        }

        // The processors are already sorted by the priority by which they are rendered. We go through the
        // portEntries vector and remove the current processor from all outports that are connected to it.
        // If an outport in result has no more connected processors, its target in the TC can be used again
        if (reuseTextureContainerTargets_) {
            for (size_t z = 0; z < processor->getInports().size(); ++z) {
                Port* finishedPort = processor->getInports()[z];
                for (size_t j = 0; j < portEntries_.size(); ++j) {
                    std::vector<Port*> portVector = portEntries_.at(j)->connectedPorts;
                    bool entryFound = true;

                    while (entryFound) {
                        // search the finished processor
                        size_t k = 0;
                        while ((k < portVector.size()) && (portVector[k] != finishedPort))
                            ++k;

                        if (k >= portVector.size())
                            entryFound = false;
                        else {
                            // if found erase it from all outports connected to it
                            portVector.erase(portVector.begin() + k);
                            // check if there are still connected processors and if not, insert the target to the queue
                            // again
                            if (portVector.size() < 1) {
                                // the queue always returns the biggest target, so we have to insert the original value
                                // we got.
                                //
                                std::vector<PortData*> freeTargets = portMap_[portEntries_[j]->port];
                                for (size_t w = 0; w < freeTargets.size(); ++w)
                                    targetList.push(dynamic_cast<PortDataTexture*>(freeTargets[w])->getData());
                            }   // if
                        }   // else
                    }   // while (entryFound)
                } // for (j
            } // for (z
        } // if (reuseTextureContainerTargets_
    } // for (i) (loop for processors)

    // This has been added when Jennis discovered the combinepp bug, that the order in which
    // the connections were made didn't affect the portmapping, which is important in the combiner.
    // This reorders the PortData* vector for inports (and coprocessor inports)
    // that can have multiple connections. Maybe this could be done in the loop above, I'm not sure (Stephan).
    for (size_t i = 0; i < processors_.size(); ++i) {
        for (size_t j = 0; j < processors_[i]->getInports().size(); ++j) {
            Port* p = processors_[i]->getInports()[j];
            if (p->allowMultipleConnections()) {    //only do this for inports that have more than one connection.
                std::vector<PortData*> newPortData;
                std::vector<PortData*> portData = portMap_[p];
                std::vector<Port*> connectedPorts = p->getConnected();
                for (size_t k = 0; k < connectedPorts.size(); ++k) {
                    std::vector<PortData*> connectedPortData = portMap_[connectedPorts[k]];
                    if (connectedPortData.size() > 0 ) {
                        if (connectedPortData[0] != portData[k]) {
                            int position = -1;
                            for (size_t l = 0; l < portData.size(); ++l) {
                                if (portData[l] == connectedPortData[0]) {
                                    position = l;
                                    break;
                                }
                            }
                            if (position != -1)
                                newPortData.push_back(portData[position]);
                        }
                        else {
                            newPortData.push_back(connectedPortData[0]);
                        }
                    }
                }
                portMap_[p] = newPortData;
            }

        }
        //now do this for coprocessors too.
        for (size_t j = 0; j < processors_[i]->getCoProcessorInports().size(); ++j) {
            Port* p = processors_[i]->getCoProcessorInports()[j];
            if (p->allowMultipleConnections()) {    //only do this for inports that have more than one connection.
                std::vector<PortData*> newPortData;
                std::vector<PortData*> portData = portMap_[p];
                std::vector<Port*> connectedPorts = p->getConnected();
                for (size_t k = 0; k < connectedPorts.size(); k++) {
                    std::vector<PortData*> connectedPortData = portMap_[connectedPorts[k]];
                    if (connectedPortData.size() > 0 ) {
                        if (connectedPortData[0] != portData[k]) {
                            int position = -1;
                            for (size_t l = 0; l < portData.size(); ++l) {
                                if (portData[l] == connectedPortData[0]) {
                                    position = l;
                                    break;
                                }
                            }
                            if (position != -1) {
                                newPortData.push_back(portData[position]);
                            }
                        } else {
                            newPortData.push_back(connectedPortData[0]);
                        }
                    }
                }
                portMap_[p] = newPortData;
            }

        }
    }

    clearLocalPortMap();
    delete portMapping_;
    portMapping_ = new PortMapping(portMap_);

    LocalPortMapping* localMapping = 0;
    for (size_t i = 0; i < processors_.size(); ++i) {
        localMapping = portMapping_->createLocalPortMapping(processors_[i]);
        localPortMap_.insert(std::make_pair(processors_[i], localMapping));
    }

    LINFO("portmapping created, using " << (numTargets - targetList.size()) << " render targets");
}

void NetworkEvaluator::initializeCaching() {
    // clear previous entries in both maps
    cachedProcessorsStatus_.clear();
    successorMap_.clear();

    // first we have to find the canvasrenderer in the network if this was not already done
    if (canvas_)
        canvasCaches_ = canvas_->usesCaching();
    else {
        for (size_t i = 0; i < processors_.size(); ++i) {
            canvas_ = dynamic_cast<CanvasRenderer*>(processors_[i]);
            if (canvas_ != 0) {
                canvasCaches_ = canvas_->usesCaching();
                break;
            }
        }
    }

    // now a map is created that contains all processors that are involved in caching
    // the value in the map indicates the caching status of the processor
    for (size_t i = 0; i < processors_.size(); ++i) {
        if (processors_[i]->getCacheable() && (!processors_[i]->isEndProcessor())) {
            // the processor supports caching and is not an endprocessor
            // -> test whether all outports are connected to an endprocessor or a cacherenderer
            bool insertIntoMap = true;
            std::vector<Port*> portsOut = getConnectedOutports(processors_[i]);
            for (size_t j = 0; j < portsOut.size(); ++j)
                insertIntoMap &= isConnectedToEndProcessorOrCacheRenderer(portsOut[j]);

            // Test whether the processor is after a processor that does not support caching before
            // a cacherenderer appears or the top of the network is reached. In that case the results
            // of the processor can not be cached.
            std::vector<Port*> portsIn = getConnectedInports(processors_[i]);
            for (size_t j = 0; j < portsIn.size(); ++j)
                insertIntoMap &= !isAfterNotCacheableProcessor(portsIn[j]);

            // insert processor into map
            if (insertIntoMap)
                cachedProcessorsStatus_.insert(std::make_pair(processors_[i], false));
        }
    }   // for (i... (processors)

    // for all processors in the cachingMap find the processors that are behind the processor
    std::map<Processor*, bool>::iterator it = cachedProcessorsStatus_.begin();
    for ( ; it != cachedProcessorsStatus_.end(); ++it) {
        std::vector<Processor*> result = getConnectedForward(it->first);
        successorMap_.insert(std::make_pair(it->first, result));
    }
}

std::vector<Port*> NetworkEvaluator::getConnectedOutports(const Processor* processor) const {
    std::vector<Port*> connectedOutports;
    std::vector<Port*> outports = processor->getOutports();
    for (size_t i = 0; i < outports.size(); ++i) {
        if (outports[i]->getConnected().size() > 0)
            connectedOutports.push_back(outports[i]);
    }
    std::vector<Port*> coProcessorOutports = processor->getCoProcessorOutports();
    for (size_t i = 0; i < coProcessorOutports.size(); ++i) {
        if (coProcessorOutports[i]->getConnected().size() > 0)
            connectedOutports.push_back(coProcessorOutports[i]);
    }

    return connectedOutports;
}

std::vector<Port*> NetworkEvaluator::getConnectedInports(const Processor* processor) const {
    std::vector<Port*> connectedInports;
    std::vector<Port*> inports = processor->getInports();
    for (size_t i = 0; i < inports.size(); ++i) {
        if (inports[i]->getConnected().size() > 0)
            connectedInports.push_back(inports[i]);
    }
    std::vector<Port*> coProcessorInports = processor->getCoProcessorInports();
    for (size_t i = 0; i < coProcessorInports.size(); ++i) {
        if (coProcessorInports[i]->getConnected().size() > 0)
            connectedInports.push_back(coProcessorInports[i]);
    }

    return connectedInports;
}

bool NetworkEvaluator::isConnectedToEndProcessorOrCacheRenderer(Port* p) const {
    std::vector<Port*> connected = p->getConnected();
    for (size_t i = 0; i < connected.size(); ++i) {
        Processor* processor = connected[i]->getProcessor();
        if (typeid(*processor) == typeid(CacheRenderer))
            return true;
        else if (!processor->getCacheable())
            return false;
        else if (processor->isEndProcessor()) {
            if (processor == canvas_)
                return canvasCaches_;
        }
        else {
            std::vector<Port*> outports = getConnectedOutports(processor);
            bool returnValue = true;
            for (size_t j = 0; j < outports.size(); ++j) {
                returnValue &= isConnectedToEndProcessorOrCacheRenderer(outports[j]);
            }
            return returnValue;
        }
    }
    return false;
}

bool NetworkEvaluator::isAfterNotCacheableProcessor(Port* p) const {
    std::vector<Port*> connected = p->getConnected();
    for (size_t i = 0; i < connected.size(); ++i) {
        Processor* processor = connected[i]->getProcessor();
        if (!processor->getCacheable())
            return true;
        else if (typeid(*processor) == typeid(CacheRenderer) || processor->isEndProcessor())
            return false;
        else {
            std::vector<Port*> inports = getConnectedInports(processor);
            bool returnValue = false;
            for (size_t j = 0; j < inports.size(); ++j) {
                returnValue |= isAfterNotCacheableProcessor(inports[j]);
            }
            return returnValue;
        }
    }
    return false;
}

std::vector<Processor*> NetworkEvaluator::getConnectedForward(Processor* processor) const {
    std::vector<Processor*> result;
    getConnectedForwardRecursive(processor, result);
    if (result.size() != 0)
        result.erase(result.begin());
    return result;
}

void NetworkEvaluator::getConnectedForwardRecursive(Processor* processor, std::vector<Processor*>& result) const {
    if (processor->isEndProcessor())
        return;

    std::vector<Processor*>::iterator it = std::find(result.begin(), result.end(), processor);
    if (it != result.end())
        return; // processor was already inserted

    // insert processor to the visited ones
    result.push_back(processor);
    std::vector<Port*> ports = getConnectedOutports(processor);
    for (size_t i = 0; i < ports.size(); ++i) {
        std::vector<Port*> connected = ports[i]->getConnected();
        for (size_t j = 0; j < connected.size(); ++j) {
            getConnectedForwardRecursive(connected[j]->getProcessor(), result);
        }
    }
}

void NetworkEvaluator::getConnectedBackwardRecursive(Processor* processor, std::vector<Processor*>& result) const {
    //test whether the processor was already inserted
    std::vector<Processor*>::iterator it = std::find(result.begin(), result.end(), processor);
    if (it != result.end())
        return;

    result.push_back(processor);
    std::vector<Port*> ports = processor->getInports();
    std::vector<Port*> coProc = processor->getCoProcessorInports();

    // insert coprocessor ports into ports vector
    ports.insert(ports.end(), coProc.begin(), coProc.end());
    // iterate through all ports and call this method recursively for all processors connected
    // to one port
    for (size_t i = 0; i < ports.size(); ++i) {
        std::vector<Port*> connected = ports[i]->getConnected();
        for (size_t j = 0; j < connected.size(); ++j) {
            getConnectedBackwardRecursive(connected[j]->getProcessor(), result);
        }
    }
}

void NetworkEvaluator::assignPriority(Processor* processor,int priority) {
    if (processor->getIsCoprocessor()) {
        if (processor->getInports().size() < 1) {
            //if the processor is only a coprocessor and has no normal inports, it doesn't need to render
            priorityMap_[processor] = -1;
            return;
        }
    }
    if (priorityMap_[processor] < priority)
        priorityMap_[processor] = priority;
    else
        priority=priorityMap_[processor];

    for (size_t i=0; i<processor->getInports().size(); ++i) {
        for (size_t j=0; j<processor->getInports().at(i)->getConnected().size(); j++) {
            if (!processor->getInports().at(i)->getConnected().at(j)->getProcessor()->isEndProcessor())
                assignPriority(processor->getInports().at(i)->getConnected().at(j)->getProcessor(), priority+1);
        }
    }
    for (size_t i=0; i<processor->getCoProcessorInports().size(); i++) {
        for (size_t j=0; j<processor->getCoProcessorInports().at(i)->getConnected().size(); j++)
            if (!processor->getCoProcessorInports().at(i)->getConnected().at(j)->getProcessor()->isEndProcessor())
                assignPriority(processor->getCoProcessorInports().at(i)->getConnected().at(j)->getProcessor(), priority+1);
    }
}

//TODO: Maybe add a flag to the processors indicating if they have been visited yet. That would make this
//a lot faster for big processor vectors.
//FIXME: either return an enum or throw an exception, but don't return numbers like -1 or -4!
int NetworkEvaluator::checkForLoops(Processor* processor, std::vector<Processor*>& processorVector) {
    if (checkIfVectorContainsProcessor(processor,processorVector))
        return -2;
    else {
        processorVector.push_back(processor);
        int returnValue=0;
        for (size_t i=0; i<processor->getInports().size(); i++) {
            for (size_t j=0; j<processor->getInports().at(i)->getConnected().size(); ++j) {
                returnValue=checkForLoops(processor->getInports().at(i)->getConnected().at(j)->getProcessor(),processorVector);
                if (returnValue!=0 )
                    return -2;
            }
        }
        int deletePos=-1;
        for (size_t i=0; i<processorVector.size(); ++i) {
            if (processorVector.at(i) == processor)
                deletePos=i;
        }
        if (deletePos>-1)
            processorVector.erase(processorVector.begin()+deletePos);
    }
    return 0;
}

bool NetworkEvaluator::checkIfVectorContainsProcessor(Processor* processor, std::vector<Processor*>& processorVector) {
    for (size_t i = 0; i < processorVector.size(); ++i) {
        if (processor == processorVector.at(i))
            return true;
    }
    return false;
}

std::vector<Processor*> NetworkEvaluator::findEndProcessors() {
    std::vector<Processor*> p;
    for (size_t i=0; i < processors_.size(); ++i) {
        if (processors_[i]->isEndProcessor())
            p.push_back(processors_[i]);
    }
    return p;
}

//TODO: this calls process() on the processors, so why not call is process() also? joerg
int NetworkEvaluator::evaluate() {
    if (readyToEvaluate_) {
        LGL_ERROR;
        IDManager id1;
        id1.signalizeNewRenderingPass();
        LGL_ERROR;

        for (size_t i = 0; i < processors_.size(); ++i) {
            Processor* currentProcessor = processors_[i];
            if (priorityMap_[currentProcessor] == -1)
                continue;

            // check whether the status of the useCaching property in the canvas_ has changed
            // if this is the case the cache maps must be regenerated
            if (canvasCaches_ != canvas_->usesCaching())
                initializeCaching();
            
            bool needsProcessing = false;
            std::map<Processor*, bool>::iterator it = cachedProcessorsStatus_.find(currentProcessor);
            if ((it == cachedProcessorsStatus_.end()) || (!it->second))
                needsProcessing = true;

            CacheState cacheState = CACHE_STATE_UNKNOWN;
            if (volumeCache_.isEnabled()) {
                if (currentProcessor->usesVolumeCaching())
                    cacheState = handleVolumeCaching(currentProcessor);

                switch (cacheState) {
                    case CACHE_STATE_OK:
                    case CACHE_STATE_STATE_UPDATE_NEEDED:
                        needsProcessing = false;
                        break;
                    default:
                        needsProcessing = true;
                        break;
                }
            }

            if (needsProcessing == true) {
                // processor is not involoved in caching or the render result is not valid anymore
                // -> render again
                try {
                    for (size_t j = 0; j < processWrappers_.size(); ++j)
                        processWrappers_[j]->beforeProcess(currentProcessor);

                    // run the Processor when its initialized correctly
                    if (currentProcessor->getInitStatus() == Processor::VRN_OK) {
                        //viewport does not match size of processor
                        //-> adjust viewport and remember new size
                        if (currentViewportSize_ != currentProcessor->getSize()) {
                            glViewport(0, 0, currentProcessor->getSize().x,
                                             currentProcessor->getSize().y);
                            currentViewportSize_ = currentProcessor->getSize();
                        }

                        currentProcessor->process(localPortMap_[currentProcessor]);
                    }

                    for (size_t j = 0; j < processWrappers_.size(); ++j)
                        processWrappers_[j]->afterProcess(currentProcessor);

                    // set render result valid
                    if (it != cachedProcessorsStatus_.end())
                        it->second = true;

                } catch (VoreenException& e) {
                    LERROR(e.what());
                } catch (std::exception& e) {
                    LERROR("In NetworkEvaluator::evaluate(): Exception from "
                           << currentProcessor->getClassName().getName()
                           << " (" << currentProcessor->getName() << "): " << e.what());
                }
            }   // if (needsProcessing)

            if (volumeCache_.isEnabled()) {
                switch (cacheState) {
                    case CACHE_STATE_NEED_UPDATE:
                        volumeCache_.update(processors_[i], localPortMap_[currentProcessor]);
                        // no break here
                    case CACHE_STATE_STATE_UPDATE_NEEDED:
                        updateStateMap(&(processors_[i]));
                        break;
                    default:
                        break;
                }
            }
        }   // for (i...    (loop for processors)

        // TODO: check whether this is a good place for deleting rendered Geometry
        // from the container. If so, check for Geometry pointers which may need to
        // survive calls to this method for evaluating the network. By now, all pointers
        // are disposed after a rendering / processing pass of the network and have to be
        // recreated for the next call.
        // Might be inefficient and may possibly also slow down the rendering.
        // Further methods to GeometryContainer or Geometry class probably
        // will have to be added. (Dirk)
        //
        if (geoContainer_ != 0)
            geoContainer_->clear();

        return 0;
    }
    else
        return -1;
}

int NetworkEvaluator::evaluate(Processor* endProcessor) {
    for (size_t i=0; i < processors_.size(); ++i) {
        Processor* currentProcessor = processors_.at(i);

        if (currentProcessor != endProcessor) {
            if (priorityMap_[currentProcessor] != -1 && currentProcessor->getInitStatus() == Processor::VRN_OK)
                currentProcessor->process(localPortMap_[currentProcessor]);
        }
        else
            break;
    }
    return 0;
}

void NetworkEvaluator::newPortEntry(Port* p) {
    PortEntry* newEntry = new PortEntry();
    newEntry->port = p;
    newEntry->connectedPorts = p->getConnected();
    portEntries_.push_back(newEntry);
}

/* Simple select sort to sort the processor vector by the priority of the contained processors. Maybe change this
 * to quicksort or something else, but I don't think we lose much time here, this has only to be done once and
 * the vectors should always be quite small
 */
void NetworkEvaluator::sortProcessorsByPriority() {
    for (size_t i=0; i<processors_.size(); ++i) {
        int pos=i;
        bool swap=false;
        for (size_t j=i; j<processors_.size(); ++j) {
            if (priorityMap_[processors_.at(j)] > priorityMap_[processors_.at(pos)] ) {
                pos=j;
                swap=true;
            }
        }
        if (swap) {
            Processor* swapper = processors_.at(pos);
            processors_.at(pos)=processors_.at(i);
            processors_.at(i) = swapper;
        }
    }
}

void NetworkEvaluator::setProcessors(std::vector<Processor*> processors) {
    processors_.clear();
    processors_ = processors;
    canvas_ = 0;
    
    for (size_t i=0; i < processors_.size(); i++) {
        processors_[i]->setTextureContainer(getTextureContainer());
        if (camera_)
            processors_[i]->setCamera(camera_);        
    }
    
    readyToEvaluate_ = false;
    portEntries_.clear();
    ForbiddenTextureTargets targetsToRemove;
    targetsToRemove.tc = tc_;
    targetsToRemove.forbiddenTargets = forbiddenTargets_;
    MsgDistr.postMessage(new ForbiddenTargetsMsg(removeForbiddenTargets_, targetsToRemove));
}

void NetworkEvaluator::setGeometryContainer(GeometryContainer* const geoContainer) {
    geoContainer_ = geoContainer;
}

void NetworkEvaluator::setTextureContainer(TextureContainer* tc) {
    tc_ = tc;
}

void NetworkEvaluator::invalidateRendering() {
    std::map<Processor*, bool>::iterator it = cachedProcessorsStatus_.begin();
    for ( ; it != cachedProcessorsStatus_.end(); ++it) {
        it->second = false;
    }
}

int NetworkEvaluator::initializeGL() {
    bool failed = false;
    for (size_t i=0; i < processors_.size(); ++i) {
        if (processors_[i]->initializeGL() != Processor::VRN_OK) {
            LERROR("initializeGL failed for " << processors_[i]->getClassName().getName()
                   << " (" << processors_[i]->getName() << ")");
            // don't break, try to initialize the other processors even if one failed
            failed = true;
        }
    }
    return (failed ? Processor::VRN_ERROR : Processor::VRN_OK);
}

void NetworkEvaluator::setSize(const tgt::ivec2& size) {
    // the viewport in OpenGL is already set to new size in voreenpainter (calls this method)
    // so only save new viewportsize
    currentViewportSize_ = size;
    for (size_t i=0; i < processors_.size(); ++i)
        processors_[i]->setSize(size);
}

void NetworkEvaluator::switchCoarseness(bool active) {
    if (coarsenessActive_ == active)
        return;

    coarsenessActive_ = active;

    // find all coarsenessrenderer in the network
    std::vector<CoarsenessRenderer*> coarseRenderer;
    for (size_t i=0; i<processors_.size(); ++i) {
        if (typeid(*processors_[i]) == typeid(CoarsenessRenderer))
            coarseRenderer.push_back(dynamic_cast<CoarsenessRenderer*>(processors_[i]));
    }

    for (size_t i = 0; i < coarseRenderer.size(); ++i) {
        // activate or deactivate coarseness in CoarsenessRenderer
        coarseRenderer[i]->setUseCoarseness(coarsenessActive_);
        // calculate coarsenessFactor
        float coarseFactor;
        if (coarsenessActive_)
            coarseFactor = 1.f / static_cast<float>(coarseRenderer[i]->getCoarsenessFactor());
        else
            coarseFactor = static_cast<float>(coarseRenderer[i]->getCoarsenessFactor());

        std::vector<Processor*> visitedProcessors;
        getConnectedBackwardRecursive(coarseRenderer[i], visitedProcessors);
        // ignore first entry because its the coarsenessRenderer itself
        visitedProcessors.erase(visitedProcessors.begin());
        // iterate through all processors and set new size according to coarseness factor
        std::vector<Processor*>::iterator it;
        for (it = visitedProcessors.begin(); it != visitedProcessors.end(); ++it) {
            // set new size in processor, need float size here
            (*it)->setSize((*it)->getSizeFloat() * coarseFactor);
        }
    }
}

void NetworkEvaluator::processMessage(Message* msg ,const Identifier& /*dest*/) {
    bool postMsgToProcessors = true;

    if (msg->id_ == "evaluate") {
        evaluate(msg->getValue<Processor*>());
        postMsgToProcessors = false;
    }
    else if (msg->id_ == "do.portmapping") {
        Processor* p = msg->getValue<Processor*>();
        if (processors_.end() != std::find(processors_.begin(), processors_.end(), p))
            doPortMapping();
        postMsgToProcessors = false;
    }
    else if (msg->id_ == setReuseTextureContainerTargets_) {
        reuseTextureContainerTargets_=msg->getValue<bool>();
        postMsgToProcessors = false;
    }
    else if (msg->id_ == processorInvalidated_) {
        updateCachingStatus(msg->getValue<Processor*>());
        postMsgToProcessors = false;
    }
    else if (msg->id_ == addForbiddenTargets_) {
        addForbiddenTargets(msg->getValue<ForbiddenTextureTargets>());
        postMsgToProcessors = false;
    }
    else if (msg->id_ == removeForbiddenTargets_) {
        removeForbiddenTargets(msg->getValue<ForbiddenTextureTargets>());
        postMsgToProcessors = false;
    }

    if (msg->id_ == VoreenPainter::cameraChanged_) {
        // This message is already caught by any subclass of EntryExitPoints
        // and the rendering is invalidated there. But we must do it again
        // because there are networks that don't have any EntryExitPoints processor.
        // Maybe we can check the entire network for EntryExitPoints and only invalidate
        // when there are no EEPs.
        invalidateRendering();
    }
    if (msg->id_ == VoreenPainter::removeEventListener_)
        postMsgToProcessors = false;

    if (postMsgToProcessors) {
        for (size_t i=0; i < processors_.size(); ++i)
            processors_[i]->processMessage(msg);
    }
    // perhaps the ProcessWrapper objects are interested
    for (size_t i=0; i < processWrappers_.size(); i++)
        processWrappers_[i]->processMessage(msg);
}

void NetworkEvaluator::removeProcessor(Processor* processor) {
    for (size_t i=0; i<processors_.size(); ++i) {
        if (processor == processors_.at(i)) {
            processors_.erase(processors_.begin() + i);
            if (processor == canvas_)
                canvas_ = 0;
            break;
        }
    }
}

void NetworkEvaluator::updateCachingStatus(Processor* processor) {
    // look whether the given processor is involved in caching
    std::map<Processor*, bool>::iterator invalid = cachedProcessorsStatus_.find(processor);
    if (invalid != cachedProcessorsStatus_.end()) {
        // set calling processor itself invalid
        invalid->second = false;

        // get all successors for the processor and invalidate them when they are in the
        // map of caching processors
        std::map<Processor*, std::vector<Processor*> >::iterator found = successorMap_.find(processor);
        std::vector<Processor*>::iterator toInvalidate = found->second.begin();
        for ( ; toInvalidate != found->second.end(); ++toInvalidate) {
            invalid = cachedProcessorsStatus_.find(*toInvalidate);
            if (invalid != cachedProcessorsStatus_.end())
                // processor found -> result invalidated
                invalid->second = false;
        }
    }
}


NetworkEvaluator::CacheState NetworkEvaluator::handleVolumeCaching(Processor* const processor) {
    // at first test for the two simplest cases
    //
    if (processor == 0)
        return CACHE_STATE_ERROR_PROCESSOR_INVALID;

    if (stateMap_.empty())
        return CACHE_STATE_ERROR_STATEMAP_EMPTY;

    if (!volumeCache_.isCompatible(processor))
        return CACHE_STATE_INCOMPATIBLE_PROCESSOR;

    // try to find the processor's state in the StateMap
    //
    StateMap::const_iterator it = stateMap_.find(processor);
    if (it == stateMap_.end())
        return CACHE_STATE_ERROR_STATE_MISSING;
    const std::string& prevState = it->second;

    // compare previous state with current one:
    // if the states are equal, the cache value from the cache has
    // already been taken and no further operation is required.
    //
    if (processor->getState() == prevState)
        return CACHE_STATE_OK;

    // if the states are not equal, the cache has to be queried for
    // eventually cached data and the PortMapping needs to be remapped. 
    // this is especially necessary if the processors state from the StateMap 
    // is empty on first evaluation of the currently used network
    //
    if (remapVolumeHandlePortMapping(processor) == true)
        return CACHE_STATE_STATE_UPDATE_NEEDED;

    return CACHE_STATE_NEED_UPDATE;
}

bool NetworkEvaluator::remapVolumeHandlePortMapping(Processor* const processor) {
    std::vector<Port*> outports = volumeCache_.getCacheConcernedOutports(processor);
    LocalPortMapping* portMapping = localPortMap_[processor];
    bool result = true;

    for (size_t i = 0; i < outports.size(); ++i) {
        VolumeHandle* handle = volumeCache_.find(processor, outports[i], portMapping);
        result = result && (handle != 0);
        if (result == false)
            break;

        // Using this construction, the VolumeHandle* in handle is directly 'injected' to
        // the processor's attribute holding usually its VolumeHandle which is usually set
        // by calling process().
        // This work-around permits to not deal with the PortMapping again. But as VolumeHandle
        // ports ALWAYS take the ADDRESS OF A POINTER to a VolumeHandle, this is no real hack.
        // doPortMapping() basically does the same, but it takes the VolumeHandle** rather from
        // the processor directly than from the cache. (dirk)
        //
        VolumeHandle** handleAddr = portMapping->getGenericData<VolumeHandle**>(outports[i]->getType());
        if (handleAddr != 0)
            *handleAddr = handle;
    }
    return result;
}

void NetworkEvaluator::updateStateMap(Processor** const processorAddr) {
    if ((processorAddr == 0) || (*processorAddr == 0))
        return;

    Processor* processor = *processorAddr;

    // The processor's inport configuration has to be treated as part of
    // its internal state to ensure notifications, when the inport data 
    // change as this involves the cache...
    //
    LocalPortMapping* portMapping = localPortMap_[processor];
    std::string inportConfig = volumeCache_.getProcessorsInportConfig(processor, portMapping);

    std::string state = processor->getState();
    state += ",inports=" + inportConfig;

    std::pair<StateMap::iterator, bool> res = 
        stateMap_.insert(std::make_pair(processor, state));
    if (res.second == false)
        (res.first)->second = state;
}

void NetworkEvaluator::addForbiddenTargets(ForbiddenTextureTargets forbidden) {
    if (forbidden.tc != tc_)
        return;

    std::vector<int> newForbidden = forbidden.forbiddenTargets;
    for (size_t i = 0; i < newForbidden.size(); ++i) {
        std::vector<int>::iterator it = std::find(forbiddenTargets_.begin(), forbiddenTargets_.end(), newForbidden[i]);
        if (it == forbiddenTargets_.end())
            forbiddenTargets_.push_back(newForbidden[i]);
    }
}

void NetworkEvaluator::removeForbiddenTargets(ForbiddenTextureTargets forbidden) {
    if (forbidden.tc != tc_)
        return;

    std::vector<int> toRemove = forbidden.forbiddenTargets;
    for (size_t i = 0; i < toRemove.size(); ++i) {
        std::vector<int>::iterator it = std::find(forbiddenTargets_.begin(), forbiddenTargets_.end(), toRemove[i]);
        if (it != forbiddenTargets_.end())
            forbiddenTargets_.erase(it);
    }
}

void NetworkEvaluator::addRenderTargetsUsedInCaching() {
    std::map<Processor*, bool>::iterator it = cachedProcessorsStatus_.begin();
    for ( ; it != cachedProcessorsStatus_.end(); ++it) {
        std::vector<Port*> outports = it->first->getOutports();
        for (size_t j = 0; j < outports.size(); ++j) {
            Port* currentPort = outports[j];
            if (currentPort->getConnected().size() > 0) {
                std::string portType = currentPort->getType().getSubString(0);
                // TODO: why not take the only allowed kind of ports "image"? 
                // anything else would not work (df)
                //
                if (portType != "geometry"     && portType != "volumeset" &&
                    portType != "volumehandle" && portType != "volume")
                {
                    std::vector<PortData*> targets = portMap_[currentPort];
                    for (size_t k = 0; k < targets.size(); ++k)
                        tempForbiddenTargets_.push_back(dynamic_cast<PortDataTexture*>(targets[k])->getData());
                }
            }
        }
    }
}

int NetworkEvaluator::getTextureContainerTarget(Port* p, int pos) throw (VoreenException) {
    if (p == 0)
        throw VoreenException("No port with the given identifier found in getTextureContainerTarget().");

    std::vector<PortData*> targets = portMap_[p];

    if (targets.size() < static_cast<size_t>(pos+1))
        throw VoreenException("No data was mapped for that port.");

    PortDataTexture* pdt = dynamic_cast<PortDataTexture*>(targets.at(pos));

    if (!pdt)
        throw VoreenException("The data mapped to this is port is not a TextureContainer target.");

    return pdt->getData();
}

namespace {

bool checkGL(GLenum pname, bool value) {
    GLboolean b;
    glGetBooleanv(pname, &b);
    return (static_cast<bool>(b) == value);
}

bool checkGL(GLenum pname, GLint value) {
    GLint i;
    glGetIntegerv(pname, &i);
    return (i == value);
}

bool checkGL(GLenum pname, GLfloat value) {
    GLfloat f;
    glGetFloatv(pname, &f);
    return (f == value);
}

bool checkGL(GLenum pname, const tgt::vec4 value) {
    tgt::vec4 v;
    glGetFloatv(pname, reinterpret_cast<float*>(&v.elem));
    return (v == value);
}

} // namespace

void NetworkEvaluator::CheckOpenGLStateProcessWrapper::afterProcess(Processor* p) {
    if (!checkGL(GL_BLEND, false)) {
        glDisable(GL_BLEND);
        warn(p, "GL_BLEND was enabled");
    }

    if (!checkGL(GL_DEPTH_TEST, true)) {
        glEnable(GL_DEPTH_TEST);
        warn(p, "GL_DEPTH_TEST was not enabled");
    }

    if (!checkGL(GL_CULL_FACE, false)) {
        glDisable(GL_CULL_FACE);
        warn(p, "GL_CULL_FACE was enabled");
    }

    if (!checkGL(GL_COLOR_CLEAR_VALUE, tgt::vec4(0.f))) {
        glClearColor(0.f, 0.f, 0.f, 0.f);
        warn(p, "glClearColor() was not set to all zeroes");
    }

    if (!checkGL(GL_DEPTH_CLEAR_VALUE, 1.f)) {
        glClearDepth(1.0);
        warn(p, "glClearDepth() was not set to 1.0");
    }

    if (!checkGL(GL_LINE_WIDTH, 1.0f)) {
        glLineWidth(1.f);
        warn(p, "glLineWidth() was not set to 1.0");
    }

    if (!checkGL(GL_MATRIX_MODE, GL_MODELVIEW)) {
        glMatrixMode(GL_MODELVIEW);
        warn(p, "glMatrixMode was not set to GL_MODELVIEW");
    }

    if (!checkGL(GL_ACTIVE_TEXTURE, GL_TEXTURE0)) {
        glActiveTexture(GL_TEXTURE0);
        warn(p, "glActiveTexture was not set to GL_TEXTURE0");
    }

    if (!checkGL(GL_DEPTH_FUNC, GL_LESS)) {
        glDepthFunc(GL_LESS);
        warn(p, "glDepthFunc was not set to GL_LESS");
    }

    if (!checkGL(GL_CULL_FACE_MODE, GL_BACK)) {
        glCullFace(GL_BACK);
        warn(p, "glCullFace was not set to GL_BACK");
    }

    /*
      Check also:
      identity matrix for GL_MODELVIEW, GL_PROJECTION, GL_TEXTURE
    */
}

void NetworkEvaluator::CheckOpenGLStateProcessWrapper::warn(Processor* p, const std::string& message) {
    LWARNING(p->getClassName().getName() << " (" << p->getName()
             << "): invalid OpenGL state after processing: " << message);
}

void NetworkEvaluator::addProcessWrapper(ProcessWrapper* w) {
    processWrappers_.push_back(w);
}

void NetworkEvaluator::removeProcessWrappers() {
    for (size_t i=0; i < processWrappers_.size(); i++)
        delete processWrappers_[i];
    processWrappers_.clear();
}

} // namespace voreen
