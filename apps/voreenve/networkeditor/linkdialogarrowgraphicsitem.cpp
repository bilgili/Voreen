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

#include "linkdialogarrowgraphicsitem.h"
#include "linkdialogpropertygraphicsitem.h"

#include <QGraphicsSceneMouseEvent>

namespace {
    const qreal eventRadius = 15.f;

    qreal distance(const QPointF& p1, const QPointF& p2) {
        qreal a = p1.x() - p2.x();
        qreal b = p1.y() - p2.y();
        return sqrt(a*a + b*b);
    }
}

namespace voreen {

LinkDialogArrowGraphicsItem::LinkDialogArrowGraphicsItem(LinkDialogPropertyGraphicsItem* source, LinkDialogPropertyGraphicsItem* destination, bool bidirectional)
    : ArrowGraphicsItem(source, destination)
    , bidirectional_(bidirectional)
    , movedAwayInEvent_(false)
{
    tgtAssert(source, "passed null pointer");

    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);

    if (destination)
        setDestinationItem(destination);
}

int LinkDialogArrowGraphicsItem::type() const {
    return Type;
}

LinkDialogPropertyGraphicsItem* LinkDialogArrowGraphicsItem::getSourceItem() const {
    LinkDialogPropertyGraphicsItem* i = static_cast<LinkDialogPropertyGraphicsItem*>(sourceItem_);
    tgtAssert(i, "sourceItem was of wrong type");
    return i;
}

LinkDialogPropertyGraphicsItem* LinkDialogArrowGraphicsItem::getDestinationItem() const {
    if (destinationItem_) {
        LinkDialogPropertyGraphicsItem* i = static_cast<LinkDialogPropertyGraphicsItem*>(destinationItem_);
        tgtAssert(i, "destinationItem was of wrong type");
        return i;
    }
    else
        return 0;
}

void LinkDialogArrowGraphicsItem::setDestinationItem(LinkDialogPropertyGraphicsItem* dest) {
    tgtAssert(dest, "passed null pointer");
    prepareGeometryChange();
    destinationItem_ = dest;
}

void LinkDialogArrowGraphicsItem::setBidirectional(bool bidirectional) {
    bidirectional_ = bidirectional;
}

QPointF LinkDialogArrowGraphicsItem::getSourcePoint() const {
    LinkDialogPropertyGraphicsItem* propItem = getSourceItem();
    switch (propItem->getPosition()) {
    case ColumnPositionLeft:
            return propItem->getRightDockingPoint();
        case ColumnPositionCenter:
        case ColumnPositionRight:
            return propItem->getLeftDockingPoint();
        default:
            tgtAssert(false, "shouldn't get here");
    }
    return ArrowGraphicsItem::getSourcePoint();
}

QPointF LinkDialogArrowGraphicsItem::getDestinationPoint() const {
    if (destinationItem_) {
        LinkDialogPropertyGraphicsItem* propItem = getDestinationItem();
        switch (propItem->getPosition()) {
        case ColumnPositionLeft:
            return propItem->getRightDockingPoint();
        case ColumnPositionCenter:
        case ColumnPositionRight:
            return propItem->getLeftDockingPoint();
        default:
            tgtAssert(false, "shouldn't get here");
            return ArrowGraphicsItem::getDestinationPoint();
        }
    }
    else
        return destinationPoint_;
}

QPainterPath LinkDialogArrowGraphicsItem::shape() const {
    const QPointF defl = QPointF(150.f, 0.f);
    LinkDialogPropertyGraphicsItem* src = getSourceItem();

    QPainterPath path;
    switch (src->getPosition()) {
    case ColumnPositionLeft:
        path.moveTo(src->getRightDockingPoint());
        path.cubicTo(src->getRightDockingPoint() + defl, getDestinationPoint() - defl, getDestinationPoint());
        path.cubicTo(getDestinationPoint() - defl, src->getRightDockingPoint() + defl, src->getRightDockingPoint());

        destinationHeadDirection_ = ArrowHeadDirectionWE;
        if (bidirectional_)
            sourceHeadDirection_ = ArrowHeadDirectionEW;
        else
            sourceHeadDirection_ = ArrowHeadDirectionNone;

        break;
    case ColumnPositionCenter:
        {
        QPointF srcPoint = src->getLeftDockingPoint();
        QPointF dstPoint = getDestinationPoint();
        path.moveTo(srcPoint);
        path.lineTo(srcPoint - QPointF(10.0, 0.0));
        path.cubicTo(srcPoint - QPointF(50.0, 0.0), dstPoint - QPointF(50.0, 0.0), dstPoint - QPointF(10.0, 0.0));
        path.lineTo(dstPoint);
        path.lineTo(dstPoint - QPointF(10, 0));
        path.cubicTo(dstPoint - QPointF(50.0, 0.0), srcPoint - QPointF(50.0, 0.0), srcPoint - QPointF(10.0, 0.0));
        path.lineTo(srcPoint);

        destinationHeadDirection_ = ArrowHeadDirectionWE;
        if (bidirectional_)
            sourceHeadDirection_ = ArrowHeadDirectionWE;
        else
            sourceHeadDirection_ = ArrowHeadDirectionNone;
        }
        break;
    case ColumnPositionRight:
        path.moveTo(src->getLeftDockingPoint());
        path.cubicTo(src->getLeftDockingPoint() - defl, getDestinationPoint() + defl, getDestinationPoint());
        path.cubicTo(getDestinationPoint() + defl, src->getLeftDockingPoint() - defl, src->getLeftDockingPoint());

        destinationHeadDirection_ = ArrowHeadDirectionEW;
        if (bidirectional_)
            sourceHeadDirection_ = ArrowHeadDirectionWE;
        else
            sourceHeadDirection_ = ArrowHeadDirectionNone;

        break;
    }
    return path;
}

void LinkDialogArrowGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    clickedPoint_ = event->scenePos();
    movedAwayInEvent_ = false;
}

void LinkDialogArrowGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (!movedAwayInEvent_) {
        if (distance(event->scenePos(), clickedPoint_) > eventRadius)
            movedAwayInEvent_ = true;
    }

    if (movedAwayInEvent_) {
        getSourceItem()->setCurrentArrow(this);
        getSourceItem()->mouseMoveEvent(event);
    }
    else
        QGraphicsItem::mouseMoveEvent(event);
}

void LinkDialogArrowGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (movedAwayInEvent_)
        getSourceItem()->mouseReleaseEvent(event);
    else
        QGraphicsItem::mouseReleaseEvent(event);
}

} // namespace
