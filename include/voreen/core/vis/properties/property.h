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

#ifndef VRN_PROPERTY_H
#define VRN_PROPERTY_H

#include "tgt/vector.h"
#include "voreen/core/vis/identifier.h"
#include "voreen/core/xml/serializable.h"
#include "tinyxml/tinyxml.h"

#include <string>
#include <vector>
#include <set>
#include <sstream>

namespace voreen {

class Processor;
class PropertyWidget;
class PropertyWidgetFactory;

/**
 * Base class for properties in the messaging sytem. All data sent with messages is
 * encapsulated in properties.
 * TODO: remove messaging system leftovers
 */
class Property : public Serializable {
public:
    /**
     * Every property has a level of detail setting which determines, whether the property
     * should be visible in a network editing mode (=DEVELOPER) or in a user environment.
     */
    enum LODSetting {
        USER = 0,
        DEVELOPER = 1
    };

    /**
     * Constructor - sets standard-values
     */
    Property(const std::string& id, const std::string& guiText);

    virtual ~Property();

    /**
     * Returns the string that is displayed in the gui.
     */
    std::string getGuiText() const;

    void setVisible(bool state);

    /**
     * Returns visibility state.
     */
    bool isVisible() const;

    /**
     * Returns the associated identifier of the property
     */
    Identifier getIdent() const;

    const static Identifier getIdent(TiXmlElement* propertyElem);

    std::string getId() const;

    /**
    * Returns the name of the xml element uses when serializing the object
    */
    virtual std::string getXmlElementName() const;

    /**
     * Serializes the property to XML. Derived classes should implement their own version.
     */
    virtual TiXmlElement* serializeToXml() const;

    /**
     * Updates the property from XML. Derived classes should implement their own version.
     */
    virtual void updateFromXml(TiXmlElement* propElem);

    virtual void setOwner(Processor* processor);

    Processor* getOwner() const;

    /**
     * add Widget
     */
    void addWidget(PropertyWidget* widget);

    /**
     * remove Widget (does not delete it though)
     */
    void removeWidget(PropertyWidget* widget);

    /**
     * calls update() on the widgets
     */
    void updateWidgets();

    /**
     * calls disconnect() on the widgets
     */
    void disconnectWidgets();

    /**
     * calls setVisible(state) on the widgets
     */
    void setWidgetsVisible(bool state);

    /**
     * Creates a Widget for this Property, but
     * Does NOT add it to the Property. The given PropertyWidgetFactory will actually
     * build the Widget.
     */
    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);

    /**
     * Creates a Widget for this Property, and
     * adds it to the Property
     */
    PropertyWidget* createAndAddWidget(PropertyWidgetFactory* f);

    std::set<PropertyWidget*> getPropertyWidgets() const;

    /**
     * Returns a TiXmlElement containing the serialized meta data.
     * May be null.
     */
    const TiXmlElement* getMetaData() const;

    LODSetting getLevelOfDetail();
    void setLevelOfDetail(LODSetting lod);

    virtual std::string toString() const = 0;

    template<typename T>
    static std::string valueToString(const T& value) {
        std::stringstream oss;
        oss << value;
        return oss.str();
    }

    static const std::string XmlElementName_;

    /** 
     * Returns whether property type information is to be
     * serialized.
     */
    static bool getSerializeTypeInformation();

    /**
     * Determines whether property type information is to be
     * serialized (e.g. the property's class name, GUI label,
     * min/max values).
     */
    static void setSerializeTypeInformation(bool enable);

 protected:
    std::string id_;
    std::string guiText_;

    Processor* owner_;
    bool visible_;
    LODSetting lod_;

    std::set<PropertyWidget*> widgets_;

private:
    static bool serializeTypeInformation_; 
    TiXmlElement* metaData_;

};

typedef std::vector<Property*> Properties;

} // namespace voreen

#endif // VRN_PROPERTY_H
