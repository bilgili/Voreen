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

#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/condition.h"
#include "voreen/core/properties/propertywidgetfactory.h"


namespace voreen {

BoolProperty::BoolProperty(const std::string& id, const std::string& guiText, bool value,
                   Processor::InvalidationLevel invalidationLevel)
    : TemplateProperty<bool>(id, guiText, value, invalidationLevel)
{}

void BoolProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("value", value_);
}

void BoolProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    bool value;
    s.deserialize("value", value);
    try {
        set(value);
    }
    catch (Condition::ValidationFailed& e) {
        s.addError(e);
    }
}

PropertyWidget* BoolProperty::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

std::string BoolProperty::getTypeString() const {
    return "Boolean";
}

}   // namespace
