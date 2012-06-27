/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include <queue>
#include <numeric>

namespace voreen {

const Identifier NetworkEvaluator::setReuseTextureContainerTargets_("set.reuseTextureContainerTargets");
const Identifier NetworkEvaluator::setCachedBackward_("set.cached.backward");
const Identifier NetworkEvaluator::unsetCachedForward_("unset.cached.forward");
const Identifier NetworkEvaluator::addForbiddenTargets_("add.forbidden.targets");

const std::string NetworkEvaluator::loggerCat_("voreen.NetworkEvaluator");

NetworkEvaluator::NetworkEvaluator()
    : readyToEvaluate_(false)
    , tc_(0)
    , geoContainer_(0)
    , reuseTextureContainerTargets_(false)
    , currentViewportSize_(0)
    , coarsenessActive_(false)
{        
    setTag("evaluator");

#ifdef DEBUG
    // add process wrapper for checking OpenGL state
    addProcessWrapper(new CheckOpenGLStateProcessWrapper());
#endif
}

NetworkEvaluator::NetworkEvaluator(std::vector<Processor*> processors)
    : readyToEvaluate_(false),
      tc_(0),
      geoContainer_(0),
      processors_(processors),
      reuseTextureContainerTargets_(false)
{
    setTag("evaluator");

#ifdef DEBUG
    // add process wrapper for checking OpenGL state
    addProcessWrapper(new CheckOpenGLStateProcessWrapper());
#endif
}

NetworkEvaluator::~NetworkEvaluator() {
    removeProcessWrappers();    
}

//FIXME: either return an enum or throw an exception, but don't return numbers like -1 or -4!
int NetworkEvaluator::analyze() {
	priorityMap_.clear();
    if (geoContainer_)
        geoContainer_->clear();
	readyToEvaluate_ = false;

	//go through all processors and make some checks
	for (size_t i=0; i<processors_.size(); ++i) {
        processors_.at(i)->setGeometryContainer(geoContainer_);
		processors_.at(i)->setTextureContainer(tc_);
		//processors_.at(i)->setSize(tc_->getSize());
		
		//now check if there are any loops in the network
		//Every visited processor is put into this vector, and if it comes up again, there is a loop. This
		//could be done only for the last processor in the network, but in the future there might be more than
		//one, and then we would have to do this anyway. 
		std::vector<Processor*> processorVector;		
		int isThereLoop = checkForLoops(processors_.at(i),processorVector);
		if (isThereLoop != 0)
			return -2;
		
		//give every processor the priority -1, meaning that is shouldn't be rendered at all.
		//The real priority is set in the function assignPriority()
		priorityMap_.insert(std::pair<Processor*,int>(processors_.at(i),-1));
	}

	//TODO: Maybe add this routine to the loop above
	//finds the processors that have no outports. 
    std::vector<Processor*> end = findEndProcessorsInNetwork();
    if (end.size() == 0)
        return -4;
	for (size_t i=0; i < end.size(); ++i) {
		//assign each processor a priority based on their (maximal) distance(amount of processors) to the end Processor
		assignPriority(end[i], 0);
    }
    
	//now sort the processors by that priority
	sortProcessorsByPriority();
    
    	//give every port of every processor its appropriate PortData
	doPortMapping();

	//analyze which render results might be cached during the render process
	initializeCaching();

	//after caching we can check which rendertargets aren't allowed to be overwritten by
	//other evaluators
	tempForbiddenTargets_ = getRenderTargetsUsedInCaching(tempForbiddenTargets_);

	newForbiddenTargets_.clear();

	while (tempForbiddenTargets_.size() != 0) {
		int currentTarget = tempForbiddenTargets_.front();
		newForbiddenTargets_.push_back(currentTarget);
		//this removes ALL entrys with that value, thereby eliminating duplicated entrys
		tempForbiddenTargets_.remove(currentTarget);
	}

	//everything set up and ready
	readyToEvaluate_=true;

	ForbiddenTargetsOfEvaluator forbiddenTargetsOfEval;
	forbiddenTargetsOfEval.evaluator = this;
	forbiddenTargetsOfEval.forbiddenTargets=newForbiddenTargets_;
	MsgDistr.postMessage(new ForbiddenTargetsMsg(NetworkEvaluator::addForbiddenTargets_,forbiddenTargetsOfEval));
	
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

TextureContainer* NetworkEvaluator::initTextureContainer(int finalTarget) {
    tc_ = TextureContainer::createTextureContainer(finalTarget + 1);
    tc_->setFinalTarget(finalTarget);
    
    if (!tc_->initializeGL()) {
        delete tc_;
        tc_ = 0;
        return tc_;
    }

#ifndef __APPLE__
    int renderTargetType =
        TextureContainer::VRN_RGBA_FLOAT16 |
        TextureContainer::VRN_DEPTH |
        TextureContainer::VRN_DEPTH_TEX;
#else
    // FIXME: support for depth textures on Apple ;)
    int renderTargetType =
        TextureContainer::VRN_RGBA_FLOAT16 |
        //    TextureContainer::VRN_DEPTH |
        TextureContainer::VRN_DEPTH_TEX;
#endif

    for (int i = 0 ; i < finalTarget ; ++i)
        tc_->initializeTarget(i, renderTargetType);

    tc_->initializeTarget(finalTarget, TextureContainer::VRN_FRAMEBUFFER);
    tc_->setFinalTarget(finalTarget);
    MsgDistr.postMessage(new BoolMsg(NetworkEvaluator::setReuseTextureContainerTargets_,false),"evaluator");
    
    return tc_;
}

void NetworkEvaluator::doPortMapping() {
    if (geoContainer_)
        geoContainer_->clear();

    localPortMap_.clear();
	portMap_.clear();
	portEntries_.clear();

	// this queue contains all available targets in the TC whereas the target with the smallest
    // ID has the highest priority! (fxme done by dirk)
    //
    std::priority_queue<int, std::vector<int>, std::greater<std::vector<int>::value_type> > targetList;
	int numTargets = tc_->getNumAvailable();

	//At first all targets are free.
	std::vector<int> freeTargets;
	for (int i = (numTargets - 1); i >= 0; --i)
		freeTargets.push_back(i);
	
	//However other evaluators could have used those targets already,
	//so remove them. 
	std::map<NetworkEvaluator*,std::vector<int> >::iterator it = forbiddenTargetsFromOtherEvaluators_.begin();

	while (it != forbiddenTargetsFromOtherEvaluators_.end() ) {
		std::vector<int> forbiddenTargets = it->second;
		for (size_t i=0; i<forbiddenTargets.size(); ++i) {
			for (size_t j=0; j<freeTargets.size(); ++j) {
				if (freeTargets.at(j) == forbiddenTargets.at(i) ) {
					freeTargets.erase(freeTargets.begin() + j);
					break;
				}
			}
		}
		++it;
	}

	//Only targets not yet used remain, and are inserted into the queue
	for (size_t i=0; i<freeTargets.size(); ++i)
		targetList.push(freeTargets.at(i) );

    // Prevent the IDManger to choose a target, which is the Framebuffer.
    // This would happen if e.g. target #0 is of type VRN_FRAMEBUFFER.
    // Thererfore the list of free targets (priority in ascendeing order!) is
    // popped until a non-framebuffer target is found!
    //
    int idManTarget = -1;
    do {
        idManTarget = targetList.top(); // should be 0 on first pass.
        targetList.pop(); 
    } while ( (tc_->getAttr(idManTarget) & TextureContainer::VRN_FRAMEBUFFER) != 0 );

	//give the id Manager its target in the texturecontainer
    //
	IDManager id1;
    id1.getContent()->textureTarget_ = idManTarget;
	
	PortDataTexture* pdt;

	int target;
	//go through all outports of all processors to determine their targets in the TC , 
	//their dataset inputs, and their coprocessors
	for (size_t i=0; i<processors_.size(); ++i) {
		Processor* processor = processors_.at(i);
		std::vector<Port*> outports = processor->getOutports();
        for (size_t j=0; j<outports.size(); ++j) {
			Port* port = outports.at(j);

            // Do port mapping for special geometry ports.
            //
			if (port->getType().getSubString(0) == "geometry") {
                if (geoContainer_) {
                    const int geometryContainerNumber = geoContainer_->getNextID();
				    std::vector<PortData*> tempVec;
				    PortDataGeometry* pdg = new PortDataGeometry(geometryContainerNumber);
				    tempVec.push_back(pdg);
				    portMap_.insert(std::pair<Port*,std::vector<PortData*> >(port,tempVec));

				    std::vector<Port*> connectedPorts = port->getConnected();
				    //every port connected to this one must read its input from the same target
				    for (size_t k=0; k<connectedPorts.size(); ++k) {
					    std::map<Port*,std::vector<PortData*> >::iterator finder;
					    finder=portMap_.find(connectedPorts.at(k));
					    if (finder != portMap_.end())
						    portMap_[connectedPorts.at(k)].push_back(pdg);
					    else
						    portMap_.insert(std::pair<Port*,std::vector<PortData*> >(connectedPorts.at(k),tempVec));
				    }
                }
                else {
                    LERROR("No geometry container");
                }
            }
            else if (port->getType().getSubString(0) == "volumeset") {
                // the processor knows the pointer to the volumeset because 
                // the current port is the outport of the current processor!
                //
                if (typeid(*processor) == typeid(VolumeSetSourceProcessor)) {
                    VolumeSetSourceProcessor* vssp = dynamic_cast<VolumeSetSourceProcessor*>(processor);
			        std::vector<PortData*> tempVec;
			        PortDataGeneric<VolumeSet**>* pdGen = new PortDataGeneric<VolumeSet**>(vssp->getVolumeSetAddress(),
                                                                                           "portdata.volumeset");
			        tempVec.push_back(pdGen);
			        portMap_.insert(std::pair<Port*,std::vector<PortData*> >(port,tempVec));

                    std::vector<Port*> connectedPorts = port->getConnected();
			        //every port connected to this one must read its input from the same target
			        for (size_t k=0; k<connectedPorts.size(); ++k) {
				        std::map<Port*,std::vector<PortData*> >::iterator finder;
				        finder=portMap_.find(connectedPorts.at(k));

                        if (finder != portMap_.end())
					        portMap_[connectedPorts.at(k)].push_back(pdGen);
				        else
					        portMap_.insert(std::pair<Port*,std::vector<PortData*> >(connectedPorts.at(k),tempVec));
			        }
                }
                else
                    LERROR("No volumeset for mapping!");
            }
            else if (port->getType().getSubString(0) == "volumehandle") {
                // the processor knows the pointer to the volume because 
                // the current port is the outport of the current processor!
                if (typeid(*processor) == typeid(VolumeSelectionProcessor)) {
                    VolumeSelectionProcessor* vsp = dynamic_cast<VolumeSelectionProcessor*>(processor);
			        std::vector<PortData*> tempVec;
                    PortDataGeneric<VolumeHandle**>* pdGen = new PortDataGeneric<VolumeHandle**>(vsp->getVolumeHandleAddress(),
                                                                                                 "portdata.volumehandle");
			        tempVec.push_back(pdGen);
			        portMap_.insert(std::pair<Port*,std::vector<PortData*> >(port,tempVec));

                    std::vector<Port*> connectedPorts = port->getConnected();
			        //every port connected to this one must read its input from the same target
			        for (size_t k=0; k<connectedPorts.size(); ++k) {
				        std::map<Port*,std::vector<PortData*> >::iterator finder;
				        finder=portMap_.find(connectedPorts.at(k));
				        if (finder != portMap_.end())
					        portMap_[connectedPorts.at(k)].push_back(pdGen);
				        else
					        portMap_.insert(std::pair<Port*,std::vector<PortData*> >(connectedPorts.at(k),tempVec));
			        }
                } else
                    LERROR("No volumepointer for mapping!");
            } else {
                //Do PortMapping for "normal" ports
				//newPortEntry() creates a new entry in the portEntries_ map for the current port, saving
				//the port together with its connected ports. This is needed to determine when to free
				//the TC target this port renders to. If the port is set to persistent, no entry is made
				//and as a result the target is never freed.
				
				//If the port isn't connected to another port, it is unused and doesn't need target. If
				//you want a target even though the port isn't connected, get a private port.
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
						}
                        else {
							target = targetList.top();
							if (isPortForbiddenForOtherEvaluators)
								tempForbiddenTargets_.push_back(target);
							//remove the target from the queue, thereby indicating that it is used
							targetList.pop();
						}
					}
                    else {
						if (targetList.empty())
							throw VoreenException("No more free targets available. Try to enable the target reusing.");
						target = targetList.top();
						if (isPortForbiddenForOtherEvaluators)
							tempForbiddenTargets_.push_back(target);
						//remove the target from the queue, thereby indicating that it is used
						targetList.pop();
					}
	         		
					//map this port to the target we just got
					std::vector<PortData*> tempVec;
					pdt = new PortDataTexture(target);
					tempVec.push_back(pdt);
					portMap_.insert(std::pair<Port*,std::vector<PortData*> >(port,tempVec));
					
					std::vector<Port*> connectedPorts = port->getConnected();
					//every port connected to this one must read its input from the same target
					for (size_t k=0; k<connectedPorts.size(); ++k) {
						std::map<Port*,std::vector<PortData*> >::iterator finder;
						finder=portMap_.find(connectedPorts.at(k));
						if (finder != portMap_.end()) {
							portMap_[connectedPorts.at(k)].push_back(pdt);
						} else {
							portMap_.insert(std::pair<Port*,std::vector<PortData*> >(connectedPorts.at(k),tempVec));
						}
					}
				} 
            }
		}
		
