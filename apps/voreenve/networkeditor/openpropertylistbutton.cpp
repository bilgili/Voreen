/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "openpropertylistbutton.h"

#include "networkeditor_common.h"
#include "rootgraphicsitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace {
    QPen borderPen = QPen(Qt::black, 1);

    // color if the parent has a link attached to it
    const QColor linkExistsBackgroundColor = QColor(255,0,0,128);
    // color if the parent has a link and the mouse cursor is hovering above this item
    const QColor linkExistsHoverColor = QColor(255,100,100);
    // the normal background color
    const QColor backgroundColor = QColor(255,255,255,128);
    // the normal hovering color
    const QColor hoverColor = QColor(150,100,100,128);
    // the color if the button is pressed
    const QColor sunkenColor = QColor(170,100,100,128);
    // the color used for the glossy effect
    const QColor glossColor = QColor(255,255,255,128);

    // size of the OpenPropertyListButton
    const QSize size = QSize(10, 10);
}

namespace voreen {

OpenPropertyListButton::OpenPropertyListButton(QGraphicsItem* parent)
#if (QT_VERSION >= 0x040600)
    : QGraphicsObject(parent)
#else
    : QObject()
    , QGraphicsItem(parent)
#endif
{
    setZValue(ZValueOpenPropertyListButton);
    setAcceptsHoverEvents(true);
    setToolTip(tr("show/hide list of properties"));
}

int OpenPropertyListButton::type() const {
    return Type;
}

QRectF OpenPropertyListButton::boundingRect() const {
    return QRect(0, 0, size.width(), size.height());
}

QPointF OpenPropertyListButton::dockingPoint() const {
    QRectF bounds = boundingRect();
    qreal centerX = bounds.x() + bounds.width() / 2.f;
    qreal centerY = bounds.y() + bounds.height() / 2.f;
    return mapToScene(QPointF(centerX, centerY));
}

void OpenPropertyListButton::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    // add gloss effect
    QRectF glossArea = boundingRect();
    glossArea.setLeft(glossArea.left() + glossArea.width() * 0.05);
    glossArea.setHeight(glossArea.height() * 0.5);
    glossArea.setWidth(glossArea.width() * 0.9);
    painter->setPen(QPen(QColor(0,0,0,0), 0));
    painter->setBrush(glossColor);
    painter->drawEllipse(glossArea);

    painter->setPen(borderPen);
    painter->setBrush(backgroundColor);

    if (option->state & QStyle::State_Sunken)
        painter->setBrush(sunkenColor);
    if (option->state & QStyle::State_MouseOver)
        painter->setBrush(hoverColor);

    painter->drawEllipse(boundingRect());
}

void OpenPropertyListButton::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton)
        emit pressed();
    else
        QGraphicsItem::mousePressEvent(event);
}

void OpenPropertyListButton::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    RootGraphicsItem* parent = static_cast<RootGraphicsItem*>(parentItem());
    parent->mouseMoveEvent(event);
}

void OpenPropertyListButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    RootGraphicsItem* parent = static_cast<RootGraphicsItem*>(parentItem());
    parent->mouseReleaseEvent(event);
}

} // namespace voreen
