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

#ifndef VRN_PROPERTYSET_H
#define VRN_PROPERTYSET_H

#include "voreen/core/vis/processors/render/volumeraycaster.h"

namespace voreen {

class Processor;


/**
 * This class has a vector of Processors. The properties of PropertySet consist
 * of the intersection of the properties from the Processor vector.
 * Changing the properties of the set causes the properties of the Processors to change accordingly.
 *
 * To handle transfer functions too, it inherits from VolumeRenderer.
 * TODO: this should be changed somehow. joerg
 */
class PropertySet : public VolumeRaycaster {
public:
    /**
     * Constructor.
     * @param equalize if true the processors properties will be adjusted when they are added
     */
    PropertySet(bool equalize = true);
    /**
     * Constructor.
     * @param processors the processors to share their properties
     * @param equalize if true the processors properties will be adjusted when they are added
     */
    PropertySet(std::vector<Processor*> processors, bool equalize = true);


    virtual const Identifier getClassName() const {return "Miscellaneous.Propertyset";}

    /**
     * Set the processors for this property set.
     */
    void setProcessors(std::vector<Processor*> processors);

    /**
     * Add a processor to the Processor vector.
     */
    void addProcessor(Processor* processor);

    /**
     * Remove a Processor from the Processor vector.
     */
    bool removeProcessor(Processor* processor);

    /**
     * Clear the processor vector.
     */
    void clear();

    /**
     * Returns the processor-vector.
     */
    std::vector<Processor*> getProcessors() const { return processors_; }

    /*
     * Pure virtual function from superclass.
     */
    virtual const Identifier getClassName() {return "PropertySet.PropertySet";}
    virtual Processor* create() const {return new PropertySet;}
    void process(LocalPortMapping* /*localPortMapping*/) {}


    /**
     * Sets the properties from the set in the contained processors (id = "all" for setting all properties)
     */
    void setProperties(Identifier id);

    virtual void processMessage(Message* msg, const Identifier& dest);

    /**
     * Returns the volumeHandle that is used in this propertyset
     */
    VolumeHandle* getVolumeHandle();

    /**
     * Returns the lower threshold of the volumeRenderer in this propertyset
     */
    float getLowerThreshold() const;

    /**
     * Returns the upper threshold of the volumeRenderer in this propertyset
     */
    float getUpperThreshold() const;

   /**
    * serializes the PropertySet to xml
    */
    virtual TiXmlElement* serializeToXml() const;
    virtual TiXmlElement* serializeToXml(const std::map<Processor*, int> idMap) const;

    /**
    * Updates the PropertySet from xml
    */
    virtual void updateFromXml(TiXmlElement* propertysetElem);
    virtual void updateFromXml(TiXmlElement* propertysetElem, const std::map<int, Processor*> idMap);

    void addToMeta(TiXmlElement* elem) { meta_.addData(elem); }
    void removeFromMeta(std::string elemName) { meta_.removeData(elemName); }
    void clearMeta() { meta_.clearData(); }
    TiXmlElement* getFromMeta(std::string elemName) { return meta_.getData(elemName); }
    std::vector<TiXmlElement*> getAllFromMeta() { return meta_.getAllData(); }

    static const std::string XmlElementName_;

    /**
     * Returns a static PropertySet, that can be used for temporary property sets,
     * e.g. when two processors are selected in RptGui, the PropertyListWidget shows their intersection.
     */
    static PropertySet* getTmpPropSet();

    virtual std::string getXmlElementName() const;

private:
    // properties get created depending on the contained processors
    void createProperties();

    std::vector<Processor*> processors_;
    bool equalize_;

    static PropertySet* tmpPropSet_;

    MetaSerializer meta_;
};

}

#endif //VRN_PROPERTYSET_H
