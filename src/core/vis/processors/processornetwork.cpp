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

#include "voreen/core/vis/processors/processornetwork.h"

#include "voreen/core/vis/processors/processorfactory.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"

namespace voreen {

const std::string ProcessorNetwork::loggerCat_("voreen.ProcessorNetwork");

ProcessorNetwork::ProcessorNetwork()
    : Serializable()
    , reuseTCTargets_(true)
    , version_(2)
    , meta_()
{
}

ProcessorNetwork::ProcessorNetwork(const ProcessorNetwork& othernet)
    : Serializable()
    , version_(2)
{
    initializeFrom(othernet);
}

void ProcessorNetwork::operator=(const ProcessorNetwork& othernet) {
    for (size_t i = 0; i < processors_.size(); ++i)
        delete processors_[i];
    processors_.clear();

    initializeFrom(othernet);
}

void ProcessorNetwork::initializeFrom(const ProcessorNetwork& othernet) {
    reuseTCTargets_ = othernet.reuseTCTargets_;
    meta_ = othernet.meta_;

    // duplicate the vectors

    std::map<Processor*,Processor*> cloneMap; // maps old Processor to its duplicate
    // duplicate
    // all std::vector<Processor*> processors; including metadata
    for (size_t i=0; i < othernet.processors_.size(); ++i) {
        Processor* current = othernet.processors_.at(i);
        Processor* cloned = current->clone();
        // FIXME temporary hack to have Metadata
        // copy metadata over
        std::vector<TiXmlElement*> metaData = current->getAllFromMeta();
        for(size_t i = 0; i<metaData.size();++i) {
            cloned->addToMeta(metaData.at(i));
        }
        processors_.push_back(cloned);
        cloneMap[current] = cloned;
    }
    // now rewire connections
    for (size_t i=0; i < othernet.processors_.size(); ++i) {
        Processor* current = othernet.processors_.at(i);
        Processor* cloned = cloneMap[current];

        // go through all the ports
        std::vector<Port*> outports = current->getOutports();
        std::vector<Port*> coprocessoroutports = current->getCoProcessorOutports();

        // append coprocessoroutports to outports because we can handle them identically
        outports.insert(outports.end(), coprocessoroutports.begin(), coprocessoroutports.end());

        for (size_t i=0; i < outports.size(); ++i) {
            // connect the cloned processors ports correspondingly
            Port* cloneport = cloned->getPort(outports[i]->getTypeIdentifier());

            std::vector<Port*> connectedPorts = outports[i]->getConnected();
            for (size_t j=0; j < connectedPorts.size(); ++j) {

                // To connected inports
                Processor* connectedclone = cloneMap[connectedPorts[j]->getProcessor()];
                Port* connectedcloneport = connectedclone->getPort(connectedPorts[j]->getTypeIdentifier());
                cloned->connect(cloneport, connectedcloneport);
            }
        }
    }

    notifyObservers();
}

ProcessorNetwork::~ProcessorNetwork() {
    // FIXME: this is not good! the pointers will be lost eternally!
    // the pointers may have been assigned to GUI ProcessorItems, etc.!
    // Consider the possibility of making ProcessorNetwork objects
    // uncopyable and define the responsibility of deleting the pointers
    // (e.g. ProcessorItem's dot()?)(df)
    //
    for (size_t i = 0; i < processors_.size(); ++i)
       delete processors_[i];
}

TiXmlElement* ProcessorNetwork::serializeToXml() const {
    TiXmlElement* networkElem = new TiXmlElement(getXmlElementName());
    // metadata
    TiXmlElement* metaElem = meta_.serializeToXml();
    networkElem->LinkEndChild(metaElem);
    // misc
    networkElem->SetAttribute("version",version_);
    // general Settings for the Network
    TiXmlElement* settingsElem = new TiXmlElement("Settings");
    networkElem->LinkEndChild( settingsElem );
    settingsElem->SetAttribute("reuseTCTargets", reuseTCTargets_ ? "true" : "false");
    // give Processors an id
    std::map<Processor*,int> idMap;
    for (size_t i=0; i < processors_.size(); ++i)
        idMap[processors_[i]] = i;

    // Serialize Processors and add them to the network element
    for (size_t i=0; i< processors_.size(); ++i)
        // Processors need to know the ids for connectioninfo
        networkElem->LinkEndChild(processors_[i]->serializeToXml(idMap));

    return networkElem;
}

void ProcessorNetwork::updateMetaFromXml(TiXmlElement* elem) {
    errors_.clear();
    // meta
    TiXmlElement* metaElem = elem->FirstChildElement(meta_.getXmlElementName());
    if (metaElem) {
        meta_.updateFromXml(metaElem);
        errors_.store(meta_.errors());
    }
    else
        errors_.store(XmlElementException("Metadata missing!")); // TODO better exception
}

void ProcessorNetwork::updateFromXml(TiXmlElement* elem) {
    updateMetaFromXml(elem);
    // read general settings
    TiXmlElement* settingsElem = elem->FirstChildElement("Settings");
    if (settingsElem)
        reuseTCTargets_ = std::string("true").compare(settingsElem->Attribute("reuseTCTargets")) == 0;
    else {
        errors_.store(XmlElementException("Settings missing!")); // TODO Better exception
        reuseTCTargets_ = false; // TODO need way to store defaults - maybe in the class
    }

    // Prepare maps necessary to create connections
    // maps the id to the Processor
    std::map<int, Processor*> idMap;
    // maps (ProcessorId, PortIdentifier) to corresponding (ProcessorId, PortIdentifier)
    Processor::ConnectionMap connectionMap;

    // deserialize Processors
    TiXmlElement* processorElem;
    for (processorElem = elem->FirstChildElement(Processor::XmlElementName_);
         processorElem;
         processorElem = processorElem->NextSiblingElement(Processor::XmlElementName_))
    {
        try {
            // read the type of the Processor
            Identifier type = Processor::getClassName(processorElem);

            // compatibility layer: CubeEntryExitPoints has been merged 
            // with its superclass EntryExitPoints
            if (type == "EntryExitPoints.CubeEntryExitPoints") {
                type = "EntryExitPoints.EntryExitPoints";
                LINFO("Replaced CubeEntryExitPoints by EntryExitPoints");
            }

            Processor* processor = ProcessorFactory::getInstance()->create(type.getSubString(1));
            if (processor == 0)
                throw XmlAttributeException("Failed to create a Processor of class "
                                            + type.getName()); // TODO Better exception
            // deserialize processor
            std::pair<int, Processor::ConnectionMap> localConnectionInfo
                = processor->getMapAndUpdateFromXml(processorElem);
            errors_.store(processor->errors());
            int processorId = localConnectionInfo.first;
            Processor::ConnectionMap localConnectionMap = localConnectionInfo.second;
            // add processor to the idMap
            idMap[processorId] = processor;
            connectionMap.insert(connectionMap.end(), localConnectionMap.begin(), localConnectionMap.end());
            processors_.push_back(processor);
        }
        catch (SerializerException& e) {
            errors_.store(e);
        }
    }

    // sort connectionMap so incoming Connections are connected in the right order (i.e. combinepp)
    Processor::ConnectionCompare connComp;
    std::sort(connectionMap.begin(), connectionMap.end(), connComp);
    Processor::ConnectionMap::iterator iter;
    for (iter = connectionMap.begin(); iter != connectionMap.end(); ++iter ) {
        if (idMap.find(iter->first.processorId) == idMap.end() ||
            idMap.find(iter->second.processorId) == idMap.end())
        {
            errors_.store(SerializerException("There are references to nonexisting Processors!"));
            // TODO: Better exception
        }
        else {
            Processor* processor = idMap[iter->first.processorId];
            Port* port = processor->getPort(iter->first.portId);
            Processor* otherprocessor = idMap[iter->second.processorId];
            Port* otherport = otherprocessor->getPort(iter->second.portId);
            if (port == 0 || otherport == 0 || !processor->connect(port,otherport))
                errors_.store(SerializerException("The Connections of this file are messed up!"));
            // TODO: Better exception
            // I'd prefer Processor::connect to throw an Exception
        }
    }

    notifyObservers();
   
}

std::vector<Processor*>& ProcessorNetwork::getProcessors() {
    return processors_;
}

void ProcessorNetwork::setProcessors( std::vector<Processor*>& processors ) {
    processors_ = processors;
    notifyObservers();
}

void ProcessorNetwork::setVersion(int version) {
    version_ = version;
}

int ProcessorNetwork::getVersion() const {
    return version_;
}

void ProcessorNetwork::addObserver(ProcessorNetworkObserver* observer) {

    std::vector<ProcessorNetworkObserver*>::const_iterator foundObserver =
        std::find(observers_.begin(), observers_.end(), observer) ;
    if (foundObserver == observers_.end()) {
        observers_.push_back(observer) ;
    }
    
}

void ProcessorNetwork::removeObserver(ProcessorNetworkObserver* observer) {

    std::vector<ProcessorNetworkObserver*>::iterator foundObserver =
        std::find(observers_.begin(), observers_.end(), observer) ;
    if (foundObserver != observers_.end()) {
        observers_.erase(foundObserver);
    }

}

void ProcessorNetwork::notifyObservers() {
    
    std::vector<ProcessorNetworkObserver*>::const_iterator observerIter = observers_.begin();
    while (observerIter != observers_.end()) {
        (*observerIter)->networkChanged();    
    }

}

int ProcessorNetwork::getNumProcessors() const {
    return processors_.size();
}

void ProcessorNetwork::addProcessor(Processor* processor) {

    // add processor
    processors_.push_back(processor);

    // notify observers
    for (size_t i=0; i<observers_.size(); ++i)
        observers_[i]->processorAdded(processor);

}

void ProcessorNetwork::removeProcessor(Processor* processor){
    
    // remove processor if it is contained by the network
    bool processorRemoved = false;
    for (size_t i=0; i<processors_.size(); ++i) {
        if (processor == processors_.at(i)) {
            processors_.erase(processors_.begin() + i);
            processorRemoved = true;
            break;
        }
    }

    // notify observers
    if (processorRemoved) {
        for (size_t i=0; i<observers_.size(); ++i)
            observers_[i]->processorRemoved(processor);
    }

}

void ProcessorNetwork::setReuseTargets(bool reuse) {
    reuseTCTargets_ = reuse;
}

bool ProcessorNetwork::getReuseTargets() const {
    return reuseTCTargets_;
}

} // namespace voreen
