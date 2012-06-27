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

#ifndef VRN_NETWORKSERIALIZER_H
#define VRN_NETWORKSERIALIZER_H

#include "tinyxml/tinyxml.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/propertyset.h"
#include "voreen/core/xml/serializable.h"

namespace voreen {

// Information about the connections of a network. Elements have the form
// (ProcessorId, PortIdentifier) -> (ProcessorId, PortIdentifier)
//typedef std::multimap< std::pair<int,std::string>, std::pair<int,std::string> > ConnectionMap;

/**
* This class will replace the NetworkInformation and Networkinfos
* (and finally two (Serializers) become one (Serializer))
*/

class ProcessorNetwork : public Serializable {

public:
    ProcessorNetwork();
    ~ProcessorNetwork();

    /**
    * Returns the name of the xml element uses when serializing the object
    */
    virtual std::string getXmlElementName() const { return "ProcessorNetwork"; }

    /**
    * Serializes the Network to XML.
    */
    virtual TiXmlElement* serializeToXml() const;
    
    /**
    * Updates the Network from XML.
    */
    virtual void updateFromXml(TiXmlElement* elem);

    void addToMeta(TiXmlElement* elem) { meta_.addData(elem); }
    void removeFromMeta(std::string elemName) { meta_.removeData(elemName); }
    void clearMeta() { meta_.clearData(); }
    TiXmlElement* getFromMeta(std::string elemName) const { return meta_.getData(elemName); }
    bool hasInMeta(std::string elemName) const { return meta_.hasData(elemName); }


    /*
    * Set all processors of the network
    */
    ProcessorNetwork& setTextureContainer(TextureContainer* tc);
    ProcessorNetwork& setCamera(tgt::Camera* camera);
    
    std::vector<Processor*> processors;
    std::vector<PropertySet*> propertySets;
    bool reuseTCTargets;
    int version;
    
    std::vector<SerializerException> errors;
    
    /* The ProcessorNetwork can also store XML Metadata via Serializable
    which may be used to store information about i.e. aggregations */
private:
    MetaSerializer meta_;
};

/**
* This struct saves information about a network loaded by the networkserializer. It contains all the processors
* in the network, and all the propertysets.
*/

struct NetworkInformation {
	std::vector<Processor*> processors;
	std::vector<PropertySet*> propertySets;
    int version;
};

struct ConnectedProcessor {
	int processorNumber;
	Identifier portType;
};

/**
* This struct contains the information about one port. It is used to save and load the connections between
* processors. You can't store pointers in xml, so you have to give ports and processors numbers. Something like
* "outport 1 of processor 3 is connected to inport 5 of processor 7". 
*/ 
struct PortConnection {
	/**
	* The port number is the position this port has in the in (or) outports vector of its processor.
	* So if a processor has 3 outports, the first one will have number 0 and so on. 
	*/
	int portNumber;
	/**
	* The type of this port, image.entrypoints for example.
	*/ 
	Identifier type;
	/**
	* This map stores the connections this port has to other processors. An entry in this map consists of the
	* number of the processor and the number of the port that are connected to it. Like "this port is connected
	* to port 2 of processor 4". First value is the number of the processor, the second one the number for the 
	* port. 
	*/
	std::map<int,int> connectedProcessors;
	std::map<int,Identifier> connectedProcessorsMap;
	std::vector<ConnectedProcessor> connectedProcessorsVector;
};

/**
* Thist struct holds the entire connection information for one Processor. Only the output connections
* are relevant, because all the connect methods in processor automatically create the inport 
* connections when creating an output connection. 
*/
struct ConnectionInfoForOneProcessor {
	/**
	* You can't store pointers in xml, so you have to give each processor a number. This is the id. 
	*/ 
	int id;
	/**
	* Vector containing all the output connections of the processor. For every port there is a PortConnection
	* in the vector. 
	*/ 
	std::vector<PortConnection*> outputs;
	/**
	* Vector containing all the input connections of the processor. For every port there is a PortConnection
	* in the vector. 
	*/ 
	std::vector<PortConnection*> inputs;
};

/**
* This class serializes the data structure of processors to an XML file (currently not working). It can also
* read an XML file and build the data structure again. (working)
*/
class NetworkSerializer {
public:
	/**
	* Standard Constructor
	*/
	NetworkSerializer();

    ~NetworkSerializer();

    /**
    * serializes a ProcessorNetwork to xml
    */
    void serializeToXml(const ProcessorNetwork& network, std::string filename);
	
	/**
	* Reads a .snvf (saved voreen network file) and creates the processors and propertysets saved in it.
	* NetworkInformation is a struct that stores these objects
	*/ 
	ProcessorNetwork readNetworkFromFile(std::string filename);
	int readVersionFromFile(std::string filename);

protected:
    /**
    * Finds the Version of the saved network.
    */
    int findVersion(TiXmlNode* node);

	/**
	* Uses the information stored in graphConnectionInformation_ to connect the Processor objects in
	* the right way. This is needed because when reading and creating the processors from the xml file, some processors might 
	* not exist yet. That means that we first have to create all the processors, and after that we can connect them. This is
	* what this function does. 
	*/
    int connectProcessors();
    int connectProcessorsVersion1();
	
	/**
	* Saves the property vector of one processor to xml. (shamelessely copied and modified from widgetgenerator)
	*/
	//int saveProperties(std::vector<Property*> props, TiXmlElement* xmlParent);
	
	/**
	* Loads the property vector of one processor from xml. (shamelessely copied and modified from widgetgenerator)
	*/
	int loadProperties(std::vector<Property* > props, TiXmlElement* xmlParent, Processor* processor);
	
	/**
	* Serializes one processor to xml.
	*/ 
	int serializeProcessor(Processor* processor,TiXmlElement* xmlParent, int id);

	/**
	* Reads an xml file and creates all processors and propertysets stored in it. 
	* These Processors are however not connected yet. That has to be done with connectProcessors()
	*/
	int readProcessorsFromXml(TiXmlNode* node);
    int readProcessorsFromXmlVersion1(TiXmlNode* node);

	/**
	* Returns the Processor with the given id if it is found in processors_
	*/
	Processor* findProcessor(int id);
	
	/**
	* Stores which processor is connected to which processor. This is neccessary because you can't store pointers
	* in the xml files, only the ids of the processors. So we have to create all the processors before we can
	* connect them. That's why we have to store the connection info somewhere until they are created.
	*/
	std::vector<ConnectionInfoForOneProcessor*> graphConnectionInformation_;

	/**
	* This vector holds all the processors created from an xml file. 
	*/
	std::vector<Processor*> processors_;

	/**
	* This vector holds all the propertysets created from an xml file.
	*/
	std::vector<PropertySet*> propertySets_;

	/**
	* Used when creating the processor objects from a xml file. It counts the amount of processors created.
	*/
	int count_;

	/**
	* This map stores and id for every processor. Every time a processor is read from xml, it is given
	* an id which is stored here. 
	*/
	std::map<Processor*,int> idMap_;

    std::string defFileExt_;
};

} //namespace voreen

#endif