		//Every processor can reserve TextureContainer targets that are not part of the network,
		//the Combine postprocessor for example needs two temporary targets to function. These 
		//are mapped here
		std::vector<Port*> privatePorts = processor->getPrivatePorts();
		for (size_t j=0; j<privatePorts.size(); ++j) {
			Port* port = privatePorts.at(j);
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

		//Now we go through the CoProcessorOutports. Luckily we don't have to free any targets etc
		//as in the "normal" outports
		std::vector<Port*> coProcPorts = processor->getCoProcessorOutports();
		for (size_t j=0; j< coProcPorts.size(); ++j) {
			Port* currentPort = coProcPorts.at(j);
			PortDataCoProcessor* pdcp = new PortDataCoProcessor(processor,currentPort->getFunctionPointer());
			std::vector<PortData*> tempVec;
			tempVec.push_back(pdcp);
			portMap_.insert(std::pair<Port*,std::vector<PortData*> >(currentPort,tempVec));

			std::vector<Port*> connectedPorts = currentPort->getConnected();
			std::map<Port*,std::vector<PortData*> >::iterator finder;
			for (size_t k=0; k<connectedPorts.size(); ++k) {
				finder=portMap_.find(connectedPorts.at(k));
				if (finder != portMap_.end())
					portMap_[connectedPorts.at(k)].push_back(pdcp);
				else
					portMap_.insert(std::pair<Port*,std::vector<PortData*> >(connectedPorts.at(k),tempVec));
			}
		}
		
		//The processors are already sorted by the priority by which they are rendered. We go through the
		//portEntries vector and remove the current processor from all outports that are connected to it. 
		//If an outport in result has no more connected processors, its target in the TC can be used again
		if (reuseTextureContainerTargets_) {
			for (size_t z=0; z < processor->getInports().size(); ++z) {
				Port* finishedPort = processor->getInports().at(z);
				for (size_t j=0; j < portEntries_.size(); ++j) {
					std::vector<Port*> portVector = portEntries_.at(j)->connectedPorts;
					bool entryFound=true;
					
					while (entryFound) {
						size_t k=0;
						//search the finished processor
						while (k<portVector.size() && portVector.at(k) != finishedPort) {
							k++;
						}
						if (k >= portVector.size())
							entryFound=false;
						else {
							//if found erase it from all outports connected to it
							portVector.erase(portVector.begin()+k);
							//check if there are still connected processors and if not, insert the target to the queue
							//again
							if (portVector.size() < 1) {
								//the queue always returns the biggest target, so we have to insert the original value
								//we got.
                                //
								std::vector<PortData*> freeTargets = portMap_[portEntries_.at(j)->port];
								for ( size_t w = 0; w < freeTargets.size(); ++w) {
									targetList.push(dynamic_cast<PortDataTexture*>(freeTargets[w])->getData() );
                                }
							}
						}
					}
				}
			}
		} 
	} // for loop through processors

