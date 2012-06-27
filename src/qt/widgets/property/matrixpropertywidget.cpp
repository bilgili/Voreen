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

#include "voreen/core/properties/property.h"

#include "voreen/qt/widgets/property/matrixpropertywidget.h"

#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QPushButton>

namespace voreen {

MatrixPropertyWidget::MatrixPropertyWidget(Property* prop, QWidget* parent)
    : QPropertyWidget(prop, parent)
{
    editButton_ = new QPushButton(tr("Edit"), this);
    updateButton_ = new QPushButton(tr("Update"), this);
    identityButton_ = new QPushButton(tr("Identity"), this);
    autoUpdate_ = new QCheckBox("Auto Update", this);
    QFontInfo fontInfo(font());
    editButton_->setFont(QFont(fontInfo.family(), QPropertyWidget::fontSize_));
    layout_->addWidget(editButton_);
    matrixDialog_ = new QDialog(this);
    mainLayout_ = new QGridLayout(matrixDialog_);
    connect(editButton_, SIGNAL(clicked()), this, SLOT(executeDialog()));
    connect(updateButton_, SIGNAL(clicked()), this, SLOT(updateValue()));
    mainLayout_->addWidget(autoUpdate_, 0, 0);
    mainLayout_->addWidget(updateButton_, 0, 1);
    mainLayout_->addWidget(identityButton_, 0, 2);
    connect(identityButton_, SIGNAL(clicked()), this, SLOT(identity()));
    addVisibilityControls();
}

void MatrixPropertyWidget::executeDialog() {
    matrixDialog_->move(QPoint(QCursor::pos().x() - 500, QCursor::pos().y()));
    matrixDialog_->show();
}

void MatrixPropertyWidget::autoUpdate() {
    if(autoUpdate_->isChecked())
        updateValue();
}

} //namespace voreen
