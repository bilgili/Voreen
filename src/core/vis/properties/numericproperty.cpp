/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/properties/numericproperty.h"
#include <sstream>

namespace voreen {

template<typename T>
void NumericProperty<T>::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);

    const std::string componentNames = "xyzw";
    bool res = true;
    double temp = 0.0;
    T vector;
    size_t size = T::size;
    for (size_t i = 0; (i < size && i < 4 && res); ++i) {
        res = (res && (propElem->QueryDoubleAttribute(componentNames.substr(i, 1), &temp) == TIXML_SUCCESS));
        vector.elem[i] = static_cast<typename T::ElemType>(temp);
    }

    if (res) {
        try {
            set(vector);
        } catch (Condition::ValidationFailed& e) {
            errors_.store(e);
        }
    }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));

    if (T::size > 4)
        errors_.store(std::string("Deserialization of Property incomplete! TgtVector has more than 4 components!"));
}

template<typename T>
TiXmlElement* NumericProperty<T>::serializeToXml() const {
    TiXmlElement* propElem = Property::serializeToXml();

    // I admit that this looks quite hackish, but therefore we do not need
    // to write this method for every derived class and can make use of
    // typedefs instead (df)
    std::ostringstream oss;
    if (Property::getSerializeTypeInformation()) {
        size_t size = T::size;
        oss << "Vec" << size << "Property";
        propElem->SetAttribute("class", oss.str());
    }

    const std::string componentNames = "xyzw";
    TiXmlElement* min = new TiXmlElement("minValue");
    TiXmlElement* max = new TiXmlElement("maxValue");
    for (size_t i = 0; i < T::size; ++i) {
        oss.str(""); oss << value_[i];
        propElem->SetAttribute(componentNames.substr(i, 1), oss.str());

        oss.str(""); oss << minValue_[i];
        min->SetAttribute(componentNames.substr(i, 1), oss.str());

        oss.str(""); oss << maxValue_[i];
        max->SetAttribute(componentNames.substr(i, 1), oss.str());
    }
    if (Property::getSerializeTypeInformation()) {
        propElem->LinkEndChild(min);
        propElem->LinkEndChild(max);
    }
    
    return propElem;
}

// specializations for float and int template arguments

// ----------------------------------------------------------------------------

template<>
void NumericProperty<float>::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    float value;
    if (propElem->QueryFloatAttribute("value", &value) == TIXML_SUCCESS) {
        try {
            set(value);
        } catch (Condition::ValidationFailed& e) {
            errors_.store(e);
        }
    } 
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in property element of " + getIdent().getName()));
}

template<>
TiXmlElement* NumericProperty<float>::serializeToXml() const {
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetDoubleAttribute("value", value_);

    if (getSerializeTypeInformation()) {
        propElem->SetAttribute("class", "FloatProperty");
        propElem->SetDoubleAttribute("minValue", minValue_);
        propElem->SetDoubleAttribute("maxValue", maxValue_);
    }
    
    return propElem;
}

// ----------------------------------------------------------------------------

template<>
void NumericProperty<int>::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    int value;
    if (propElem->QueryIntAttribute("value", &value) == TIXML_SUCCESS) {
        try {
            set(value);
        } catch (Condition::ValidationFailed& e) {
            errors_.store(e);
        }
    }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in property element of " + getIdent().getName()));
}

template<>
TiXmlElement* NumericProperty<int>::serializeToXml() const {
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetAttribute("value", value_);

    if (getSerializeTypeInformation()) {
        propElem->SetAttribute("class", "IntProperty");   
        propElem->SetAttribute("minValue", minValue_);
        propElem->SetAttribute("maxValue", maxValue_);
    }

    return propElem;
}

// explicit template instantiation to enable distribution of
// implementation of template class methods over .h and .cpp files
//
template class NumericProperty<int>;
template class NumericProperty<float>;
template class NumericProperty<tgt::vec2>;
template class NumericProperty<tgt::vec3>;
template class NumericProperty<tgt::vec4>;
template class NumericProperty<tgt::ivec2>;
template class NumericProperty<tgt::ivec3>;
template class NumericProperty<tgt::ivec4>;

} // namespace voreen
