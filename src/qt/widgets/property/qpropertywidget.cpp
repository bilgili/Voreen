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

#include "voreen/qt/widgets/property/qpropertywidget.h"

#include "voreen/core/vis/properties/property.h"
#include <QAction>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>

#include <QToolButton>
namespace voreen {

    /*QPropertyWidget::QPropertyWidget(QWidget* parent)
    : QWidget(parent)
    , PropertyWidget()
    {}

    QPropertyWidget::~QPropertyWidget() {
    } */

QPropertyWidget::QPropertyWidget(Property* prop, QWidget* parent, bool showNameLabel)
    : QWidget(parent),
      disconnected_(false),
      prop_(prop),
      lodControl_(0)
{

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    layout_ = new QHBoxLayout(this);
    layout_->setSpacing(1);
    layout_->setMargin(1);

    // Add GuiText
    if (prop_ != 0 && showNameLabel)
        layout_->addWidget(new QLabel(tr(prop_->getGuiText().c_str())), 1);
}

QPropertyWidget::~QPropertyWidget() {
    if (!disconnected_ && prop_) {
        prop_->removeWidget(this);
        prop_ = 0;
    }
}

void QPropertyWidget::addVisibilityControls() {
    layout_->addSpacing(10);

    lodControl_ = new QToolButton(this);
    lodControl_->setToolTip(tr("Show property in visualization mode"));
    lodControl_->setCheckable(true);
    lodControl_->setMaximumSize(16, 16);
    if (prop_ != 0)
        setLevelOfDetail(prop_->getLevelOfDetail());

    connect(lodControl_, SIGNAL(clicked(bool)), this, SLOT(setLevelOfDetail(bool)));

    layout_->addWidget(lodControl_);
}

QSize QPropertyWidget::sizeHint() const {
    return QSize(280, 0);
}

/*
 * TODO rather use insertLayout and insertWidget, so that serial addWidget into BoxLayout is not more necessary
 */
void QPropertyWidget::addWidget(QWidget* widget) {
    layout_->addWidget(widget, 1);
}

void QPropertyWidget::addLayout(QLayout* layout) {
    layout_->addLayout(layout, 1);
}

void QPropertyWidget::hideLODControls() {
    if (lodControl_)
        lodControl_->hide();
}

void QPropertyWidget::showLODControls() {
    if (lodControl_)
        lodControl_->show();
}

void QPropertyWidget::disconnect() {
    disconnected_ = true;
    prop_ = 0;
}

void QPropertyWidget::setLevelOfDetail(bool value) {
    setLevelOfDetail(value ? Property::USER : Property::DEVELOPER);
}

void QPropertyWidget::setLevelOfDetail(Property::LODSetting value) {
    if (prop_ == 0)
        return;

    prop_->setLevelOfDetail(value);

    if (lodControl_) {
        if (value == Property::USER) {
            lodControl_->setChecked(true);
            lodControl_->setIcon(QIcon(":/voreenve/icons/eye.png"));
        } else {
            lodControl_->setChecked(false);
            lodControl_->setIcon(QIcon(":/voreenve/icons/eye-crossedout.png"));
        }
    }

    emit levelOfDetailChanged(value);
}

void QPropertyWidget::setEnabled(bool enabled) {
    QWidget::setEnabled(enabled);
}

void QPropertyWidget::setVisible(bool state) {
    QWidget::setVisible(state);
}

void QPropertyWidget::toggleInteractionMode(bool im) {
    if(prop_)
        prop_->toggleInteractionMode(im, this);
}

} // namespace
