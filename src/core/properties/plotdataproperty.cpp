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

#include "voreen/core/properties/plotdataproperty.h"

#include "voreen/core/properties/propertywidgetfactory.h"
#include "voreen/core/plotting/plotdata.h"

namespace voreen {

PlotDataProperty::PlotDataProperty(const std::string& id, const std::string& guiText,
                                   const PlotData* value,
                                   Processor::InvalidationLevel invalidationLevel) :
    TemplateProperty<const PlotData*>(id, guiText, value, invalidationLevel)
{
}

void PlotDataProperty::set(const voreen::PlotData* data) {
    value_ = data;
    notifyChange();
}

void PlotDataProperty::notifyChange() {
    updateWidgets();
    invalidateOwner();
}

PropertyWidget* PlotDataProperty::createWidget(PropertyWidgetFactory* f)     {
    return f->createWidget(this);
}

void PlotDataProperty::serialize(XmlSerializer& /*s*/) const {
    // PlotData shall not be serialised, so we do nothing.
}

void PlotDataProperty::deserialize(XmlDeserializer& /*s*/) {
    // PlotData shall not be serialised, so we do nothing.
}

std::string PlotDataProperty::getTypeString() const {
    return "PlotData";
}

} // namespace voreen
