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

#include "voreen/qt/widgets/compactproperty/compactintvec2propertywidget.h"

#include "voreen/core/vis/properties/vectorproperty.h"

#include <QSpinBox>
#include <QVBoxLayout>

namespace voreen {

CompactIntVec2PropertyWidget::CompactIntVec2PropertyWidget(IntVec2Prop* prop, QWidget* parent)
    : CompactPropertyWidget(prop, parent)
    , property_(prop)
    , spinBox_x(new QSpinBox)
    , spinBox_y(new QSpinBox)
{
    update();

    QWidget* w = new QWidget;
    QLayout* l = new QVBoxLayout(w);
    l->setSpacing(1);
    l->setMargin(1);
    l->addWidget(spinBox_x);
    l->addWidget(spinBox_y);

    addWidget(w);

    connect(spinBox_x, SIGNAL(valueChanged(int)), this, SLOT(setPropertyX(int)));
    connect(spinBox_y, SIGNAL(valueChanged(int)), this, SLOT(setPropertyY(int)));

    addVisibilityControls();
}

void CompactIntVec2PropertyWidget::update() {
    spinBox_x->blockSignals(true);
    spinBox_x->setRange(property_->getMinValue().x, property_->getMaxValue().x);
    spinBox_x->setValue(property_->get().x);
    spinBox_x->blockSignals(false);

    spinBox_y->blockSignals(true);
    spinBox_y->setRange(property_->getMinValue().y, property_->getMaxValue().y);
    spinBox_y->setValue(property_->get().y);
    spinBox_y->blockSignals(false);
}

void CompactIntVec2PropertyWidget::setPropertyX(int x) {
    if (!disconnected_) {
        tgt::ivec2 newValue(x, property_->get().y);
        property_->set(newValue);
        emit propertyChanged();
    }
}

void CompactIntVec2PropertyWidget::setPropertyY(int y) {
    if (!disconnected_) {
        tgt::ivec2 newValue(property_->get().x, y);
        property_->set(newValue);
        emit propertyChanged();
    }
}

} // namespace
