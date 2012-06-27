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

#include "voreen/qt/widgets/compactproperty/compactfloatvec3propertywidget.h"

#include "voreen/core/vis/properties/vectorproperty.h"

#include <QDoubleSpinBox>
#include <QVBoxLayout>

namespace voreen {

CompactFloatVec3PropertyWidget::CompactFloatVec3PropertyWidget(FloatVec3Prop* prop, QWidget* parent)
    : CompactPropertyWidget(prop, parent)
    , property_(prop)
    , spinBox_x(new QDoubleSpinBox)
    , spinBox_y(new QDoubleSpinBox)
    , spinBox_z(new QDoubleSpinBox)
{
    update();

    QWidget* w = new QWidget;
    QLayout* l = new QVBoxLayout(w);
    l->setSpacing(1);
    l->setMargin(1);
    l->addWidget(spinBox_x);
    l->addWidget(spinBox_y);
    l->addWidget(spinBox_z);

    addWidget(w);

    connect(spinBox_x, SIGNAL(valueChanged(double)), this, SLOT(setPropertyX(double)));
    connect(spinBox_y, SIGNAL(valueChanged(double)), this, SLOT(setPropertyY(double)));
    connect(spinBox_z, SIGNAL(valueChanged(double)), this, SLOT(setPropertyZ(double)));

    addVisibilityControls();
}

void CompactFloatVec3PropertyWidget::update() {
    spinBox_x->blockSignals(true);
    spinBox_x->setRange(property_->getMinValue().x, property_->getMaxValue().x);
    spinBox_x->setValue(property_->get().x);
    spinBox_x->setDecimals(property_->getNumDecimals());
    spinBox_x->blockSignals(false);

    spinBox_y->blockSignals(true);
    spinBox_y->setRange(property_->getMinValue().y, property_->getMaxValue().y);
    spinBox_y->setValue(property_->get().y);
    spinBox_y->setDecimals(property_->getNumDecimals());
    spinBox_y->blockSignals(false);

    spinBox_z->blockSignals(true);
    spinBox_z->setRange(property_->getMinValue().z, property_->getMaxValue().z);
    spinBox_z->setValue(property_->get().z);
    spinBox_z->setDecimals(property_->getNumDecimals());
    spinBox_z->blockSignals(false);
}

void CompactFloatVec3PropertyWidget::setPropertyX(double x) {
    if (!disconnected_) {
        tgt::vec3 newValue(property_->get());
        newValue.x = x;
        property_->set(newValue);
        emit propertyChanged();
    }
}

void CompactFloatVec3PropertyWidget::setPropertyY(double y) {
    if (!disconnected_) {
        tgt::vec3 newValue(property_->get());
        newValue.y = y;
        property_->set(newValue);
        emit propertyChanged();
    }
}

void CompactFloatVec3PropertyWidget::setPropertyZ(double z) {
    if (!disconnected_) {
        tgt::vec3 newValue(property_->get());
        newValue.z = z;
        property_->set(newValue);
        emit propertyChanged();
    }
}

} // namespace
