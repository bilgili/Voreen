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

#include "voreen/qt/widgets/property/optionpropertywidget.h"

#include "voreen/core/vis/properties/optionproperty.h"

#include <QComboBox>

namespace voreen {

OptionPropertyWidget::OptionPropertyWidget(OptionPropertyBase* prop, QWidget* parent)
    : QPropertyWidget(prop, parent)
    , property_(prop)
    , cBox_(new QComboBox)
{
    updateFromProperty();
    addWidget(cBox_);
    connect(cBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(setProperty(int)));

    addVisibilityControls();
}

void OptionPropertyWidget::updateFromProperty() {
    cBox_->blockSignals(true);
    cBox_->clear();

    std::vector<std::string> ids = property_->getKeys();
    std::map<std::string, std::string> descriptions = property_->getDescriptions();

    for (size_t i=0; i< ids.size(); i++)
        cBox_->insertItem(i, descriptions[ids[i]].c_str());
    cBox_->setCurrentIndex(indexOf(property_->get(), ids));
    cBox_->blockSignals(false);
}

void OptionPropertyWidget::setProperty(int index) {
    if (!disconnected_) {
        std::string value = property_->getKeys().at(index);
        property_->set(value);
        emit modified();
    }
}

size_t OptionPropertyWidget::indexOf(const std::string& s, const std::vector<std::string>& v) {
    size_t i;
    for (i = 0; i < v.size(); ++i) {
        if (s == v[i])
            return i;
    }
    return 0;
}

} // namespace
