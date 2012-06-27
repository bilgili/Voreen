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

#ifndef VRN_IISERIALIZABLEFACTORY_H
#define VRN_IISERIALIZABLEFACTORY_H

#include <typeinfo>
#include <string>
#include <vector>

namespace voreen {

class Serializable;

/**
 * The @c SerializableFactory is an interface for realizing classes which are supposed to
 * create @c Serializable objects according to a given type string.
 *
 * @par
 * Here is a short example of realizing the @c SerializableFactory interface:
 * @code
 * class Parent : public Serializable {
 * public:
 *     virtual void serialize(XmlSerializer& s) const {}
 *     virtual void deserialize(XmlDeserializer& s) {}
 * };
 *
 * class Child : public Parent {};
 *
 * class Factory : public SerializableFactory {
 * public:
 *     virtual const std::string getTypeString(const type_info& type) const {
 *         if (type == typeid(Parent))
 *             return "Parent";
 *         else if (type == typeid(Child))
 *             return "Child";
 *         else
 *             return "";
 *     }
 *
 *     virtual Serializable* createType(const std::string& typeString) {
 *         if (typeString == "Parent")
 *             return new Parent();
 *         else if (typeString == "Child")
 *             return new Child();
 *         else
 *             return "";
 *     }
 * };
 * @endcode
 * For more complex examples and interaction with other classes of the serialization framework
 * see the &quot;serializertest&quot; application in &quot;apps/&quot; directory.
 *
 * @note The @c SerializableFactory is necessary for implementation of (de-)serialization
 *       of polymorphic user defined data classes.
 *
 * @see Serializable
 * @see XmlSerializer
 * @see XmlDeserializer
 */
class SerializableFactory {
public:
    /**
     * Virtual default destructor.
     */
    virtual ~SerializableFactory() {}

    /**
     * Returns a type string corresponding with the given @c type_info object,
     * which is created using @c typeid, in case this type is supported by the factory.
     *
     * @param type the @c type_info object
     *
     * @returns either the string corresponding with the given @c type_info object,
     *          or an empty string if the type is not supported by this factory.
     */
    virtual const std::string getTypeString(const std::type_info& type) const = 0;

    /**
     * Returns a new instance of an object corresponding with the given @c typeString.
     *
     * @param typeString the type string
     *
     * @returns either the new instance or @c 0 if the type string is not supported by this factory.
     */
    virtual Serializable* createType(const std::string& typeString) = 0;
};

} // namespace

#endif // VRN_IISERIALIZABLEFACTORY_H
