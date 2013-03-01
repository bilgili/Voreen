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

#include "voreen/qt/widgets/animation/currentframegraphicsitem.h"
#include "voreen/qt/widgets/animation/propertytimelineview.h"

#include <QPainter>
#include <QPen>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <iostream>

namespace voreen {

    CurrentFrameGraphicsItem::CurrentFrameGraphicsItem(bool movable, bool selectable)
        : QGraphicsItem()
    {
    if (movable)
        setFlag(ItemIsMovable);
    if (selectable)
        setFlag(ItemIsSelectable);
    currentFrameWidget_ = new QWidget();
    currentFrameLabel_ = new QLabel(currentFrameWidget_);
    //currentFrameWidget_->hide();
    QHBoxLayout* currentLayout = new QHBoxLayout(currentFrameWidget_);
    currentLayout->addWidget(currentFrameLabel_);
    setAcceptsHoverEvents(true);
    setVisible(true);
}

void CurrentFrameGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*  /*option*/, QWidget* /*widget*/) {
    QRectF itemRect(boundingRect().x()+1, boundingRect().y()+1, boundingRect().width()-1, boundingRect().height()-1);
    QLinearGradient gradient(0,0,0, itemRect.height() );
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, Qt::green);
    gradient.setColorAt(1.0, Qt::green);

    QBrush brush(gradient);
    painter->setBrush(gradient);
    painter->setOpacity(0.5);
    painter->drawRect(itemRect);
    setVisible(true);
}

QRectF CurrentFrameGraphicsItem::boundingRect() const {
    return QRectF(-3, -10, 6, 35 );
}
void CurrentFrameGraphicsItem::mousePressEvent ( QGraphicsSceneMouseEvent * event ) {
    if(flags() == QGraphicsItem::ItemIsMovable) {
        setPos(event->scenePos().x(), 20);
        QGraphicsItem::mousePressEvent(event);
    }
}

void CurrentFrameGraphicsItem::mouseMoveEvent ( QGraphicsSceneMouseEvent* event ) {
    if(flags() == QGraphicsItem::ItemIsMovable) {
        setPos(event->scenePos().x(), 20);
    }
}

void CurrentFrameGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if(flags() == QGraphicsItem::ItemIsMovable) {
        setPos(event->scenePos().x(), 20);
    }
}

} // namespace voreen
