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

#include "voreen/core/vis/properties/boolproperty.h"
#include "voreen/core/vis/properties/condition.h"
#include "voreen/core/vis/propertywidgetfactory.h"


namespace voreen {

BoolProp::BoolProp(const std::string& id, const std::string& guiText, bool value,
                   bool invalidate, bool invalidateShader)
    : TemplateProperty<bool>(id, guiText, value, invalidate, invalidateShader)
{}

void BoolProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    if (propElem->Attribute("value"))
        try {
            set(std::string("true").compare(propElem->Attribute("value")) == 0 ? true : false);
        } catch (Condition::ValidationFailed& e) {
            errors_.store(e);
        }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in property element of " + getIdent().getName()));
}

TiXmlElement* BoolProp::serializeToXml() const {
    TiXmlElement* propElem = Property::serializeToXml();
    propElem->SetAttribute("value", ((value_ == true) ? "true" : "false"));
    if (getSerializeTypeInformation())
        propElem->SetAttribute("class", "BoolProperty");
    return propElem;
}

PropertyWidget* BoolProp::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

}   // namespace
