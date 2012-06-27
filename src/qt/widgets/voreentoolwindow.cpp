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

#include "voreen/qt/widgets/voreentoolwindow.h"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QPainter>
#include <iostream>

namespace voreen {

VoreenToolWindow::VoreenToolWindow(QAction* action, QWidget* parent, QWidget* child, const QString& name)
	: QWidget(parent, Qt::Tool),
      action_(action)
{
	setWindowTitle(action->text());
	setWindowIcon(action->icon());
	setObjectName(name);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(child);
	setLayout(layout);

	// connection action and widget visibility
	connect(action, SIGNAL(toggled(bool)), this, SLOT(setVisible(bool)));
	connect(this, SIGNAL(visibilityChanged(bool)), action, SLOT(setChecked(bool)));
}

void VoreenToolWindow::hideEvent(QHideEvent* event) {
	QWidget::hideEvent(event);
	if (!isVisible())
		emit visibilityChanged(false);
}

void VoreenToolWindow::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);
	if (isVisible())
		emit visibilityChanged(true);
}

//----------------------------------------------------------------------------------------------------------------

VoreenToolDockWindowTitle::VoreenToolDockWindowTitle(QDockWidget *parent)
	: QWidget(parent)
{
	closeButton_ = QPixmap(":/widgetstyle/closebutton.png");
	undockButton_ = QPixmap(":/widgetstyle/undockbutton.png");
}

QSize VoreenToolDockWindowTitle::sizeHint() const {
	return QSize(40,20);
}

QSize VoreenToolDockWindowTitle::minimumSizeHint() const {
	return sizeHint();
}

void VoreenToolDockWindowTitle::paintEvent(QPaintEvent* /*e*/) {
	QRect button_rect = rect();
	QPainter painter(this);

	QColor button_color = QColor(50, 50, 50);
	QColor m_shadow = QColor(Qt::black);
	float roundness = 0.0;

    // outline
	painter.setPen(QPen(QBrush(Qt::black), 2.0));
	QPainterPath outline;
	outline.addRoundRect(0, 0, button_rect.width(), button_rect.height(), roundness, roundness);
	painter.setOpacity(1.0);
	painter.drawPath(outline);

	// gradient
	QLinearGradient gradient(0, 0, 0, button_rect.height());
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
	painter_path.addRoundRect(1, 1, button_rect.width() - 2, button_rect.height() - 2, roundness, roundness);
	painter.setClipPath(painter_path);

	painter.setOpacity(1.0);
	painter.drawRoundRect(1, 1, button_rect.width() - 2, button_rect.height() - 2, roundness, roundness);

	// glass highlight
	painter.setBrush(QBrush(Qt::white));
	painter.setPen(QPen(QBrush(Qt::white), 0.01));
	painter.setOpacity(0.30);
	painter.drawRect(1, 1, button_rect.width() - 2, (button_rect.height() / 2) - 1);

	// text
	QDockWidget* dockWidget = qobject_cast<QDockWidget*>(parentWidget());
	QString text = dockWidget->windowTitle();
	if (!text.isEmpty()) {
		painter.setFont(font());
		painter.setPen(Qt::white);
		painter.setOpacity(1.0);

        QRect text_rect = button_rect;
        text_rect.setLeft(8);
        text_rect.translate(0, 1);
        painter.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }

	// draw logos
	QRect logoPos = button_rect;
	logoPos.setLeft(button_rect.right() - 32);
	logoPos.setTop(button_rect.top() + 4);
	painter.drawPixmap(logoPos.topLeft(), undockButton_);
	logoPos = button_rect;
	logoPos.setLeft(button_rect.right() - 16);
	logoPos.setTop(button_rect.top() + 4);
	painter.drawPixmap(logoPos.topLeft(), closeButton_);

}

void VoreenToolDockWindowTitle::mousePressEvent(QMouseEvent *event) {
	QPoint pos = event->pos();
	QRect rect = this->rect();

	QRect buttonRectClose = QRect(rect.right()-16, rect.top()+4, 14, 14);
	QRect buttonRectUndock = QRect(rect.right()-32, rect.top()+4, 14, 14);
	if (buttonRectClose.contains(pos)) {
		event->accept();
		QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
		dw->close();
	} else if (buttonRectUndock.contains(pos)) {
		event->accept();
		QDockWidget *dw = qobject_cast<QDockWidget*>(parentWidget());
		dw->setFloating(!dw->isFloating());
	} else
		event->ignore();
}

//----------------------------------------------------------------------------------------------------------------

VoreenToolDockWindow::VoreenToolDockWindow(QAction* action, QWidget* parent, QWidget* child, const QString& name)
	: QDockWidget(name, parent)
{
	setWindowTitle(action->text());
	setWindowIcon(action->icon());
	setObjectName(name);
	setWindowFlags(Qt::FramelessWindowHint);

	setWidget(child);

    // if we have a stylesheet we want the fancy window title, please
    if (!qApp->styleSheet().isEmpty())
        setTitleBarWidget(new VoreenToolDockWindowTitle(this));

	connect(action, SIGNAL(toggled(bool)), this, SLOT(setVisible(bool)));
	connect(this, SIGNAL(visibilityChanged(bool)), action, SLOT(setChecked(bool)));
}

void VoreenToolDockWindow::showEvent(QShowEvent* event) {
	QDockWidget::showEvent(event);
	if (isVisible())
		emit visibilityChanged(true);
}

void VoreenToolDockWindow::hideEvent(QHideEvent* event) {
	QDockWidget::hideEvent(event);
	if (!isVisible())
		emit visibilityChanged(false);
}

} // namespace
