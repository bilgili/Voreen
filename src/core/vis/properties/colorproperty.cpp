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

#include "voreen/core/vis/properties/colorproperty.h"
#include "voreen/core/vis/properties/condition.h"
#include "voreen/core/vis/propertywidgetfactory.h"
#include <sstream>

namespace voreen {

ColorProp::ColorProp(const std::string& id, const std::string& guiText,
                     tgt::Color value, bool invalidate, bool invalidateShader)
    : TemplateProperty<tgt::vec4>(id, guiText, value, invalidate, invalidateShader)
{}

void ColorProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    tgt::vec4 color(0.0f);
    if (propElem->QueryFloatAttribute("r", &color.r) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("g", &color.g) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("b", &color.b) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("a", &color.a) == TIXML_SUCCESS)

        try {
            set(color);
        } catch (Condition::ValidationFailed& e) {
            errors_.store(e);
        }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
}

TiXmlElement* ColorProp::serializeToXml() const {
    TiXmlElement* propElem = Property::serializeToXml();

    if (getSerializeMetaData())
        propElem->SetAttribute("class", "ColorProperty");
    
    TiXmlElement* min = new TiXmlElement("minValue");
    TiXmlElement* max = new TiXmlElement("maxValue");
    const std::string fields = "rgba";
    for (size_t i = 0; i < 4; ++i) {
        propElem->SetDoubleAttribute(fields.substr(i, 1).c_str(), value_[i]);
        min->SetDoubleAttribute(fields.substr(i, 1).c_str(), 0.0);
        max->SetDoubleAttribute(fields.substr(i, 1).c_str(), 1.0);
    }
    if (getSerializeMetaData()) {
        propElem->LinkEndChild(min);
        propElem->LinkEndChild(max);
    }

    return propElem;
}

PropertyWidget* ColorProp::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

}   // namespace