	//This has been added when Jennis discovered the combinepp bug, that the order in which
	//the connections were made didn't affect the portmapping, which is important in the combiner.
	//This reorders the PortData* vector for inports (and coprocessor inports)
	//that can have multiple connections. Maybe this could be done in the loop above, I'm not sure (Stephan).
	for (size_t i=0; i<processors_.size(); ++i) {
		for (size_t j=0; j<processors_.at(i)->getInports().size(); ++j) {
			Port* p = processors_.at(i)->getInports().at(j);
			if (p->allowMultipleConnections()) {	//only do this for inports that have more than one connection.
				std::vector<PortData*> newPortData;
				std::vector<PortData*> portData = portMap_[p];
				std::vector<Port*> connectedPorts = p->getConnected();
				for (size_t k=0; k< connectedPorts.size(); ++k) {
					std::vector<PortData*> connectedPortData = portMap_[connectedPorts.at(k)];
					if (connectedPortData.size() > 0 ) {
						if (connectedPortData.at(0) != portData.at(k) ) {
							int position=-1;
							for (size_t l=0; l< portData.size(); ++l) {
								if (portData.at(l) == connectedPortData.at(0)) {
									position=l;
									break;
								}
							}
							if (position != -1)
								newPortData.push_back(portData.at(position));
						}
                        else {
							newPortData.push_back(connectedPortData.at(0));
						}
					}
				}
				portMap_[p] = newPortData;
			}
			
		}
		//now do this for coprocessors too.
		for (size_t j=0; j<processors_.at(i)->getCoProcessorInports().size(); ++j) {
			Port* p = processors_.at(i)->getCoProcessorInports().at(j);
			if (p->allowMultipleConnections()) {	//only do this for inports that have more than one connection.
				std::vector<PortData*> newPortData;
				std::vector<PortData*> portData = portMap_[p];
				std::vector<Port*> connectedPorts = p->getConnected();
				for (size_t k=0; k< connectedPorts.size(); k++) {
					std::vector<PortData*> connectedPortData = portMap_[connectedPorts.at(k)];
					if (connectedPortData.size() > 0 ) {
						if (connectedPortData.at(0) != portData.at(k) ) {
							int position=-1;
							for (size_t l=0; l< portData.size(); ++l) {
								if (portData.at(l) == connectedPortData.at(0)) {
									position=l;
									break;
								}
							}
							if (position != -1) {
								newPortData.push_back(portData.at(position));
							}
						} else {
							newPortData.push_back(connectedPortData.at(0));
						}
					}
				}
				portMap_[p] = newPortData;
			}
			
		}
	}
	PortMapping* portMapping = new PortMapping(portMap_);
	
