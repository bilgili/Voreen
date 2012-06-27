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

#include "voreen/core/properties/numericproperty.h"
#include <sstream>

namespace voreen {

template<typename T>
NumericProperty<T>::NumericProperty(const std::string& id, const std::string& guiText, const T& value,
                                    const T& minValue, const T& maxValue, const T& stepping,
                                    Processor::InvalidationLevel invalidationLevel)
    : TemplateProperty<T>(id, guiText, value, invalidationLevel),
    minValue_(minValue),
    maxValue_(maxValue),
    stepping_(stepping),
    tracking_(true),
    numDecimals_(2)
{
    addValidation(NumericPropertyValidation<T>(this));
    std::string errorMsg;
    validate(value, errorMsg);
    if (!errorMsg.empty())
        LWARNINGC("voreen.TemplateProperty", errorMsg);
}

template<typename T>
void NumericProperty<T>::setMaxValue( const T& maxValue ) {
    maxValue_ = maxValue;
    this->updateWidgets();
}

template<typename T>
const T& NumericProperty<T>::getMaxValue() const {
    return maxValue_;
}

template<typename T>
void NumericProperty<T>::setMinValue(const T& minValue) {
    minValue_ = minValue;
    this->updateWidgets();
}

template<typename T>
const T& NumericProperty<T>::getMinValue() const {
    return minValue_;
}

template<typename T>
void NumericProperty<T>::setStepping(const T stepping) {
    stepping_ = stepping;
    this->updateWidgets();
}

template<typename T>
T NumericProperty<T>::getStepping() const {
    return stepping_;
}

template<typename T>
size_t NumericProperty<T>::getNumDecimals() const {
    return numDecimals_;
}

template<typename T>
void NumericProperty<T>::setNumDecimals(size_t numDecimals) {
    tgtAssert(numDecimals <= 64, "Invalid number of decimals");
    numDecimals_ = numDecimals;
    this->updateWidgets();
}

template<typename T>
bool NumericProperty<T>::hasTracking() const {
    return tracking_;
}

template<typename T>
void NumericProperty<T>::setTracking(bool tracking) {
    tracking_ = tracking;
    this->updateWidgets();
}

template<typename T>
void NumericProperty<T>::increase() {
    set(value_ + stepping_);
}

template<typename T>
void NumericProperty<T>::decrease() {
    set(value_ - stepping_);
}

template<typename T>
void NumericProperty<T>::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("value", value_);

    // serialize tracking mode, if it differs from default value
    if (!tracking_)
        s.serialize("tracking", tracking_);
}

template<typename T>
void NumericProperty<T>::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    // deserialize value
    T value;
    s.deserialize("value", value);
    try {
        set(value);
    }
    catch (Condition::ValidationFailed& e) {
        s.addError(e);
    }

    // deserialize tracking mode, if available
    try {
        s.deserialize("tracking", tracking_);
    }
    catch (XmlSerializationNoSuchDataException&) {
        s.removeLastError();
    }
}

// explicit template instantiation to enable distribution of
// implementation of template class methods over .h and .cpp files
// and speed up compilation time.
//
template class NumericProperty<int>;
template class NumericProperty<float>;
template class NumericProperty<double>;
template class NumericProperty<tgt::vec2>;
template class NumericProperty<tgt::vec3>;
template class NumericProperty<tgt::vec4>;
template class NumericProperty<tgt::ivec2>;
template class NumericProperty<tgt::ivec3>;
template class NumericProperty<tgt::ivec4>;
template class NumericProperty<tgt::dvec2>;
template class NumericProperty<tgt::dvec3>;
template class NumericProperty<tgt::dvec4>;
template class NumericProperty<tgt::mat2>;
template class NumericProperty<tgt::mat3>;
template class NumericProperty<tgt::mat4>;

} // namespace voreen
