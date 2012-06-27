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

#include "rptnetworkserializergui.h"
#include "rptarrow.h"
#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/transfunc/transfuncintensitykeys.h"

namespace voreen {

RptNetworkSerializerGui::RptNetworkSerializerGui():defFileExt_(".vnw") {
	processorItems.clear();
	count_=0;
	aggroMap_.clear();
	propertySetInfos_.clear();
}

/**
* Reads a XML file and creates the objects stored in it. Not all objects are immediately created however.
* The AggregationGuiItems and PropertySetGuiItems have to be created afterwards by using the neccessary information
* returned in the NetworkInfos* pointer. The reason for this is, that PropertySetGuiItems need to know
* the GraphicsScene they are to be added to in the constructor, and this class has no access to that information.
* For AggregationGuiItems similar reasons exist. Furthermore the RptProcessorItems are not connected to
* one another yet. This can only be done when the file reading is done, because otherwise a processor might
* try to connect to another processor which doesn't exist yet, because it wasn't yet read from xml. The
* function connectGuiItems does just that. But it has to be called AFTER the GuiItems are added to the
* GraphicsScene(at least the ProcessorItems). The arrows created when connecting the GuiItems need a scene.
* and if the GuiItems aren't added to a scene they don't get it, and the program crashes.
* This is not optimal (Stephan)
*/
NetworkInfos* RptNetworkSerializerGui::readNetworkFromFile(std::string filename) {
	count_=0;
	processorItems.clear();
	graphConnectionInformation_.clear();
	aggroMap_.clear();
	aggroNameMap_.clear();
	idMap_.clear();
	propertySetInfos_.clear();
	TiXmlDocument doc(filename);
	if (!doc.LoadFile()) 
		return 0;

	TiXmlHandle documentHandle(&doc);

	TiXmlElement* element;
	TiXmlHandle rootHandle(0);
    
    

	element=documentHandle.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!element) return 0;
	std::string s=element->Value();

	// save this for later
	rootHandle=TiXmlHandle(element);
	
	TiXmlNode* node=&doc;
    
    if (node->Type() == TiXmlNode::DOCUMENT) {
        std::cout << "Reading file: "<<node->Value() << "\n";
    }

    TiXmlNode* versionFinder = node;            //just for finding out the version, not needed afterwards
    int version = findVersion(versionFinder);
    
    if (version == 0)
        readProcessorsFromXml(node);
    else if (version ==1)
        readProcessorsFromXmlVersion1(node);
    else return 0;

    NetworkInfos* result = new NetworkInfos();
    result->processorItems = processorItems;
    result->aggroMap=aggroMap_;
    result->aggroNameMap = aggroNameMap_;
    result->propertySetInfos = propertySetInfos_;
    result->version = version;
    
    // Look for saved reuseTCTargets infos and set them if found 
    TiXmlElement* reuseTCTargetsElem = documentHandle.FirstChild( "reuseTCTargets" ).Element();
    if (reuseTCTargetsElem) {
        std::string value = reuseTCTargetsElem->Attribute( "Value" );
        result->reuseTCTargets = ( value == "TRUE" ? true : false );
    }
    else { // No Infos found - set to default (FIXME?)
        result->reuseTCTargets = false;
    }

    return result;
}

int RptNetworkSerializerGui::readVersionFromFile(std::string filename) {
    TiXmlDocument doc(filename);
    if (!doc.LoadFile()) 
        throw SerializerException("Could not load network file!");
    int version = findVersion(&doc);
    return version;
}


