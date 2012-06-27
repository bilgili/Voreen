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

#include "voreen/qt/widgets/compactproperty/compactpropertywidget.h"

#include "voreen/core/vis/properties/property.h"
#include <QAction>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>

#include <QToolButton>
namespace voreen {

CompactPropertyWidget::CompactPropertyWidget(Property* prop, QWidget* parent)
    : QPropertyWidget(parent)
    , disconnected_(false)
    , prop_(prop)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    layout_ = new QHBoxLayout(this);
    layout_->setSpacing(1);
    layout_->setMargin(1);

    // Add GuiText
    layout_->addWidget(new QLabel(tr(prop->getGuiText().c_str())));
}

CompactPropertyWidget::~CompactPropertyWidget() {
    if (!disconnected_)
        prop_->removeWidget(this);
}

void CompactPropertyWidget::addVisibilityControls() {
    layout_->addSpacing(10);
    
    lodControl_ = new QToolButton(this);
    lodControl_->setToolTip(tr("Show property in visualization mode"));
    lodControl_->setCheckable(true);
    lodControl_->setMaximumSize(16,16);
    setLevelOfDetail(prop_->getLevelOfDetail());
    
    connect(lodControl_, SIGNAL(clicked(bool)), this, SLOT(setLevelOfDetail(bool)));

    layout_->addWidget(lodControl_);
}

QSize CompactPropertyWidget::sizeHint() const {
    return QSize(280, 0);
}

void CompactPropertyWidget::addWidget(QWidget* w) {
    layout_->addWidget(w);
}

void CompactPropertyWidget::hideLODControls() {
    lodControl_->hide();
}

void CompactPropertyWidget::showLODControls() {
    lodControl_->show();
}

void CompactPropertyWidget::disconnect() {
    disconnected_ = true;
}

void CompactPropertyWidget::setLevelOfDetail(bool value) {
    setLevelOfDetail( value ? Property::USER : Property::DEVELOPER);
}

void CompactPropertyWidget::setLevelOfDetail(Property::LODSetting value) {
    prop_->setLevelOfDetail(value);
    
    if (value == Property::USER) {
        lodControl_->setChecked(true);
        lodControl_->setIcon(QIcon(":/icons/eye.png"));
    }
    else {
        lodControl_->setChecked(false);
        lodControl_->setIcon(QIcon(":/icons/eye_crossedout.png"));
    }

    emit levelOfDetailChanged(value);
}

} // namespace

