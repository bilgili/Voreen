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

#include "voreen/core/properties/plotselectionproperty.h"
#include "voreen/core/properties/propertywidgetfactory.h"

#include <sstream>

namespace voreen {

const std::string PlotSelectionProperty::loggerCat_("voreen.Properties.PlotSelectionProperty");

PlotSelectionProperty::PlotSelectionProperty(const std::string& id, const std::string& guiText, bool showThirdDimension, voreen::Processor::InvalidationLevel invalidationLevel)
    : TemplateProperty< std::vector< PlotZoomState > >(id, guiText, std::vector< PlotZoomState >(), invalidationLevel)
    , isDeserialized_(false)
    , showThirdDimension_(showThirdDimension)
    {
}

void PlotSelectionProperty::push(const PlotZoomState& value) {
    value_.push_back(value);
    notifyAll();
}

PlotZoomState PlotSelectionProperty::pop() {
    PlotZoomState toReturn = value_.back();
    value_.pop_back();
    notifyAll();
    return toReturn;
}

PlotZoomState PlotSelectionProperty::top() const {
    if (value_.empty())
        return PlotZoomState();
    else
        return value_.back();
}


void PlotSelectionProperty::clear() {
    value_.clear();
    notifyAll();
}

const std::vector<PlotZoomState>& PlotSelectionProperty::get() const {
    return value_;
}

void PlotSelectionProperty::set(const std::vector< PlotZoomState >& value) {
    clear();
    value_ = value;
    notifyAll();
}

bool PlotSelectionProperty::getShowThirdDimension() const {
    return showThirdDimension_;
}

void PlotSelectionProperty::setShowThirdDimension(bool value) {
    showThirdDimension_ = value;
    updateWidgets();
}

int PlotSelectionProperty::size() const {
    return static_cast<int>(value_.size());
}

void PlotSelectionProperty::notifyAll() {
    std::vector< PlotZoomState > cpy = value_;

    // execute links
    executeLinks(cpy, cpy);

    // check if conditions are met and exec actions
    for (size_t j = 0; j < conditions_.size(); ++j)
        conditions_[j]->exec();

    updateWidgets();

    // invalidate owner:
    invalidateOwner();
}


void PlotSelectionProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("values", value_);
}

void PlotSelectionProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    s.deserialize("values", value_);
    isDeserialized_ = true;
}

PropertyWidget* PlotSelectionProperty::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

std::string PlotSelectionProperty::getTypeString() const {
    return "PlotSelection";
}


}   // namespace