	LocalPortMapping* localMapping;
	for (size_t i=0; i<processors_.size(); ++i) {
		localMapping = portMapping->createLocalPortMapping(processors_.at(i)); 
		localPortMap_.insert(std::pair<Processor*,LocalPortMapping*>(processors_.at(i),localMapping));
	}

}

void NetworkEvaluator::initializeCaching() {
	//First we have to set every outports cached attribute to false. That's because if there
	//previously was a cacherenderer in the network, that attribute might still be set to true
	for (size_t i = 0; i < processors_.size(); ++i) {
		std::vector<Port*> outports = processors_[i]->getOutports();
		for (size_t j = 0; j < outports.size(); ++j) {
            outports[j]->setCached(false);
		}
	}
	for (size_t i = 0; i < processors_.size(); ++i) {
        if (typeid(*processors_[i]) == typeid(CacheRenderer) ||
            typeid(*processors_[i]) == typeid(CanvasRenderer)) {
			setCachingOfInports(processors_[i]);
        }
	}
	//if every outport of a processor is connected to a cacherenderer, the result of the 
    //processor is cacheable
	for (size_t i = 0; i < processors_.size(); ++i) {
        std::vector<Port*> outports = processors_[i]->getOutports();
        //test whether the connected outports are cached
		bool cached = true;
		for (size_t j = 0; j < outports.size(); ++j) {
            if (outports[j]->getConnected().size() > 0)
                cached &= outports[j]->getCached();
		}
        processors_[i]->setCachable(cached);
	}
}

