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

#include "voreen/core/properties/link/boxobject.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/properties/volumehandleproperty.h"
#include "voreen/core/datastructures/volume/volumecollection.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

#include <sstream>

#define VP(a) (*(a*)value_)

namespace voreen {

using std::string;
using std::vector;
using std::stringstream;
using tgt::vec2;
using tgt::vec3;
using tgt::vec4;
using tgt::ivec2;
using tgt::ivec3;
using tgt::ivec4;
using tgt::mat2;
using tgt::mat3;
using tgt::mat4;
using tgt::Camera;

template<class T>
void BoxObject::set(const T& value, BoxObjectType type) {
    if (type != currentType_) {
        deleteValue();
        currentType_ = type;
    }
    if (value_ == 0) {
        value_ = new T;
    }
    *(T*)value_ = value;
}

void BoxObject::set(const TransFunc& value, BoxObjectType type) {
    if (type != currentType_) {
        deleteValue();
        currentType_ = type;
    }
    value_ = (void*)&value;
}

void BoxObject::set(const Camera& value, BoxObjectType type) {
    if (type != currentType_) {
        deleteValue();
        currentType_ = type;
    }
    value_ = (void*)&value;
}

void BoxObject::set(const VolumeHandle& value, BoxObjectType type) {
    if (type != currentType_) {
        deleteValue();
        currentType_ = type;
    }
    value_ = (void*)&value;
}

void BoxObject::set(const VolumeCollection& value, BoxObjectType type) {
    if (type != currentType_) {
        deleteValue();
        currentType_ = type;
    }
    value_ = (void*)&value;
}

template<class T>
const string BoxObject::toString(const T& value) const {
    stringstream s;
    string result;
    s << value;
    s >> result;
    return result;
}


BoxObject::BoxObject()
    : value_(0)
    , currentType_(NIL)
{}

BoxObject::BoxObject(const BoxObject& obj) : value_(0) {
    switch (obj.getType()) {
    case NIL:
        break;
    case BOOL:
        set<bool>(obj.getBool(), BOOL);
        break;
    case DOUBLE:
        set<double>(obj.getDouble(), DOUBLE);
        break;
    case FLOAT:
        set<float>(obj.getFloat(), FLOAT);
        break;
    case INTEGER:
        set<int>(obj.getInt(), INTEGER);
        break;
    case LONG:
        set<long>(obj.getLong(), LONG);
        break;
    case STRING:
        set<string>(obj.getString(), STRING);
        break;
    case STRINGVEC:
        set<vector<string> >(obj.getStringVec(), STRINGVEC);
        break;
    case IVEC2:
        set<ivec2>(obj.getIVec2(), IVEC2);
        break;
    case IVEC3:
        set<ivec3>(obj.getIVec3(), IVEC3);
        break;
    case IVEC4:
        set<ivec4>(obj.getIVec4(), IVEC4);
        break;
    case VEC2:
        set<vec2>(obj.getVec2(), VEC2);
        break;
    case VEC3:
        set<vec3>(obj.getVec3(), VEC3);
        break;
    case VEC4:
        set<vec4>(obj.getVec4(), VEC4);
        break;
    case MAT2:
        set<mat2>(obj.getMat2(), MAT2);
        break;
    case MAT3:
        set<mat3>(obj.getMat3(), MAT3);
        break;
    case MAT4:
        set<mat4>(obj.getMat4(), MAT4);
        break;
    case SHADER:
        set<ShaderSource>(obj.getShader(), SHADER);
        break;
    case TRANSFUNC:
        set(*obj.getTransFunc(), TRANSFUNC);
        break;
    case CAMERA:
        set(*obj.getCamera(), CAMERA);
        break;
    case VOLUMEHANDLE:
        set(*obj.getVolumeHandle(), VOLUMEHANDLE);
        break;
    case VOLUMECOLLECTION:
        set(*obj.getVolumeCollection(), VOLUMECOLLECTION);
        break;
    default:
        LWARNINGC("voreen.BoxObject", "Unknown data type (missing case in switch-statement)");
    }
}

BoxObject::BoxObject(int value) : value_(0), currentType_(INTEGER) {
    set<int>(value, INTEGER);
}

BoxObject::BoxObject(float value) : value_(0), currentType_(FLOAT) {
    set<float>(value, FLOAT);
}

BoxObject::BoxObject(double value) : value_(0), currentType_(DOUBLE) {
    set<double>(value, DOUBLE);
}

BoxObject::BoxObject(bool value) : value_(0), currentType_(BOOL) {
    set<bool>(value, BOOL);
}

BoxObject::BoxObject(const string& value) : value_(0), currentType_(STRING) {
    set<string>(value, STRING);
}

BoxObject::BoxObject(const vector<string>& value) : value_(0), currentType_(STRINGVEC) {
    set<vector<string> >(value, STRINGVEC);
}

BoxObject::BoxObject(long value) : value_(0), currentType_(LONG) {
    set<long>(value, LONG);
}

BoxObject::BoxObject(const vec2& value) : value_(0), currentType_(VEC2) {
    set<vec2>(value, VEC2);
}

BoxObject::BoxObject(const vec3& value) : value_(0), currentType_(VEC3) {
    set<vec3>(value, VEC3);
}

BoxObject::BoxObject(const vec4& value) : value_(0), currentType_(VEC4) {
    set<vec4>(value, VEC4);
}

BoxObject::BoxObject(const ivec2& value) : value_(0), currentType_(IVEC2) {
    set<ivec2>(value, IVEC2);
}

BoxObject::BoxObject(const ivec3& value) : value_(0), currentType_(IVEC3) {
    set<ivec3>(value, IVEC3);
}

BoxObject::BoxObject(const ivec4& value) : value_(0), currentType_(IVEC4) {
    set<ivec4>(value, IVEC4);
}

BoxObject::BoxObject(const mat2& value) : value_(0), currentType_(MAT2) {
    set<mat2>(value, MAT2);
}

BoxObject::BoxObject(const mat3& value) : value_(0), currentType_(MAT3) {
    set<mat3>(value, MAT3);
}

BoxObject::BoxObject(const mat4& value) : value_(0), currentType_(MAT4) {
    set<mat4>(value, MAT4);
}

BoxObject::BoxObject(const ShaderSource& value) : value_(0), currentType_(SHADER) {
    set<ShaderSource>(value, SHADER);
}

BoxObject::BoxObject(const TransFunc* value) : value_(0), currentType_(TRANSFUNC) {
    set(*value, TRANSFUNC);
}

BoxObject::BoxObject(const Camera* value) : value_(0), currentType_(CAMERA) {
    set(*value, CAMERA);
}

BoxObject::BoxObject(const VolumeHandle* value) : value_(0), currentType_(VOLUMEHANDLE) {
    set(*value, VOLUMEHANDLE);
}

BoxObject::BoxObject(const VolumeCollection* value) : value_(0), currentType_(VOLUMECOLLECTION) {
    set(*value, VOLUMECOLLECTION);
}



BoxObject::~BoxObject() {
    deleteValue();
}

void BoxObject::deleteValue() {
    if (value_ != 0) {
        switch (currentType_) {
        case NIL:
            break;
        case BOOL:
            delete static_cast<bool*>(value_);
            break;
        case DOUBLE:
            delete static_cast<double*>(value_);
            break;
        case FLOAT:
            delete static_cast<float*>(value_);
            break;
        case INTEGER:
            delete static_cast<int*>(value_);
            break;
        case LONG:
            delete static_cast<long*>(value_);
            break;
        case STRING:
            delete static_cast<string*>(value_);
            break;
        case STRINGVEC:
            delete static_cast<vector<string>*>(value_);
            break;
        case IVEC2:
            delete static_cast<ivec2*>(value_);
            break;
        case IVEC3:
            delete static_cast<ivec3*>(value_);
            break;
        case IVEC4:
            delete static_cast<ivec4*>(value_);
            break;
        case VEC2:
            delete static_cast<vec2*>(value_);
            break;
        case VEC3:
            delete static_cast<vec3*>(value_);
            break;
        case VEC4:
            delete static_cast<vec4*>(value_);
            break;
        case MAT2:
            delete static_cast<mat2*>(value_);
            break;
        case MAT3:
            delete static_cast<mat3*>(value_);
            break;
        case MAT4:
            delete static_cast<mat4*>(value_);
            break;
        case TRANSFUNC:
            //delete static_cast<TransFuncIntensity*>(value_);
            break;
        case CAMERA:
            //delete static_cast<Camera*>(value_);
            break;
        case VOLUMEHANDLE:
            //delete static_cast<VolumeHandle*>(value_);
            break;
        case VOLUMECOLLECTION:
            //delete static_cast<VolumeCollection*>(value_);
            break;
        case SHADER:
            delete static_cast<ShaderSource*>(value_);
            break;
        default:
            LWARNINGC("voreen.BoxObject", "Unable to delete value due to unknown data type (missing case in switch-statement)");
        }
        value_ = 0;
    }
}

BoxObject::BoxObjectType BoxObject::getType() const {
    return currentType_;
}

std::string BoxObject::getTypeName(BoxObjectType type) {
    switch (type) {
        case NIL:
            return "<empty>";
            break;
        case BOOL:
            return "bool";
            break;
        case DOUBLE:
            return "double";
            break;
        case FLOAT:
            return "float";
            break;
        case INTEGER:
            return "integer";
            break;
        case LONG:
            return "long";
            break;
        case STRING:
            return "string";
            break;
        case STRINGVEC:
            return "string vector";
            break;
        case IVEC2:
            return "ivec2";
            break;
        case IVEC3:
            return "ivec3";
            break;
        case IVEC4:
            return "ivec4";
            break;
        case VEC2:
            return "vec2";
            break;
        case VEC3:
            return "vec3";
            break;
        case VEC4:
            return "vec4";
            break;
        case MAT2:
            return "mat2";
            break;
        case MAT3:
            return "mat3";
            break;
        case MAT4:
            return "mat4";
            break;
        case SHADER:
            return "shader";
            break;
        case TRANSFUNC:
            return "transfer function";
            break;
        case CAMERA:
            return "camera";
            break;
        case VOLUMEHANDLE:
            return "volume handle";
            break;
        case VOLUMECOLLECTION:
            return "volume collection";
            break;

        default:
            return "<unknown>";
    }
}

bool BoxObject::getBool() const throw (VoreenException) {
    switch (currentType_) {
    case BOOL:
        return VP(bool);
        break;
    case DOUBLE:
        return static_cast<bool>(VP(double));
        break;
    case INTEGER:
        return static_cast<bool>(VP(int));
        break;
    case FLOAT:
        return static_cast<bool>(VP(float));
        break;
    case STRING: {
        string s = VP(string);
        for (unsigned int i = 0; i < s.length(); ++i)
            s[i] = tolower(s[i]);

        if (s == "1" || s == "true") {
            return true;
        }
        if (s == "0" || s == "false") {
            return false;
        }
        throw VoreenException("String->Bool conversion failed");
        break;
    }
    case LONG:
        return static_cast<bool>(VP(long));
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to bool not implemented");
    }
    return true; //never reached...fixes warning
}

double BoxObject::getDouble() const throw (VoreenException) {
    switch (currentType_) {
    case BOOL:
        return static_cast<double>(VP(bool));
        break;
    case DOUBLE:
        return VP(double);
        break;
    case FLOAT:
        return static_cast<double>(VP(float));
        break;
    case INTEGER:
        return static_cast<double>(VP(int));
        break;
    case LONG:
        return static_cast<double>(VP(long));
        break;
    case STRING: {
            stringstream s(VP(string));
            double result;
            if ( (s >> result).fail() )
                throw VoreenException("String->Double conversion failed");
            return result;
            break;
        }
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to double not implemented");
    }
    return 0.0; //never reached...fixes warning
}

float BoxObject::getFloat() const throw (VoreenException) {
    switch (currentType_) {
    case BOOL:
        return static_cast<float>(VP(bool));
        break;
    case DOUBLE:
        return static_cast<float>(VP(double));
        break;
    case FLOAT:
        return VP(float);
        break;
    case INTEGER:
        return static_cast<float>(VP(int));
        break;
    case LONG:
        return static_cast<float>(VP(long));
        break;
    case STRING: {
        stringstream s(VP(string));
        float result;
        if ( (s >> result).fail() )
            throw VoreenException("String->Double conversion failed");
        return result;
        break;
     }
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to float not implemented");
    }
    return 0.0f; //never reached...fixes warning
}

int BoxObject::getInt() const throw (VoreenException) {
    switch (currentType_) {
    case BOOL:
        return static_cast<int>(VP(bool));
        break;
    case DOUBLE:
        return static_cast<int>(VP(double));
        break;
    case FLOAT:
        return static_cast<int>(VP(float));
        break;
    case INTEGER:
        return VP(int);
        break;
    case LONG:
        return static_cast<int>(VP(long));
        break;
    case STRING: {
            stringstream s(VP(string));
            int result;
            if ( (s >> result).fail() )
                throw VoreenException("String->Int conversion failed");
            return result;
            break;
        }
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to integer not implemented");
    }
    return 0; //never reached...fixes warning
}

long BoxObject::getLong() const throw (VoreenException) {
    switch (currentType_) {
    case BOOL:
        return static_cast<long>(VP(bool));
        break;
    case DOUBLE:
        return static_cast<long>(VP(double));
        break;
    case FLOAT:
        return static_cast<long>(VP(float));
        break;
    case INTEGER:
        return static_cast<long>(VP(int));
        break;
    case LONG:
        return VP(long);
        break;
    case STRING: {
            stringstream s(VP(string));
            long result;
            if ( (s >> result).fail() )
                throw VoreenException("String->Int conversion failed");
            return result;
            break;
        }
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to long not implemented");
    }
    return 0; //never reached...fixes warning
}

string BoxObject::getString() const throw (VoreenException) {
    switch (currentType_) {
    case BOOL:
        return VP(bool) ? "true" : "false";
        break;
    case DOUBLE:
        return toString<double>(VP(double));
        break;
    case FLOAT:
        return toString<float>(VP(float));
        break;
    case INTEGER:
        return toString<int>(VP(int));
        break;
    case LONG:
        return toString<long>(VP(long));
        break;
    case STRING:
        return VP(string);
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to string not implemented");
    }
}

vector<string> BoxObject::getStringVec() const throw (VoreenException) {
    switch (currentType_) {
    case STRINGVEC:
        return VP(vector<string>);
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion not implemented");
    }
}

ivec2 BoxObject::getIVec2() const throw (VoreenException) {
    switch (currentType_) {
    case IVEC2:
        return VP(ivec2);
        break;
    case IVEC3:
        return VP(ivec3).xy();
        break;
    case IVEC4:
        return VP(ivec4).xy();
        break;
    case VEC2:
        return ivec2(static_cast<int>(VP(vec2).x), static_cast<int>(VP(vec2).y));
        break;
    case VEC3:
        return ivec2(static_cast<int>(VP(vec3).x), static_cast<int>(VP(vec3).y));
        break;
    case VEC4:
        return ivec2(static_cast<int>(VP(vec4).x), static_cast<int>(VP(vec4).y));
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to ivec2 not implemented");
    }
}

ivec3 BoxObject::getIVec3() const throw (VoreenException) {
    switch (currentType_) {
    case IVEC2:
        return ivec3(VP(ivec2).x, VP(ivec2).y, 0);
        break;
    case IVEC3:
        return VP(ivec3);
        break;
    case IVEC4:
        return VP(ivec4).xyz();
        break;
    case VEC2:
        return ivec3(static_cast<int>(VP(vec2).x), static_cast<int>(VP(vec2).y), 0);
        break;
    case VEC3:
        return ivec3(static_cast<int>(VP(vec3).x), static_cast<int>(VP(vec3).y), static_cast<int>(VP(vec3).z));
        break;
    case VEC4:
        return ivec3(static_cast<int>(VP(vec4).x), static_cast<int>(VP(vec4).y), static_cast<int>(VP(vec4).z));
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to ivec3 not implemented");
    }
}

ivec4 BoxObject::getIVec4() const throw (VoreenException) {
    switch (currentType_) {
    case IVEC2:
        return ivec4(VP(ivec2).x, VP(ivec2).y, 0, 0);
        break;
    case IVEC3:
        return ivec4(VP(ivec3).x, VP(ivec3).y, VP(ivec3).z, 0);
        break;
    case IVEC4:
        return VP(ivec4);
        break;
    case VEC2:
        return ivec4(static_cast<int>(VP(vec2).x), static_cast<int>(VP(vec2).y), 0, 0);
        break;
    case VEC3:
        return ivec4(static_cast<int>(VP(vec3).x), static_cast<int>(VP(vec3).y), static_cast<int>(VP(vec3).z), 0);
        break;
    case VEC4:
        return ivec4(static_cast<int>(VP(vec4).x), static_cast<int>(VP(vec4).y), static_cast<int>(VP(vec4).z), static_cast<int>(VP(vec4).w));
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to ivec4 not implemented");
    }
}

vec2 BoxObject::getVec2() const throw (VoreenException) {
    switch (currentType_) {
    case IVEC2:
        return vec2(static_cast<float>(VP(ivec2).x), static_cast<float>(VP(ivec2).y));
        break;
    case IVEC3:
        return vec2(static_cast<float>(VP(ivec3).x), static_cast<float>(VP(ivec3).y));
        break;
    case IVEC4:
        return vec2(static_cast<float>(VP(ivec4).x), static_cast<float>(VP(ivec4).y));
        break;
    case VEC2:
        return VP(vec2);
        break;
    case VEC3:
        return VP(vec3).xy();
        break;
    case VEC4:
        return VP(vec4).xy();
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to vec2 not implemented");
    }
}

vec3 BoxObject::getVec3() const throw (VoreenException) {
    switch (currentType_) {
    case IVEC2:
        return vec3(static_cast<float>(VP(ivec2).x), static_cast<float>(VP(ivec2).y), 0);
        break;
    case IVEC3:
        return vec3(static_cast<float>(VP(ivec3).x), static_cast<float>(VP(ivec3).y), static_cast<float>(VP(ivec3).z));
        break;
    case IVEC4:
        return vec3(static_cast<float>(VP(ivec4).x), static_cast<float>(VP(ivec4).y), static_cast<float>(VP(ivec4).z));
        break;
    case VEC2:
        return vec3(VP(vec2).x, VP(vec2).y, 0);
        break;
    case VEC3:
        return VP(vec3);
        break;
    case VEC4:
        return VP(vec4).xyz();
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to vec3 not implemented");
    }
}

vec4 BoxObject::getVec4() const throw (VoreenException) {
    switch (currentType_) {
    case IVEC2:
        return vec4(static_cast<float>(VP(ivec2).x), static_cast<float>(VP(ivec2).y), 0, 0);
        break;
    case IVEC3:
        return vec4(static_cast<float>(VP(ivec3).x), static_cast<float>(VP(ivec3).y), static_cast<float>(VP(ivec3).z), 0);
        break;
    case IVEC4:
        return vec4(static_cast<float>(VP(ivec4).x), static_cast<float>(VP(ivec4).y), static_cast<float>(VP(ivec4).z), static_cast<float>(VP(ivec4).w));
        break;
    case VEC2:
        return vec4(VP(vec2).x, VP(vec2).y, 0, 0);
        break;
    case VEC3:
        return vec4(VP(vec3).x, VP(vec3).y, VP(vec3).z, 0);
        break;
    case VEC4:
        return VP(vec4);
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to vec4 not implemented");
    }
}

mat2 BoxObject::getMat2() const throw (VoreenException) {
    switch (currentType_) {
    case MAT2:
        return VP(mat2);
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to mat2 not implemented");
    }
}

mat3 BoxObject::getMat3() const throw (VoreenException) {
    switch (currentType_) {
    case MAT3:
        return VP(mat3);
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to mat3 not implemented");
    }
}

mat4 BoxObject::getMat4() const throw (VoreenException) {
    switch (currentType_) {
    case MAT4:
        return VP(mat4);
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to mat4 not implemented");
    }
}

const TransFunc* BoxObject::getTransFunc() const throw (VoreenException) {
    switch (currentType_) {
    case TRANSFUNC:
        return &VP(TransFunc);
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to transfer function not implemented");
    }
}

const Camera* BoxObject::getCamera() const throw (VoreenException) {
    switch (currentType_) {
    case CAMERA:
        return &VP(Camera);
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to camera not implemented");
    }
}

const VolumeHandle* BoxObject::getVolumeHandle() const throw (VoreenException) {
    switch (currentType_) {
    case VOLUMEHANDLE:
        return &VP(VolumeHandle);
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to volume handle not implemented");
    }
}

const VolumeCollection* BoxObject::getVolumeCollection() const throw (VoreenException) {
    switch (currentType_) {
    case VOLUMECOLLECTION:
        return &VP(VolumeCollection);
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to volume collection not implemented");
    }
}

ShaderSource BoxObject::getShader() const throw (VoreenException) {
    switch (currentType_) {
    case SHADER:
        return VP(ShaderSource);
        break;
    case NIL:
        throw VoreenException("Conversion tried on empty boxobject");
    default:
        throw VoreenException("BoxObject: Conversion from " + getTypeName(currentType_) + " to shader not implemented");
    }
}

BoxObject& BoxObject::operator= (const BoxObject& rhs) {
    if (this != &rhs) {
        switch (rhs.getType()) {
        case NIL:
            break;
        case BOOL:
            set<bool>(rhs.getBool(), BOOL);
            break;
        case DOUBLE:
            set<double>(rhs.getDouble(), DOUBLE);
            break;
        case FLOAT:
            set<float>(rhs.getFloat(), FLOAT);
            break;
        case INTEGER:
            set<int>(rhs.getInt(), INTEGER);
            break;
        case LONG:
            set<long>(rhs.getLong(), LONG);
            break;
        case STRING:
            set<string>(rhs.getString(), STRING);
            break;
        case STRINGVEC:
            set<vector<string> >(rhs.getStringVec(), STRINGVEC);
            break;
        case IVEC2:
            set<ivec2>(rhs.getIVec2(), IVEC2);
            break;
        case IVEC3:
            set<ivec3>(rhs.getIVec3(), IVEC3);
            break;
        case IVEC4:
            set<ivec4>(rhs.getIVec4(), IVEC4);
            break;
        case VEC2:
            set<vec2>(rhs.getVec2(), VEC2);
            break;
        case VEC3:
            set<vec3>(rhs.getVec3(), VEC3);
            break;
        case VEC4:
            set<vec4>(rhs.getVec4(), VEC4);
            break;
        case MAT2:
            set<mat2>(rhs.getMat2(), MAT2);
            break;
        case MAT3:
            set<mat3>(rhs.getMat3(), MAT3);
            break;
        case MAT4:
            set<mat4>(rhs.getMat4(), MAT4);
            break;
        case TRANSFUNC:
            set(*rhs.getTransFunc(), TRANSFUNC);
            break;
        case CAMERA:
            set(*rhs.getCamera(), CAMERA);
            break;
        case VOLUMEHANDLE:
            set(*rhs.getVolumeHandle(), VOLUMEHANDLE);
            break;
        case VOLUMECOLLECTION:
            set(*rhs.getVolumeCollection(), VOLUMECOLLECTION);
            break;
        case SHADER:
            set<ShaderSource>(rhs.getShader(), SHADER);
            break;
        default:
            throw VoreenException("BoxObject: Conversion not implemented");
        }
    }
    return *this;
}

BoxObject& BoxObject::operator= (const bool& rhs) {
    set<bool>(rhs, BOOL);
    return *this;
}

BoxObject& BoxObject::operator= (const double& rhs) {
    set<double>(rhs, DOUBLE);
    return *this;
}

BoxObject& BoxObject::operator= (const float& rhs) {
    set<float>(rhs, FLOAT);
    return *this;
}

BoxObject& BoxObject::operator= (const int& rhs) {
    set<int>(rhs, INTEGER);
    return *this;
}

BoxObject& BoxObject::operator= (const long& rhs) {
    set<long>(rhs, LONG);
    return *this;
}

BoxObject& BoxObject::operator= (const string& rhs) {
    set<string>(rhs, STRING);
    return *this;
}

BoxObject& BoxObject::operator= (const vector<string>& rhs) {
    set<vector<string> >(rhs, STRINGVEC);
    return *this;
}

BoxObject& BoxObject::operator= (const ivec2& rhs) {
    set<ivec2>(rhs, IVEC2);
    return *this;
}

BoxObject& BoxObject::operator= (const ivec3& rhs) {
    set<ivec3>(rhs, IVEC3);
    return *this;
}

BoxObject& BoxObject::operator= (const ivec4& rhs) {
    set<ivec4>(rhs, IVEC4);
    return *this;
}

BoxObject& BoxObject::operator= (const vec2& rhs) {
    set<vec2>(rhs, VEC2);
    return *this;
}

BoxObject& BoxObject::operator= (const vec3& rhs) {
    set<vec3>(rhs, VEC3);
    return *this;
}

BoxObject& BoxObject::operator= (const vec4& rhs) {
    set<vec4>(rhs, VEC4);
    return *this;
}

BoxObject& BoxObject::operator= (const mat2& rhs) {
    set<mat2>(rhs, MAT2);
    return *this;
}

BoxObject& BoxObject::operator= (const mat3& rhs) {
    set<mat3>(rhs, MAT3);
    return *this;
}

BoxObject& BoxObject::operator= (const mat4& rhs) {
    set<mat4>(rhs, MAT4);
    return *this;
}

BoxObject& BoxObject::operator= (const TransFunc* rhs) {
    set(*rhs, TRANSFUNC);
    return *this;
}

BoxObject& BoxObject::operator= (const Camera* rhs) {
    set(*rhs, CAMERA);
    return *this;
}

BoxObject& BoxObject::operator= (const VolumeHandle* rhs) {
    set(*rhs, VOLUMEHANDLE);
    return *this;
}

BoxObject& BoxObject::operator= (const VolumeCollection* rhs) {
    set(*rhs, VOLUMECOLLECTION);
    return *this;
}

BoxObject& BoxObject::operator= (const ShaderSource& rhs) {
    set<ShaderSource>(rhs, SHADER);
    return *this;
}

} // namespace
