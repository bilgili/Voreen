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

#include "voreen/qt/widgets/voreentoolbar.h"
#include "voreen/qt/widgets/voreenbutton.h"

#include <QBoxLayout>

namespace voreen {

VoreenToolBar::VoreenToolBar(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
      dragging_(false)
{
    logo_ = QPixmap(":/widgetstyle/logo.png");

    QPalette newPalette;
    newPalette.setBrush(QPalette::Window, QBrush(Qt::black));
    setPalette(newPalette);
    setBackgroundRole(QPalette::Window);

    setWindowTitle(tr("Voreen Toolbar"));
    setObjectName("VoreenToolBar");
    setAttribute(Qt::WA_AlwaysShowToolTips);

    QHBoxLayout* contentLayout = new QHBoxLayout();
    setLayout(contentLayout);
    contentLayout->setSizeConstraint(QLayout::SetFixedSize);

    // spacer for the logo and toolbar height
    QSpacerItem* spacer = new QSpacerItem(60, 46, QSizePolicy::Fixed, QSizePolicy::Fixed);

    // check buttons
    buttonLayout_ = new QHBoxLayout();
    buttonLayout_->setSpacing(0);

    // action buttons
    toolLayout_ = new QHBoxLayout();
    toolLayout_->setSpacing(0);

    // fit everything together and leave some space for the logo
    contentLayout->addItem(spacer);
    contentLayout->addLayout(toolLayout_);
    contentLayout->addLayout(buttonLayout_);
}

void VoreenToolBar::registerToolWindow(VoreenToolWindow* toolWindow) {
    VoreenButton* button = new VoreenButton(toolWindow->action()->icon());

    button->setCheckable(true);
    button->setToolTip(toolWindow->action()->text());
    buttonLayout_->addWidget(button);

    // connect button and action
    connect(button, SIGNAL(toggled(bool)), toolWindow->action(), SLOT(setChecked(bool)));
    connect(toolWindow->action(), SIGNAL(toggled(bool)), button, SLOT(setChecked(bool)));
}

void VoreenToolBar::addToolButtonAction(QAction* action){
    VoreenButton* button = new VoreenButton(action->icon());
    button->setToolTip(action->toolTip());
    toolLayout_->addWidget(button);
    connect(button, SIGNAL(clicked()), action, SLOT(trigger()));
}

void VoreenToolBar::paintEvent(QPaintEvent* /*e*/) {
    QPainter painter(this);

    QColor button_color = QColor(50, 50, 50);
    QColor m_shadow = QColor(Qt::black);

    // outline
    painter.setPen(QPen(QBrush(Qt::black), 2.0));
    QPainterPath outline;
    outline.addRect(0, 0, rect().width(), rect().height());
    painter.setOpacity(1.0);
    painter.drawPath(outline);

    // gradient
    QLinearGradient gradient(0, 0, 0, rect().height());
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, button_color);
    gradient.setColorAt(0.4, m_shadow);
    gradient.setColorAt(0.6, m_shadow);
    gradient.setColorAt(1.0, button_color);

    QBrush brush(gradient);
    painter.setBrush(brush);
    painter.setPen(QPen(QBrush(button_color), 2.0));

    // main button
    QPainterPath painter_path;
    painter_path.addRect(1, 1, rect().width() - 2, rect().height() - 2);
    painter.setClipPath(painter_path);

    painter.setOpacity(1.0);
    painter.drawRect(1, 1, rect().width() - 2, rect().height() - 2);

    // draw logo
    QRect logoPos = rect();
    float logoOffset = (rect().height() - logo_.height()) / 2.0;
    logoPos.setLeft(static_cast<int>(logoPos.left() + logoOffset));
    logoPos.setTop(static_cast<int>(logoPos.top() + logoOffset));
    painter.drawPixmap(logoPos.topLeft(), logo_);

    // glass highlight
    painter.setBrush(QBrush(Qt::white));
    painter.setPen(QPen(QBrush(Qt::white), 0.01));
    painter.setOpacity(0.30);
    painter.drawRect(1, 1, rect().width() - 2, (rect().height() / 2) - 2);
}

void VoreenToolBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragPosition_ = event->globalPos() - frameGeometry().topLeft();
        event->accept();
        dragging_ = true;
    }
}

void VoreenToolBar::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton)
        dragging_ = false;
}

void VoreenToolBar::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_ && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - dragPosition_);
        event->accept();
    }
}

void VoreenToolBar::closeEvent(QCloseEvent* event) {
    //TODO: send close event to parent window
    event->ignore();
}

} // namespace
