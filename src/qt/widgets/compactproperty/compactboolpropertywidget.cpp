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

#include "voreen/qt/widgets/compactproperty/compactboolpropertywidget.h"

#include "voreen/core/vis/properties/boolproperty.h"

#include <QCheckBox>

namespace voreen {

CompactBoolPropertyWidget::CompactBoolPropertyWidget(BoolProp* prop, QWidget* parent)
    : CompactPropertyWidget(prop, parent)
    , property_(prop)
    , chkBox_(new QCheckBox)
{
    chkBox_->setChecked(prop->get());
    addWidget(chkBox_);

    connect(chkBox_, SIGNAL(toggled(bool)), this, SLOT(setProperty(bool)));

    addVisibilityControls();
}

void CompactBoolPropertyWidget::update() {
    chkBox_->blockSignals(true);
    chkBox_->setChecked(property_->get());
    chkBox_->blockSignals(false);
}

void CompactBoolPropertyWidget::setProperty(bool value) {
    if (!disconnected_) {
        property_->set(value);
        emit propertyChanged();
    }
}

} // namespace
