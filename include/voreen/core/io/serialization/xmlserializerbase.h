/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#ifndef VRN_XMLSERIALIZERBASE_H
#define VRN_XMLSERIALIZERBASE_H

#include <string>
#include <vector>

#include "tinyxml/tinyxml.h"

#include "voreen/core/voreencoreapi.h"
#include "voreen/core/io/serialization/serializationexceptions.h"
#include "voreen/core/io/serialization/xmlserializationconstants.h"
#include "voreen/core/io/serialization/serializable.h"
#include "voreen/core/io/serialization/serializablefactory.h"

namespace voreen {

/**
 * The @c XmlSerializerBase class encapsulates functions that are common to @c XmlSerializer
 * and @c XmlDeserializer.
 *
 * @see XmlSerializer
 * @see XmlDeserializer
 */
class VRN_CORE_API XmlSerializerBase {
public:
    /**
     * Default constructor.
     */
    XmlSerializerBase();

    /**
     * Adds the given error @c message to the error list.
     *
     * @param message the error message
     */
    void addError(const std::string& message);

    /**
     * Adds the error message of the given @c exception to the error list.
     *
     * @param exception the exception
     */
    void addError(const std::exception& exception);

    /**
     * Adds the error message from the given @c exception to the error list
     * and raise the exception afterwards.
     *
     * @tparam T exception type
     *
     * @param exception the exception
     *
     * @throws SerializationException the exception is always thrown
     */
    template<class T>
    void raise(const T& exception)
        throw (SerializationException);

    /**
     * Removes the last error message from the error list.
     */
    void removeLastError();

    /**
     * Returns the error list.
     *
     * @return the error list
     */
    std::vector<std::string> getErrors() const;

    /**
     * Returns whether primitive data is serialized as XML attributes or XML nodes.
     *
     * @return @c true if primitive data is serialized as XML attributes and @c false otherwise
     */
    bool getUseAttributes() const;

    /**
     * Sets whether to serialize primitive data as XML attributes or XML nodes.
     *
     * @attention All pointer references to primitive data which are serialized
     *            using XML attributes cannot be resolved. Furthermore using the
     *            same key for different values leads to a
     *            @c XmlSerializationAttributeNamingException.
     *
     * @param useAttributes if @c true serialize primitive data as XML attributes,
     *                      otherwise as XML nodes
     */
    void setUseAttributes(const bool& useAttributes);

    /**
     * Returns whether pointers are always serialized as content instead of references.
     *
     * @return @c true if pointers are always serialized as content and @c false otherwise
     */
    bool getUsePointerContentSerialization() const;

    /**
     * Sets whether to serialize pointer always as content instead of references.
     *
     * @attention This is not a cascading setting, which means that contained pointers
     *            are not serialized as content.
     *
     * @attention Serialization of all pointers as content can lead to redundant data.
     *
     * @param usePointerContentSerialization if @c true pointers are always serialized as content,
     *                                       otherwise as references when possible.
     */
    void setUsePointerContentSerialization(const bool& usePointerContentSerialization);

    /**
     * Registers an @c SerializableFactory to support serialization of polymorphic classes.
     *
     * @attention Ensure that the factory exists during the whole serialization process.
     *
     * @param factory the factory
     */
    void registerFactory(SerializableFactory* factory);

    /**
     * Convenience method for serialization factory registration.
     *
     * @see registerFactory
     */
    void registerFactories(const std::vector<SerializableFactory*>& factories);

protected:
    /**
     * Category for logging.
     */
    static const std::string loggerCat_;

protected:
    /**
     * Converts the given data to @c std::string.
     *
     * @tparam T type of data to convert
     *
     * @param data data to convert
     *
     * @return the string
     */
    template<class T>
    inline std::string convertDataToString(const T& data);

    /**
     * Converts the given data to @c std::string.
     *
     * @tparam T type of data to convert
     *
     * @param data data to convert
     *
     * @return the string
     */
    std::string convertDataToString(const float& data);

    /**
     * Converts the given data to @c std::string.
     *
     * @tparam T type of data to convert
     *
     * @param data data to convert
     *
     * @return the string
     */
    std::string convertDataToString(const double& data);

    /**
     * Converts the given @c std::string to data.
     *
     * @tparam T type of data to convert
     *
     * @param str the string
     * @param data variable to store converted data
     */
    template<class T>
    inline void convertStringToData(const std::string& str, T& data);

