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

#ifndef VRN_RPTNETWORKSERIALIZERGUI_H
#define VRN_RPTNETWORKSERIALIZERGUI_H

#include "voreen/core/vis/processors/networkserializer.h"
#include "voreen/core/vis/processors/propertyset.h"
#include "rptprocessoritem.h"
#include "rptaggregationitem.h"
#include "rptpropertysetitem.h"
#include "tinyxml/tinyxml.h"

namespace voreen {

/**
* This struct is used when loading propertysets from xml. It stores the name,
* position, and connected processors of a propertyset.
*/
struct PropertySetInfos {
    std::string name;
    int xpos;
    int ypos;
    std::vector<RptProcessorItem*> processorItems;
};

/**
* This class will replace Networkinfos
* (and finally two (Serializers) become one (Serializer))
*/

class RptNetwork {

public:
    RptNetwork() {}
    //RptNetwork(const ProcessorNetwork& net); // RptSerializer has this functionality
    //~RptNetwork();
    //ProcessorNetwork makeProcessorNetwork() const; // RptSerializer has this functionality
    //TODO: Maybe the Serializer should be a singleton - then I could add this functionality through the Serializer for convenience

    std::vector<RptProcessorItem*> processorItems;
    std::vector<RptAggregationItem*> aggregationItems;
    std::vector<RptPropertySetItem*> propertySetItems;
    bool reuseTCTargets;
    int version; // TODO do i need a version here?
};

    /**
     * Throw when detecting old Format
     */
    class AncientVersionException : public SerializerException {
    public:
        AncientVersionException(std::string what = "") : SerializerException(what) {}
        //NetworkInfos* netinfos;
    };


/**
* This struct is used when loading a network from xml. It stores all the ProcessorItems 
* (and thereby the processors), all the information about propertysets and the information about
* aggregations. This is a bit suboptimal, the aggregations could be done nicer. 
*/
struct NetworkInfos {
    std::vector<RptProcessorItem*> processorItems;
    std::vector<PropertySetInfos*> propertySetInfos;
    std::map<int,std::vector<RptProcessorItem*>*> aggroMap;  //every aggregation gets an id, and 
    //a vector with the ProcessorItems in the
    //Aggregation
    bool reuseTCTargets;
    std::map<int,std::string> aggroNameMap;					   //Every aggregation represented by its id gets
															   //a name. We could and should use 
															   //an extra struct for the 
															   //aggregation info in my opinion.
    int version;                //the version of this network. Up to now there can be 2 different
                                //versions. One with version number 0, and one with version number 1(28.04.08)
};

/**
* This class serializes all the objects in rptgui (Processor, Aggregations and Propertysets) to an XML file. 
* It can also read an XML file and build the data structure again. This class is differerent from 
* NetworkSerializer and doesn't inherit from it, because the methods involved when reading XML files 
* don't even have the same return type. One could workaround that of course, but this way the normal
* NetworkSerializer is really easy to use. 
*/
class RptNetworkSerializerGui {
public:
	
	/**
	* Standard Constructor
	*/
	RptNetworkSerializerGui();
	
	/**
	*Takes the RptProcessorItem vector and aggregation vector and PropertySetGuiItem vectir and serializes them to xml
	*/  
	int serializeToXml(std::vector<RptProcessorItem*> processorItems, std::vector<RptAggregationItem*> aggros, std::vector<RptPropertySetItem*> propertySetGuiItems, bool reuseTCTargets, std::string filename);

    /**
    * Takes the RptNetwork an serializes it to XML
    */
    void serializeToXml(const RptNetwork& rptnet, std::string filename);

    /**
     * Methods for converting from Base Networks to GUI-Networks and vice versa, which handle saving and loading of Metadata
     */
    ProcessorNetwork makeProcessorNetwork(const RptNetwork& rptnet);
    RptNetwork makeRptNetwork(const ProcessorNetwork& net);


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
	NetworkInfos* readNetworkFromFile(std::string filename);
    int readVersionFromFile(std::string filename);

	/**
	* Uses the information stored in graphConnectionInformation_ to connect the objects in
	* the right way. You can call this function after having read a network from file AND after adding
	* the read objects to the GraphicsScene. 
	*/
	int connectGuiItems();
	int connectGuiItemsVersion1();

protected:

	
	/**
	* Saves the property vector of one processor to xml. (shamelessely copied and modified from widgetgenerator)
	*/
	int saveProperties(std::vector<Property*> props, TiXmlElement* xmlParent);

