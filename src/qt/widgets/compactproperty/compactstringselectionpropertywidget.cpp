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

#include "voreen/qt/widgets/compactproperty/compactstringselectionpropertywidget.h"

#include "voreen/qt/widgets/choicelistcombobox.h"
#include "voreen/core/vis/properties/stringselectionproperty.h"

namespace voreen {

CompactStringSelectionPropertyWidget::CompactStringSelectionPropertyWidget(StringSelectionProp* prop, QWidget* parent)
    : CompactPropertyWidget(prop, parent)
    , property_(prop)
{
    cBox_ = new ChoiceListComboBox(choices_, property_->getEditable());
    addWidget(cBox_);

    if (property_->getEditable())
        connect(cBox_, SIGNAL(editTextChanged(const QString&)), this, SLOT(setProperty()));
    else
        connect(cBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(setProperty()));    

    update();

    addVisibilityControls();
}

void CompactStringSelectionPropertyWidget::update() {
    choices_ = property_->getChoices();

    if (property_->getEditable()) {
        cBox_->setEditText(property_->get().c_str());
    } else {
        // We update the internal list temporarily, as it will be updated again before the
        // popup list is shown.
        static_cast<ChoiceListComboBox*>(cBox_)->setIgnore(true);
        if (cBox_->count() == 0)
            cBox_->addItem("");
        cBox_->setItemText(0, property_->get().c_str());
        cBox_->setCurrentIndex(0);
        static_cast<ChoiceListComboBox*>(cBox_)->setIgnore(true);
    }
}

void CompactStringSelectionPropertyWidget::setProperty() {
    if (!disconnected_ && !static_cast<ChoiceListComboBox*>(cBox_)->ignore()) {
        property_->set(cBox_->currentText().toStdString());
        emit propertyChanged();       
    }
}

} // namespace