void NetworkEvaluator::setCachingOfInports(Processor* processor) {
    std::vector<Port*> inports = processor->getInports();
	for (size_t i = 0; i < inports.size(); ++i) {
		std::vector<Port*>& connectedPorts = inports[i]->getConnected();
		for (size_t j = 0; j < connectedPorts.size(); ++j) {
			connectedPorts[j]->setCached(true);
			setCachingOfInports(connectedPorts[j]->getProcessor());
		}
	}
}

void NetworkEvaluator::getConnectedProcessors(Processor* proc, std::list<Processor*>& processors, bool backward) {
    //test whether the processor was already inserted
    std::list<Processor*>::iterator result = std::find(processors.begin(), processors.end(), proc);
    if (result != processors.end())
        return;

    processors.push_back(proc);
    std::vector<Port*> ports;
    std::vector<Port*> coProc;
    if (backward) {
        ports = proc->getInports();
        coProc = proc->getCoProcessorInports();
    }
    else {
        ports = proc->getOutports();
        coProc = proc->getCoProcessorOutports();
    }
    ports.insert(ports.end(), coProc.begin(), coProc.end());
    for (size_t i = 0; i < ports.size(); ++i) {
        std::vector<Port*> connected = ports[i]->getConnected();
        for (size_t j = 0; j < connected.size(); ++j) {
            getConnectedProcessors(connected[j]->getProcessor(), processors, backward);
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
			assignPriority(processor->getInports().at(i)->getConnected().at(j)->getProcessor(), priority+1);
		}
	}
	for (size_t i=0; i<processor->getCoProcessorInports().size(); i++) {
		for (size_t j=0; j<processor->getCoProcessorInports().at(i)->getConnected().size(); j++)
			assignPriority(processor->getCoProcessorInports().at(i)->getConnected().at(j)->getProcessor(), priority+1);
	}
}
 
