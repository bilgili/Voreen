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

#include "voreen/qt/widgets/compactproperty/compactfloatpropertywidget.h"

#include "voreen/core/vis/properties/floatproperty.h"

#include <QDoubleSpinBox>

namespace voreen {

CompactFloatPropertyWidget::CompactFloatPropertyWidget(FloatProp* prop, QWidget* parent)
    : CompactPropertyWidget(prop, parent)
    , property_(prop)
    , spinBox_(new QDoubleSpinBox)
{
    addWidget(spinBox_);
    update();

    connect(spinBox_, SIGNAL(valueChanged(double)), this, SLOT(setProperty(double)));

    addVisibilityControls();
}

void CompactFloatPropertyWidget::update() {
    spinBox_->blockSignals(true);
    spinBox_->setDecimals(property_->getNumDecimals()); // set this before setting range
    spinBox_->setRange(property_->getMinValue(), property_->getMaxValue());
    spinBox_->setValue(property_->get());
    spinBox_->setSingleStep(property_->getStepping());
    spinBox_->blockSignals(false);
}

void CompactFloatPropertyWidget::setProperty(double value) {
    if (!disconnected_) {
        property_->set(value);
        emit propertyChanged();
    }
}

} // namespace
