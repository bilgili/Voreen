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

#include "voreen/qt/widgets/compactproperty/compactenumpropertywidget.h"

#include "voreen/core/vis/properties/enumproperty.h"

#include <QComboBox>

namespace voreen {

CompactEnumPropertyWidget::CompactEnumPropertyWidget(EnumProp* prop, QWidget* parent)
    : CompactPropertyWidget(prop, parent)
    , property_(prop)
    , cBox_(new QComboBox)
{
    std::vector<std::string> strings = prop->getStrings();
    for (size_t i = 0; i< strings.size(); i++)
        cBox_->insertItem(i, strings[i].c_str());
    cBox_->setCurrentIndex(prop->get());

    addWidget(cBox_);

    connect(cBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(setProperty(int)));

    addVisibilityControls();
}

void CompactEnumPropertyWidget::update() {
    cBox_->blockSignals(true);
    QString curText = cBox_->currentText();
    cBox_->clear();
    std::vector<std::string> strings = property_->getStrings();
    for (size_t i=0; i< strings.size(); i++) {
        cBox_->insertItem(i, strings[i].c_str());
    }
    int index = cBox_->findText(curText);
    if (index == -1)
        cBox_->setCurrentIndex(property_->get());
    else
        cBox_->setCurrentIndex(index);
    property_->set(cBox_->currentIndex());
    cBox_->blockSignals(false);
}

void CompactEnumPropertyWidget::setProperty(int value) {
    if (!disconnected_) {
        property_->set(value);
        emit propertyChanged();
    }
}

} // namespace