int RptNetworkSerializerGui::findVersion(TiXmlNode* node) {
    if (node->Type() != TiXmlNode::ELEMENT) {
        TiXmlNode* pChild;
        for (pChild = node->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
		    int result=findVersion(pChild);
            if (result!=0)
                return result;
	    }
    } else {
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

/**
* Reads an xml file and creates all ProcessorItems. Furthermore it gathers all the necessary information
* about PropertySets and Aggregations. This method is called by readNetworkFromFile(..). 
*/
int RptNetworkSerializerGui::readProcessorsFromXml(TiXmlNode* node) {
	RptProcessorItem* newGuiItem;								//the next ProcessorItem to be read from file
	ConnectionInfoForOneProcessor* newInfo;						//stores connections for the next ProcessorItem
	if(node->Type()== TiXmlNode::ELEMENT) {
		std::string s = node->Value();
		if (s == "RptProcessorItem") {
			TiXmlElement* element=node->ToElement();
			TiXmlAttribute* attribute=element->FirstAttribute();
			
			//Read the id of the stored processor. Remember that you can't store pointers, so
			//every processor has been given a number/id during the save process
			int guiItemId=attribute->IntValue();
			
			//Read the classtype of the processor, for example "volumeraycaster.fancyraycaster"
			attribute=attribute->Next();
			std::string type=attribute->Value();

            //FIX: FinalRenderer -> CanvasRenderer
            // sometime conversion outdatet
            if (type == "Miscellaneous.FinalRenderer")
                type = "Miscellaneous.Canvas";

            Identifier ident(type);

			//we have all the info we need to create the ProcessorItem (but not the connections)
			//to create the ProcessorItem we need the specific type, like "fancyraycaster" for the 
			//processorfactory, so we use the Identifier class to get that
			newGuiItem = new RptProcessorItem(ident.getSubString(1));
			
			
			attribute=attribute->Next();
			int xpos;
			int ypos;
			std::string isThereAggregation = attribute->Name();
			//If the name of the next attribute is Aggregation_Name this ProcessorItem is part of an aggregation,
			//otherwise it is not. Depending on that attribute name we have to read different things.
			if (isThereAggregation == "Aggregation_Name") {
				//the name of the aggregation
				std::string aggregationName = attribute->Value();
				attribute=attribute->Next();

				//the number(id) of the aggregation
				int aggregationNumber = attribute->IntValue();
				
				//To keep track of all the aggregations, we insert that information into the maps
				aggroNameMap_.insert(std::pair<int,std::string>(aggregationNumber,aggregationName));
				insertIntoAggroMap(aggregationNumber,newGuiItem);
				
				//Now we read the x and y position of the ProcessorItem
				attribute=attribute->Next();
				xpos = attribute->IntValue();
				attribute=attribute->Next();
				ypos = attribute->IntValue();	
			} else {
				//if the ProcessorItem is not part of an aggregation, we can read the x and y position directly
				xpos = attribute->IntValue();
				attribute=attribute->Next();
				ypos = attribute->IntValue();
			}
			
			newGuiItem->setPos(QPoint(xpos,ypos));
			//inser the processor of the guiItem and its id into the idMap
			idMap_.insert(std::pair<Processor*,int>(newGuiItem->getProcessor(),guiItemId));
			//add the new ProcessorItem to the ProcessorItem Vector
			processorItems.push_back(newGuiItem);
			//now get the connection Info and store it in graphConnectionInformation_ so we can create the connections between the
			//ProcessorItems later


			TiXmlNode* temp;	//this is just to make the navigation in the xml file easier, we still need node
			TiXmlNode* temp2;	//later and therefore can't change it

			//create a new ConnectionInfoForOneProcessorObject to store the connections
			graphConnectionInformation_.push_back(new ConnectionInfoForOneProcessor());			
			newInfo = graphConnectionInformation_.at(count_);
			count_++;

			//the id of ProcessorItem this information is for
			newInfo->id = guiItemId;
			
			if (node->FirstChild()->FirstChild()) {		//this checks if there are outputs saved in the xml file
				temp=node->FirstChild()->FirstChild();

				//if there are outputs go through them
				for (temp; temp != 0; temp = temp->NextSibling()) {
					element=temp->ToElement();
					attribute=element->FirstAttribute();
					
					//the type of the output port
					std::string target = attribute->Value();

					//create a new PortConnection object, that stores all the information about this Port(type, id, and connected Ports of
					//other processors
					PortConnection* newConnection = new PortConnection();
					newConnection->type=target;
					newConnection->portNumber=element->LastAttribute()->IntValue();
					
					//now get the ids of the processors connected to this port
					if (temp->FirstChild()) {
						temp2=temp->FirstChild();
						for (temp2; temp2 != 0; temp2 = temp2->NextSibling()) {
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
				for (temp; temp != 0; temp = temp->NextSibling()) {
					//now we start getting the output connections				
					element=temp->ToElement();
					attribute=element->FirstAttribute();
					std::string target = attribute->Value();
					
					//create a new PortConnection object, that stores all the information about this Port(type, id, and connected Ports of
					//other processors
					PortConnection* newConnection = new PortConnection();
					newConnection->type=target;
					newConnection->portNumber=element->LastAttribute()->IntValue();
					
					//now get the ids of the processors connected to this port
					if (temp->FirstChild()) {
						temp2=temp->FirstChild();
						for (temp2; temp2 != 0; temp2 = temp2->NextSibling()) {
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
				std::vector<Property*> props = newGuiItem->getProcessor()->getProperties();
				loadProperties(props,propertyElem,newGuiItem->getProcessor());

			}
		
		//We found a propertyset, so read it
        }else if (s == "PropertySet") {
			TiXmlElement* element=node->ToElement();
			TiXmlAttribute* attribute=element->FirstAttribute();
		
			attribute = attribute->Next();
			std::string propertySetName = attribute->Value();
			attribute = attribute->Next();
			int xpos = attribute->IntValue();
			attribute = attribute->Next();
			int ypos = attribute->IntValue();
			PropertySetInfos *propSetInfo = new PropertySetInfos();
			propSetInfo->name = propertySetName;
			propSetInfo->xpos = xpos;
			propSetInfo->ypos = ypos;
			if (node->FirstChild()->FirstChild()) {		//this checks if there are processors in this propertyset
				TiXmlNode* temp=node->FirstChild()->FirstChild();
				for (temp; temp != 0; temp = temp->NextSibling()) {
					//we found a processor and add it to the property set
					element=temp->ToElement();
					int processorId =element->FirstAttribute()->IntValue();
                    propSetInfo->processorItems.push_back(findGuiItem(processorId));
					
				}
			}
			propertySetInfos_.push_back(propSetInfo);
		}
        /*else 
            std::cout << s << std::endl;*/
    } /*else 
        std::cout << node->Type() << std::endl;*/

	
	//search for other processors and propertysets in the xml file
	TiXmlNode* pChild;
	for (pChild = node->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
		readProcessorsFromXml(pChild);
	}
	return 0;
}

int RptNetworkSerializerGui::readProcessorsFromXmlVersion1(TiXmlNode* node) {
	RptProcessorItem* newGuiItem;								//the next ProcessorItem to be read from file
	ConnectionInfoForOneProcessor* newInfo;						//stores connections for the next ProcessorItem
	if(node->Type()== TiXmlNode::ELEMENT) {
		std::string s = node->Value();
		if (s == "RptProcessorItem") {
			TiXmlElement* element=node->ToElement();
			TiXmlAttribute* attribute=element->FirstAttribute();
			
			//Read the id of the stored processor. Remember that you can't store pointers, so
			//every processor has been given a number/id during the save process
			int guiItemId=attribute->IntValue();
			
			//Read the classtype of the processor, for example "volumeraycaster.fancyraycaster"
			attribute=attribute->Next();
			std::string type=attribute->Value();


            //FIX: FinalRenderer -> CanvasRenderer
            // sometime conversion outdatet
            if (type == "Miscellaneous.FinalRenderer")
                type = "Miscellaneous.Canvas";

            Identifier ident(type);

			//we have all the info we need to create the ProcessorItem (but not the connections)
			//to create the ProcessorItem we need the specific type, like "fancyraycaster" for the 
			//processorfactory, so we use the Identifier class to get that
			newGuiItem = new RptProcessorItem(ident.getSubString(1));
			
            if (newGuiItem->getProcessor()) {
                
			    attribute=attribute->Next();
                //If the name of the next attribute is Processor_Name we have saved it before
                std::string dowehaveaName = attribute->Name();
                if (dowehaveaName == "Processor_Name") {
                    std::string canvasMerge = attribute->Value();  //FIXME: FinalRenderer -> CanvasRenderer
                    if (canvasMerge == "FinalRenderer")
                        canvasMerge = "Canvas";
                    newGuiItem->setName(canvasMerge /*attribute->Value()*/);
                    attribute=attribute->Next();
                }

			    int xpos;
			    int ypos;
			    std::string isThereAggregation = attribute->Name();
			    //If the name of the next attribute is Aggregation_Name this ProcessorItem is part of an aggregation,
			    //otherwise it is not. Depending on that attribute name we have to read different things.
			    if (isThereAggregation == "Aggregation_Name") {
				    //the name of the aggregation
				    std::string aggregationName = attribute->Value();
				    attribute=attribute->Next();

				    //the number(id) of the aggregation
				    int aggregationNumber = attribute->IntValue();
    				
				    //To keep track of all the aggregations, we insert that information into the maps
				    aggroNameMap_.insert(std::pair<int,std::string>(aggregationNumber,aggregationName));
				    insertIntoAggroMap(aggregationNumber,newGuiItem);
    				
				    //Now we read the x and y position of the ProcessorItem
				    attribute=attribute->Next();
				    xpos = attribute->IntValue();
				    attribute=attribute->Next();
				    ypos = attribute->IntValue();	
			    } else {
				    //if the ProcessorItem is not part of an aggregation, we can read the x and y position directly
				    xpos = attribute->IntValue();
				    attribute=attribute->Next();
				    ypos = attribute->IntValue();
			    }
    			
			    newGuiItem->setPos(QPoint(xpos,ypos));
			    //inser the processor of the guiItem and its id into the idMap
			    idMap_.insert(std::pair<Processor*,int>(newGuiItem->getProcessor(),guiItemId));
			    //add the new ProcessorItem to the ProcessorItem Vector
			    processorItems.push_back(newGuiItem);
			    //now get the connection Info and store it in graphConnectionInformation_ so we can create the connections between the
			    //ProcessorItems later


			    TiXmlNode* temp;	//this is just to make the navigation in the xml file easier, we still need node
			    TiXmlNode* temp2;	//later and therefore can't change it

			    //create a new ConnectionInfoForOneProcessorObject to store the connections
			    newInfo = new ConnectionInfoForOneProcessor();
			    graphConnectionInformation_.push_back(newInfo);			
			    count_++;

			    //the id of ProcessorItem this information is for
			    newInfo->id = guiItemId;
    			
			    if (node->FirstChild()->FirstChild()) {		//this checks if there are inputs saved in the xml file
				    temp=node->FirstChild()->FirstChild();

				    //if there are inputs go through them
				    for (temp; temp != 0; temp = temp->NextSibling()) {
					    element=temp->ToElement();
					    attribute=element->FirstAttribute();
    					
					    //the type of the input port
					    std::string target = attribute->Value();

					    //create a new PortConnection object, that stores all the information about this Port(type, id, and connected Ports of
					    //other processors
					    PortConnection* newConnection = new PortConnection();
					    newConnection->type=target;
					    newConnection->portNumber=element->LastAttribute()->IntValue();
    					
					    //now get the ids of the processors connected to this port
					    if (temp->FirstChild()) {
						    temp2=temp->FirstChild();
						    for (temp2; temp2 != 0; temp2 = temp2->NextSibling()) {
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
				    for (temp; temp != 0; temp = temp->NextSibling()) {
					    //now we start getting the output connections				
					    element=temp->ToElement();
					    attribute=element->FirstAttribute();
					    std::string target = attribute->Value();
    					
					    //create a new PortConnection object, that stores all the information about this Port(type, id, and connected Ports of
					    //other processors
					    PortConnection* newConnection = new PortConnection();
					    newConnection->type=target;
					    newConnection->portNumber=element->LastAttribute()->IntValue();
    					
					    //now get the ids of the processors connected to this port
					    if (temp->FirstChild()) {
						    temp2=temp->FirstChild();
						    for (temp2; temp2 != 0; temp2 = temp2->NextSibling()) {
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
				    std::vector<Property*> props = newGuiItem->getProcessor()->getProperties();
				    loadProperties(props,propertyElem,newGuiItem->getProcessor());

			    }
            }
		}
        else if (s == "PropertySet") {
            //We found a propertyset, so read it
			TiXmlElement* element=node->ToElement();
			TiXmlAttribute* attribute=element->FirstAttribute();
		
			attribute = attribute->Next();
			std::string propertySetName = attribute->Value();
			attribute = attribute->Next();
			int xpos = attribute->IntValue();
			attribute = attribute->Next();
			int ypos = attribute->IntValue();
			PropertySetInfos *propSetInfo = new PropertySetInfos();
			propSetInfo->name = propertySetName;
			propSetInfo->xpos = xpos;
			propSetInfo->ypos = ypos;
			if (node->FirstChild()->FirstChild()) {		//this checks if there are processors in this propertyset
				TiXmlNode* temp=node->FirstChild()->FirstChild();
				for (temp; temp != 0; temp = temp->NextSibling()) {
					//we found a processor and add it to the property set
					element=temp->ToElement();
					int processorId =element->FirstAttribute()->IntValue();
                    propSetInfo->processorItems.push_back(findGuiItem(processorId));
					
				}
			}
			propertySetInfos_.push_back(propSetInfo);
		}
	} 

	
	//search for other processors and propertysets in the xml file
	TiXmlNode* pChild;
	for (pChild = node->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
		readProcessorsFromXmlVersion1(pChild);
	}
	return 0;
}

/**
* Uses the information stored in graphConnectionInformation_ to connect the objects in
* the right way. You can call this function after having read a network from file AND after adding
* the read objects to the GraphicsScene. 
*/
int RptNetworkSerializerGui::connectGuiItems() {
	if ((processorItems.size()<1) || (graphConnectionInformation_.size()<1) )
		return -1;

	RptProcessorItem* currentGuiItem;
	ConnectionInfoForOneProcessor* currentInfo;
	
	for (size_t i=0;i < graphConnectionInformation_.size();i++) {
		
		//Each element in graphConnectionInformation stores the connection info for one ProcessorItem
		//This ProcessorItem is searched by its id
		currentInfo=graphConnectionInformation_.at(i);
		currentGuiItem=findGuiItem(currentInfo->id);
		
		if (!currentGuiItem)
			return -1;
		else {
			
			//Get the output connection informations for this processor
			std::vector<PortConnection*> out;
			out=currentInfo->outputs;
			
			//iterate throuh it and connect the ProcessorItems
			for (size_t j=0;j<out.size();j++) {
				PortConnection* con = out.at(j);
				std::string temp =con->type.getSubString(0);

				//coprocessor connections have to be handled differently so we have to check
				if (temp != "coprocessor") {
					if ((size_t)con->portNumber < currentGuiItem->getOutports().size() ) {
						RptPortItem* sourcePort = currentGuiItem->getOutports().at(con->portNumber);

						//We have the source port and now get the map that contains the information
						//to which port of which processors this port is connected. 
						std::map<int,int> connectedProcessors = con->connectedProcessors;
						//first entry stands for the processor id, the second for the port number
						for(std::map<int,int>::iterator k = connectedProcessors.begin(); k != connectedProcessors.end(); ++k) {
							RptProcessorItem* dest = findGuiItem(k->first);
							if ((size_t)k->second < dest->getInports().size()) {
								RptPortItem* destPort = dest->getInports().at(k->second);
								currentGuiItem->connectAndCreateArrow(sourcePort,destPort);
							}
						}
					}
				//we found a coprocessor connection
				} else {
					if ((size_t)con->portNumber < currentGuiItem->getCoProcessorOutports().size()) {
						RptPortItem* sourcePort = currentGuiItem->getCoProcessorOutports().at(con->portNumber);
						//We have the source port and now get the map that contains the information
						//to which port of which processors this port is connected. 
						std::map<int,int> connectedProcessors = con->connectedProcessors;
						//first entry stands for the processor id, the second for the port number
						for(std::map<int,int>::const_iterator k = connectedProcessors.begin(); k != connectedProcessors.end(); ++k) {
							RptProcessorItem* dest = findGuiItem(k->first);
							if ((size_t)k->second < dest->getCoProcessorInports().size()) {
								RptPortItem* destPort = dest->getCoProcessorInports().at(k->second);
								currentGuiItem->connect(sourcePort,destPort);
							}
						}
					}
				}
			}
		}
	}
	return 0;

}

int RptNetworkSerializerGui::connectGuiItemsVersion1() {
	if ((processorItems.size()<1) || (graphConnectionInformation_.size()<1) )
		return -1;

	RptProcessorItem* currentGuiItem;
	ConnectionInfoForOneProcessor* currentInfo;
	
	for (size_t i=0;i < graphConnectionInformation_.size();i++) {
		
		//Each element in graphConnectionInformation stores the connection info for one ProcessorItem
		//This ProcessorItem is searched by its id
		currentInfo=graphConnectionInformation_.at(i);
		currentGuiItem=findGuiItem(currentInfo->id);
		
		if (!currentGuiItem)
			return -1;
		else {
			
			//Get the input connection informations for this processor
			std::vector<PortConnection*> in;
			in=currentInfo->inputs;
			
			//iterate throuh it and connect the ProcessorItems
			for (size_t j=0;j<in.size();j++) {
				PortConnection* con = in.at(j);

				if (currentGuiItem->getPortItem(con->type) ) {
					RptPortItem* destPort = currentGuiItem->getPortItem(con->type);

					std::vector<ConnectedProcessor> connectedProcessors = con->connectedProcessorsVector;

					for (size_t k=0; k<connectedProcessors.size(); k++) {
						RptProcessorItem* source = findGuiItem(connectedProcessors.at(k).processorNumber);
                        if (source) {
                            RptPortItem* sourcePort = source->getPortItem(connectedProcessors.at(k).portType);
						    if (sourcePort) {
							    source->connectAndCreateArrow(sourcePort,destPort);
						    }
                        }
					}
					
				}
			}
		}
	}
	return 0;

}

/**
* This function inserts the ProcessorItem with the aggroNumber into the aggroMap_. If the map already has a key
* equal to aggroNumber the guiItem is inserted into its vector, otherwise a key is created and the vector initialised with
* the guiItem. 
*/
void RptNetworkSerializerGui::insertIntoAggroMap(int aggroNumber, RptProcessorItem* guiItem) {
	bool found =false;
	for(std::map<int,std::vector<RptProcessorItem*>*>::iterator i = aggroMap_.begin(); i != aggroMap_.end(); ++i) {
		if (i->first == aggroNumber) {
			i->second->push_back(guiItem);
			found=true;
			break;
		}
	}
	if (found==false) {
		std::vector<RptProcessorItem*>* newVector = new std::vector<RptProcessorItem*>;
		newVector->push_back(guiItem);
		aggroMap_.insert(std::pair<int,std::vector<RptProcessorItem*>*>(aggroNumber,newVector));
	}
}

/**
*Returns the RptProcessorItem with the given id if it is found in processorItems
*/
RptProcessorItem* RptNetworkSerializerGui::findGuiItem(int id) {
	for (size_t i=0;i<processorItems.size();i++) {
		if (idMap_[processorItems.at(i)->getProcessor()] == id)
			return processorItems.at(i);
	}
	return 0;
}

/**
*Takes the RptProcessorItem vector and aggregation vector and PropertySetGuiItem vector and serializes them to xml
*/
int RptNetworkSerializerGui::serializeToXml(std::vector<RptProcessorItem*> guiItems, std::vector<RptAggregationItem*> aggros, std::vector<RptPropertySetItem*> propertySetGuiItems, bool reuseTCTargets, std::string filename) {
	idMap_.clear();
	int id=0;

	//give every processor an id
	for (size_t i=0; i< guiItems.size(); i++) {
		idMap_.insert(std::pair<Processor*,int>(guiItems.at(i)->getProcessor(),id));
		id++;
	}

	//give every processor in the aggregations an id too
	for (size_t i=0; i< aggros.size(); i++) {
		for (size_t j=0; j< aggros.at(i)->getProcessorItems().size(); j++) {
			idMap_.insert(std::pair<Processor*,int>(aggros.at(i)->getProcessorItems().at(j)->getProcessor(),id));
			id++;
		}
	}

	//give the propertysets an id too
	for (size_t i=0; i< propertySetGuiItems.size(); i++) {
			idMap_.insert(std::pair<Processor*,int>(propertySetGuiItems.at(i)->getProcessor(0),id));
			id++;
	}
	
	TiXmlDocument doc;  
 	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	TiXmlElement * version = new TiXmlElement("Saved_Voreen_Network_File");
    version->SetAttribute("Version",1);
	doc.LinkEndChild(version);

    TiXmlElement* reuseTCTargetsElem = new TiXmlElement( "reuseTCTargets" );
    doc.LinkEndChild( reuseTCTargetsElem );
    reuseTCTargetsElem->SetAttribute("Value", reuseTCTargets ? "TRUE" : "FALSE");

	TiXmlElement * root = new TiXmlElement( "RptGuiDataStructure" );  
	doc.LinkEndChild( root );  
	
	//serialize the ProcessorItems
	for (size_t i=0; i< guiItems.size(); i++) {
		serializeGuiItemVersion1(guiItems.at(i),root,idMap_[guiItems.at(i)->getProcessor()],false,0,"");
	}
	
	//serialize the ProcessorItems in the aggregations
	for (size_t i=0; i< aggros.size(); i ++) {
		for (size_t j=0; j<aggros.at(i)->getProcessorItems().size(); j++) {
			serializeGuiItemVersion1(aggros.at(i)->getProcessorItems().at(j),root,idMap_[aggros.at(i)->getProcessorItems().at(j)->getProcessor()],true,i,aggros.at(i)->getName());
		}
	}
	
	TiXmlElement * propSetXmlElem = new TiXmlElement( "PropertySets" ); 
	doc.LinkEndChild(propSetXmlElem);

	//serialize the propertysets. It is important to serialize them last!
	for (size_t i=0; i< propertySetGuiItems.size(); i ++) {
		serializePropertySet(propertySetGuiItems.at(i),propSetXmlElem,id);
	}

    // ensure that the file extension is correct.
    //
    unsigned int len = filename.length();   // unsigned to resolve conflict
    unsigned int extLen = defFileExt_.length();
    if( len >= extLen )   // check whether length may already be able to contain extension
    {
        // append file extension to filename if not already contained
        //
        std::string ext = filename.substr(len - extLen, len);
        if( ext != defFileExt_ )
        {
            filename += defFileExt_;
        }
    }
    doc.SaveFile(filename);
    
    return 0;
}

/**
* Serializes one PropertySetGuiItem to xml
* @param xmlParent A TiXmlElement object holding the file and the location to write to
* @param id The id of the PropertySetGuiItem (determined in serializeToXml(...) )
*/
void RptNetworkSerializerGui::serializePropertySet(RptPropertySetItem* propSetGuiItem, TiXmlElement* root,int id) {
	TiXmlElement * xmlPropertySetElem = new TiXmlElement("PropertySet");  
	root->LinkEndChild( xmlPropertySetElem );  
	xmlPropertySetElem->SetAttribute("Id",id);
	xmlPropertySetElem->SetAttribute("Name",propSetGuiItem->getProcessor(0)->getName()); //this has to become propSetGuiItem->getName()!
	xmlPropertySetElem->SetAttribute("xpos",propSetGuiItem->pos().x());
	xmlPropertySetElem->SetAttribute("ypos",propSetGuiItem->pos().y());

	TiXmlElement* connections = new TiXmlElement("Connections");
	xmlPropertySetElem->LinkEndChild(connections);

	std::vector<Processor*> connectedProcessors = propSetGuiItem->getPropertySet()->getProcessors();
	for (size_t i=0; i<connectedProcessors.size(); i++) {
		TiXmlElement* processor = new TiXmlElement("Renderer");
		processor->SetAttribute("Id",idMap_[connectedProcessors.at(i)]);
		connections->LinkEndChild(processor);

	}
}

int RptNetworkSerializerGui::serializeGuiItemVersion1(RptProcessorItem* guiItem,TiXmlElement* root, int id, bool hasAggro, int aggroNumber, std::string aggroName ) {
	TiXmlElement * xmlGuiElem = new TiXmlElement("RptProcessorItem");
	root->LinkEndChild( xmlGuiElem );  
	
	//write the id, class and so on to xml
	xmlGuiElem->SetAttribute("Id",id);
	xmlGuiElem->SetAttribute("Type", guiItem->getProcessor()->getClassName().getName());
    xmlGuiElem->SetAttribute("Processor_Name", guiItem->getName());
	
	//if this guiItem is part of an aggregation, write that info to xml too
	if (hasAggro) {
		xmlGuiElem->SetAttribute("Aggregation_Name", aggroName);
		xmlGuiElem->SetAttribute("Aggregation_Number",aggroNumber);		
	}

	//write its position to xml
	xmlGuiElem->SetAttribute("xpos",guiItem->pos().x());
	xmlGuiElem->SetAttribute("ypos",guiItem->pos().y());

	//Write Output Connections to XML
	TiXmlElement* inputcons = new TiXmlElement("Inputs");
	xmlGuiElem->LinkEndChild(inputcons);

	std::vector<RptPortItem*> ports=guiItem->getInports();
	std::vector<TiXmlElement*> connectionElements;
	TiXmlElement* connectionToGuiItem;
	
	//write the outport information to xml
	for (size_t i=0; i < ports.size() ; i++) {
		connectionElements.push_back(new TiXmlElement("Input_Port"));
		connectionElements.at(i)->SetAttribute("type",ports.at(i)->getPortType().getName());
		connectionElements.at(i)->SetAttribute("port_number",i);

		//Now add every port that is connected to this one
		std::vector<Port*> portList = ports.at(i)->getPort()->getConnected();

		for (size_t j=0 ; j < portList.size(); j++) {
			connectionToGuiItem=new TiXmlElement("Renderer");
			Processor* targetProcessor = portList.at(j)->getProcessor();
			connectionToGuiItem->SetAttribute("renderer_id",idMap_[targetProcessor]);
			std::string portType = portList.at(j)->getType().getName();
			connectionToGuiItem->SetAttribute("port_type",portType);
			int targetPortNumber = findPortNumber(targetProcessor ,portList.at(j));
			if (targetPortNumber != -1) {
				connectionToGuiItem->SetAttribute("portitem_number",targetPortNumber);
			}
			connectionElements.at(i)->LinkEndChild(connectionToGuiItem);
		}
		inputcons->LinkEndChild(connectionElements.at(i));
	}
	
	//--------------------------------------------------------------------
	//New code for the coprocessors (appears to be working now)
	//--------------------------------------------------------------------

	//write the coprocessor outport information to xml
	TiXmlElement* coProcessorInputCons = new TiXmlElement("CoProcessor-Inputs");
	xmlGuiElem->LinkEndChild(coProcessorInputCons);

	std::vector<RptPortItem*> coProcessorPorts=guiItem->getCoProcessorInports();
	connectionElements.clear();

	//go through the coprocessor outports and write them to xml
	for (size_t i=0; i < coProcessorPorts.size() ; i++) {
		connectionElements.push_back(new TiXmlElement("CoProcessorInput_Port"));
		connectionElements.at(i)->SetAttribute("type",coProcessorPorts.at(i)->getPortType().getName());
		connectionElements.at(i)->SetAttribute("port_number",i);

		//Now add every port that isconnected to this one
		std::vector<Port*> portList = coProcessorPorts.at(i)->getPort()->getConnected();

		for (size_t j=0 ; j < portList.size(); j++) {
			connectionToGuiItem=new TiXmlElement("Renderer");
			Processor* targetProcessor = portList.at(j)->getProcessor();
			connectionToGuiItem->SetAttribute("renderer_id",idMap_[targetProcessor]);
			std::string portType = portList.at(j)->getType().getName();
			connectionToGuiItem->SetAttribute("port_type",portType);
			int targetPortNumber = findCoProcessorPortNumber(targetProcessor ,portList.at(j));
			if (targetPortNumber != -1) {
				connectionToGuiItem->SetAttribute("portitem_number",targetPortNumber);
			}
			connectionElements.at(i)->LinkEndChild(connectionToGuiItem);
		}
		coProcessorInputCons->LinkEndChild(connectionElements.at(i));
	}
	
	//save the properties of the processor
	TiXmlElement * propertyElem = new TiXmlElement("RendererProperties");
	xmlGuiElem->LinkEndChild(propertyElem);
	std::vector<Property*> props = guiItem->getProcessor()->getProperties();
	saveProperties(props,propertyElem);
	return 0;
}

int RptNetworkSerializerGui::serializeGuiItem(RptProcessorItem* guiItem,TiXmlElement* root, int id, bool hasAggro, int aggroNumber, std::string aggroName ) {
	TiXmlElement * xmlGuiElem = new TiXmlElement("RptProcessorItem");
	root->LinkEndChild( xmlGuiElem );  
	
	//write the id, class and so on to xml
	xmlGuiElem->SetAttribute("Id",id);
	xmlGuiElem->SetAttribute("Type", guiItem->getProcessor()->getClassName().getName());
	
	//if this guiItem is part of an aggregation, write that info to xml too
	if (hasAggro) {
		xmlGuiElem->SetAttribute("Aggregation_Name", aggroName);
		xmlGuiElem->SetAttribute("Aggregation_Number",aggroNumber);		
	}

	//write its position to xml
	xmlGuiElem->SetAttribute("xpos",guiItem->pos().x());
	xmlGuiElem->SetAttribute("ypos",guiItem->pos().y());

	//Write Output Connections to XML
	TiXmlElement* outputcons = new TiXmlElement("Outputs");
	xmlGuiElem->LinkEndChild(outputcons);

	std::vector<RptPortItem*> ports=guiItem->getOutports();
	std::vector<TiXmlElement*> connectionElements;
	TiXmlElement* connectionToGuiItem;
	
	//write the outport information to xml
	for (size_t i=0; i < ports.size() ; i++) {
		connectionElements.push_back(new TiXmlElement("Output_Port"));
		connectionElements.at(i)->SetAttribute("type",ports.at(i)->getPortType().getName());
		connectionElements.at(i)->SetAttribute("port_number",i);

		//Now add every port that is connected to this one
		std::vector<Port*> portList = ports.at(i)->getPort()->getConnected();

		for (size_t j=0 ; j < portList.size(); j++) {
			connectionToGuiItem=new TiXmlElement("Renderer");
			Processor* targetProcessor = portList.at(j)->getProcessor();
			connectionToGuiItem->SetAttribute("renderer_id",idMap_[targetProcessor]);
			int targetPortNumber = findPortNumber(targetProcessor ,portList.at(j));
			if (targetPortNumber != -1) {
				connectionToGuiItem->SetAttribute("portitem_number",targetPortNumber);
			}
			connectionElements.at(i)->LinkEndChild(connectionToGuiItem);
		}
		outputcons->LinkEndChild(connectionElements.at(i));
	}
	
	//--------------------------------------------------------------------
	//New code for the coprocessors (appears to be working now)
	//--------------------------------------------------------------------

	//write the coprocessor outport information to xml
	TiXmlElement* coProcessorOutputCons = new TiXmlElement("CoProcessor-Outputs");
	xmlGuiElem->LinkEndChild(coProcessorOutputCons);

	std::vector<RptPortItem*> coProcessorPorts=guiItem->getCoProcessorOutports();
	connectionElements.clear();

	//go through the coprocessor outports and write them to xml
	for (size_t i=0; i < coProcessorPorts.size() ; i++) {
		connectionElements.push_back(new TiXmlElement("CoProcessorOutput_Port"));
		connectionElements.at(i)->SetAttribute("type",coProcessorPorts.at(i)->getPortType().getName());
		connectionElements.at(i)->SetAttribute("port_number",i);

		//Now add every port that isconnected to this one
		std::vector<Port*> portList = coProcessorPorts.at(i)->getPort()->getConnected();

		for (size_t j=0 ; j < portList.size(); j++) {
			connectionToGuiItem=new TiXmlElement("Renderer");
			Processor* targetProcessor = portList.at(j)->getProcessor();
			connectionToGuiItem->SetAttribute("renderer_id",idMap_[targetProcessor]);
			int targetPortNumber = findCoProcessorPortNumber(targetProcessor ,portList.at(j));
			if (targetPortNumber != -1) {
				connectionToGuiItem->SetAttribute("portitem_number",targetPortNumber);
			}
			connectionElements.at(i)->LinkEndChild(connectionToGuiItem);
		}
		coProcessorOutputCons->LinkEndChild(connectionElements.at(i));
	}
	
	//save the properties of the processor
	TiXmlElement * propertyElem = new TiXmlElement("RendererProperties");
	xmlGuiElem->LinkEndChild(propertyElem);
	std::vector<Property*> props = guiItem->getProcessor()->getProperties();
	saveProperties(props,propertyElem);
	return 0;
}


/**
* Finds the port number of a port in a processor. This only works for INPORTS. Every processor has a vector of inports,
* and this function searches the given Port in that vector, and returns its place in the vector. So if the Port is the
* third port in the vector, it returns 2.
*/ 
int RptNetworkSerializerGui::findPortNumber(Processor* processor,Port* p) {
	for (size_t i=0; i< processor->getInports().size(); i++) {
		if (processor->getInports().at(i) == p) {
			return i;
		}
	}
	return -1;
}

/**
* Same as findPortNumber, just for CoProcessorInports.
*/ 
int RptNetworkSerializerGui::findCoProcessorPortNumber(Processor* processor,Port* p) {
	for (size_t i=0; i< processor->getCoProcessorInports().size(); i++) {
		if (processor->getCoProcessorInports().at(i) == p) {
			return i;
		}
	}
	return -1;
}

/**
* Loads the property vector of one processor from xml. (shamelessely copied and modified from widgetgenerator)
*/
int RptNetworkSerializerGui::loadProperties(std::vector<Property* > props, TiXmlElement* xmlParent, Processor* processor) {
    std::string property_text;
    std::string conditionIdentifier_text;
    std::string propText;
    TiXmlElement* pElem;
    //TiXmlElement* cElem;

    pElem = xmlParent->FirstChildElement();
    for( ; pElem; pElem=pElem->NextSiblingElement()) {

        property_text = pElem->Attribute("Property_text");
        //std::cout << "property: " << property_text <<std::endl;
        for(size_t j = 0; j < props.size(); j++) {
            if (props.at(j)->getGuiText() == property_text ) {
                // Found a property in the current pipeline which can get
                // some attributes from xml file
                if (property_text=="empty") {
                    break;
                }
                
                int property_type;
                pElem->QueryIntAttribute("Property_type", &property_type);
                
                // first set autochange false
                props.at(j)->setAutoChange(false);
                //std::cout << "Type: " << property_type<<std::endl;
                //props.at(j)->updateFromXml(pElem);
                
                switch(property_type) {
                    case Property::FLOAT_PROP : {
                        try {
                            float value_tmp;
							FloatProp* prop = dynamic_cast<FloatProp*>(props.at(j));
                            pElem->QueryFloatAttribute("Value", &value_tmp);
							if (prop) {
								prop->set(value_tmp);
								processor->postMessage(new FloatMsg(prop->getIdent(), value_tmp));
							}
                        } 
						catch (const std::bad_cast& //ex
						) {    
						}
                        break;
                    }
                    case Property::INT_PROP : {
                        try {
                            IntProp* prop = dynamic_cast<IntProp*>(props.at(j));
                            int value_tmp;
                            pElem->QueryIntAttribute("Value", &value_tmp);
							if (prop) {
								prop->set(value_tmp);
								processor->postMessage(new IntMsg(prop->getIdent(), value_tmp));
							}
						}
                        catch (const std::bad_cast& //ex
) {
							//std::cout<< "args tot";
                        }
                        break;
                    }
											  //TODO: Check if this is working correctly
                    case Property::BOOL_PROP : {
                        try {
                            pElem->QueryIntAttribute("Property_type", &property_type);
                            BoolProp* prop = dynamic_cast<BoolProp*>(props.at(j));
                            if (prop) {
                                int value_tmp = 0;
                                pElem->QueryIntAttribute("Value", &value_tmp);
                                if (value_tmp == 1) {
								    prop->set(true);
								    processor->postMessage(new BoolMsg(prop->getIdent(), true));
								}
                                else
								{
									prop->set(false);
									processor->postMessage(new BoolMsg(prop->getIdent(), false));
								}
                            } 
                        } 
                        catch (const std::bad_cast& //ex
) {
                            
                        }
                        break;
                    }
                    case Property::COLOR_PROP : {
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
                        }
                        catch (const std::bad_cast& //ex
) {
                    
                        }
                       break;
                    }
                    case Property::ENUM_PROP : {
                        try {
                            EnumProp* prop = dynamic_cast<EnumProp*>(props.at(j));
                            int value_tmp;
                            pElem->QueryIntAttribute("Value", &value_tmp);
							if (prop) {
								prop->set(value_tmp);
								if (prop->getSendStringMsg() ) {
									processor->postMessage(new StringMsg(prop->getIdent(),prop->getStrings().at(value_tmp))); 
								}
								else {
									processor->postMessage(new IntMsg(prop->getIdent(),value_tmp));
								}
							}
                        } 
                        catch (const std::bad_cast& //ex
) {
                        }
                        break;
                    }
                    case Property::TRANSFUNC_PROP : {
                        try {
                             TransFuncIntensityKeys* tf = new TransFuncIntensityKeys();
							 tf->clearKeys();

                             //iterate through all markers
                             //first set conditions for the current item if there're some
                             if (!pElem->NoChildren()) {
								 TiXmlElement* cElem;
                                 cElem = pElem->FirstChildElement();
                                 for(; cElem; cElem=cElem->NextSiblingElement()) {
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
 								    } else {
                                         myKey->setSplit(false);
 								    }
                                     // push key to vector
                                     tf->addKey(myKey);
                                 } // for( pElem; pElem; pElem=pElem->NextSiblingElement())
								 tf->updateTexture();
								 processor->postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, tf) ); 
                                 
                             } // if (!pElem->NoChildren())
                        } 
                        catch (const std::bad_cast& //ex
) {
                             
                        }
                        break;
                    }
                    case Property::INTEGER_VEC2_PROP : {
                        try {
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
                        } 
                        catch (const std::bad_cast& //ex
) {
                            
                        }
                        break;
                    }
                    case Property::INTEGER_VEC3_PROP : {
                        try{
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
                        }
                        catch (const std::bad_cast& //ex
) {
                        }
                        break;
                    }
                    case Property::INTEGER_VEC4_PROP : {
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
                        } 
                        catch (const std::bad_cast& //ex
) {

                        }
                        break;
                    }
                    case Property::FLOAT_VEC2_PROP : {
                        try{
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
                        }
                        catch (const std::bad_cast& //ex
) {
                        }
                        break;
                    }
                    case Property::FLOAT_VEC3_PROP : {
                        try{
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
                        } 
                        catch (const std::bad_cast& //ex
) {
                        }
                        break;
                    }
                    case Property::FLOAT_VEC4_PROP : {
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
                        } 
                        catch (const std::bad_cast& //ex
) {
                        }
                        break;
                    }
                } // switch(property_type)
                
                // set autochange
                int autoChange_tmp;
                pElem->QueryIntAttribute("Autochange", &autoChange_tmp);
                if (autoChange_tmp == 1) {
					props.at(j)->setAutoChange(true);
                } else {
                    props.at(j)->setAutoChange(false);
                }
            } // if( renderproperty_list.at(i).second.at(j)->getGuiText() == property_text )
        } // for(size_t j = 1; j < renderproperty_list.at(i).second.size())
    } // for( pElem; pElem; pElem=pElem->NextSiblingElement())
	return 0;
}

/**
* Saves the property vector of one processor to xml. (shamelessely copied and modified from widgetgenerator)
*/
int RptNetworkSerializerGui::saveProperties(std::vector<Property*> props, TiXmlElement* xmlParent) {
    std::stringstream myStream;
    std::stringstream myStream_cond;
    std::stringstream myStream_marker;
    //iterate through all properties of the processor

    for(size_t i = 0; i < props.size(); ++i){
        // Skip properties which should not be serialized based on the
        // value of isSerializable(). By default, all properties
        // are to be serialized.
        
        if( (props[i] != 0) && (props[i]->isSerializable() == false) )
        {
            continue;
        }
        

        //iterate through all properties and save them to xml
        
        TiXmlElement *xmlProperty = new TiXmlElement("Property");
        xmlParent->LinkEndChild(xmlProperty);
        
        /* 
        if( props[i]->isSerializable() ) {
            TiXmlElement* propElem = props[i]->serializeToXml();
            xmlParent->LinkEndChild(propElem);
        }
        */
        
        
        xmlProperty->SetAttribute("Property_text" , props.at(i)->getGuiText());
        xmlProperty->SetAttribute("Grouped", props.at(i)->getGroup().getName());
        xmlProperty->SetAttribute("Autochange", props.at(i)->getAutoChange());

        switch (props.at(i)->getType()) {
            case Property::FLOAT_PROP : {
                FloatProp* prop1 = dynamic_cast<FloatProp*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::FLOAT_PROP);
                xmlProperty->SetDoubleAttribute("Value", prop1->get());
                xmlProperty->SetAttribute("Max_value" ,(int)prop1->getMaxValue());
                xmlProperty->SetAttribute("Min_value" ,(int)prop1->getMinValue());
                break;
                                        }
            case Property::INT_PROP : {
				IntProp* prop1 = dynamic_cast<IntProp*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::INT_PROP);
                xmlProperty->SetAttribute("Value", prop1->get());
                xmlProperty->SetAttribute("Max_value" ,prop1->getMaxValue());
                xmlProperty->SetAttribute("Min_value" ,prop1->getMinValue());
            break;
                                      }
            case Property::BOOL_PROP : {
                BoolProp* prop1 = dynamic_cast<BoolProp*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::BOOL_PROP);
                xmlProperty->SetAttribute("Value", prop1->get());
                break;
                                       }
            case Property::COLOR_PROP : {
				ColorProp* prop1 = dynamic_cast<ColorProp*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::COLOR_PROP);
                xmlProperty->SetDoubleAttribute("Color_r", prop1->get().r);
                xmlProperty->SetDoubleAttribute("Color_g", prop1->get().g);
                xmlProperty->SetDoubleAttribute("Color_b", prop1->get().b);
                xmlProperty->SetDoubleAttribute("Color_a", prop1->get().a);
                break;
                                        }
            case Property::ENUM_PROP : {
				EnumProp* prop1 = dynamic_cast<EnumProp*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::ENUM_PROP);
                xmlProperty->SetAttribute("Value", prop1->get());
                break;
                                       }
            case Property::BUTTON_PROP : {
                xmlProperty->SetAttribute("Property_type", Property::BUTTON_PROP);
                break;
                                         }
            case Property::FILEDIALOG_PROP : {
				FileDialogProp* prop1 = dynamic_cast<FileDialogProp*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::FILEDIALOG_PROP);
                xmlProperty->SetAttribute("Value", prop1->get());
                xmlProperty->SetAttribute("Dialog_caption", prop1->getDialogCaption());
                xmlProperty->SetAttribute("Dialog_directory", prop1->getDirectory());
                xmlProperty->SetAttribute("Dialog_filefilter", prop1->getFileFilter());
                break;
                                             }
            case Property::TRANSFUNC_PROP : {
                 xmlProperty->SetAttribute("Property_type", Property::TRANSFUNC_PROP);
                 TransFuncProp* prop1 = dynamic_cast<TransFuncProp*>(props.at(i));//transfer->getTransferFunc();
				 TransFuncIntensityKeys* tf = dynamic_cast<TransFuncIntensityKeys*>(prop1->get() );
				 if (tf) {
					 // iterate through all markers
					 for (int m = 0; m < tf->getNumKeys(); ++m) {
						 // prepare xml
						 myStream_marker.str("");
						 myStream_marker << m;
						 TiXmlElement *xmlMarker = new TiXmlElement("Marker" + myStream_marker.str());
						 //save markers to xml
						 xmlMarker->SetDoubleAttribute("Source_value", tf->getKey(m)->getIntensity());
						 xmlMarker->SetDoubleAttribute("Dest_left_value", tf->getKey(m)->getAlphaL());
						 xmlMarker->SetAttribute("isSplit", tf->getKey(m)->isSplit());
						 xmlMarker->SetAttribute("Color_r", tf->getKey(m)->getColorL().r);
						 xmlMarker->SetAttribute("Color_g", tf->getKey(m)->getColorL().g);
						 xmlMarker->SetAttribute("Color_b", tf->getKey(m)->getColorL().b);
						 xmlMarker->SetAttribute("Color_a", tf->getKey(m)->getColorL().a);
						 if (tf->getKey(m)->isSplit()) {
							 xmlMarker->SetAttribute("Color_alt_r", tf->getKey(m)->getColorR().r);
							 xmlMarker->SetAttribute("Color_alt_g", tf->getKey(m)->getColorR().g);
							 xmlMarker->SetAttribute("Color_alt_b", tf->getKey(m)->getColorR().b);
							 xmlMarker->SetAttribute("Color_alt_a", tf->getKey(m)->getColorR().a);
							 xmlMarker->SetDoubleAttribute("Dest_right_value", tf->getKey(m)->getAlphaR());
						 }else{
							 xmlMarker->SetAttribute("Color_alt_r", 0);
							 xmlMarker->SetAttribute("Color_alt_g", 0);
							 xmlMarker->SetAttribute("Color_alt_b", 0);
							 xmlMarker->SetAttribute("Color_alt_a", 0);
							 xmlMarker->SetDoubleAttribute("Dest_right_value", 0);
						 }

						 xmlProperty->LinkEndChild(xmlMarker);
					 } // for (size_t m=0; m<transfer->getNumKeys(); ++m)
				 }
                break;
											   }
            case Property::INTEGER_VEC2_PROP : {
                IntVec2Prop* prop1 = dynamic_cast<IntVec2Prop*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::INTEGER_VEC2_PROP);
                xmlProperty->SetAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetAttribute("Vector_y", prop1->get().y);
                break;
            }
            case Property::INTEGER_VEC3_PROP : {
                IntVec3Prop* prop1 = dynamic_cast<IntVec3Prop*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::INTEGER_VEC3_PROP);
                xmlProperty->SetAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetAttribute("Vector_y", prop1->get().y);
                xmlProperty->SetAttribute("Vector_z", prop1->get().z);
                break;
            }
            case Property::INTEGER_VEC4_PROP : {
                IntVec4Prop* prop1 = dynamic_cast<IntVec4Prop*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::INTEGER_VEC4_PROP);
                xmlProperty->SetAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetAttribute("Vector_y", prop1->get().y);
                xmlProperty->SetAttribute("Vector_z", prop1->get().z);
                xmlProperty->SetAttribute("Vector_w", prop1->get().w);
                break;
            }
            case Property::FLOAT_VEC2_PROP : {
                FloatVec2Prop* prop1 = dynamic_cast<FloatVec2Prop*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::FLOAT_VEC2_PROP);
                xmlProperty->SetDoubleAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetDoubleAttribute("Vector_y", prop1->get().y);
                break;
            }
            case Property::FLOAT_VEC3_PROP : {
                FloatVec3Prop* prop1 = dynamic_cast<FloatVec3Prop*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::FLOAT_VEC3_PROP);
                xmlProperty->SetDoubleAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetDoubleAttribute("Vector_y", prop1->get().y);
                xmlProperty->SetDoubleAttribute("Vector_z", prop1->get().z);
                break;
            }
            case Property::FLOAT_VEC4_PROP : {
                FloatVec4Prop* prop1 = dynamic_cast<FloatVec4Prop*>(props.at(i));
                xmlProperty->SetAttribute("Property_type", Property::FLOAT_VEC4_PROP);
                xmlProperty->SetDoubleAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetDoubleAttribute("Vector_y", prop1->get().y);
                xmlProperty->SetDoubleAttribute("Vector_z", prop1->get().z);
                xmlProperty->SetDoubleAttribute("Vector_w", prop1->get().w);
                break;
            }
            default: break;
				
        } // switch (props_tmp[i]->getType()
        
    } // for(size_t i = 0; i < props_tmp.size(); ++i)
	return 0;
}

