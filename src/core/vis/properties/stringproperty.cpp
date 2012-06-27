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

#include "voreen/core/vis/properties/stringproperty.h"
#include "voreen/core/vis/properties/condition.h"
#include "voreen/core/vis/propertywidgetfactory.h"
#include <sstream>

namespace voreen {

StringProp::StringProp(const std::string& id, const std::string& guiText,
                       const std::string& value, bool invalidate, bool invalidateShader)
    : TemplateProperty<std::string>(id, guiText, value, invalidate, invalidateShader)
{}

void StringProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    if (propElem->Attribute("value"))
        try {
            set(propElem->Attribute("value"));
        } catch (Condition::ValidationFailed& e) {
            errors_.store(e);
        }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in property element of " + getIdent().getName()));
}

TiXmlElement* StringProp::serializeToXml() const {
    TiXmlElement* propElem = Property::serializeToXml();
    propElem->SetAttribute("value", value_);
    if (getSerializeTypeInformation())
        propElem->SetAttribute("class", "StringProperty");

    return propElem;
}

PropertyWidget* StringProp::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

// ============================================================================

StringVectorProp::StringVectorProp(const std::string& id, const std::string& guiText,
                                   const std::vector<std::string>& value, bool invalidate, bool invalidateShader)
    : TemplateProperty<std::vector<std::string> >(id, guiText, value, invalidate, invalidateShader)
{}

PropertyWidget* StringVectorProp::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

std::string StringVectorProp::toString() const {
    std::stringstream oss;
    for (size_t i = 0; i < value_.size(); ++i)
        oss << value_[i];
    return oss.str();
}

}   // namespace