//TODO: Maybe add a flag to the processors indicating if they have been visited yet. That would make this
//a lot faster for big processor vectors.
//FIXME: either return an enum or throw an exception, but don't return numbers like -1 or -4!
int NetworkEvaluator::checkForLoops(Processor* processor,std::vector<Processor*>& processorVector) {
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
	for (size_t i = 0; i<processorVector.size(); ++i) {
		if (processor == processorVector.at(i))
			return true;
	}
	return false;
}

std::vector<Processor*> NetworkEvaluator::findEndProcessorsInNetwork() {
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
		IDManager id1;
        id1.signalizeNewRenderingPass();
		LGL_ERROR;

		for (size_t i=0; i < processors_.size(); ++i) {
		    Processor* currentProcessor = processors_.at(i);
		    if (priorityMap_[currentProcessor] != -1) {
		        if (!currentProcessor->getCachable() || !currentProcessor->getCached()) {
                    try {
                        for (size_t j=0; j < processWrappers_.size(); j++)
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

                        for (size_t j=0; j < processWrappers_.size(); j++)
                            processWrappers_[j]->afterProcess(currentProcessor);
                    }
                    catch (std::exception& e) {
                        LERROR("In NetworkEvaluator::evaluate(): Exception from "
                               << currentProcessor->getClassName().getName()
                               << " (" << currentProcessor->getName() << "): " << e.what());
                    }
		        }
		    }
        }

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
    else {
		return -1;
	}
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
	processors_=processors;
	readyToEvaluate_=false;
	portEntries_.clear();
}

