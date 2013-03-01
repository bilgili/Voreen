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

#ifndef VRN_PRIMITIVEMETADATA_H
#define VRN_PRIMITIVEMETADATA_H

#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/datastructures/datetime.h"

#include "voreen/core/utils/stringutils.h"

namespace voreen {

/**
 * Generic abstract base class for meta data of primitive types.
 */
template<typename T>
class PrimitiveMetaDataBase : public MetaDataBase {
public:
    PrimitiveMetaDataBase() {}
    PrimitiveMetaDataBase(T value);

    virtual std::string toString() const = 0;
    virtual std::string toString(const std::string& /*component*/) const;

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
    BoolMetaData() : PrimitiveMetaDataBase<bool>() {}
    BoolMetaData(bool value) : PrimitiveMetaDataBase<bool>(value) {}

    virtual std::string getClassName() const { return "BoolMetaData"; }
    virtual MetaDataBase* create() const     { return new BoolMetaData(); }
    virtual MetaDataBase* clone() const      { return new BoolMetaData(getValue()); }

    virtual std::string toString() const {
        if (getValue())
            return "true";
        else
            return "false";
    }

};

//---------------------------------------------------------------------------------------

/**
 * Holds a single string value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class StringMetaData : public PrimitiveMetaDataBase<std::string> {
public:
    StringMetaData() : PrimitiveMetaDataBase<std::string>() {}
    StringMetaData(std::string value) : PrimitiveMetaDataBase<std::string>(value) {}

    virtual std::string getClassName() const { return "StringMetaData"; }
    virtual MetaDataBase* create() const     { return new StringMetaData(); }
    virtual MetaDataBase* clone() const      { return new StringMetaData(getValue()); }
    virtual std::string toString() const { return getValue();}

};

//---------------------------------------------------------------------------------------

/**
 * Holds a single int value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class IntMetaData : public PrimitiveMetaDataBase<int> {
public:
    IntMetaData() : PrimitiveMetaDataBase<int>() {}
    IntMetaData(int value) : PrimitiveMetaDataBase<int>(value) {}

    virtual std::string getClassName() const { return "IntMetaData"; }
    virtual MetaDataBase* create() const     { return new IntMetaData(); }
    virtual MetaDataBase* clone() const      { return new IntMetaData(getValue()); }
    virtual std::string toString() const { return itos(getValue());}

};

//---------------------------------------------------------------------------------------

/**
 * Holds a single size_t value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class SizeTMetaData : public PrimitiveMetaDataBase<size_t> {
public:
    SizeTMetaData() : PrimitiveMetaDataBase<size_t>() {}
    SizeTMetaData(size_t value) : PrimitiveMetaDataBase<size_t>(value) {}

    virtual std::string getClassName() const { return "SizeTMetaData"; }
    virtual MetaDataBase* create() const     { return new SizeTMetaData(); }
    virtual MetaDataBase* clone() const      { return new SizeTMetaData(getValue()); }
    virtual std::string toString() const { return itos(getValue()); }

};

//---------------------------------------------------------------------------------------

/**
 * Holds a single float value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class FloatMetaData : public PrimitiveMetaDataBase<float> {
public:
    FloatMetaData() : PrimitiveMetaDataBase<float>() {}
    FloatMetaData(float value) : PrimitiveMetaDataBase<float>(value) {}

    virtual std::string getClassName() const { return "FloatMetaData"; }
    virtual MetaDataBase* create() const     { return new FloatMetaData(); }
    virtual MetaDataBase* clone() const      { return new FloatMetaData(getValue()); }
    virtual std::string toString() const { return ftos(getValue());}

};

//---------------------------------------------------------------------------------------

/**
 * Holds a single double value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class DoubleMetaData : public PrimitiveMetaDataBase<double> {
public:
    DoubleMetaData() : PrimitiveMetaDataBase<double>() {}
    DoubleMetaData(double value) : PrimitiveMetaDataBase<double>(value) {}

    virtual std::string getClassName() const { return "DoubleMetaData"; }
    virtual MetaDataBase* create() const     { return new DoubleMetaData(); }
    virtual MetaDataBase* clone() const      { return new DoubleMetaData(getValue()); }
    virtual std::string toString() const { return dtos(getValue());}

};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::vec2 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class Vec2MetaData : public PrimitiveMetaDataBase<tgt::vec2> {
public:
    Vec2MetaData() : PrimitiveMetaDataBase<tgt::vec2>() {}
    Vec2MetaData(tgt::vec2 value) : PrimitiveMetaDataBase<tgt::vec2>(value) {}

    virtual std::string getClassName() const { return "Vec2MetaData"; }
    virtual MetaDataBase* create() const     { return new Vec2MetaData(); }
    virtual MetaDataBase* clone() const      { return new Vec2MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

    virtual std::string toString(const std::string& component) const {
        if (component == "x")
            return ftos(getValue().x);
        else if (component == "y")
            return ftos(getValue().y);
        else
            return toString();
    }
};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::ivec2 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class IVec2MetaData : public PrimitiveMetaDataBase<tgt::ivec2> {
public:
    IVec2MetaData() : PrimitiveMetaDataBase<tgt::ivec2>() {}
    IVec2MetaData(tgt::ivec2 value) : PrimitiveMetaDataBase<tgt::ivec2>(value) {}

    virtual std::string getClassName() const { return "IVec2MetaData"; }
    virtual MetaDataBase* create() const     { return new IVec2MetaData(); }
    virtual MetaDataBase* clone() const      { return new IVec2MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

    virtual std::string toString(const std::string& component) const {
        if (component == "x")
            return itos(getValue().x);
        else if (component == "y")
            return itos(getValue().y);
        else
            return toString();
    }
};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::vec3 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class Vec3MetaData : public PrimitiveMetaDataBase<tgt::vec3> {
public:
    Vec3MetaData() : PrimitiveMetaDataBase<tgt::vec3>() {}
    Vec3MetaData(tgt::vec3 value) : PrimitiveMetaDataBase<tgt::vec3>(value) {}

    virtual std::string getClassName() const { return "Vec3MetaData"; }
    virtual MetaDataBase* create() const     { return new Vec3MetaData(); }
    virtual MetaDataBase* clone() const      { return new Vec3MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

    virtual std::string toString(const std::string& component) const {
        if (component == "x")
            return ftos(getValue().x);
        else if (component == "y")
            return ftos(getValue().y);
        else if (component == "z")
            return ftos(getValue().z);
        else
            return toString();
    }
};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::dvec2 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class DVec2MetaData : public PrimitiveMetaDataBase<tgt::dvec2> {
public:
    DVec2MetaData() : PrimitiveMetaDataBase<tgt::dvec2>() {}
    DVec2MetaData(tgt::dvec2 value) : PrimitiveMetaDataBase<tgt::dvec2>(value) {}

    virtual std::string getClassName() const { return "DVec2MetaData"; }
    virtual MetaDataBase* create() const     { return new DVec2MetaData(); }
    virtual MetaDataBase* clone() const      { return new DVec2MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

    virtual std::string toString(const std::string& component) const {
        if (component == "x")
            return dtos(getValue().x);
        else if (component == "y")
            return dtos(getValue().y);
        else
            return toString();
    }


};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::dvec3 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class DVec3MetaData : public PrimitiveMetaDataBase<tgt::dvec3> {
public:
    DVec3MetaData() : PrimitiveMetaDataBase<tgt::dvec3>() {}
    DVec3MetaData(tgt::dvec3 value) : PrimitiveMetaDataBase<tgt::dvec3>(value) {}

    virtual std::string getClassName() const { return "DVec3MetaData"; }
    virtual MetaDataBase* create() const     { return new DVec3MetaData(); }
    virtual MetaDataBase* clone() const      { return new DVec3MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

    virtual std::string toString(const std::string& component) const {
        if (component == "x")
            return dtos(getValue().x);
        else if (component == "y")
            return dtos(getValue().y);
        else if (component == "z")
            return dtos(getValue().z);
        else
            return toString();
    }
};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::ivec3 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class IVec3MetaData : public PrimitiveMetaDataBase<tgt::ivec3> {
public:
    IVec3MetaData() : PrimitiveMetaDataBase<tgt::ivec3>() {}
    IVec3MetaData(tgt::ivec3 value) : PrimitiveMetaDataBase<tgt::ivec3>(value) {}

    virtual std::string getClassName() const { return "IVec3MetaData"; }
    virtual MetaDataBase* create() const     { return new IVec3MetaData(); }
    virtual MetaDataBase* clone() const      { return new IVec3MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

    virtual std::string toString(const std::string& component) const {
        if (component == "x")
            return itos(getValue().x);
        else if (component == "y")
            return itos(getValue().y);
        else if (component == "z")
            return itos(getValue().z);
        else
            return toString();
    }
};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::vec4 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class Vec4MetaData : public PrimitiveMetaDataBase<tgt::vec4> {
public:
    Vec4MetaData() : PrimitiveMetaDataBase<tgt::vec4>() {}
    Vec4MetaData(tgt::vec4 value) : PrimitiveMetaDataBase<tgt::vec4>(value) {}

    virtual std::string getClassName() const { return "Vec4MetaData"; }
    virtual MetaDataBase* create() const     { return new Vec4MetaData(); }
    virtual MetaDataBase* clone() const      { return new Vec4MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

    virtual std::string toString(const std::string& component) const {
        if (component == "x")
            return ftos(getValue().x);
        else if (component == "y")
            return ftos(getValue().y);
        else if (component == "z")
            return ftos(getValue().z);
        else if (component == "w")
            return ftos(getValue().w);
        else
            return toString();
    }
};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::ivec4 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class IVec4MetaData : public PrimitiveMetaDataBase<tgt::ivec4> {
public:
    IVec4MetaData() : PrimitiveMetaDataBase<tgt::ivec4>() {}
    IVec4MetaData(tgt::ivec4 value) : PrimitiveMetaDataBase<tgt::ivec4>(value) {}

    virtual std::string getClassName() const { return "IVec4MetaData"; }
    virtual MetaDataBase* create() const     { return new IVec4MetaData(); }
    virtual MetaDataBase* clone() const      { return new IVec4MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

    virtual std::string toString(const std::string& component) const {
        if (component == "x")
            return itos(getValue().x);
        else if (component == "y")
            return itos(getValue().y);
        else if (component == "z")
            return itos(getValue().z);
        else if (component == "w")
            return itos(getValue().w);
        else
            return toString();
    }
};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::dvec4 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class DVec4MetaData : public PrimitiveMetaDataBase<tgt::dvec4> {
public:
    DVec4MetaData() : PrimitiveMetaDataBase<tgt::dvec4>() {}
    DVec4MetaData(tgt::dvec4 value) : PrimitiveMetaDataBase<tgt::dvec4>(value) {}

    virtual std::string getClassName() const { return "DVec4MetaData"; }
    virtual MetaDataBase* create() const     { return new DVec4MetaData(); }
    virtual MetaDataBase* clone() const      { return new DVec4MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

    virtual std::string toString(const std::string& component) const {
        if (component == "x")
            return dtos(getValue().x);
        else if (component == "y")
            return dtos(getValue().y);
        else if (component == "z")
            return dtos(getValue().z);
        else if (component == "w")
            return dtos(getValue().w);
        else
            return toString();
    }
};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::mat2 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class Mat2MetaData : public PrimitiveMetaDataBase<tgt::mat2> {
public:
    Mat2MetaData() : PrimitiveMetaDataBase<tgt::mat2>() {}
    Mat2MetaData(tgt::mat2 value) : PrimitiveMetaDataBase<tgt::mat2>(value) {}

    virtual std::string getClassName() const { return "Mat2MetaData"; }
    virtual MetaDataBase* create() const     { return new Mat2MetaData(); }
    virtual MetaDataBase* clone() const      { return new Mat2MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::mat3 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class Mat3MetaData : public PrimitiveMetaDataBase<tgt::mat3> {
public:
    Mat3MetaData() : PrimitiveMetaDataBase<tgt::mat3>() {}
    Mat3MetaData(tgt::mat3 value) : PrimitiveMetaDataBase<tgt::mat3>(value) {}

    virtual std::string getClassName() const { return "Mat3MetaData"; }
    virtual MetaDataBase* create() const     { return new Mat3MetaData(); }
    virtual MetaDataBase* clone() const      { return new Mat3MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

};

//---------------------------------------------------------------------------------------

/**
 * Holds a tgt::mat4 value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class Mat4MetaData : public PrimitiveMetaDataBase<tgt::mat4> {
public:
    Mat4MetaData() : PrimitiveMetaDataBase<tgt::mat4>() {}
    Mat4MetaData(tgt::mat4 value) : PrimitiveMetaDataBase<tgt::mat4>(value) {}

    virtual std::string getClassName() const { return "Mat4MetaData"; }
    virtual MetaDataBase* create() const     { return new Mat4MetaData(); }
    virtual MetaDataBase* clone() const      { return new Mat4MetaData(getValue()); }
    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

};

//---------------------------------------------------------------------------------------

/**
 * Holds a DateTime value to be stored as serialization meta data.
 *
 * @see MetaDataBase
 */
class DateTimeMetaData : public PrimitiveMetaDataBase<DateTime> {
public:
    DateTimeMetaData() : PrimitiveMetaDataBase<DateTime>() {}
    DateTimeMetaData(DateTime value) : PrimitiveMetaDataBase<DateTime>(value) {}

