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

#include "voreen/core/properties/colormapproperty.h"
#include "voreen/core/properties/condition.h"
#include "voreen/core/properties/propertywidgetfactory.h"
#include <sstream>

namespace voreen {

ColorMapProperty::ColorMapProperty(const std::string& id, const std::string& guiText,
                     ColorMap value, Processor::InvalidationLevel invalidationLevel)
    : TemplateProperty<ColorMap>(id, guiText, value, invalidationLevel)
{}

void ColorMapProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("colors", value_);
}

void ColorMapProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    ColorMap cm = ColorMap::createColdHot();
    s.deserialize("colors", cm);
    try {
        value_ = cm;
    }
    catch (Condition::ValidationFailed& e) {
        s.addError(e);
    }
}

PropertyWidget* ColorMapProperty::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

std::string ColorMapProperty::getTypeString() const {
    return "ColorMap";
}

}   // namespace
