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

#include "voreen/core/properties/propertyowner.h"
#include "voreen/core/properties/property.h"

namespace voreen {

const std::string PropertyOwner::loggerCat_("voreen.PropertyOwner");

PropertyOwner::PropertyOwner()
    : invalidationLevel_(50)
{}

PropertyOwner::~PropertyOwner() {
}

void PropertyOwner::addProperty(Property* prop) {
    tgtAssert(prop, "Null pointer passed");
    if (getPropertyByID(prop->getID())) {
        LERROR(getName() << ": Duplicate property id '" << prop->getID() << "'");
    }
    properties_.push_back(prop);
    prop->setOwner(this);
}

void PropertyOwner::addProperty(Property& prop) {
    addProperty(&prop);
}

const Properties& PropertyOwner::getProperties() const {
    return properties_;
}

Property* PropertyOwner::getPropertyByID(const std::string& id) const {
    for (size_t i = 0; i < properties_.size(); ++i) {
        if (properties_[i]->getID() == id)
            return properties_[i];
    }
    return 0;
}

Property* PropertyOwner::getPropertyByName(const std::string& name) const {
    for (size_t i = 0; i < properties_.size(); ++i) {
        if (properties_[i]->getGuiName() == name)
            return properties_[i];
    }
    return 0;
}

int PropertyOwner::getInvalidationLevel() const {
    return invalidationLevel_;
}

void PropertyOwner::invalidate(int inv) {
    invalidationLevel_ = std::max(invalidationLevel_, inv);
}

bool PropertyOwner::interactionMode() const {
    return (!interactionModeSources_.empty());
}

void PropertyOwner::interactionModeToggled() {
}

void PropertyOwner::toggleInteractionMode(bool interactionMode, void* source) {
    if (interactionMode) {
        if (interactionModeSources_.find(source) == interactionModeSources_.end()) {
            interactionModeSources_.insert(source);

            if (interactionModeSources_.size() == 1)
                interactionModeToggled();
        }
    }
    else {
        if (interactionModeSources_.find(source) != interactionModeSources_.end()) {
            interactionModeSources_.erase(source);

            if (interactionModeSources_.empty())
                interactionModeToggled();
        }
    }
}

bool PropertyOwner::isValid() const {
    return (getInvalidationLevel() == 0);
}

void PropertyOwner::setValid() {
    invalidationLevel_ = 0;
}

void PropertyOwner::serialize(XmlSerializer& s) const {
    // create temporary property map for serialization
    std::map<std::string, Property*> propertyMap;
    for (std::vector<Property*>::const_iterator it = properties_.begin(); it != properties_.end(); ++it)
        propertyMap[(*it)->getID()] = *it;

    // serialize properties
    const bool usePointerContentSerialization = s.getUsePointerContentSerialization();
    s.setUsePointerContentSerialization(true);
    try {
        s.serialize("Properties", propertyMap, "Property", "name");
    }
    catch (SerializationException& e) {
        LWARNING(e.what());
    }
    s.setUsePointerContentSerialization(usePointerContentSerialization);
}

void PropertyOwner::deserialize(XmlDeserializer& s) {
    // create temporary property map for deserialization
    std::map<std::string, Property*> propertyMap;
    for (std::vector<Property*>::const_iterator it = properties_.begin(); it != properties_.end(); ++it)
        propertyMap[(*it)->getID()] = *it;

    // deserialize properties
    const bool usePointerContentSerialization = s.getUsePointerContentSerialization();
    s.setUsePointerContentSerialization(true);
    try {
        s.deserialize("Properties", propertyMap, "Property", "name");
    }
    catch (SerializationException& e) {
        LWARNING(e.what());
    }
    s.setUsePointerContentSerialization(usePointerContentSerialization);
}

} // namespace voreen
