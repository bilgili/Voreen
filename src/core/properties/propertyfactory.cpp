/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/core/properties/propertyfactory.h"

#include "voreen/core/properties/property.h"

namespace voreen {

const std::string PropertyFactory::loggerCat_("voreen.PropertyFactory");

PropertyFactory::PropertyFactory() {
}

PropertyFactory::~PropertyFactory() {
}

Serializable* PropertyFactory::createType(const std::string& typeString) {

    std::map<std::string, const Property*>::iterator it = classNameMap_.find(typeString);
    if (it != classNameMap_.end()) {
        tgtAssert(it->second, "mapped value is null");
        Property* prop = it->second->create();
        tgtAssert(prop, "no property created");
        return prop;
    }
    else {
        return 0;
    }
}

const std::string PropertyFactory::getTypeString(const std::type_info& type) const {
    for (std::map<std::string, const Property*>::const_iterator it = classNameMap_.begin();
         it != classNameMap_.end(); ++it)
    {
        if (type == typeid(*(it->second)))
            return it->first;
    }

    return "";
}

void PropertyFactory::registerProperty(const Property* property) {
    tgtAssert(property, "null pointer passed");

    if (classNameMap_.find(property->getClassName()) == classNameMap_.end()) {
        properties_.push_back(property);
        classNameMap_.insert(std::make_pair(property->getClassName(), property));
    }
    else {
        LWARNING("Property class '" << property->getClassName() << "' has already been registered. Skipping.");
    }
}

const std::vector<const Property*>& PropertyFactory::getRegisteredProperties() const {
    return properties_;
}

} // namespace voreen
