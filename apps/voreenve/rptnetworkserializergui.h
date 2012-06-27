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
 * Holds an equivalent of a ProcessorNetwork using RptGuiItems
 */
class RptNetwork {
public:
    RptNetwork();
    ~RptNetwork();
    static RptNetwork* load(const std::string& filename);

    void addProcessor(RptProcessorItem* processor);
    bool hasProcessor(RptProcessorItem* processor);
    
    std::vector<RptProcessorItem*> processorItems;
    std::vector<RptAggregationItem*> aggregationItems;
    std::vector<RptPropertySetItem*> propertySetItems;
    bool reuseTCTargets;
    int version; // TODO do i need a version here?
    VolumeSetContainer* volumeSetContainer;
    bool serializeVolumeSetContainer;

    std::vector<std::string> errors;
};

/**
 * Throw when detecting old Format
 */
class AncientVersionException : public SerializerException {
public:
    AncientVersionException(std::string what = "") : SerializerException(what) {}
};

    /**
* Provides methods to convert between ProcessorNetwork and RptNetwork. The actual serialization
* is a task of NetworkSerializer.
*/
    class RptNetworkSerializerGui {
    public:

        /**
        * Standard Constructor
        */
        RptNetworkSerializerGui();

        /**
        * @brief Takes the RptNetwork an serializes it to XML
        *
        * This is a convenience method. The network is converted and then given to the NetworkSerializer
        */
        void serializeToXml(RptNetwork* rptnet, std::string filename);


        // Methods for converting from Base Networks to GUI-Networks and vice versa, which handle saving and loading of Metadata

        /**
        * Creates a ProcessorNetwork from an RptNetwork. Gui metadata is stored in the respective objects.
        * WARNING! This will empty the RptNetwork and ownership of the contents will transfer to the Processornetwork
        */
        static ProcessorNetwork* makeProcessorNetwork(RptNetwork* rptnet);

        /**
        * Creates a RptNetwork from an ProcessorNetwork. Gui metadata is loaded from the respective objects.
        * WARNING! This will empty the ProcessorNetwork and ownership of the contents will transfer to the RptNetwork
        */
        static RptNetwork* makeRptNetwork(ProcessorNetwork* net);

        /**
        * Creates a RptNetwork from another RptNetwork.
        * This creates a deep clone and the original Network will not be modified
        */
        RptNetwork* duplicateRptNetwork(const RptNetwork& net);

        int readVersionFromFile(std::string filename);

    protected:

        /**
        * Finds the Version of the saved network.
        */
        int findVersion(TiXmlNode* node);
    };

} //namespace voreen

#endif //VRN_RPTNETWORKSERIALIZERGUI_H
