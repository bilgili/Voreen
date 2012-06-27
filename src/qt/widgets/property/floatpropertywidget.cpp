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

#include "voreen/qt/widgets/property/floatpropertywidget.h"

#include "voreen/core/vis/properties/floatproperty.h"
#include "voreen/qt/widgets/sliderspinboxwidget.h"

namespace voreen {

FloatPropertyWidget::FloatPropertyWidget(FloatProperty* prop, QWidget* parent, bool addVisibilityControl)
    : QPropertyWidget(prop, parent),
    property_(prop),
    widget_(new DoubleSliderSpinBoxWidget)
{
    widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QPropertyWidget::layout_->addWidget(widget_, Qt::AlignLeft);

    updateFromProperty();

    connect(widget_, SIGNAL(valueChanged(double)), this, SLOT(setProperty(double)));
    connect(widget_, SIGNAL(sliderPressedChanged(bool)), this, SLOT(toggleInteractionMode(bool)));

    if (addVisibilityControl)
        QPropertyWidget::addVisibilityControls();
}

FloatPropertyWidget::~FloatPropertyWidget() {
    widget_->disconnect();
    delete widget_;
}

void FloatPropertyWidget::updateFromProperty() {
    if (property_ != 0) {
        widget_->blockSignals(true);
        widget_->setDecimals(property_->getNumDecimals());
        widget_->setMinValue(property_->getMinValue());
        widget_->setMaxValue(property_->getMaxValue());
        widget_->setSingleStep(property_->getStepping());
        widget_->setValue(property_->get());
        widget_->blockSignals(false);
    }
}

void FloatPropertyWidget::setWidget(const float value, const float minValue,
                                  const float maxValue, const float stepping)
{
    widget_->blockSignals(true);
    widget_->setMinValue(static_cast<const double>(minValue));
    widget_->setMaxValue(static_cast<const double>(maxValue));
    widget_->setSingleStep(static_cast<const double>(stepping));
    widget_->setValue(static_cast<const double>(value));
    widget_->blockSignals(false);
}

void FloatPropertyWidget::setProperty(double value) {
    if ((property_ != 0) && (!disconnected_))
        property_->set(value);

    emit valueChanged(value);
    emit modified();
}

} // namespace
