/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/qt/widgets/property/qpropertywidget.h"
#include "voreen/qt/widgets/customlabel.h"

#include "voreen/core/properties/property.h"
#include <QAction>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>

#include <QToolButton>

namespace voreen {

#ifdef __APPLE__
    const int QPropertyWidget::fontSize_ = 13;
#else
    const int QPropertyWidget::fontSize_ = 8;
#endif

QPropertyWidget::QPropertyWidget(Property* prop, QWidget* parent, bool showNameLabel)
    : QWidget(parent)
    , PropertyWidget(prop)
    , disconnected_(false)
    , lodControl_(0)
    , nameLabel_(0)
    , showNameLabel_(showNameLabel)
{
    showNameLabel = false;
    tgtAssert(prop, "No property passed");
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    layout_ = new QHBoxLayout(this);
    layout_->addSpacing(2);
    layout_->setContentsMargins(1, 1, 1, 1);
    QFontInfo fontInfo(font());
    setFont(QFont(fontInfo.family(), QPropertyWidget::fontSize_));

    setMinimumWidth(125);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    setMouseTracking(true);     // This is kind of a hack needed to update propertytimelinewidgets for animation. Look at them for further explanation

    connect(this, SIGNAL(updateFromPropertySignal()), this, SLOT(updateFromPropertySlot()));
}

QPropertyWidget::~QPropertyWidget() {
    if (!disconnected_ && prop_) {
        prop_->removeWidget(this);
        prop_ = 0;
    }
    delete nameLabel_;
    nameLabel_ = 0;
}

void QPropertyWidget::updateFromProperty() {
    emit updateFromPropertySignal();
}

std::string QPropertyWidget::getPropertyGuiName() {
    if (disconnected_ || !prop_)
        return "";

    return prop_->getGuiName().c_str();
}

void QPropertyWidget::setPropertyGuiName(std::string name) {
    prop_->setGuiName(name);
}

void QPropertyWidget::addVisibilityControls() {
    lodControl_ = new QToolButton(this);
    lodControl_->setToolTip(tr("Show property in visualization mode"));
    lodControl_->setCheckable(true);
    lodControl_->setFixedSize(13, 13);
    if (prop_ != 0)
        setLevelOfDetail(prop_->getLevelOfDetail());
    lodControl_->hide();

    connect(lodControl_, SIGNAL(clicked(bool)), this, SLOT(setLevelOfDetail(bool)));

    layout_->addWidget(lodControl_, 0);
}

QSize QPropertyWidget::sizeHint() const {
    return QSize(150, 0);
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
    if (disconnected_ || !prop_)
        return;

    if (lodControl_)
        lodControl_->hide();
}

void QPropertyWidget::showLODControls() {
    if (disconnected_ || !prop_)
        return;

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
    if (disconnected_ || !prop_)
        return;

    prop_->setLevelOfDetail(value);

    if (lodControl_) {
        if (value == Property::USER) {
            lodControl_->setChecked(true);
            lodControl_->setIcon(QIcon(":/qt/icons/eye.png"));
        } else {
            lodControl_->setChecked(false);
            lodControl_->setIcon(QIcon(":/qt/icons/eye-crossedout.png"));
        }
    }

    emit levelOfDetailChanged(value);
}

void QPropertyWidget::setEnabled(bool enabled) {
    if (disconnected_ || !prop_)
        return;

    QWidget::setEnabled(enabled);
    if (nameLabel_ != 0)
        nameLabel_->setEnabled(enabled);
}

void QPropertyWidget::setVisible(bool state) {
    if (disconnected_ || !prop_)
        return;

    QWidget::setVisible(state);
    if (nameLabel_ != 0)
        nameLabel_->setVisible(state);
    emit visibilityChanged();
}

void QPropertyWidget::toggleInteractionMode(bool im) {
    if (disconnected_ || !prop_)
        return;

    prop_->toggleInteractionMode(im, this);
}

void QPropertyWidget::mouseMoveEvent(QMouseEvent* event) {
    emit mouseClicked();
    QWidget::mouseMoveEvent(event);
}

CustomLabel* QPropertyWidget::getNameLabel() const {
    if (!nameLabel_) {
        nameLabel_ = new CustomLabel(prop_->getGuiName().c_str(), const_cast<QPropertyWidget*>(this), const_cast<QPropertyWidget*>(this), 0, false, true);
        nameLabel_->setMinimumWidth(80);
        nameLabel_->setWordWrap(true);
        QFontInfo fontInfo(font());
        nameLabel_->setFont(QFont(fontInfo.family(), fontSize_));
        nameLabel_->setToolTip("id: " + QString::fromStdString(prop_->getID()));

        nameLabel_->setVisible(isVisible());
        nameLabel_->setEnabled(isEnabled());
    }
    return nameLabel_;
}

void QPropertyWidget::showNameLabel(bool visible) {
    if (nameLabel_) {
        if (showNameLabel_)
            nameLabel_->setVisible(visible);
        else
            nameLabel_->setVisible(false);
    }
}

} // namespace