    /**
     * Returns type string corresponding with given @c type,
     * when it is supported by a registered factory.
     *
     * @param type the type
     *
     * @returns either the type string or an empty string
     *     when the type is not supported by any registered factory.
     */
    std::string getTypeString(const std::type_info& type);

    /**
     * Returns if given type is a primitive data type.
     *
     * @return @c true if it is a primitive data type and @c false otherwise
     */
    bool isPrimitiveType(const std::type_info& type) const;

    /**
     * Returns if given type is a primitive data pointer type.
     *
     * @return @c true if it is a primitive data pointer type and @c false otherwise.
     */
    bool isPrimitivePointerType(const std::type_info& type) const;

    /**
     * This is a helper class to ensure correct state of the XML node
     * for inserting or reading data.
     *
     * @note As C++ does not support a finally block statement, we need this
     *       class to ensure that cleanup code concerning the XML node
     *       for inserting or reading data is executed.
     */
    class VRN_CORE_API TemporaryNodeChanger {
    public:
        /**
         * Creates a @c TemporaryNodeChange, which changes the actual XML node for
         * inserting or reading data.
         *
         * @param serializer serializer whose XML node should be changed
         * @param node the new node
         */
        TemporaryNodeChanger(XmlSerializerBase& serializer, TiXmlNode* node);

        /**
         * Destructor ensures restoring the XML node for inserting or reading data
         * which was set before this instance was created.
         */
        ~TemporaryNodeChanger();

    private:
        /**
         * Serializer whose XML node is changed.
         */
        XmlSerializerBase& serializer_;

        /**
         * XML node which was set before this @c TemporaryNodeChanger was created.
         */
        TiXmlNode* storedNode_;
    };

    /**
     * This is a helper class to ensure correct use pointer content serialization state.
     *
     * @note As C++ does not support a finally block statement, we need this
     *       class to ensure that cleanup code concerning the XML node
     *       for inserting or reading data is executed.
     */
    class VRN_CORE_API TemporaryUsePointerContentSerializationChanger {
    public:
        /**
         * Creates a @c TemporaryUsePointerContentSerializationChanger,
         * which changes the actual use pointer content serialization setting.
         *
         * @param serializer
         *     serializer whose use pointer content serialization setting should be changed
         * @param node the new setting
         */
        TemporaryUsePointerContentSerializationChanger(XmlSerializerBase& serializer, const bool& usePointerContentSerialization);

        /**
         * Destructor ensures restoring the use pointer content serialization setting
         * which was set before this instance was created.
         */
        ~TemporaryUsePointerContentSerializationChanger();

    private:
        /**
         * Serializer whose use pointer content serialization setting is changed.
         */
        XmlSerializerBase& serializer_;

        /**
         * Use pointer content setting which was set before
         * this @c TemporaryUsePointerContentSettingChanger was created.
         */
        const bool storedUsePointerContentSerialization_;
    };

    /**
     * XML document that contains the already serialized or deserialized data.
     *
     * @attention Keep in mind that may not all pointer references are already resolved.
     */
    TiXmlDocument document_;

    /**
     * XML node for inserting or reading data.
     */
    friend class TemporaryNodeChanger;
    TiXmlNode* node_;

    /**
     * If @c true all primitive data is serialized as XML attributes, otherwise as XML nodes.
     */
    bool useAttributes_;

    /**
     * If @c true all pointers are serialized as content instead of references.
     */
    friend class TemporaryUsePointerContentSerializationChanger;
    bool usePointerContentSerialization_;

    /**
     * Type definition for an @c SerializableFactory list.
     *
     * @note The @c XmlSerializerBase does not own the @c SerializableFactory objects, so remember
     *       to delete the @c SerializableFactory objects where you have created them.
     */
    typedef std::vector<SerializableFactory*> FactoryListType;

    /**
     * List of registered @c SerializableFactory objects.
     *
     * @note @c SerializableFactory objects are necessary
     *       to support polymorphic class serialization.
     */
    FactoryListType factories_;

    /**
     * A list containing all error messages that were raised during serialization process.
     */
    std::vector<std::string> errors_;
};

template<class T>
void XmlSerializerBase::raise(const T& exception)
    throw (SerializationException)
{
    addError(exception);
    throw exception;
}

template<class T>
inline std::string XmlSerializerBase::convertDataToString(const T& data) {
    std::stringstream stream;
    stream << data;
    return stream.str();
}

template<class T>
inline void XmlSerializerBase::convertStringToData(const std::string& str, T& data) {
    std::stringstream stream;
    stream << str;
    stream >> data;
}

} // namespace

#endif // VRN_XMLSERIALIZER_H
