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

#include "voreen/core/vis/processors/networkserializer.h"

#include "voreen/core/vis/processors/processorfactory.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"

namespace voreen {

ProcessorNetwork::ProcessorNetwork()
    : Serializable()
    , reuseTCTargets(true)
    , version(2)
    , volumeSetContainer(0)
    , serializeVolumeSetContainer(false)
    , meta_()
{
}

ProcessorNetwork::ProcessorNetwork(const ProcessorNetwork& othernet)
    : Serializable()
    , version(2)
{
    initializeFrom(othernet);
}

void ProcessorNetwork::operator=(const ProcessorNetwork& othernet) {
    for (size_t i = 0; i < processors.size(); ++i)
        delete processors[i];
    processors.clear();
    for (size_t i = 0; i < propertySets.size(); ++i)
        delete propertySets[i];
    propertySets.clear();

    initializeFrom(othernet);
}

void ProcessorNetwork::initializeFrom(const ProcessorNetwork& othernet) {
    reuseTCTargets = othernet.reuseTCTargets;
    volumeSetContainer = othernet.volumeSetContainer;
    serializeVolumeSetContainer = othernet.serializeVolumeSetContainer;
    meta_ = othernet.meta_;

    // duplicate the vectors

    std::map<Processor*,Processor*> cloneMap; // maps old Processor to its duplicate
    // duplicate
    // all std::vector<Processor*> processors; including metadata -> Aggregations
    for (size_t i=0; i < othernet.processors.size(); ++i) {
        Processor* current = processors.at(i);
        Processor* cloned = current->clone();
        // FIXME temporary hack to have Metadata
        // copy metadata over
        std::vector<TiXmlElement*> metaData = current->getAllFromMeta();
        for(size_t i = 0; i<metaData.size();++i) {
            cloned->addToMeta(metaData.at(i));
        }
        processors.push_back(cloned);
        cloneMap[current] = cloned;
    }
    // now rewire connections
    for (size_t i=0; i < othernet.processors.size(); ++i) {
        Processor* current = processors.at(i);
        Processor* cloned = cloneMap[current];

        // go through all the ports
        std::vector<Port*> outports = current->getOutports();
        std::vector<Port*> coprocessoroutports = current->getCoProcessorOutports();

        // append coprocessoroutports to outports because we can handle them identically
        outports.insert(outports.end(), coprocessoroutports.begin(), coprocessoroutports.end());

        for (size_t i=0; i < outports.size(); ++i) {
            // connect the cloned processors ports correspondingly
            Port* cloneport = cloned->getPort(outports.at(i)->getType());

            std::vector<Port*> connectedPorts = outports[i]->getConnected();
            for (size_t j=0; j < connectedPorts.size(); ++j) {

                // To connected inports
                Processor* connectedclone = cloneMap[connectedPorts.at(j)->getProcessor()];
                Port* connectedcloneport = connectedclone->getPort(connectedPorts.at(j)->getType());
                cloned->connect(cloneport, connectedcloneport);
            }
        }
    }
}

ProcessorNetwork::~ProcessorNetwork() {
    // FIXME: this is not good! the pointers will be lost eternally!
    // the pointers may have been assigned to GUI ProcessorItems, etc.!
    // Consider the possibility of making ProcessorNetwork objects
    // uncopyable and define the responsibility of deleting the pointers
    // (e.g. ProcessorItem's dot()?)(df)
    //
    //for (size_t i = 0; i < processors.size(); ++i)
    //   delete processors[i];

    //for (size_t i = 0; i < propertySets.size(); ++i)
    //   delete propertySets[i];
}

TiXmlElement* ProcessorNetwork::serializeToXml() const {
    TiXmlElement* networkElem = new TiXmlElement(getXmlElementName());
    // metadata
    TiXmlElement* metaElem = meta_.serializeToXml();
    networkElem->LinkEndChild(metaElem);
    // misc
    networkElem->SetAttribute("version",version);
    // general Settings for the Network
    TiXmlElement* settingsElem = new TiXmlElement("Settings");
    networkElem->LinkEndChild( settingsElem );
    settingsElem->SetAttribute("reuseTCTargets", reuseTCTargets ? "true" : "false");
    // give Processors an id
    std::map<Processor*,int> idMap;
    for (size_t i=0; i < processors.size(); ++i)
        idMap[processors[i]] = i;

    // Serialize Processors and add them to the network element
    for (size_t i=0; i< processors.size(); ++i)
        // Processors need to know the ids for connectioninfo
        networkElem->LinkEndChild(processors[i]->serializeToXml(idMap));

    // Serialize PropertySets and add them to the network element
    for (size_t i=0; i< propertySets.size(); ++i)
        // PropertySets need to know the ids of their connected processors
        networkElem->LinkEndChild(propertySets[i]->serializeToXml(idMap));


    // Serialize the VolumeSetContainer
    if (serializeVolumeSetContainer && volumeSetContainer)
        networkElem->LinkEndChild(volumeSetContainer->serializeToXml());

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
        reuseTCTargets = std::string("true").compare(settingsElem->Attribute("reuseTCTargets")) == 0;
    else {
        errors_.store(XmlElementException("Settings missing!")); // TODO Better exception
        reuseTCTargets = false; // TODO need way to store defaults - maybe in the class
    }

    // deserialize VolumeSetContainer
    if (serializeVolumeSetContainer) {
        TiXmlElement* vscElem = elem->FirstChildElement(VolumeSetContainer::XmlElementName);
        if (vscElem) {
            if (!volumeSetContainer)
                volumeSetContainer = new VolumeSetContainer();
            volumeSetContainer->updateFromXml(vscElem);
            errors_.store(volumeSetContainer->errors());
        }
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
            processors.push_back(processor);
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

    // deserialize PropertySets
    TiXmlElement* propertysetElem;
    for (propertysetElem = elem->FirstChildElement(PropertySet::XmlElementName_);
         propertysetElem;
         propertysetElem = propertysetElem->NextSiblingElement(PropertySet::XmlElementName_))
    {
        try {
            PropertySet* propertySet = new PropertySet();
            propertySet->updateFromXml(propertysetElem, idMap);
            errors_.store(propertySet->errors());
            propertySets.push_back(propertySet);
        }
        catch (SerializerException &e) {
            errors_.store(e);
        }
    }
}

bool ProcessorNetwork::hasVolumeSetContainer(TiXmlElement* elem) {
    TiXmlElement* vscElem = elem->FirstChildElement(VolumeSetContainer::XmlElementName);
    return (vscElem != 0);
}

ProcessorNetwork& ProcessorNetwork::setTextureContainer(TextureContainer* tc) {
    for (size_t i=0; i< processors.size(); ++i)
        processors.at(i)->setTextureContainer(tc);

    return *this;
}

ProcessorNetwork& ProcessorNetwork::setCamera(tgt::Camera* camera) {
    for (size_t i = 0; i< processors.size(); ++i)
        processors.at(i)->setCamera(camera);

    return *this;
}

/* ------------------------------------------------------------------------------------- */

NetworkSerializer::NetworkSerializer() {}

NetworkSerializer::~NetworkSerializer() {}

ProcessorNetwork* NetworkSerializer::readNetworkFromFile(std::string filename, bool loadVolumeSetContainer)
    throw (SerializerException) {
    //Prepare the TiXmlDoxument
    TiXmlDocument doc(filename);
    if (!doc.LoadFile())
        throw SerializerException("Could not load network file!");
    // Still a bit messy in here...
    TiXmlHandle documentHandle(&doc);

    TiXmlElement* element;
    TiXmlHandle rootHandle(0);

    element = documentHandle.FirstChildElement().Element();
    if (!element)
        throw SerializerException("Did not find root element!");

    std::string s = element->Value();

    rootHandle=TiXmlHandle(element);

    TiXmlNode* node=&doc;

    TiXmlNode* versionFinder = node;            //just for finding out the version, not needed afterwards
    int version = findVersion(versionFinder);

    ProcessorNetwork* net = new ProcessorNetwork();

    switch (version) {
    case 2:
        net->serializeVolumeSetContainer = loadVolumeSetContainer;
        net->updateFromXml(doc.RootElement());
        break;
    default:
        throw SerializerException("Unsupported or unknown network version!");
    }
    return net;
}

bool NetworkSerializer::hasVolumeSetContainer(std::string filename) {
    TiXmlDocument doc(filename);
    if (!doc.LoadFile())
        throw SerializerException("Could not load network file " + filename);
    return ProcessorNetwork::hasVolumeSetContainer(doc.RootElement());
}

int NetworkSerializer::readVersionFromFile(std::string filename) {
    TiXmlDocument doc(filename);
    if (!doc.LoadFile())
        throw SerializerException("Could not load network file " + filename);
    int version = findVersion(&doc); // FIXME this is not safe
    return version;
}

int NetworkSerializer::findVersion(TiXmlNode* node) {
    if (node->Type() != TiXmlNode::ELEMENT) {
        TiXmlNode* pChild;
        for (pChild = node->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
            int result=findVersion(pChild);
            if (result!=0)
                return result;
        }
    }
    else {
        TiXmlElement* element=node->ToElement();
        if (element->FirstAttribute() ) {
            TiXmlAttribute* attribute=element->FirstAttribute();
            std::string s = attribute->Name();
            if (s == "Version" || s == "version")
                return attribute->IntValue();
        }
    }
    return 0;

}

void NetworkSerializer::serializeToXml(const ProcessorNetwork* network, const std::string& filename)
    throw (SerializerException)
{
    TiXmlDocument doc;
    TiXmlDeclaration* declNode = new TiXmlDeclaration("1.0", "ISO-8859-1", "");
    doc.LinkEndChild(declNode);
    // Serialize the network and add it as root element
    TiXmlElement* networkElem = network->serializeToXml();
    doc.LinkEndChild(networkElem);

    if (!doc.SaveFile(filename))
        throw SerializerException("Could not write network file " + filename);
}

// static methods
//

size_t NetworkSerializer::connectEventListenersToHandler(tgt::EventHandler* const eventHandler,
                                                         const std::vector<Processor*>& processors,
                                                         const bool addToFront)
{
    if (eventHandler == 0)
        return 0;

    size_t counter = 0;
    for (size_t i = 0; i < processors.size(); ++i) {
        tgt::EventListener* listener = dynamic_cast<tgt::EventListener*>(processors[i]);
        if (listener != 0) {
            if (addToFront == true)
                eventHandler->addListenerToFront(listener);
            else
                eventHandler->addListenerToBack(listener);
            ++counter;
        }
    }
    return counter;
}

size_t NetworkSerializer::removeEventListenersFromHandler(tgt::EventHandler* const eventHandler,
                                                          const std::vector<Processor*>& processors)
{
    if (eventHandler == 0)
        return 0;

    size_t counter = 0;
    for (size_t i = 0; i < processors.size(); ++i) {
        tgt::EventListener* listener = dynamic_cast<tgt::EventListener*>(processors[i]);
        if (listener != 0) {
            eventHandler->removeListener(listener);
            ++counter;
        }
    }
    return counter;
}

} // namespace voreen
