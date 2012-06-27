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

#ifndef VRN_PROPERTYFACTORY_H
#define VRN_PROPERTYFACTORY_H

#include "voreen/core/io/serialization/serialization.h"

#include <vector>
#include <map>
#include <string>
#include <typeinfo>

namespace voreen {

class Property;

class VRN_CORE_API PropertyFactory : public SerializableFactory {
public:
    PropertyFactory();
    ~PropertyFactory();

    /**
     * Creates a Property object of the passed type.
     * If the type is not known to the factory, 0 is returned.
     *
     * @see registerClass
     *
     * @see SerializableFactory::createType
     */
    virtual Serializable* createType(const std::string& typeString);

    /**
     * Returns the type string of the passed Property type.
     * If the type is not known to the factory, an empty string is returned.
     *
     * @see SerializableFactory::getTypeString
     */
    virtual const std::string getTypeString(const std::type_info& type) const;

    /**
     * Registers the passed Property at the factory.
     */
    void registerProperty(const Property* property);

    /**
     * Returns the properties registered at this factory.
     */
    const std::vector<const Property*>& getRegisteredProperties() const;

private:
    /// Stores the registered processors.
    std::vector<const Property*> properties_;

    /// Maps from Property class name to the corresponding Property instance.
    std::map<std::string, const Property*> classNameMap_;

    /// category used for logging
    static const std::string loggerCat_;
};

} // namespace

#endif // VRN_PROPERTYFACTORY_H