void NetworkEvaluator::setGeometryContainer(GeometryContainer* const geoContainer) {
    geoContainer_ = geoContainer;
}

void NetworkEvaluator::setTextureContainer(TextureContainer* tc) {
	tc_ = tc;
}

void NetworkEvaluator::invalidate() {
	evaluate();
}

void NetworkEvaluator::invalidateRendering() {
    for (size_t i=0; i < processors_.size(); ++i)
	    processors_[i]->setCached(false);	
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
    //the viewport in opengl is already set to new size in voreenpainter (calls this method)
    //so only save new viewportsize
    currentViewportSize_ = size;
    for (size_t i=0; i < processors_.size(); ++i) {
		processors_[i]->setSize(size);
        processors_[i]->setCanvasSize(size);
    }
}

void NetworkEvaluator::switchCoarseness(bool active) {
    //find all coarsenessrenderer in the network
    std::vector<CoarsenessRenderer*> coarseRenderer;
    for (size_t i=0; i<processors_.size(); ++i) {
        if (typeid(*processors_[i]) == typeid(CoarsenessRenderer)) {
            coarseRenderer.push_back(dynamic_cast<CoarsenessRenderer*>(processors_[i]));
        }
    }

    for (size_t i = 0; i < coarseRenderer.size(); ++i) {
        //activate or deactivate coarseness in CoarsenessRenderer
        coarseRenderer[i]->setUseCoarseness(active);
        //calculate coarsenessFactor
        float coarseFactor;
        if (active)
            coarseFactor = 1.f / static_cast<float>(coarseRenderer[i]->getCoarsenessFactor());
        else
            coarseFactor = static_cast<float>(coarseRenderer[i]->getCoarsenessFactor());

        std::list<Processor*> visitedProcessors;
        getConnectedProcessors(coarseRenderer[i], visitedProcessors, true);
        //ignore first entry because its the coarsenessRenderer itself
        visitedProcessors.pop_front();
        //iterate through all processors and set new size according to coarseness factor
        std::list<Processor*>::iterator it;
        for (it = visitedProcessors.begin(); it != visitedProcessors.end(); ++it) {
            tgt::vec2 newSize = tgt::vec2((*it)->getSizeFloat().x * coarseFactor,
                                          (*it)->getSizeFloat().y * coarseFactor);

            //set new size in processor
		    (*it)->setSize(newSize);  
        }
    }
}

