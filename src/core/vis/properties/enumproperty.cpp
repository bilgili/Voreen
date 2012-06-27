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

#include "voreen/core/vis/properties/enumproperty.h"
#include "voreen/core/vis/propertywidgetfactory.h"

namespace voreen {

EnumProp::EnumProp(const std::string& id, const std::string& guiText, const std::vector<std::string>& value,
                   int startindex, bool invalidate, bool invalidateShader)
    : TemplateProperty<int>(id, guiText, startindex, invalidate, invalidateShader)
    , strings_(value)
{
}

void EnumProp::setStrings(const std::vector<std::string>& strings) {
    strings_ = strings;
    updateWidgets();
}

void EnumProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    if (propElem->Attribute("value")) {
        bool found = false;
        for (size_t j = 0; j < strings_.size(); ++j) {
            if (strings_.at(j) == propElem->Attribute("value")) {
                set(j); // The compiler might complain about size_t to int conversion...
                found = true;
                break;
            }
        }
        if (!found)
             errors_.store(XmlAttributeException(std::string("Invalid enum value: ")
                                                 + propElem->Attribute("value")));
    }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
}

TiXmlElement* EnumProp::serializeToXml() const {
    TiXmlElement* propElem = Property::serializeToXml();

    if (getSerializeMetaData()) {    
        propElem->SetAttribute("class", "EnumProperty");

        for (size_t i = 0; i < strings_.size(); ++i) {
            TiXmlElement* allowed = new TiXmlElement("allowedValue");
            allowed->SetAttribute("value", i);
            allowed->SetAttribute("label", strings_[i].c_str());
            propElem->LinkEndChild(allowed);
        }
    }
    
    propElem->SetAttribute("value", strings_[value_]);
    return propElem;
}

PropertyWidget* EnumProp::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

} // namespace voreen
