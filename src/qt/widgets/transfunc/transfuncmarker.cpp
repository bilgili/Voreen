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

//
// NOTE: This file is currently not used.
//
#include <QtGui>

#include "voreen/qt/widgets/transfermarker.h"

TransferMarker::TransferMarker(QWidget *parent) : QWidget(parent) {
}

void TransferMarker::setColor(QColor color) {
	color_ = color;
}

QColor TransferMarker::getColor() {
	return color_;
}

void TransferMarker::setTransparency(unsigned int transparency) {
	transparency_ = transparency;
	updateToolTip();
}

unsigned int TransferMarker::getTransparency() {
	return transparency_;
}

void TransferMarker::setIntensity(unsigned int intensity) {
	intensity_ = intensity;
	updateToolTip();
}

unsigned int TransferMarker::getIntensity() {
	return intensity_;
}

void TransferMarker::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int markerSize_ = 15;
    painter.setPen(Qt::black);
    painter.setBrush(color_);
    painter.drawEllipse(QRect(QPoint(0,0), QPoint(markerSize_, markerSize_)));
}

void TransferMarker::updateToolTip() {
	QString toolTip = "(";
	toolTip.append(QString::number(intensity_));
	toolTip.append(",");
	toolTip.append(QString::number(transparency_));
	toolTip.append(")");
	setToolTip(toolTip);
}
} // namespace voreen