    /**
    * Finds the Version of the saved network.
    */
    int findVersion(TiXmlNode* node);
	
	/**
	* Loads the property vector of one processor from xml. (shamelessely copied and modified from widgetgenerator)
	*/
	int loadProperties(std::vector<Property* > props, TiXmlElement* xmlParent, Processor* processor);
	
	/**
	* Serializes one RptProcessorItem to xml.
	* @param xmlParent A TiXmlElement object holding the file and the location to write to
	* @param id The id the ProcessorItem has been given
	* @param hasAggro Is this ProcessorItem part of an aggregation?
	* @param aggroNumber The number of that aggregation (determined in serializeToXml(..) )
	* @param aggroName The name of that aggregation
	*/ 
	int serializeGuiItem(RptProcessorItem* guiItem,TiXmlElement* xmlParent, int id,bool hasAggro,int aggroNumber,std::string aggroName);
	int serializeGuiItemVersion1(RptProcessorItem* guiItem,TiXmlElement* xmlParent, int id,bool hasAggro,int aggroNumber,std::string aggroName);

	/**
	* Serializes one PropertySetGuiItem to xml
	* @param xmlParent A TiXmlElement object holding the file and the location to write to
	* @param id The id of the PropertySetGuiItem (determined in serializeToXml(...) )
	*/
	void serializePropertySet(RptPropertySetItem* propSetGuiItem, TiXmlElement* xmlParent, int id);
	

	/**
	* Reads an xml file and creates all ProcessorItems. Furthermore it gathers all the necessary information
	* about PropertySets and Aggregations. This method is called by readNetworkFromFile(..). 
	*/
	int readProcessorsFromXml(TiXmlNode* node);
	int readProcessorsFromXmlVersion1(TiXmlNode* node);

	/**
	* Finds the port number of a port in a processor. This only works for INPORTS. Every processor has a vector of inports,
	* and this function searches the given Port in that vector, and returns its place in the vector. So if the Port is the
	* third port in the vector, it returns 2.
	*/ 
	int findPortNumber(Processor* processor,Port* p);
	
	/**
	* Same as findPortNumber, just for CoProcessorInports.
	*/ 
	int findCoProcessorPortNumber(Processor* processor,Port* p);

	/**
	*Returns the RptProcessorItem with the given id if it is found in processorItems
	*/
	RptProcessorItem* findGuiItem(int id);
	
	/**
	* This function inserts the ProcessorItem with the aggroNumber into the aggroMap_. If the map already has a key
	* equal to aggroNumber the guiItem is inserted into its vector, otherwise a key is created and the vector initialised with
	* the guiItem. 
	*/
	void insertIntoAggroMap(int aggroNumber, RptProcessorItem* guiItem);

    /** stores the default filename extension (e.g. ".vnw")
     */
    std::string defFileExt_;

	/**
	*Stores the connections between ProcessorItems. This is neccessary because you cant store pointers
	*in the xml files, only the ids of the ProcessorItems. So we have to create all the ProcessorItems before we can
	*connect them. That's why we have to store the connection info somewhere until they are created.
	*/
	std::vector<ConnectionInfoForOneProcessor*> graphConnectionInformation_;

	/**
	*This vector holds all the RptProcessorItems created from an Xml file. 
	*/
	std::vector<RptProcessorItem*> processorItems;

	/**
	*Used when creating the RptProcessorItemn objects from a Xml file. It counts the amount of objects created.
	*/
	int count_;

	/**
	* This map stores and id for every processor. Every time a processor is read from xml, it is given
	* an id which is stored here. 
	*/
	std::map<Processor*,int> idMap_;

	/**
	* Every aggregation has a key in this map, and a corresponding vector of ProcessorItems. The key is the
	* id of the aggregation, and the vector contains all the ProcessorItems in it.
	*/

	std::map<int,std::vector<RptProcessorItem*>*> aggroMap_;

	/**
	* Every aggregation has a key int his map, and a sting as value. The key is the id of the aggregation, and the sting
	* its name
	*/
	std::map<int,std::string> aggroNameMap_;

	/**
	* This vector contains propertySetInfos pointers, each holding all the necessary information for one PropertySet
	*/
	std::vector<PropertySetInfos*> propertySetInfos_;

};

} //namespace voreen

#endif //VRN_RPTNETWORKSERIALIZERGUI_H
