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

#include "voreen/qt/widgets/property/intpropertywidget.h"

#include "voreen/core/properties/intproperty.h"
#include "voreen/qt/widgets/sliderspinboxwidget.h"

#include <QHBoxLayout>

namespace voreen {

IntPropertyWidget::IntPropertyWidget(IntProperty* prop, QWidget* parent, bool addVisibilityControl)
    : QPropertyWidget(prop, parent),
    property_(prop),
    widget_(new SliderSpinBoxWidget(this))
{
    addWidget(widget_);

    updateFromProperty();

    connect(widget_, SIGNAL(valueChanged(int)), this, SLOT(setProperty(int)));
    connect(widget_, SIGNAL(sliderPressedChanged(bool)), this, SLOT(toggleInteractionMode(bool)));
    connect(widget_, SIGNAL(valueChanged(int)), this, SIGNAL(widgetChanged()));

    if (addVisibilityControl == true)
        QPropertyWidget::addVisibilityControls();
}

IntPropertyWidget::~IntPropertyWidget() {
    widget_->disconnect();
    delete widget_;
}

void IntPropertyWidget::updateFromProperty() {
    if (property_ != 0) {
        widget_->blockSignals(true);
        widget_->setMinValue(property_->getMinValue());
        widget_->setMaxValue(property_->getMaxValue());
        widget_->setSingleStep(property_->getStepping());
        widget_->setValue(property_->get());
        widget_->blockSignals(false);
    }
}

void IntPropertyWidget::setWidget(const int value, const int minValue,
                                  const int maxValue, const int stepping)
{
    widget_->blockSignals(true);
    widget_->setMinValue(minValue);
    widget_->setMaxValue(maxValue);
    widget_->setSingleStep(stepping);
    widget_->setValue(value);
    widget_->blockSignals(false);
}

void IntPropertyWidget::setProperty(int value) {
    if ((property_ != 0) && (!disconnected_)) {
        property_->set(value);
        emit modified();
    }

    emit valueChanged(value);
}

} // namespace
