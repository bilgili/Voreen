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

#include "voreen/qt/widgets/network/editor/openpropertylistbutton.h"

#include "voreen/qt/voreenqtglobal.h"
#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace {
    QPen borderPen = QPen(Qt::black, 1);
    QColor linkExistsBackgroundColor = QColor(255,0,0,128);
    QColor linkExistsHoverColor = QColor(255,100,100);
    QColor backgroundColor = QColor(255,255,255,128);
    QColor hoverColor = QColor(150,100,100,128);
    QColor sunkenColor = QColor(170,100,100,128);
    QColor glossColor = QColor(255,255,255,128);

    QSize size = QSize(10, 10);
}

namespace voreen {

OpenPropertyListButton::OpenPropertyListButton(ProcessorGraphicsItem* parent)
    : QObject()
    , QGraphicsItem(parent)
    , processorGraphicsItem_(parent)
    , checked_(false)
{
    setZValue(ZValueOpenPropertyListButton);
    setAcceptsHoverEvents(true);
    setToolTip(tr("show/hide list of properties"));
}

QRectF OpenPropertyListButton::boundingRect() const {
    return QRect(0,0,size.width(),size.height());
}

QPointF OpenPropertyListButton::dockingPoint() const {
    QRectF bounds = boundingRect();
    qreal centerX = bounds.x() + bounds.width() / 2.f;
    qreal centerY = bounds.y() + bounds.height() / 2.f;
    return mapToScene(QPointF(centerX, centerY));
}

void OpenPropertyListButton::setChecked(bool value) {
    checked_ = value;
    update();
}

void OpenPropertyListButton::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    // add gloss effect
    QRectF glossArea = boundingRect();
    glossArea.setLeft(glossArea.left()+glossArea.width()*0.05);
    glossArea.setHeight(glossArea.height()*0.5);
    glossArea.setWidth(glossArea.width()*0.9);
    painter->setPen(QPen(QColor(0,0,0,0), 0));
    painter->setBrush(glossColor);
    painter->drawEllipse(glossArea);

    painter->setPen(borderPen);
    if (checked_) {
        if (option->state & QStyle::State_MouseOver)
            painter->setBrush(linkExistsHoverColor);
        else
            painter->setBrush(linkExistsBackgroundColor);
    }
    else {
        painter->setBrush(backgroundColor);

        if (option->state & QStyle::State_Sunken)
            painter->setBrush(sunkenColor);
        if (option->state & QStyle::State_MouseOver)
            painter->setBrush(hoverColor);
    }

    painter->drawEllipse(boundingRect());
}

void OpenPropertyListButton::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton)
        emit pressed();
    QGraphicsItem::mousePressEvent(event);
}

} // namespace voreen
