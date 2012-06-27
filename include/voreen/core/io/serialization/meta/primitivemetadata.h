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

#ifndef VRN_PRIMITIVEMETADATA_H
#define VRN_PRIMITIVEMETADATA_H

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

/**
 * Generic abstract base class for meta data of primitive types.
 *
 * @note New meta data classes must also be added to MetaDataContainer::initializeFactories .
 */
template<typename T>
class PrimitiveMetaDataBase : public MetaDataBase {

public:
    /**
     * Assigns the value to be stored.
     */
    void setValue(T value);

    /**
     * Returns the stored value.
     */
    T getValue() const;

    /// @see Serializable::serialize
    virtual void serialize(XmlSerializer& s) const;
    /// @see Serializable::deserialize
    virtual void deserialize(XmlDeserializer& s);

    /// @see SerializableFactory::getTypeString
    virtual const std::string getTypeString(const std::type_info& type) const = 0;
    /// @see SerializableFactory::createType
    virtual Serializable* createType(const std::string& typeString) = 0;

private:
    T value_;
};

//---------------------------------------------------------------------------------------

/**
 * Holds a single boolean value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class BoolMetaData : public PrimitiveMetaDataBase<bool> {
public:
    virtual const std::string getTypeString(const std::type_info& type) const;
    virtual Serializable* createType(const std::string& typeString);
};

//---------------------------------------------------------------------------------------

/**
 * Holds a single string value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class StringMetaData : public PrimitiveMetaDataBase<std::string> {
public:
    virtual const std::string getTypeString(const std::type_info& type) const;
    virtual Serializable* createType(const std::string& typeString);
};

//---------------------------------------------------------------------------------------

/**
 * Holds a single int value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class IntMetaData : public PrimitiveMetaDataBase<int> {
public:
    virtual const std::string getTypeString(const std::type_info& type) const;
    virtual Serializable* createType(const std::string& typeString);
};

//---------------------------------------------------------------------------------------

/**
 * Holds a single float value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class FloatMetaData : public PrimitiveMetaDataBase<float> {
public:
    virtual const std::string getTypeString(const std::type_info& type) const;
    virtual Serializable* createType(const std::string& typeString);
};


//---------------------------------------------------------------------------------------
//template definitions

template <typename T>
void PrimitiveMetaDataBase<T>::setValue(T value) {
    value_ = value;
}

template <typename T>
T PrimitiveMetaDataBase<T>::getValue() const {
    return value_;
}

template <typename T>
void PrimitiveMetaDataBase<T>::serialize(XmlSerializer& s) const {
    s.serialize("value", value_);
}

template <typename T>
void PrimitiveMetaDataBase<T>::deserialize(XmlDeserializer& s) {
    s.deserialize("value", value_);
}

} // namespace

#endif // VRN_PRIMITIVEMETADATA_H
