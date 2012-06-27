/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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
#include "voreen/core/properties/link/propertylink.h"
#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/processors/processor.h"

#include <string>
#include <vector>
#include <set>
#include <sstream>

namespace voreen {

class PropetyOwner;
class PropertyWidget;
class PropertyWidgetFactory;
class ProcessorNetwork;

/**
 * Base class for processor properties.
 */
class Property : public AbstractSerializable {

    friend class PropertyLink;
    friend class PropertyVector;
    friend class Processor;
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
     *
     * @param property's identifier: Must be not empty
     *        and must be unique among all properties of a PropertyOwner
     * @param guiName textual representation of the property in the GUI
     * @param invalidationLevel the level the owner is invalidated with upon change
     */
    Property(const std::string& id, const std::string& guiName,
             Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT);

    virtual ~Property();

    /**
     * @brief Returns the InvalidationLevel of this property.
     *
     * @return The owner is invalidated with this InvalidationLevel upon change.
     */
    Processor::InvalidationLevel getInvalidationLevel();

    /**
     * Returns the identifier of the property.
     */
    std::string getID() const;

    /**
     * Returns the identifier of the property preceeded by
     * its owner's name, e.g. "Background.color1".
     *
     * If the property is not assigned to an owner,
     * only its id is returned.
     *
     * @see PropertyOwner::getName
     */
    std::string getFullyQualifiedID() const;

    /**
     * Returns the string that is to be displayed
     * in the user interface.
     */
    std::string getGuiName() const;

    /**
     * Returns the GUI name of the property preceeded by
     * its owner's name, e.g. "Background.First Color".
     *
     * If the property is not assigned to an owner,
     * only its GUI name is returned.
     *
     * @see PropertyOwner::getName
     * @see getGuiName
     */
    std::string getFullyQualifiedGuiName() const;

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
    virtual void setOwner(PropertyOwner* owner);

    /**
     * Returns the processor this property is assigned to.
     */
    PropertyOwner* getOwner() const;

    /**
     * Notifies the property that its stored value has changed.
     */
    virtual void invalidate();

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

    //-------------------------------------------------

    /**
     * Returns the property links currently registered
     * at the property.
     */
    const std::vector<PropertyLink*>& getLinks() const;

    /**
     * Returns the property link from this property to the destination property,
     * or null if no link is established between them.
     */
    PropertyLink* getLink(const Property* dest) const;

    /**
     * Returns whether a link from this property to the destination
     * property is established.
     *
     * @param dest the destination property whose link state is to be checked
     * @param transitive if set to true, not only direct links between source
     *      and destination are considered, but also indirect ones over multiple
     *      intermediate hops.
     *
     * @return true, if the dest property is reachable over links or if
     *      this == dest
     */
    bool isLinkedWith(const Property* dest, bool transitive = false) const;

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

    /**
     * Returns a textual description of the type of the given property.
     */
    static std::string getPropertyTypeText(const Property* prop);

 protected:

    /**
     * Override this method for performing initializations
     * of the property. It is usually called by the owning Processor's
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
     * Override this method for performing deinitializations
     * of the property.
     *
     * @note All OpenGL deinitializations must be done here,
     *       instead of the destructor!
     *
     * @throw VoreenException if the deinitialization failed
     */
    virtual void deinitialize() throw (VoreenException);

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
    std::string guiName_;

    PropertyOwner* owner_;
    Processor::InvalidationLevel invalidationLevel_;
    bool widgetsEnabled_;
    bool visible_;
    LODSetting lod_;

    std::set<PropertyWidget*> widgets_;
    std::vector<PropertyLink*> links_;

private:
    /**
     * Adds the passed property link to the property.
     * Is called by the owning ProcessorNetwork and by
     * during the PropertyLink object.
     */
    void registerLink(PropertyLink* link);

    /**
     * Removes the passed link from the property, but does not delete it.
     * Is called by the PropertyLink's destructor.
     */
    void removeLink(PropertyLink* link);

    /// Used for cycle prevention during interaction mode propagation
    bool interactionModeVisited_;

    /// Used for cycle prevention during check whether two props are linked
    mutable bool linkCheckVisited_;

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
