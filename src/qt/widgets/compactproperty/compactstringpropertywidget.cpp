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

#include "voreen/qt/widgets/compactproperty/compactstringpropertywidget.h"

#include <QLineEdit>

namespace voreen {

CompactStringPropertyWidget::CompactStringPropertyWidget(StringProp* prop, QWidget* parent)
    : CompactPropertyWidget(prop, parent)
    , property_(prop)
{
    lineEdit_ = new QLineEdit(QString(prop->get().c_str()));
    addWidget(lineEdit_);
 
    connect(lineEdit_, SIGNAL(textChanged(QString)), this, SLOT(setProperty(QString)));

    addVisibilityControls();
}

void CompactStringPropertyWidget::update() {
    lineEdit_->blockSignals(true);
    lineEdit_->setText(QString(property_->get().c_str()));
    lineEdit_->blockSignals(false);
}

void CompactStringPropertyWidget::setProperty(const QString& text) {
    if (!disconnected_) {
        property_->set(text.toStdString());
        emit propertyChanged();
    }
    else {
        update();
    }
}

} // namespace