//--------------------------- new serializer Methods

/**
* Takes the RptNetwork an serializes it to XML
*/
void RptNetworkSerializerGui::serializeToXml(const RptNetwork& rptnet, std::string filename) {
    ProcessorNetwork net = makeProcessorNetwork(rptnet);
    // get a Serializer and serialize
    // TODO: This Serializer should be a member of the RptSerializerGui
    NetworkSerializer networkserializer = NetworkSerializer();
    networkserializer.serializeToXml(net, filename);
}

ProcessorNetwork RptNetworkSerializerGui::makeProcessorNetwork(const RptNetwork& rptnet) {
    ProcessorNetwork net = ProcessorNetwork();
    net.reuseTCTargets = rptnet.reuseTCTargets;
    // Prepare metadata
    // net.clearMeta();
    TiXmlElement* meta = new TiXmlElement("RptAggregationItems");
    // Get all the processors
    for (size_t i=0; i< rptnet.processorItems.size(); i++) {
        net.processors.push_back(rptnet.processorItems.at(i)->saveMeta().getProcessor());
        // TODO Think about where to clean meta from old stuff
        if (net.processors.back()->hasInMeta("RptAggregationItem"))
            net.processors.back()->removeFromMeta("RptAggregationItem");
    }
    // Serialize aggregarions to metadata and get contained processors
    for (size_t i=0; i< rptnet.aggregationItems.size(); i++) {
        // Serialize aggregation to metadata
        // TODO consider putting this code into the AggregationItem Class
        TiXmlElement* aggregationElement = new TiXmlElement("RptAggregationItem");
        aggregationElement->SetAttribute("id", i);
        aggregationElement->SetAttribute("name", rptnet.aggregationItems.at(i)->getName());
        aggregationElement->SetAttribute("x", rptnet.aggregationItems.at(i)->x());
        aggregationElement->SetAttribute("y", rptnet.aggregationItems.at(i)->y());
        meta->LinkEndChild(aggregationElement);
        // get processors in the aggregation
        for (size_t j=0; j<rptnet.aggregationItems.at(i)->getProcessorItems().size(); j++) {
            net.processors.push_back(rptnet.aggregationItems.at(i)->getProcessorItems().at(j)->saveMeta().getProcessor());
            // add metadata for aggregations to processors
            TiXmlElement* aggregationElement = new TiXmlElement("RptAggregationItem");
            aggregationElement->SetAttribute("id", i);
            net.processors.back()->addToMeta(aggregationElement);
        }
    }
    // Get all the Property Sets
    for (size_t i=0; i< rptnet.propertySetItems.size(); i ++) {
            net.propertySets.push_back(rptnet.propertySetItems.at(i)->saveMeta().getPropertySet());
    }
    // Add metadata
    net.addToMeta(meta);
    return net;
}