void NetworkEvaluator::processMessage(Message* msg ,const Identifier& /*dest=Message::all_*/) {
    bool postMsgToProcessors = true;
    if (msg->id_ == "evaluate") {
		evaluate(msg->getValue<Processor*>());
        postMsgToProcessors = false;
    }
    else if (msg->id_ == VoreenPainter::switchCoarseness_) {
        if (coarsenessActive_ != msg->getValue<bool>()) {
            coarsenessActive_ = !coarsenessActive_;
            switchCoarseness(coarsenessActive_);
            //the rendering is not invalidated here because the processor invalidates
            //when the new size is set
        }
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
    else if (msg->id_ == setCachedBackward_) {
        updateCaching(msg->getValue<Processor*>(), true);
        postMsgToProcessors = false;
	}
    else if (msg->id_ == unsetCachedForward_) {
        updateCaching(msg->getValue<Processor*>(), false);
        postMsgToProcessors = false;
	}
	else if (msg->id_ == addForbiddenTargets_) {
		ForbiddenTargetsOfEvaluator forbiddenTargetsOfEvaluator = msg->getValue<ForbiddenTargetsOfEvaluator>();
		addForbiddenTargets(forbiddenTargetsOfEvaluator.evaluator,forbiddenTargetsOfEvaluator.forbiddenTargets);
        postMsgToProcessors = false;
	}
    
    if (msg->id_ == VoreenPainter::cameraChanged_) {
        //this message is already caught by any subclass of EntryExitPoints
        //and the rendering is invalidated there. But we must do it again
        //because there are networks that doesnt have any EntryExitPoints-Processor.
        //Maybe we can check the entire network for EntryExitPoints and only invalidate
        //when there are no EEPs.
        invalidateRendering();
        postMsgToProcessors = true;
    }
    if (msg->id_ == VoreenPainter::removeEventListener_) 
        postMsgToProcessors = false;
    if (postMsgToProcessors == true) {
		for (size_t i=0; i<processors_.size(); ++i)
			processors_.at(i)->processMessage(msg);
	}
    // perhaps the ProcessWrapper objects are interested
    for (size_t i=0; i < processWrappers_.size(); i++)
        processWrappers_[i]->processMessage(msg);
}

void NetworkEvaluator::removeProcessor(Processor* processor) {
    for (size_t i=0; i<processors_.size(); ++i) {
	    if (processor == processors_.at(i)) {
		    processors_.erase(processors_.begin() + i);
            break;
        }
    }
}

void NetworkEvaluator::updateCaching(Processor* processor, bool cached) {
    std::list<Processor*> processors;
    getConnectedProcessors(processor, processors, cached);
    if (typeid(*processor) == typeid(CanvasRenderer)) {
        processors.pop_front();
    }
    std::list<Processor*>::iterator it = processors.begin();
    for (; it != processors.end(); ++it)
        (*it)->setCached(cached);
}

void NetworkEvaluator::init() {
	for (size_t i=0; i<processors_.size(); ++i)
		processors_.at(i)->init();
}

void NetworkEvaluator::deinit() {
	for (size_t i=0; i<processors_.size(); ++i)
		processors_.at(i)->deinit();
}

std::vector<int> NetworkEvaluator::getNewForbiddenTargets() {
	return newForbiddenTargets_;
}

void NetworkEvaluator::addForbiddenTargets(NetworkEvaluator* eval ,std::vector<int> forbiddenTargets) {
	if (eval == this)
		return;
    
	std::map<NetworkEvaluator*,std::vector<int> >::iterator finder;
	finder = forbiddenTargetsFromOtherEvaluators_.find(eval);

    if (finder == forbiddenTargetsFromOtherEvaluators_.end())
        forbiddenTargetsFromOtherEvaluators_.insert(std::pair<NetworkEvaluator*, std::vector<int> >(eval, forbiddenTargets));
	else
		finder->second = forbiddenTargets;
}

std::list<int> NetworkEvaluator::getRenderTargetsUsedInCaching(std::list<int> targetList) {
	for (size_t i=0; i < processors_.size(); ++i) {
		Processor* currentProcessor = processors_.at(i);

        if (currentProcessor->getCachable()) {
			std::vector<Port*> outports = currentProcessor->getOutports();
			for (size_t j=0; j < outports.size(); ++j) {
				Port* currentPort = outports.at(j);

                if (currentPort->getConnected().size() > 0) {
					std::string portType = currentPort->getType().getSubString(0);
					if (portType != "geometry" && portType != "volumeset" &&
                        portType != "volumehandle" && portType != "volume")
                    {
						std::vector<PortData*> targets = portMap_[currentPort];
						for (size_t k=0; k<targets.size(); ++k)
							targetList.push_front(dynamic_cast<PortDataTexture*>(targets.at(k))->getData());
					}
				}
			}
		}
	}
	return targetList;
}

int NetworkEvaluator::getTextureContainerTarget(Port* p,int pos) throw (VoreenException) {
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
    
    /* TODO:
      Check these also:
      glDepthFunc(GL_LESS);
      glMatrixMode(GL_MODELVIEW);
      glCullFace(GL_BACK);
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
