/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "portarrowgraphicsitem.h"

#include "portgraphicsitem.h"
#include "rootgraphicsitem.h"
#include "voreen/core/ports/coprocessorport.h"
#include "arrowheadselectiongraphicsitem.h"

#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>

namespace {
    const QColor transparentColor(80, 80, 80, 100);

    const qreal eventRadius = 15.f;

    qreal distance(const QPointF& p1, const QPointF& p2) {
        qreal a = p1.x() - p2.x();
        qreal b = p1.y() - p2.y();
        return sqrt(a*a + b*b);
    }
}

namespace voreen {

PortArrowGraphicsItem::PortArrowGraphicsItem(PortGraphicsItem* sourceItem)
    : ArrowGraphicsItem(sourceItem, 0)
    , oldDestinationItem_(0)
{
    if (!dynamic_cast<CoProcessorPort*>(sourceItem->getPort()))
        destinationHeadDirection_ = ArrowHeadDirectionNS;

    if (sourceItem->getPort()->isLoopPort())
        setNormalColor(Qt::lightGray);
}

int PortArrowGraphicsItem::type() const {
    return Type;
}

QPainterPath PortArrowGraphicsItem::shape() const {
    QPointF s = getSourcePoint();
    QPointF d = getDestinationPoint();

    QPainterPath path(s);

    PortGraphicsItem* portItem = getSourceItem();

    qreal defl = fabs(s.y() - d.y()) / 2.f + fabs(s.x() - d.x()) / 2.0;
    if (dynamic_cast<CoProcessorPort*>(portItem->getPort())) {
        path.cubicTo(s + QPointF(defl, 0),
                     d - QPointF(defl, 0),
                     d);
        path.cubicTo(d - QPointF(defl, 0),
                     s + QPointF(defl, 0),
                     s);
    }
    else {
        Port* sourcePort = getSourceItem()->getPort();

        if (sourcePort->isLoopPort()) {
            path.lineTo(s + QPointF(0.0, 5.0));
            path.cubicTo(s + QPointF(defl, defl + 5.0),
                         d + QPointF(defl, -defl + 5.0),
                         d - QPointF(0, arrowHeadSize_ + 5.0));
            path.lineTo(d);
            path.lineTo(d - QPointF(0, arrowHeadSize_ + 5.0));
            path.cubicTo(d + QPointF(defl, -defl + 5.0),
                         s + QPointF(defl, defl + 5.0),
                         s + QPointF(0.0, 5.0));
            path.lineTo(s);
        }
        else {
            path.cubicTo(s + QPointF(0, defl/2.0),
                         d - QPointF(0, defl/2.0),
                         d - QPointF(0, arrowHeadSize_));
            //some space for the arrow's head
            path.lineTo(d);
            path.lineTo(d - QPointF(0, arrowHeadSize_));
            path.cubicTo(d - QPointF(0, defl/2.0),
                         s + QPointF(0, defl/2.0),
                         s);
        }
    }
    return path;

}

QGraphicsItem* PortArrowGraphicsItem::tooltip() const {
    PortGraphicsItem* portItem = getSourceItem();
    return portItem->tooltip();
}

QPointF PortArrowGraphicsItem::getSourcePoint() const {
    QPointF s = ArrowGraphicsItem::getSourcePoint();

    if (!dynamic_cast<CoProcessorPort*>(getSourceItem()->getPort())) {
        s.rx() += sourceItem_->boundingRect().width() / 2.0;
        s.ry() += sourceItem_->boundingRect().height();
    } else {
        s.rx() += sourceItem_->boundingRect().width();
        s.ry() += sourceItem_->boundingRect().height() / 2.0;
    }

    return s;
}

QPointF PortArrowGraphicsItem::getDestinationPoint() const {
    if (!destinationItem_)
        return destinationPoint_;

    QPointF d = ArrowGraphicsItem::getDestinationPoint();

    if (!dynamic_cast<CoProcessorPort*>(getDestinationItem()->getPort()))
        d.rx() += destinationItem_->boundingRect().width() / 2.0;
    else
        d.ry() += destinationItem_->boundingRect().height() / 2.0;

    return d;
}

PortGraphicsItem* PortArrowGraphicsItem::getSourceItem() const {
    PortGraphicsItem* i = static_cast<PortGraphicsItem*>(sourceItem_);
    tgtAssert(i, "sourceItem was of wrong type");
    return i;
}

PortGraphicsItem* PortArrowGraphicsItem::getDestinationItem() const {
    if (destinationItem_) {
        PortGraphicsItem* i = static_cast<PortGraphicsItem*>(destinationItem_);
        tgtAssert(i, "destinationItem was of wrong type");
        return i;
    }
    else
        return 0;
}

PortGraphicsItem* PortArrowGraphicsItem::getOldPortGraphicsItem() const {
    return oldDestinationItem_;
}

void PortArrowGraphicsItem::setLayer(NetworkEditorLayer layer) {
    switch (layer) {
    case NetworkEditorLayerDataflow:
        setFlag(ItemIsSelectable);
        setAcceptsHoverEvents(true);

        if (sourceSelectionItem_) {
            sourceSelectionItem_->setFlag(ItemIsSelectable);
            sourceSelectionItem_->setAcceptHoverEvents(true);
        }
        if (destinationSelectionItem_) {
            destinationSelectionItem_->setFlag(ItemIsSelectable);
            destinationSelectionItem_->setAcceptHoverEvents(true);
        }

        if (getSourceItem()->getPort()->isLoopPort())
            setNormalColor(Qt::lightGray);
        else
            setNormalColor(Qt::black);
        break;
    case NetworkEditorLayerLinking:
        setFlag(ItemIsSelectable, false);
        setAcceptsHoverEvents(false);

        if (sourceSelectionItem_) {
            sourceSelectionItem_->setFlag(ItemIsSelectable, false);
            sourceSelectionItem_->setAcceptHoverEvents(false);
        }
        if (destinationSelectionItem_) {
            destinationSelectionItem_->setFlag(ItemIsSelectable,false );
            destinationSelectionItem_->setAcceptHoverEvents(false);
        }

        setNormalColor(transparentColor);
        break;
    default:
        tgtAssert(false, "shouldn't get here");
    }
}

void PortArrowGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    clickedPoint_ = event->scenePos();
    movedAwayInEvent_ = false;
    emit getSourceItem()->startedArrow();
    QGraphicsItem::mousePressEvent(event);
}

void PortArrowGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (!movedAwayInEvent_) {
        if (distance(event->scenePos(), clickedPoint_) > eventRadius)
            movedAwayInEvent_ = true;
    }

    if (movedAwayInEvent_) {
        getSourceItem()->setCurrentArrow(this);
        if (getDestinationItem())
            oldDestinationItem_ = getDestinationItem();
        getSourceItem()->mouseMoveEvent(event);
    }
    else
        QGraphicsItem::mouseMoveEvent(event);
}

void PortArrowGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (movedAwayInEvent_)
        getSourceItem()->mouseReleaseEvent(event);
    else
        QGraphicsItem::mouseReleaseEvent(event);
}

} // namespace
