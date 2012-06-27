/**********************************************************************
*                                                                    *
* Voreen - The Volume Rendering Engine                               *
*                                                                    *
* Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/qt/widgets/voreenbutton.h"

const int VoreenButton::BUTTON_SIZE = 30;

//TODO: transfer this to a Qt style

VoreenButton::VoreenButton(QWidget * parent) 
  : QToolButton(parent), 
    hovered_(false), 
    pressed_(false), 
    color_(Qt::gray),
    highlightColor_(Qt::lightGray),
    shadowColor_(Qt::black),
    opacity_(1.0),
    roundness_(0)
{
    setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    setFocusPolicy(Qt::NoFocus);
}

VoreenButton::VoreenButton(const QString & text, QWidget * parent) 
  : QToolButton(parent), 
    hovered_(false), 
    pressed_(false), 
    color_(Qt::gray),
    highlightColor_(Qt::lightGray),
    shadowColor_(Qt::black),
    opacity_(1.0),
    roundness_(0)
{
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    setText(text);
}

VoreenButton::VoreenButton(const QIcon & icon, const QString & text, QWidget * parent) 
  : QToolButton(parent), 
    hovered_(false), 
    pressed_(false), 
    color_(Qt::gray),
    highlightColor_(Qt::lightGray),
    shadowColor_(Qt::black),
    opacity_(1.0),
    roundness_(0)
{
    setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    setIcon(icon);
    setText(text);
}

void VoreenButton::paintEvent(QPaintEvent* /*pe*/) {
	QPainter painter(this);  
	painter.setRenderHint(QPainter::Antialiasing);

	//test for state changes
	QColor button_color;
	if (isEnabled()) {
		hovered_ ? button_color = highlightColor_ : button_color = color_;

		if (pressed_)
			button_color = highlightColor_.darker(250);
	} else {
		button_color = QColor(50, 50, 50);
	}

	if (isChecked())
		button_color = QColor(80, 30, 30);;//QColor(102, 77, 0);

	QRect button_rect = geometry();

	//outline
	painter.setPen(QPen(QBrush(Qt::black), 2.0));
	QPainterPath outline;
	outline.addRoundRect(0, 0, button_rect.width(), button_rect.height(), roundness_, roundness_);
	painter.setOpacity(opacity_);
	painter.drawPath(outline);

	//gradient
	QLinearGradient gradient(0, 0, 0, button_rect.height());
	gradient.setSpread(QGradient::ReflectSpread);
	gradient.setColorAt(0.0, button_color);
	gradient.setColorAt(0.4, shadowColor_);
	gradient.setColorAt(0.6, shadowColor_);
	gradient.setColorAt(1.0, button_color);

	QBrush brush(gradient);
	painter.setBrush(brush); 
	painter.setPen(QPen(QBrush(button_color), 2.0));

	//main button
	QPainterPath painter_path;
	painter_path.addRoundRect(1, 1, button_rect.width() - 2, button_rect.height() - 2, roundness_, roundness_);
	painter.setClipPath(painter_path);

	painter.setOpacity(opacity_);
	painter.drawRoundRect(1, 1, button_rect.width() - 2, button_rect.height() - 2, roundness_, roundness_);

	//glass highlight
	painter.setBrush(QBrush(Qt::white));
	painter.setPen(QPen(QBrush(Qt::white), 0.01));
	painter.setOpacity(0.30);
	painter.drawRect(1, 1, button_rect.width() - 2, (button_rect.height() / 2) - 2);

	//text
	if (!text().isNull()) {
		painter.setFont(font());
		painter.setPen(Qt::white);
		painter.setOpacity(1.0);
		painter.drawText(0, 0, button_rect.width(), button_rect.height(), Qt::AlignCenter, text());
	}

	//icon
	if (!icon().isNull()) {
		QSize icon_size = iconSize();
		QRect icon_position = calculateIconPosition(button_rect, icon_size);
		painter.setOpacity(1.0);
		painter.drawPixmap(icon_position, QPixmap(icon().pixmap(icon_size)));
	}
}

void VoreenButton::enterEvent(QEvent* e) {
	hovered_ = true;
    repaint();

	QToolButton::enterEvent(e);
}

void VoreenButton::leaveEvent(QEvent* e) {
	hovered_ = false;
	repaint();

	QToolButton::leaveEvent(e);
}

void VoreenButton::mousePressEvent(QMouseEvent* e) {
	pressed_ = true;
	repaint();

	QToolButton::mousePressEvent(e);
}

void VoreenButton::mouseReleaseEvent(QMouseEvent* e) {
	pressed_ = false;
	repaint();

	QToolButton::mouseReleaseEvent(e);
}

QRect VoreenButton::calculateIconPosition(QRect button_rect, QSize icon_size) {
	int x = (button_rect.width() / 2) - (icon_size.width() / 2);
	int y = (button_rect.height() / 2) - (icon_size.height() / 2);
	int width = icon_size.width(); 
	int height = icon_size.height();

	QRect icon_position;
	icon_position.setX(x);
	icon_position.setY(y);
	icon_position.setWidth(width);
	icon_position.setHeight(height);

	return icon_position;
}