    virtual std::string getClassName() const { return "DateTimeMetaData"; }
    virtual MetaDataBase* create() const     { return new DateTimeMetaData(); }
    virtual MetaDataBase* clone() const      { return new DateTimeMetaData(getValue()); }

    virtual std::string toString() const {
        std::stringstream s;
        s << getValue();
        return s.str();
    }

    virtual std::string toString(const std::string& component) const {

        if (component == "year")
            return itos(getValue().getYear());
        else if (component == "YYYY") {
            std::string com = itos(getValue().getYear());
            if (com.length() == 1)
                return "000" + com;
            else
                return com;
        }
        else if (component == "month")
            return itos(getValue().getMonth());
        else if (component == "MM") {
            std::string com = itos(getValue().getMonth());
            if (com.length() == 1)
                return "0" + com;
            else
                return com;
        }
        else if (component == "day")
            return itos(getValue().getDay());
        else if (component == "DD") {
            std::string com = itos(getValue().getDay());
            if (com.length() == 1)
                return "0" + com;
            else
                return com;
        }
        else if (component == "hour")
            return itos(getValue().getHour());
        else if (component == "hh") {
            std::string com = itos(getValue().getHour());
            if (com.length() == 1)
                return "0" + com;
            else
                return com;
        }
        else if (component == "minute")
            return itos(getValue().getMinute());
        else if (component == "mm") {
            std::string com = itos(getValue().getMinute());
            if (com.length() == 1)
                return "0" + com;
            else
                return com;
        }
        else if (component == "second")
            return itos(getValue().getSecond());
        else if (component == "ss") {
            std::string com = itos(getValue().getSecond());
            if (com.length() == 1)
                return "0" + com;
            else
                return com;
        }
        else if (component == "millisecond")
            return itos(getValue().getMillisecond());
        else if (component == "fff") {
            std::string fff = itos(getValue().getMillisecond());
            if (fff.length() == 1)
                fff = "00"+fff;
            if (fff.length() == 2)
                fff = "0"+fff;
            return fff;
        }
        else
            return toString();
    }
};

//---------------------------------------------------------------------------------------
//template definitions

template <typename T>
PrimitiveMetaDataBase<T>::PrimitiveMetaDataBase(T value) : value_(value) {}

template <typename T>
void PrimitiveMetaDataBase<T>::setValue(T value) {
    value_ = value;
}

template <typename T>
T PrimitiveMetaDataBase<T>::getValue() const {
    return value_;
}

template<typename T>
std::string PrimitiveMetaDataBase<T>::toString(const std::string& /*component*/) const {
    return toString();
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
