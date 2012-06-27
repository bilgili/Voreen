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
#include "voreen/core/vis/properties/link/propertylink.h"
#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/vis/processors/processor.h"

#include <string>
#include <vector>
#include <set>
#include <sstream>

namespace voreen {

class PropertyWidget;
class PropertyWidgetFactory;
class ProcessorNetwork;

/**
 * Base class for processor properties.
 */
class Property : public AbstractSerializable {

    friend class PropertyLink;
    friend class ProcessorNetwork;

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
    Property(const std::string& id, const std::string& guiText,
             Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT);

    virtual ~Property();

    /**
     * @brief Returns the InvalidationLevel of this property.
     *
     * @return The owner is invalidated with this InvalidationLevel upon change.
     */
    Processor::InvalidationLevel getInvalidationLevel();

    /**
     * Returns the string that is displayed in the gui.
     */
    std::string getGuiText() const;

    /**
     * Enables or disables all widgets of this property.
     */
    void setWidgetsEnabled(bool enabled);

    /**
     * Indicates whether the widgets of this property are enabled or disabled.
     */
    bool getWidgetsEnabled() const;

    /**
     * Sets the visibility of this property in the gui.
     */
    void setVisible(bool state);

    /**
     * Returns visibility state.
     */
    bool isVisible() const;

    /**
     * Returns the identifier of the property.
     */
    std::string getId() const;

    /**
     * Override this method for performing initializations
     * of the property. It is called by the owning Processor's
     * initialize() function.
     *
     * @note All OpenGL initializations must be done here,
     *       instead of the constructor! Time-consuming operations
     *       should also happen here.
     *
     * @throw VoreenException if the initialization failed
     */
    virtual void initialize() throw (VoreenException);

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    /**
     * Sets the processor this property is assigned to.
     */
    virtual void setOwner(Processor* processor);

    /**
     * Returns the processor this property is assigned to.
     */
    Processor* getOwner() const;

    /**
     * Notifies the property that its stored value has changed.
     *
     */
    void invalidate();

    /**
     * add Widget
     */
    void addWidget(PropertyWidget* widget);

    /**
     * remove Widget (does not delete it though)
     */
    void removeWidget(PropertyWidget* widget);

    /**
     * Calls updateFromProperty() on the widgets
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
     * Creates a Widget for this Property, sets enabled+visible and
     * adds it to the Property.
     */
    PropertyWidget* createAndAddWidget(PropertyWidgetFactory* f);

    /**
     * Returns all property widgets of this property.
     */
    const std::set<PropertyWidget*> getPropertyWidgets() const;

    LODSetting getLevelOfDetail();
    void setLevelOfDetail(LODSetting lod);

	//only needed for processor state (volume caching):
    virtual std::string toString() const = 0;

    template<typename T>
    static std::string valueToString(const T& value) {
        std::stringstream oss;
        oss << value;
        return oss.str();
    }
	//-------------------------------------------------

    /**
     * Returns the property links currently registered
     * at the property.
     */
    const std::vector<PropertyLink*>& getLinks() const;

    /**
     *
     * Switch interactionmode on or off.
     *
     * @param interactionMode
     * @param source The source (usually a GUI element) that has issued the interaction mode.
     */
    void toggleInteractionMode(bool interactionmode, void* source);

    /** 
     * Returns the meta data container of this processor.
     * External objects, such as GUI widgets, can use it
     * to store and retrieve persistent meta data without 
     * having to bother with the serialization themselves.
     *
     * @see MetaDataContainer
     */
    MetaDataContainer& getMetaDataContainer() const;

 protected:
    /**
     * Invalidates the owner with the InvalidationLevel set in the constructor
     */
    void invalidateOwner();

    /**
     * Invalidates the owner with a given InvalidationLevel.
     *
     * @param invalidationLevel Use this InvalidationLevel to invalidate
     */
    void invalidateOwner(Processor::InvalidationLevel invalidationLevel);

    std::string id_;
    std::string guiText_;

    Processor* owner_;
    Processor::InvalidationLevel invalidationLevel_;
    bool widgetsEnabled_;
    bool visible_;
    LODSetting lod_;

    std::set<PropertyWidget*> widgets_;
    std::vector<PropertyLink*> links_;

private:
    /**
     * Adds the passed property link to the property.
     */
    void registerLink(PropertyLink* link);

    /**
     * Removes the passed link from the property, but does not delete it.
     */
    void removeLink(PropertyLink* link);
    
    /// Used for cycle prevention during interaction mode propagation
    bool interactionModeVisited_;

    /**
     * Contains the associated meta data.
     * 
     * We want to return a non-const reference to it from a const member function 
     * and since the MetaDataContainer does not affect the processor itself,
     * mutable appears justifiable.
     */
    mutable MetaDataContainer metaDataContainer_;
};

typedef std::vector<Property*> Properties;

} // namespace voreen

#endif // VRN_PROPERTY_H
