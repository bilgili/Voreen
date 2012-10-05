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

#include "voreen/core/animation/propertykeyvalue.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/animation/interpolationfunction.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "tgt/camera.h"

using tgt::Camera;

namespace voreen {

template <>
PropertyKeyValue<TransFunc*>* PropertyKeyValue<TransFunc*>::clone() const {
    PropertyKeyValue<TransFunc*>* keyvalue = new PropertyKeyValue<TransFunc*>();
    keyvalue->after_ = after_;
    keyvalue->before_ = before_;
    if (value_)
        keyvalue->value_ = value_->clone();
    else
        LERRORC("voreen.PropertyKeyValue<TransFunc>", "No value");
    keyvalue->time_ = time_;
    keyvalue->smooth_ = smooth_;
    return keyvalue;
}

template <>
PropertyKeyValue<ShaderSource>* PropertyKeyValue<ShaderSource>::clone() const {
    PropertyKeyValue<ShaderSource>* keyvalue = new PropertyKeyValue<ShaderSource>();
    keyvalue->after_ = after_;
    keyvalue->before_ = before_;
    keyvalue->value_ = value_;
    keyvalue->time_ = time_;
    keyvalue->smooth_ = smooth_;
    return keyvalue;
}

template <>
PropertyKeyValue<tgt::Camera>* PropertyKeyValue<tgt::Camera>::clone() const {
    PropertyKeyValue<tgt::Camera>* keyvalue = new PropertyKeyValue<tgt::Camera>();
    keyvalue->after_ = after_;
    keyvalue->before_ = before_;
    keyvalue->value_ = value_;
    keyvalue->time_ = time_;
    keyvalue->smooth_ = smooth_;
    return keyvalue;
}

template <>
void PropertyKeyValue<tgt::Camera>::serialize(XmlSerializer& s) const {
    // base class props
    s.serialize("time", time_);
    s.serialize("smooth", smooth_);
    s.serialize("before", before_);
    s.serialize("after", after_);

    // camera
    s.serialize("position", value_.getPosition());
    s.serialize("focus", value_.getFocus());
    s.serialize("upVector", value_.getUpVector());
}

template <>
void PropertyKeyValue<tgt::Camera>::deserialize(XmlDeserializer& s) {
    // base class props
    s.deserialize("time", time_);
    s.deserialize("smooth", smooth_);
    s.deserialize("before", before_);
    s.deserialize("after", after_);

    // camera
    tgt::vec3 vector;
    s.deserialize("position", vector);
    value_.setPosition(vector);
    s.deserialize("focus", vector);
    value_.setFocus(vector);
    s.deserialize("upVector", vector);
    value_.setUpVector(vector);
    float winRatio = 1.f;
    try {
        s.deserialize("winRatio", winRatio);
    } catch(SerializationException&) {
        s.removeLastError();
    }
}

template class PropertyKeyValue<float>;
template class PropertyKeyValue<int>;
template class PropertyKeyValue<bool>;
template class PropertyKeyValue<tgt::ivec2>;
template class PropertyKeyValue<tgt::ivec3>;
template class PropertyKeyValue<tgt::ivec4>;
template class PropertyKeyValue<tgt::vec2>;
template class PropertyKeyValue<tgt::vec3>;
template class PropertyKeyValue<tgt::vec4>;
template class PropertyKeyValue<tgt::mat2>;
template class PropertyKeyValue<tgt::mat3>;
template class PropertyKeyValue<tgt::mat4>;
template class PropertyKeyValue<tgt::Camera>;
template class PropertyKeyValue<std::string>;
template class PropertyKeyValue<ShaderSource>;
template class PropertyKeyValue<TransFunc*>;

//PropertyKeyValueBase::PropertyKeyValueBase() {}
//
//template <class T>
//PropertyKeyValue<T>::PropertyKeyValue()
//    : time_(0)
//    , before_(0)
//    , after_(0)
//    , smooth_(true) {
//}
//
//template <class T>
//PropertyKeyValue<T>::PropertyKeyValue(T value, float time)
//    : value_(value)
//    , before_(0)
//    , after_(0)
//    , smooth_(true) {
//    time_ = floor(time * 10000.f) / 10000.f;
//}
//
//template <class T>
//PropertyKeyValue<T>::~PropertyKeyValue() {}
//
//template <class T>
//void PropertyKeyValue<T>::setTime(float time) {
//    time_ = floor(time * 10000.f) / 10000.f;
//}
//
//template <class T>
//float PropertyKeyValue<T>::getTime() const {
//    return time_;
//}
//
//template <class T>
//void PropertyKeyValue<T>::setValue(T value) {
//    value_ = value;
//}
//
//template <class T>
//const T PropertyKeyValue<T>::getValue() const {
//    return value_;
//}
//
//template <class T>
//void PropertyKeyValue<T>::setFollowingInterpolationFunction(voreen::InterpolationFunction<T>* func) {
//    after_ = func;
//}
//
//template <class T>
//void PropertyKeyValue<T>::setForegoingInterpolationFunction(voreen::InterpolationFunction<T>* func) {
//    before_ = func;
//}
//
//template <class T>
//const voreen::InterpolationFunction<T>* PropertyKeyValue<T>::getFollowingInterpolationFunction() const {
//    return after_;
//}
//
//template <class T>
//const voreen::InterpolationFunction<T>* PropertyKeyValue<T>::getForegoingInterpolationFunction() const {
//    return before_;
//}
//
//template <>
//PropertyKeyValue<TransFunc*>* PropertyKeyValue<TransFunc*>::clone() const {
//    PropertyKeyValue<TransFunc*>* keyvalue = new PropertyKeyValue<TransFunc*>();
//    keyvalue->after_ = after_;
//    keyvalue->before_ = before_;
//    if (value_)
//        keyvalue->value_ = value_->clone();
//    else
//        LERRORC("voreen.PropertyKeyValue<TransFunc>", "No value");
//    keyvalue->time_ = time_;
//    keyvalue->smooth_ = smooth_;
//    return keyvalue;
//}
//
//template <>
//PropertyKeyValue<ShaderSource>* PropertyKeyValue<ShaderSource>::clone() const {
//    PropertyKeyValue<ShaderSource>* keyvalue = new PropertyKeyValue<ShaderSource>();
//    keyvalue->after_ = after_;
//    keyvalue->before_ = before_;
//    keyvalue->value_ = value_;
//    keyvalue->time_ = time_;
//    keyvalue->smooth_ = smooth_;
//    return keyvalue;
//}
//
//template <>
//PropertyKeyValue<Camera>* PropertyKeyValue<Camera>::clone() const {
//    PropertyKeyValue<Camera>* keyvalue = new PropertyKeyValue<Camera>();
//    keyvalue->after_ = after_;
//    keyvalue->before_ = before_;
//    keyvalue->value_ = value_;
//    keyvalue->time_ = time_;
//    keyvalue->smooth_ = smooth_;
//    return keyvalue;
//}
//
//template <>
//void PropertyKeyValue<Camera>::serialize(XmlSerializer& s) const {
//    // base class props
//    s.serialize("time", time_);
//    s.serialize("smooth", smooth_);
//    s.serialize("before", before_);
//    s.serialize("after", after_);
//
//    // camera
//    s.serialize("position", value_.getPosition());
//    s.serialize("focus", value_.getFocus());
//    s.serialize("upVector", value_.getUpVector());
//}
//
//template <>
//void PropertyKeyValue<Camera>::deserialize(XmlDeserializer& s) {
//    // base class props
//    s.deserialize("time", time_);
//    s.deserialize("smooth", smooth_);
//    s.deserialize("before", before_);
//    s.deserialize("after", after_);
//
//    // camera
//    tgt::vec3 vector;
//    s.deserialize("position", vector);
//    value_.setPosition(vector);
//    s.deserialize("focus", vector);
//    value_.setFocus(vector);
//    s.deserialize("upVector", vector);
//    value_.setUpVector(vector);
//}
//
//template <class T>
//PropertyKeyValue<T>* PropertyKeyValue<T>::clone() const {
//    PropertyKeyValue<T>* keyvalue = new PropertyKeyValue<T>();
//    keyvalue->after_ = after_;
//    keyvalue->before_ = before_;
//    keyvalue->value_ = value_;
//    keyvalue->time_ = time_;
//    keyvalue->smooth_ = smooth_;
//    return keyvalue;
//}
//
//template <class T>
//void PropertyKeyValue<T>::setSmooth(bool smooth) {
//    smooth_ = smooth;
//}
//
//template <class T>
//bool PropertyKeyValue<T>::getSmooth() const {
//    return smooth_;
//}
//
//template <class T>
//bool PropertyKeyValue<T>::isSmoothed() const {
//    if (!getSmooth())
//        return false;
//
//    MultiPointInterpolationFunction<T>* f1 = dynamic_cast<MultiPointInterpolationFunction<T>*>(before_);
//    MultiPointInterpolationFunction<T>* f2 = dynamic_cast<MultiPointInterpolationFunction<T>*>(after_);
//    if (f1 && f2 && (typeid(f1) == typeid(f2)))
//        return true;
//    else
//        return false;
//}
//
//template <class T>
//void PropertyKeyValue<T>::serialize(XmlSerializer& s) const {
//    s.serialize("time", time_);
//    s.serialize("value", value_);
//    s.serialize("smooth", smooth_);
//    s.serialize("before", before_);
//    s.serialize("after", after_);
//}
//
//template <class T>
//void PropertyKeyValue<T>::deserialize(XmlDeserializer& s) {
//    s.deserialize("time", time_);
//    s.deserialize("value", value_);
//    s.deserialize("smooth", smooth_);
//    s.deserialize("before", before_);
//    s.deserialize("after", after_);
//}
//
//
//template class PropertyKeyValue<float>;
//template class PropertyKeyValue<int>;
//template class PropertyKeyValue<bool>;
//template class PropertyKeyValue<tgt::ivec2>;
//template class PropertyKeyValue<tgt::ivec3>;
//template class PropertyKeyValue<tgt::ivec4>;
//template class PropertyKeyValue<tgt::vec2>;
//template class PropertyKeyValue<tgt::vec3>;
//template class PropertyKeyValue<tgt::vec4>;
//template class PropertyKeyValue<tgt::mat2>;
//template class PropertyKeyValue<tgt::mat3>;
//template class PropertyKeyValue<tgt::mat4>;
//template class PropertyKeyValue<tgt::Camera>;
//template class PropertyKeyValue<std::string>;
//template class PropertyKeyValue<ShaderSource>;
//template class PropertyKeyValue<TransFunc*>;

} // namespace voreen