RptNetwork RptNetworkSerializerGui::makeRptNetwork(const ProcessorNetwork& net) {
    if (net.version < 2) throw AncientVersionException("This file version needs to be loaded with the old methods!");
    RptNetwork rptnet = RptNetwork();
    rptnet.reuseTCTargets = net.reuseTCTargets;
        
    //Prepare map that helps us with connections
    std::map<Processor*,RptProcessorItem*> processorMap;
    // Prepare Aggregations
    std::map< int, std::vector<RptProcessorItem*> > aggregationMap;
    // Iterate through Processors and create ProcessorItems
    for (size_t i=0; i< net.processors.size(); i++) {
        // create RptProcessorItems
        RptProcessorItem* processorItem = new RptProcessorItem(net.processors.at(i));
        processorMap[net.processors.at(i)] = processorItem;
        // read RptProcessorItem metadata
        //try {
            processorItem->loadMeta();
        //}
        //catch (XmlElementException& e) {
            // TODO log the Error
        //}
        // if Item has aggregation metadata prepare to add it to aggregation
        if (net.processors.at(i)->hasInMeta("RptAggregationItem")) {
            TiXmlElement* aggregationElem = net.processors.at(i)->getFromMeta("RptAggregationItem");
            int id;
            aggregationElem->QueryIntAttribute("id", &id);
            aggregationMap[id].push_back(processorItem);
        }
        else
            rptnet.processorItems.push_back(processorItem);
    }
    // Iterate through Processors again to create Connections between GuiItems
    for (size_t i=0; i< net.processors.size(); i++) {
        std::vector<Port*> outports = net.processors.at(i)->getOutports();
        std::vector<Port*> coprocessoroutports = net.processors.at(i)->getCoProcessorOutports();
        // append coprocessoroutports to outports because we can handle them identically FIXME is that really true?
        outports.insert(outports.end(), coprocessoroutports.begin(), coprocessoroutports.end());
        for (size_t j=0; j < outports.size(); j++) {
            // find all connections via this (out)port to (in)ports
            std::vector<Port*> connectedPorts = outports[j]->getConnected();
            for (size_t k=0; k < connectedPorts.size(); k++) {
                // get relevant data about connection
                Processor* processor = net.processors.at(i);
                Port* port = outports[j];
                std::string porttype = port->getType().getName();
                Port* connectedport = connectedPorts[k];
                Processor* connectedprocessor = connectedport->getProcessor();
                std::string connectedporttype = connectedport->getType().getName();
                // disconnect this connection
                // processor->disconnect(port, connectedport);
                // connect GuiItems (wich also reconnects the processors)
                processorMap[processor]->connectGuionly(
                    processorMap[processor]->getPortItem(porttype),
                    processorMap[connectedprocessor]->getPortItem(connectedporttype));
            }
        }
    }
        
    // create propertySetItems
    for (size_t i=0; i< net.propertySets.size(); i ++) {
        RptPropertySetItem* propertySetItem = new RptPropertySetItem(net.propertySets.at(i), processorMap);
        propertySetItem->loadMeta();
        rptnet.propertySetItems.push_back(propertySetItem);
    }
    
    // Create AggregationItems
    TiXmlElement* rptAggregationItems = net.getFromMeta("RptAggregationItems");
    TiXmlElement* aggregationElem;
    for (aggregationElem = rptAggregationItems->FirstChildElement("RptAggregationItem");
        aggregationElem;
        aggregationElem = aggregationElem->NextSiblingElement("RptAggregationItem"))
    {
        int id;
        if (aggregationElem->QueryIntAttribute("id", &id) != TIXML_SUCCESS)
            throw XmlAttributeException("Required attribute 'id' missing on aggregation"); 
        float x,y;
        if (aggregationElem->QueryFloatAttribute("x",&x) != TIXML_SUCCESS ||
            aggregationElem->QueryFloatAttribute("y",&y) != TIXML_SUCCESS)
            throw XmlAttributeException("The Position of an AggregationItem remains unknown!");
        std::string name = aggregationElem->Attribute("name") ? aggregationElem->Attribute("name") : "Aggregation";
        if (aggregationMap[id].empty())
            throw XmlElementException("Something went wrong - we have an empty aggregation!");
        RptAggregationItem* aggregationItem = new RptAggregationItem(aggregationMap[id], name);
        aggregationItem->setPos(x,y);
        rptnet.aggregationItems.push_back(aggregationItem);
    }


    return rptnet;
}

}//namespace voreen
