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
{
    meta_ = MetaSerializer(); 
}

ProcessorNetwork::~ProcessorNetwork() {
    // Nothing is deleted! That is the Way it should be!
}

TiXmlElement* ProcessorNetwork::serializeToXml() const {
    serializableSanityChecks();
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
        networkElem->LinkEndChild(processors[i]->serializeToXml(idMap)); // Processors need to know the ids for connectioninfo

    // Serialize PropertySets and add them to the network element
    for (size_t i=0; i< propertySets.size(); ++i)
        networkElem->LinkEndChild(propertySets[i]->serializeToXml(idMap)); // PropertySets need to know the ids of their connected processors
    
    // Serialize the VolumeSetContainer
    if (serializeVolumeSetContainer && volumeSetContainer)
        networkElem->LinkEndChild(volumeSetContainer->serializeToXml());
    
    return networkElem;
}

void ProcessorNetwork::updateMetaFromXml(TiXmlElement* elem) {
    errors_.clear();
    serializableSanityChecks(elem);
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
    ConnectionMap connectionMap;

    // deserialize Processors
    TiXmlElement* processorElem;
    for (processorElem = elem->FirstChildElement(Processor::XmlElementName);
        processorElem;
        processorElem = processorElem->NextSiblingElement(Processor::XmlElementName))
    {
        try {
            // read the type of the Processor
            Identifier type = Processor::getClassName(processorElem);
            Processor* processor = ProcessorFactory::getInstance()->create(type.getSubString(1));
            if (processor == 0)
                throw XmlAttributeException("Failed to create a Processor of class " + type.getName()); // TODO Better exception
            // deserialize processor
            std::pair<int, ConnectionMap> localConnectionInfo = processor->getMapAndupdateFromXml(processorElem);
            errors_.store(processor->errors());
            int processorId = localConnectionInfo.first;
            ConnectionMap localConnectionMap = localConnectionInfo.second;
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
    ConnectionCompare connComp;
    std::sort(connectionMap.begin(), connectionMap.end(), connComp);
    ConnectionMap::iterator iter;
    for (iter = connectionMap.begin(); iter != connectionMap.end(); ++iter ) {
        if (idMap.find(iter->first.processorId) == idMap.end() ||
            idMap.find(iter->second.processorId) == idMap.end())
        {
            errors_.store(SerializerException("There are references to nonexisting Processors!")); // TODO Better exception
        }
        else {
            Processor* processor = idMap[iter->first.processorId];
            Port* port = processor->getPort(iter->first.portId);
            Processor* otherprocessor = idMap[iter->second.processorId];
            Port* otherport = otherprocessor->getPort(iter->second.portId);
            if (port == 0 || otherport == 0 || !processor->connect(port,otherport))
                errors_.store(SerializerException("The Connections of this file are messed up!")); // TODO Better exception
             // I'd prefer Processor::connect to throw an Exception
        }
    }

    // deserialize PropertySets
    TiXmlElement* propertysetElem;
    for (propertysetElem = elem->FirstChildElement(PropertySet::XmlElementName);
        propertysetElem;
        propertysetElem = propertysetElem->NextSiblingElement(PropertySet::XmlElementName))
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

NetworkSerializer::NetworkSerializer() {
	count_=0; //Up to now there are zero processors loaded, so count_ is 0.
    //infos_ = 0;
}

NetworkSerializer::~NetworkSerializer() {
    //if (infos_)
    //    delete infos_;

    idMap_.clear();
}

ProcessorNetwork& NetworkSerializer::readNetworkFromFile(std::string filename, bool loadVolumeSetContainer) {
	//Clear and/or reset some variables,
	// TODO clean up
	count_=0;
	processors_.clear();
	propertySets_.clear();
	graphConnectionInformation_.clear();
	idMap_.clear();

	//Prepare the TiXmlDoxument
	TiXmlDocument doc(filename);
	if (!doc.LoadFile()) 
		throw SerializerException("Could not load network file!");

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
        
    ProcessorNetwork& net = *(new ProcessorNetwork());

    switch (version) {
    case 0:
        readProcessorsFromXml(node);
        connectProcessors();
        net.processors = processors_;
        net.propertySets = propertySets_;
        net.version = version;
        break;
    case 1:
        readProcessorsFromXmlVersion1(node);
        connectProcessorsVersion1();
        net.processors = processors_;
        net.propertySets = propertySets_;
        net.version = version;
        break;
    case 2:
        net.serializeVolumeSetContainer = loadVolumeSetContainer;
        net.updateFromXml(doc.RootElement());
        break;
    default:
        throw SerializerException("Unknown network version!");
    }
    return net;
}

bool NetworkSerializer::hasVolumeSetContainer(std::string filename) {
    TiXmlDocument doc(filename);
    if (!doc.LoadFile())
        throw SerializerException("Could not load network file!");
    return ProcessorNetwork::hasVolumeSetContainer(doc.RootElement());
}

int NetworkSerializer::readVersionFromFile(std::string filename) {
    TiXmlDocument doc(filename);
    if (!doc.LoadFile()) 
        throw SerializerException("Could not load network file!");
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

/*
* Reads an xml file and creates all processors and propertysets stored in it. 
* These Processors are however not connected yet. That has to be done with connectProcessors()
* The steps are: 
*	- If a Processor is found: Read its class, properties, normal connections, and coprocessor connections
*	- If a propertyset is found: read it :D
*/
int NetworkSerializer::readProcessorsFromXml(TiXmlNode* node) {
	Processor* newProcessor;						//the next processor to be read from file
	ConnectionInfoForOneProcessor* newInfo;		//stores connections for the next processor
	if (node->Type() == TiXmlNode::ELEMENT) {
		std::string s = node->Value();
		if (s == "RptProcessorItem") {	
			TiXmlElement* element=node->ToElement();
			TiXmlAttribute* attribute=element->FirstAttribute();
			
			//Read the id of the stored processor. Remember that you can't store pointers, so
			//every processor has been given a number/id during the save process
			int processorId=attribute->IntValue();

			//Read the classtype of the processor, for example "volumeraycaster.fancyraycaster"
			attribute = attribute->Next();
			std::string type = attribute->Value();

            //FIX: FinalRenderer -> CanvasRenderer
            // sometime conversion outdatet
            if (type == "Miscellaneous.FinalRenderer")
                type = "Miscellaneous.Canvas";
			
			//we have all the info we need to create the processor (but not the connections)
			//to create the processor we need the specific type, like "fancyraycaster" for the 
			//processorfactory, so we use the Identifier class to get that
            Identifier ident(type);
            newProcessor = ProcessorFactory::getInstance()->create(ident.getSubString(1));
			
			//insert this processor in the id map together with its id
			idMap_.insert(std::pair<Processor*,int>(newProcessor,processorId));

			processors_.push_back(newProcessor);

			//now get the connection Info and store it in graphConnectionInformation_ so we can create the connections between the
			//processors later
			TiXmlNode* temp;	//this is just to make the navigation in the xml file easier, we still need node
			TiXmlNode* temp2;	//later and therefore can't change it
			
			//create a new ConnectionInfoForOneProcessorObject to store the connections
			graphConnectionInformation_.push_back(new ConnectionInfoForOneProcessor());			
			newInfo = graphConnectionInformation_.at(count_);
			count_++;
			
			newInfo->id = processorId;
			
			if (node->FirstChild()->FirstChild()) {		//this checks if there are inputs saved in the xml file
				temp=node->FirstChild()->FirstChild();

				//if there are outputs go through them
				for (; temp != 0; temp = temp->NextSibling()) {
					element=temp->ToElement();
					attribute=element->FirstAttribute();
					
					//the type of the output port
					std::string type = attribute->Value();
					
					//create a new PortConnection object, that stores all the information about this Port(type, id, and connected Ports of
					//other processors
					PortConnection* newConnection = new PortConnection();
					newConnection->type=type;
					newConnection->portNumber=element->LastAttribute()->IntValue();
					
					//now get the ids of the processors connected to this port
					if (temp->FirstChild()) {
						temp2=temp->FirstChild();
						for (; temp2 != 0; temp2 = temp2->NextSibling()) {
							element=temp2->ToElement();
							attribute=element->FirstAttribute();
							int processor_id=attribute->IntValue();
							int port_number = element->LastAttribute()->IntValue();
							//we found a connection to a specific port of another processor and save that information.
							newConnection->connectedProcessors.insert(std::pair<int,int>(processor_id,port_number));
						}
					}
				newInfo->outputs.push_back(newConnection);
				}//for
			}//if

			//-----------------------------------------------------------------
			//new code for coprocessors. Appears to be working now
			//-----------------------------------------------------------------

			//This checks if there are coprocessor outport connections
			if (node->FirstChild()->NextSibling()->FirstChild()) {
				temp=node->FirstChild()->NextSibling()->FirstChild();
				//Go through all the coprocessor outports
				for (; temp != 0; temp = temp->NextSibling()) {
					element=temp->ToElement();
					attribute=element->FirstAttribute();
					
					std::string type = attribute->Value();

					//create a new PortConnection object, that stores all the information about this Port(type, id, and connected Ports of
					//other processors
					PortConnection* newConnection = new PortConnection();
					newConnection->type=type;
					newConnection->portNumber=element->LastAttribute()->IntValue();

					//now get the ids of the processors connected to this port
					if (temp->FirstChild()) {
						temp2=temp->FirstChild();
						for (; temp2 != 0; temp2 = temp2->NextSibling()) {
							element=temp2->ToElement();
							attribute=element->FirstAttribute();
							int processor_id=attribute->IntValue();
							int port_number = element->LastAttribute()->IntValue();
							//we found a connection to a specific port of another processor and save that information.
							newConnection->connectedProcessors.insert(std::pair<int,int>(processor_id,port_number));
						}
					}
				newInfo->outputs.push_back(newConnection);
				}//for
			}

			//Check if there are properties saved for this processor, and if so, read them
			if (node->FirstChild()->NextSibling()->NextSibling()) {
				TiXmlElement * propertyElem = node->FirstChild()->NextSibling()->NextSibling()->ToElement();
				std::vector<Property*> props = newProcessor->getProperties();
				loadProperties(props,propertyElem,newProcessor);

			}
			
		//We found a propertyset, so read it
		}
        else if (s == "PropertySet") {
			TiXmlElement* element=node->ToElement();
			TiXmlAttribute* attribute=element->FirstAttribute();
			//int propertySetId = attribute->IntValue();
			attribute = attribute->Next();
			std::string propertySetName = attribute->Value();
			
			PropertySet* newPropSet = new PropertySet();
			//newPropSet->setName(propertySetName);
			propertySets_.push_back(newPropSet);

			if (node->FirstChild()->FirstChild()) {		//this checks if there are processors in this propertyset
				TiXmlNode* temp=node->FirstChild()->FirstChild();
				for (; temp != 0; temp = temp->NextSibling()) {
					//we found a processor and add it to the property set
					element=temp->ToElement();
					int processorId =element->FirstAttribute()->IntValue();
					newPropSet->addProcessor(findProcessor(processorId));					
				}
			}
		}
	}
	//search for other processors and propertysets in the xml file
	TiXmlNode* pChild;
	for (pChild = node->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
		readProcessorsFromXml(pChild);
	}
	return 0;
}

/*
* Reads an xml file and creates all processors and propertysets stored in it. 
* These Processors are however not connected yet. That has to be done with connectProcessors()
* The steps are: 
*	- If a Processor is found: Read its class, properties, normal connections, and coprocessor connections
*	- If a propertyset is found: read it :D
*/
int NetworkSerializer::readProcessorsFromXmlVersion1(TiXmlNode* node) {
	Processor* newProcessor;						//the next processor to be read from file
	ConnectionInfoForOneProcessor* newInfo;		//stores connections for the next processor
	if (node->Type()== TiXmlNode::ELEMENT) {
		std::string s = node->Value();
		if (s == "RptProcessorItem") {	
			TiXmlElement* element=node->ToElement();
			TiXmlAttribute* attribute=element->FirstAttribute();
			
			//Read the id of the stored processor. Remember that you can't store pointers, so
			//every processor has been given a number/id during the save process
			int processorId=attribute->IntValue();

			//Read the classtype of the processor, for example "volumeraycaster.fancyraycaster"
			attribute=attribute->Next();
			std::string type=attribute->Value();

            //FIX: FinalRenderer -> CanvasRenderer
            // sometime conversion outdatet
            if (type == "Miscellaneous.FinalRenderer")
                type = "Miscellaneous.Canvas";
			
			//we have all the info we need to create the processor (but not the connections)
			//to create the processor we need the specific type, like "fancyraycaster" for the 
			//processorfactory, so we use the Identifier class to get that
            Identifier ident(type);
            newProcessor = ProcessorFactory::getInstance()->create(ident.getSubString(1));
			
            //Set Processors name (hope we don't mess up code after this
            attribute=attribute->Next();
            std::string dowehaveaName = attribute->Name();
            if (dowehaveaName == "Processor_Name") {
                std::string canvasMerge = attribute->Value();  //FIXME: FinalRenderer -> CanvasRenderer
                if (canvasMerge == "FinalRenderer")
                    canvasMerge = "Canvas";
                //newProcessor->setName(canvasMerge /*attribute->Value()*/);
            }
            attribute=attribute->Previous();

			//insert this processor in the id map together with its id
			idMap_.insert(std::pair<Processor*,int>(newProcessor,processorId));

			processors_.push_back(newProcessor);

			//now get the connection Info and store it in graphConnectionInformation_ so we can create the connections between the
			//processors later
			TiXmlNode* temp;	//this is just to make the navigation in the xml file easier, we still need node
			TiXmlNode* temp2;	//later and therefore can't change it
			
			//create a new ConnectionInfoForOneProcessorObject to store the connections
			graphConnectionInformation_.push_back(new ConnectionInfoForOneProcessor());			
			newInfo = graphConnectionInformation_.at(count_);
			count_++;
			
			newInfo->id = processorId;
			
			if (node->FirstChild()->FirstChild()) {		//this checks if there are inputs saved in the xml file
				temp=node->FirstChild()->FirstChild();

				//if there are inputs go through them
				for (; temp != 0; temp = temp->NextSibling()) {
					element=temp->ToElement();
					attribute=element->FirstAttribute();
					
					//the type of the inputs port
					std::string type = attribute->Value();
					
					//create a new PortConnection object, that stores all the information about this Port(type, id, and connected Ports of
					//other processors
					PortConnection* newConnection = new PortConnection();
					newConnection->type=type;
					newConnection->portNumber=element->LastAttribute()->IntValue();
					
					//now get the ids of the processors connected to this port
					if (temp->FirstChild()) {
						temp2=temp->FirstChild();
						for (; temp2 != 0; temp2 = temp2->NextSibling()) {
							element=temp2->ToElement();
							attribute=element->FirstAttribute();
							int processor_id=attribute->IntValue();
                            attribute=attribute->Next();
							std::string port_identifier = attribute->Value();
							int port_number = element->LastAttribute()->IntValue();
							//we found a connection to a specific port of another processor and save that information.
							newConnection->connectedProcessors.insert(std::pair<int,int>(processor_id,port_number));
                            newConnection->connectedProcessorsMap.insert(std::pair<int,Identifier>(processor_id,port_identifier));
							ConnectedProcessor connectedProcessor;
							connectedProcessor.processorNumber = processor_id;
							connectedProcessor.portType = port_identifier;
							newConnection->connectedProcessorsVector.push_back(connectedProcessor);
						}
					}
                    newInfo->inputs.push_back(newConnection);
				}//for
			}//if

			//-----------------------------------------------------------------
			//new code for coprocessors. Appears to be working now
			//-----------------------------------------------------------------

			//This checks if there are coprocessor inport connections
			if (node->FirstChild()->NextSibling()->FirstChild()) {
				temp=node->FirstChild()->NextSibling()->FirstChild();
				//Go through all the coprocessor outports
				for (; temp != 0; temp = temp->NextSibling()) {
					element=temp->ToElement();
					attribute=element->FirstAttribute();
					
					std::string type = attribute->Value();

					//create a new PortConnection object, that stores all the information about this Port(type, id, and connected Ports of
					//other processors
					PortConnection* newConnection = new PortConnection();
					newConnection->type=type;
					newConnection->portNumber=element->LastAttribute()->IntValue();

					//now get the ids of the processors connected to this port
					if (temp->FirstChild()) {
						temp2=temp->FirstChild();
						for (; temp2 != 0; temp2 = temp2->NextSibling()) {
							element=temp2->ToElement();
							attribute=element->FirstAttribute();
							int processor_id=attribute->IntValue();
                            attribute=attribute->Next();
							std::string port_identifier = attribute->Value();
							int port_number = element->LastAttribute()->IntValue();
							//we found a connection to a specific port of another processor and save that information.
							newConnection->connectedProcessors.insert(std::pair<int,int>(processor_id,port_number));
                            newConnection->connectedProcessorsMap.insert(std::pair<int,Identifier>(processor_id,port_identifier));
							ConnectedProcessor connectedProcessor;
							connectedProcessor.processorNumber = processor_id;
							connectedProcessor.portType = port_identifier;
							newConnection->connectedProcessorsVector.push_back(connectedProcessor);
						}
					}
				newInfo->inputs.push_back(newConnection);
				}//for
			}

			//Check if there are properties saved for this processor, and if so, read them
			if (node->FirstChild()->NextSibling()->NextSibling()) {
				TiXmlElement * propertyElem = node->FirstChild()->NextSibling()->NextSibling()->ToElement();
				std::vector<Property*> props = newProcessor->getProperties();
				loadProperties(props,propertyElem,newProcessor);

			}
			
		//We found a propertyset, so read it
		}else if (s == "PropertySet") {
			TiXmlElement* element=node->ToElement();
			TiXmlAttribute* attribute=element->FirstAttribute();
			//int propertySetId = attribute->IntValue();
			attribute = attribute->Next();
			std::string propertySetName = attribute->Value();
			
			PropertySet* newPropSet = new PropertySet();
			//newPropSet->setName(propertySetName);
			propertySets_.push_back(newPropSet);

			if (node->FirstChild()->FirstChild()) {		//this checks if there are processors in this propertyset
				TiXmlNode* temp=node->FirstChild()->FirstChild();
				for (; temp != 0; temp = temp->NextSibling()) {
					//we found a processor and add it to the property set
					element=temp->ToElement();
					int processorId =element->FirstAttribute()->IntValue();
					newPropSet->addProcessor(findProcessor(processorId));					
				}
			}
		}
	}
	//search for other processors and propertysets in the xml file
	TiXmlNode* pChild;
	for (pChild = node->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
		readProcessorsFromXmlVersion1(pChild);
	
    return 0;
}

/*
* Uses the information stored in graphConnectionInformation_ to connect the Processor objects in
* the right way. This is needed because when reading and creating the processors from the xml file, some processors might 
* not exist yet. That means that we first have to create all the processors, and after that we can connect them. This is
* what this function does. There are some checks that might seem unnessecary, but if you change the amount of ports
* a processor has, and then load a file from an older version, you would be in trouble
*/
int NetworkSerializer::connectProcessors() {
	if ((processors_.size()<1) || (graphConnectionInformation_.size()<1) )
		return -1;

	Processor* currentProcessor;
	ConnectionInfoForOneProcessor* currentInfo;
	
	for (size_t i=0;i < graphConnectionInformation_.size();i++) {
		//Each element in graphConnectionInformation stores the connection info for one processor.
		//This processor is searched by its id
		currentInfo=graphConnectionInformation_.at(i);
		currentProcessor=findProcessor(currentInfo->id);
		if (!currentProcessor)
			return -1;
		else {
			//Get the output connection informations for this processor
			std::vector<PortConnection*> out;
			out=currentInfo->outputs;

			//iterate throuh it and connect the processors
			for (size_t j=0; j<out.size(); ++j) {
				PortConnection* con = out.at(j);
				std::string temp =con->type.getSubString(0);
				//coprocessor connections have to be handled differently so we have to check
				if (temp != "coprocessor") {
					if (static_cast<size_t>(con->portNumber) < currentProcessor->getOutports().size() ) {
						Port* sourcePort = currentProcessor->getOutports().at(con->portNumber);
						//We have the source port and now get the map that contains the information
						//to which port of which processors this port is connected. 
						std::map<int,int> connectedProcessors = con->connectedProcessors;
						//first entry stands for the processor id, the second for the port number
						for(std::map<int,int>::const_iterator k = connectedProcessors.begin(); k != connectedProcessors.end(); ++k) {
							Processor* dest = findProcessor(k->first);
							if (static_cast<size_t>(k->second) < dest->getInports().size() ) {
								Port* destPort = dest->getInports().at(k->second);
								currentProcessor->connect(sourcePort,destPort);
							}
						}
					}
				//we found a coprocessor connection
				} else {
					if (static_cast<size_t>(con->portNumber) < currentProcessor->getCoProcessorOutports().size() ) {
						Port* sourcePort = currentProcessor->getCoProcessorOutports().at(con->portNumber);
						//We have the source port and now get the map that contains the information
						//to which port of which processors this port is connected. 
						std::map<int,int> connectedProcessors = con->connectedProcessors;
						//first entry stands for the processor id, the second for the port number
						for(std::map<int,int>::const_iterator k = connectedProcessors.begin(); k != connectedProcessors.end(); ++k) {
							Processor* dest = findProcessor(k->first);
							if (static_cast<size_t>(k->second) < dest->getCoProcessorInports().size() ) {
								Port* destPort = dest->getCoProcessorInports().at(k->second);
								currentProcessor->connect(sourcePort,destPort);
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

/*
* Uses the information stored in graphConnectionInformation_ to connect the Processor objects in
* the right way. This is needed because when reading and creating the processors from the xml file, some processors might 
* not exist yet. That means that we first have to create all the processors, and after that we can connect them. This is
* what this function does. There are some checks that might seem unnessecary, but if you change the amount of ports
* a processor has, and then load a file from an older version, you would be in trouble
*/
int NetworkSerializer::connectProcessorsVersion1() {
	if ((processors_.size()<1) || (graphConnectionInformation_.size()<1) )
		return -1;

	Processor* currentProcessor;
	ConnectionInfoForOneProcessor* currentInfo;
	
	for (size_t i=0; i < graphConnectionInformation_.size(); ++i) {
		//Each element in graphConnectionInformation stores the connection info for one processor.
		//This processor is searched by its id
		currentInfo=graphConnectionInformation_.at(i);
		currentProcessor=findProcessor(currentInfo->id);
		if (!currentProcessor)
			return -1;
		else {
			//Get the output connection informations for this processor
			std::vector<PortConnection*> in;
			in=currentInfo->inputs;

			//iterate through it and connect the processors
			for (size_t j=0; j<in.size(); ++j) {
				PortConnection* con = in.at(j);
				std::string temp =con->type.getSubString(0);
				//coprocessor connections have to be handled differently so we have to check
				if (currentProcessor->getPort(con->type) ) {
					Port* destPort = currentProcessor->getPort(con->type);

					std::vector<ConnectedProcessor> connectedProcessors = con->connectedProcessorsVector;
					
					for (size_t k=0; k<connectedProcessors.size(); ++k) {
						Processor* source = findProcessor(connectedProcessors.at(k).processorNumber);
						Port* sourcePort = source->getPort(connectedProcessors.at(k).portType);
						if (sourcePort)
							source->connect(sourcePort,destPort);
					}
				}	
			}
		}
	}
	return 0;
}

/*
* Returns the Processor with the given id if it is found in processors_
*/
Processor* NetworkSerializer::findProcessor(int id) {
	for (size_t i=0; i<processors_.size(); ++i) {
		if (idMap_[processors_.at(i)] == id)
			return processors_.at(i);
	}
	return 0;
}

/*
* Loads the property vector of one processor from xml. (shamelessely copied and modified from widgetgenerator)
*/
int NetworkSerializer::loadProperties(std::vector<Property* > props, TiXmlElement* xmlParent, Processor* processor) {
    std::string property_text;
    std::string conditionIdentifier_text;
    std::string propText;
	TiXmlElement* pElem;
	//TiXmlElement* cElem;

    pElem = xmlParent->FirstChildElement();
    for ( ; pElem; pElem=pElem->NextSiblingElement()) {
        property_text = pElem->Attribute("Property_text");
		//std::cout << "property: " << property_text <<std::endl;
        for (size_t j = 0; j < props.size(); ++j) {
            if ( props.at(j)->getGuiText() == property_text ) {
                // Found a property in the current pipeline which can get
                // some attributes from xml file
				if (property_text=="empty")
					break;
     
                int property_type;
                pElem->QueryIntAttribute("Property_type", &property_type);

                // first set autochange false
                props.at(j)->setAutoChange(false);
				//std::cout << "Type: " << property_type<<std::endl;
                switch(property_type) {
                case Property::FLOAT_PROP :
                    try {
                        float value_tmp;
						FloatProp* prop = dynamic_cast<FloatProp*>(props.at(j));
                        pElem->QueryFloatAttribute("Value", &value_tmp);
						if (prop) {
							prop->set(value_tmp);
							processor->postMessage(new FloatMsg(prop->getIdent(), value_tmp));
						}
                    } 
					catch (const std::bad_cast& /*ex*/) {    
					}
                    break;
                                            
                case Property::INT_PROP :
                    try {
                        IntProp* prop = dynamic_cast<IntProp*>(props.at(j));
                        int value_tmp;
                        pElem->QueryIntAttribute("Value", &value_tmp);
						if (prop) {
							prop->set(value_tmp);
							processor->postMessage(new IntMsg(prop->getIdent(), value_tmp));
						}
					} catch (const std::bad_cast& /*ex*/) {
						//std::cout<< "args tot";
                    }
                    break;
										  //TODO: Check if this is working correctly
                case Property::BOOL_PROP :
                    try {
                        pElem->QueryIntAttribute("Property_type", &property_type);
                        BoolProp* prop = dynamic_cast<BoolProp*>(props.at(j));
                        if (prop) {
                            int value_tmp;
                            pElem->QueryIntAttribute("Value", &value_tmp);
                            if (value_tmp==1) {
							    prop->set(true);
							    processor->postMessage(new BoolMsg(prop->getIdent(), true));
							}
                            else {
								prop->set(false);
								processor->postMessage(new BoolMsg(prop->getIdent(), false));
							}
                        }
                    } 
                    catch (const std::bad_cast& /*ex*/) {
                    }
                    break;

                case Property::COLOR_PROP : 
                    try {
                        ColorProp* prop = dynamic_cast<ColorProp*>(props.at(j));
                        float color_r;
                        float color_g;
                        float color_b;
                        float color_a;
                        pElem->QueryFloatAttribute("Color_r", &color_r);
                        pElem->QueryFloatAttribute("Color_g", &color_g);
                        pElem->QueryFloatAttribute("Color_b", &color_b);
                        pElem->QueryFloatAttribute("Color_a", &color_a);
						if (prop) {
							prop->set(tgt::Color(color_r,color_g,color_b,color_a));
							processor->postMessage(new ColorMsg(prop->getIdent(), prop->get())); 
						}
                    } catch (const std::bad_cast& /*ex*/) {
                    }
                   break;

                case Property::FILEDIALOG_PROP : 
                    try {
                        FileDialogProp* prop = dynamic_cast<FileDialogProp*>(props.at(j));
                        std::string value_tmp = std::string(pElem->Attribute("Value"));
                        if (prop) {
							prop->set(value_tmp);
							processor->postMessage(new StringMsg(prop->getIdent(), value_tmp));
						}
					} catch (const std::bad_cast& /*ex*/) {
						//std::cout<< "args tot";
                    }
                    break;

                case Property::ENUM_PROP :
                    try {
                        EnumProp* prop = dynamic_cast<EnumProp*>(props.at(j));
                        int value_tmp;
                        pElem->QueryIntAttribute("Value", &value_tmp);
						if (prop) {
							prop->set(value_tmp);
							if (prop->getSendStringMsg() ) {
								processor->postMessage(new StringMsg(prop->getIdent(),prop->getStrings().at(value_tmp))); 
							}
							else
								processor->postMessage(new IntMsg(prop->getIdent(),value_tmp));
						}
                    } catch (const std::bad_cast& /*ex*/) {
                    }
                    break;

                case Property::TRANSFUNC_PROP :
                    try {
                         TransFuncIntensity* tf = new TransFuncIntensity();
						 tf->clearKeys();

                         //iterate through all markers
                         //first set conditions for the current item if there're some
                         if (!pElem->NoChildren()) {
							 TiXmlElement* cElem;
                             cElem = pElem->FirstChildElement();
                             for (; cElem; cElem=cElem->NextSiblingElement()) {
                                 //first get the color
                                 float value_tmp;
                                 int int_tmp;
                                 tgt::col4 color_tmp;
                                 cElem->QueryIntAttribute("Color_r", &int_tmp);
                                 color_tmp.r = (uint8_t) int_tmp;
                                 cElem->QueryIntAttribute("Color_g", &int_tmp);
                                 color_tmp.g = (uint8_t) int_tmp;
                                 cElem->QueryIntAttribute("Color_b", &int_tmp);
                                 color_tmp.b = (uint8_t) int_tmp;
                                 cElem->QueryIntAttribute("Color_a", &int_tmp);
                                 color_tmp.a = (uint8_t) int_tmp;

                                 cElem->QueryFloatAttribute("Source_value", &value_tmp);
                                 TransFuncMappingKey* myKey = new TransFuncMappingKey(value_tmp, color_tmp);
//                                     myKey->setSourceValue(value_tmp);
                                 cElem->QueryFloatAttribute("Dest_left_value", &value_tmp);
                                 myKey->setAlphaL(value_tmp);
                                 cElem->QueryIntAttribute("isSplit", &int_tmp);
                                 if (int_tmp == 1) {
                                     myKey->setSplit(true);

                                     cElem->QueryFloatAttribute("Dest_right_value", &value_tmp);
                                     myKey->setAlphaR(value_tmp);
                                     cElem->QueryIntAttribute("Color_alt_r", &int_tmp);
                                     color_tmp.r = (uint8_t) int_tmp;
                                     cElem->QueryIntAttribute("Color_alt_g", &int_tmp);
                                     color_tmp.g = (uint8_t) int_tmp;
                                     cElem->QueryIntAttribute("Color_alt_b", &int_tmp);
                                     color_tmp.b = (uint8_t) int_tmp;
                                     cElem->QueryIntAttribute("Color_alt_a", &int_tmp);
                                     color_tmp.a = (uint8_t) int_tmp;
                                     myKey->setColorR(color_tmp);
							    } else
                                     myKey->setSplit(false);
            
                                // push key to vector
                                 tf->addKey(myKey);
                             } // for ( pElem; pElem; pElem=pElem->NextSiblingElement())
							 tf->updateTexture();
							 processor->postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, tf) ); 
                             
                         } // if (!pElem->NoChildren())
                     } catch (const std::bad_cast& /*ex*/) {
                     }
                    break;

                case Property::INTEGER_VEC2_PROP :
                    try{
                        IntVec2Prop* prop = dynamic_cast<IntVec2Prop*>(props.at(j));
                        int value_tmp;
                        tgt::ivec2 vector_tmp;
                        pElem->QueryIntAttribute("Vector_x", &value_tmp);
                        vector_tmp.x = (uint8_t) value_tmp;
                        pElem->QueryIntAttribute("Vector_y", &value_tmp);
                        vector_tmp.y = (uint8_t) value_tmp;

						if (prop) {
							prop->set(vector_tmp);
							processor->postMessage(new IVec2Msg(prop->getIdent(), prop->get()));  
						}
                    } catch (const std::bad_cast& /*ex*/) {
                    }
                    break;

                case Property::INTEGER_VEC3_PROP : 
                    try {
                        IntVec3Prop* prop = dynamic_cast<IntVec3Prop*>(props.at(j));
                        int value_tmp;
                        tgt::ivec3 vector_tmp;
                        pElem->QueryIntAttribute("Vector_x", &value_tmp);
                        vector_tmp.x = (uint8_t) value_tmp;
                        pElem->QueryIntAttribute("Vector_y", &value_tmp);
                        vector_tmp.y = (uint8_t) value_tmp;
                        pElem->QueryIntAttribute("Vector_z", &value_tmp);
                        vector_tmp.z = (uint8_t) value_tmp;

						if (prop) {
							prop->set(vector_tmp);
							processor->postMessage(new IVec3Msg(prop->getIdent(), prop->get()));  
						}
                    } catch (const std::bad_cast& /*ex*/) {
                    }
                    break;

                case Property::INTEGER_VEC4_PROP :
                    try {
                        IntVec4Prop* prop = dynamic_cast<IntVec4Prop*>(props.at(j));
                        int value_tmp;
                        tgt::ivec4 vector_tmp;
                        pElem->QueryIntAttribute("Vector_x", &value_tmp);
                        vector_tmp.x = (uint8_t) value_tmp;
                        pElem->QueryIntAttribute("Vector_y", &value_tmp);
                        vector_tmp.y = (uint8_t) value_tmp;
                        pElem->QueryIntAttribute("Vector_z", &value_tmp);
                        vector_tmp.z = (uint8_t) value_tmp;
                        pElem->QueryIntAttribute("Vector_w", &value_tmp);
                        vector_tmp.w = (uint8_t) value_tmp;

						if (prop) {
							prop->set(vector_tmp);
							processor->postMessage(new IVec4Msg(prop->getIdent(), prop->get()));  
						}
                    } catch (const std::bad_cast& /*ex*/) {
                    }
                    break;

                case Property::FLOAT_VEC2_PROP :
                    try {
                        FloatVec2Prop* prop = dynamic_cast<FloatVec2Prop*>(props.at(j));
                        float value_tmp;
                        tgt::vec2 vector_tmp;
                        pElem->QueryFloatAttribute("Vector_x", &value_tmp);
                        vector_tmp.x = value_tmp;
                        pElem->QueryFloatAttribute("Vector_y", &value_tmp);
                        vector_tmp.y = value_tmp;

						if (prop) {
							prop->set(vector_tmp);
							processor->postMessage(new Vec2Msg(prop->getIdent(), prop->get()));
						}
                    } catch (const std::bad_cast& /*ex*/) {
                    }
                    break;

                case Property::FLOAT_VEC3_PROP :
                    try {
                        FloatVec3Prop* prop = dynamic_cast<FloatVec3Prop*>(props.at(j));
                        float value_tmp;
                        tgt::vec3 vector_tmp;
                        pElem->QueryFloatAttribute("Vector_x", &value_tmp);
                        vector_tmp.x = value_tmp;
                        pElem->QueryFloatAttribute("Vector_y", &value_tmp);
                        vector_tmp.y = value_tmp;
                        pElem->QueryFloatAttribute("Vector_z", &value_tmp);
                        vector_tmp.z = value_tmp;

						if (prop) {
							prop->set(vector_tmp);
							processor->postMessage(new Vec3Msg(prop->getIdent(), prop->get()));
						}
                    } catch (const std::bad_cast& /*ex*/) {
                    }
                    break;

                case Property::FLOAT_VEC4_PROP :
                    try {
                        FloatVec4Prop* prop = dynamic_cast<FloatVec4Prop*>(props.at(j));
                        float value_tmp;
                        tgt::vec4 vector_tmp;
                        pElem->QueryFloatAttribute("Vector_x", &value_tmp);
                        vector_tmp.x = value_tmp;
                        pElem->QueryFloatAttribute("Vector_y", &value_tmp);
                        vector_tmp.y = value_tmp;
                        pElem->QueryFloatAttribute("Vector_z", &value_tmp);
                        vector_tmp.z = value_tmp;
                        pElem->QueryFloatAttribute("Vector_w", &value_tmp);
                        vector_tmp.w = value_tmp;

						if (prop) {
							prop->set(vector_tmp);
							processor->postMessage(new Vec4Msg(prop->getIdent(), prop->get()));
						}
                    } catch (const std::bad_cast& /*ex*/) {
                    }
                    break;

                } // switch(property_type)

                // set autochange
                int autoChange_tmp;
                pElem->QueryIntAttribute("Autochange", &autoChange_tmp);
                if (autoChange_tmp == 1)
					props.at(j)->setAutoChange(true);
                else
                    props.at(j)->setAutoChange(false);

            } // if ( renderproperty_list.at(i).second.at(j)->getGuiText() == property_text )
        } // for (size_t j = 1; j < renderproperty_list.at(i).second.size())
    } // for ( pElem; pElem; pElem=pElem->NextSiblingElement())
	return 0;
}

void NetworkSerializer::serializeToXml(const ProcessorNetwork& network, const std::string& filename) {
    TiXmlDocument doc;  
    TiXmlDeclaration* declNode = new TiXmlDeclaration("1.0", "ISO-8859-1", "");
    doc.LinkEndChild(declNode);
    // Serialize the network and add it as root element
    TiXmlElement* networkElem = network.serializeToXml();
    doc.LinkEndChild(networkElem);

    doc.SaveFile(filename);
}

} // namespace voreen
