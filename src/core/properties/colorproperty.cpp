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

#include "voreen/core/properties/colorproperty.h"
#include "voreen/core/properties/condition.h"
#include "voreen/core/properties/propertywidgetfactory.h"
#include <sstream>

namespace voreen {

ColorProperty::ColorProperty(const std::string& id, const std::string& guiText,
                     tgt::Color value, Processor::InvalidationLevel invalidationLevel)
    : TemplateProperty<tgt::vec4>(id, guiText, value, invalidationLevel)
{
    LWARNINGC("voreen.ColorProperty", "ColorProperty is DEPRECATED: Please use FloatVec4Property with view Property::COLOR instead");

}

void ColorProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("value", value_);
}

void ColorProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    tgt::vec4 value;
    s.deserialize("value", value);
    try {
        set(value);
    }
    catch (Condition::ValidationFailed& e) {
        s.addError(e);
    }
}

PropertyWidget* ColorProperty::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

}   // namespace
