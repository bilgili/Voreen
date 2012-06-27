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

#ifndef VRN_PROPERTYOWNER_H
#define VRN_PROPERTYOWNER_H

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

class Property;

class PropertyOwner : public AbstractSerializable {

    friend class Property;

public:
    PropertyOwner();
    virtual ~PropertyOwner();

    /**
     * Returns the name of the PropertyOwner instance.
     * This is supposed to be presentable to the user.
     */
    virtual std::string getName() const = 0;

    /**
     * Returns the properties assigned to this owner.
     */
    const std::vector<Property*>& getProperties() const;

    /**
     * Returns the property with the given id. If no such property was found,
     * a null-pointer is returned.
     */
    Property* getPropertyByID(const std::string& id) const;

    /**
     * Returns the property with the given GuiName. If multiple properties have the same GuiName,
     * the first one is returned. If no such property was found, a null-pointer is returned.
     */
    Property* getPropertyByName(const std::string& name) const;

    /**
     * Returns all properties matching the specified type T,
     * including subtypes.
     */
    template<class T>
    std::vector<T*> getPropertiesByType() const;

    /**
     * The new InvalidationLevel is max(inv, currentInvalidationLevel).
     * Use setValid() to mark the owner as valid.
     *
     * @note the default invalidation level 1 corresponds to Processor::INVALID_RESULT
     */
    virtual void invalidate(int inv = 1);

    /**
     * Returns true, if property is in interaction mode, false otherwise.
     *
     * @see Processor::toggleInteractionMode
     */
    virtual bool interactionMode() const;

    /**
     * Returns the property owner's current invalidation level.
     */
    int getInvalidationLevel() const;

    /**
     * Marks the property owner as valid by setting its invalidation level to 0.
     */
    virtual void setValid();

    /**
     * The standard implementation returns true, if getInvalidationLevel() == 0.
     * Override it for custom behavior.
     */
    virtual bool isValid() const;

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

protected:
    /**
     * Registers a property at the owner.
     */
    virtual void addProperty(Property* prop);

    /// \overload
    virtual void addProperty(Property& prop);

    /**
     * This method is called if the owner is switched into or out of interaction mode.
     *
     * Overwrite this method if a owner needs to react
     * The default implementation does nothing.
     */
    virtual void interactionModeToggled();

    /**
     * Call this method if the owner should be switched into or out of interaction mode.
     *
     * @param interactionMode
     * @param source The source (usually a propertywidget).
     */
    virtual void toggleInteractionMode(bool interactionMode, void* source);

    int invalidationLevel_;
    std::set<void*> interactionModeSources_;

    /// category used in logging
    static const std::string loggerCat_;

private:
    /// Stores the owner's properties.
    std::vector<Property*> properties_;

};

//---------------------------------------------------------------------------
// template definitions

template<class T>
std::vector<T*> PropertyOwner::getPropertiesByType() const {
    std::vector<T*> result;
    for (size_t i=0; i<properties_.size(); ++i) {
        if (dynamic_cast<T*>(properties_[i]))
            result.push_back(dynamic_cast<T*>(properties_[i]));
    }
    return result;
}

} // namespace voreen

#endif // VRN_PROPERTYOWNER_H
