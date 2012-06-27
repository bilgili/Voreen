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

#include "voreen/qt/widgets/expandableheaderbutton.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QToolButton>

namespace voreen {

ExpandableHeaderButton::ExpandableHeaderButton(const QString& name, QWidget* parent, bool expanded, bool expandable)
    : QFrame(parent)
    , expanded_(expanded)
    , userExpandable_(expandable)
{
    QHBoxLayout* frameLayout = new QHBoxLayout(this);
    frameLayout->setSpacing(0);
    frameLayout->setMargin(0);
    setFrameShape(QFrame::StyledPanel);

    expandLabel_ = new QLabel();
    expandLabel_->setMinimumWidth(20);
    expandLabel_->setMaximumWidth(20);
    frameLayout->addWidget(expandLabel_);
    
    rendName_ = new QLabel();
    rendName_->setAlignment(Qt::AlignCenter);
    rendName_->setText(name);
    frameLayout->addSpacing(2);
    frameLayout->addWidget(rendName_);

    lodControlSetHidden_ = new QToolButton;
    lodControlSetHidden_->setToolTip(tr("Hide all properties of this processor in visualization mode"));
    lodControlSetHidden_->setMaximumSize(15,15);
    lodControlSetHidden_->setIcon(QIcon(":/icons/eye_crossedout.png"));
    connect(lodControlSetHidden_, SIGNAL(clicked(bool)), this, SIGNAL(setLODHidden()));
    lodControlSetHidden_->hide();
    frameLayout->addWidget(lodControlSetHidden_);

    lodControlSetVisible_ = new QToolButton;
    lodControlSetVisible_->setToolTip(tr("Show all properties of this processor in visualization mode"));
    lodControlSetVisible_->setMaximumSize(15,15);
    lodControlSetVisible_->setIcon(QIcon(":/icons/eye.png"));
    connect(lodControlSetVisible_, SIGNAL(clicked(bool)), this, SIGNAL(setLODVisible()));
    lodControlSetVisible_->hide();
    frameLayout->addWidget(lodControlSetVisible_);

    updateState();
}

void ExpandableHeaderButton::mousePressEvent(QMouseEvent* event) {
    event->accept();
    if (userExpandable_ && (event->button() == Qt::LeftButton)) {
        setExpanded(!isExpanded());
        emit toggled(isExpanded());
    }
}   

void ExpandableHeaderButton::showLODControls() {
    lodControlSetHidden_->show();
    lodControlSetVisible_->show();
}

void ExpandableHeaderButton::hideLODControls() {
    lodControlSetHidden_->hide();
    lodControlSetVisible_->hide();
}

void ExpandableHeaderButton::setExpanded(bool expanded) {
    expanded_ = expanded;
    updateState();
}

bool ExpandableHeaderButton::isExpanded() const {
    return expanded_;
}

void ExpandableHeaderButton::updateState() {
    if (expanded_)
        expandLabel_->setPixmap(QPixmap(":/icons/expand1.png"));
    else
        expandLabel_->setPixmap(QPixmap(":/icons/expand0.png"));

    expandLabel_->setVisible(userExpandable_);
}

void ExpandableHeaderButton::setUserExpandable(bool userExpandable) {
    userExpandable_ = userExpandable;
    updateState();
}

bool ExpandableHeaderButton::userExpandable() const {
    return userExpandable_;
}

void ExpandableHeaderButton::updateNameLabel(const std::string& name) {
    rendName_->setText(name.c_str());
}


} // namespace
