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

#include "linkarrowgraphicsitem.h"

#include "processorgraphicsitem.h"
#include "propertygraphicsitem.h"

namespace voreen {

LinkArrowGraphicsItem::LinkArrowGraphicsItem(PropertyGraphicsItem* sourceItem, PropertyGraphicsItem* destinationItem)
    : ArrowGraphicsItem(sourceItem, destinationItem)
    , showSourceArrowHead_(false)
    , showDestinationArrowHead_(true)
{
    setZValue(ZValueLinkArrowGraphicsItem);
    setNormalColor(Qt::lightGray);
}

int LinkArrowGraphicsItem::type() const {
    return Type;
}

void LinkArrowGraphicsItem::showSourceArrowHead(bool visible) {
    showSourceArrowHead_ = visible;
}

void LinkArrowGraphicsItem::showDestinationArrowHead(bool visible) {
    showDestinationArrowHead_ = visible;
}

PropertyGraphicsItem* LinkArrowGraphicsItem::getSourceItem() const {
    PropertyGraphicsItem* i = static_cast<PropertyGraphicsItem*>(sourceItem_);
    tgtAssert(i, "sourceItem was of wrong type");
    return i;
}

PropertyGraphicsItem* LinkArrowGraphicsItem::getDestinationItem() const {
    if (destinationItem_) {
        PropertyGraphicsItem* i = static_cast<PropertyGraphicsItem*>(destinationItem_);
        tgtAssert(i, "destinationItem was of wrong type");
        return i;
    }
    else
        return 0;
}

LinkArrowGraphicsItem::LinkArrowState LinkArrowGraphicsItem::getState() const {
    tgtAssert(sourceItem_, "propertyItem is null");
    tgtAssert(destinationItem_, "propertyItem is null");

    if ((sourceItem_->isVisible()) && (destinationItem_->isVisible())) {
        return LinkArrowStatePropertyToProperty;
    } else if ((sourceItem_->isVisible()) && (!destinationItem_->isVisible())) {
        return LinkArrowStatePropertyToProcessor;
    } else if ((!sourceItem_->isVisible()) && (destinationItem_->isVisible())) {
        return LinkArrowStateProcessorToProperty;
    } else {
        return LinkArrowStateProcessorToProcessor;
    }
}

LinkArrowGraphicsItem::DockPositions LinkArrowGraphicsItem::getDockPositions() const {
    QPointF sourcePropLeftPoint = getSourceItem()->getLeftDockingPoint();
    QPointF sourcePropRightPoint = getSourceItem()->getRightDockingPoint();
    QPointF destinationPropLeftPoint = getDestinationItem()->getLeftDockingPoint();
    QPointF destinationPropRightPoint = getDestinationItem()->getRightDockingPoint();

    qreal leftLeft = QLineF(sourcePropLeftPoint, destinationPropLeftPoint).length();
    qreal leftRight = QLineF(sourcePropLeftPoint, destinationPropRightPoint).length();
    qreal rightLeft = QLineF(sourcePropRightPoint, destinationPropLeftPoint).length();
    qreal rightRight = QLineF(sourcePropRightPoint, destinationPropRightPoint).length();

    if ((leftLeft <= leftRight) && (leftLeft <= rightLeft) && (leftLeft <= rightRight)) {
        return DockPositionsLeftLeft;
    }
    else if ((leftRight <= leftLeft) && (leftRight <= rightLeft) && (leftRight <= rightRight)) {
        return DockPositionsLeftRight;
    }
    else if ((rightLeft <= leftLeft) && (rightLeft <= leftRight) && (rightLeft <= rightRight)) {
        return DockPositionsRightLeft;
    }
    else {
        return DockPositionsRightRight;
    }
}

QPointF LinkArrowGraphicsItem::getSourcePoint() const {
    DockPositions pos = getDockPositions();
    if ((pos == DockPositionsLeftLeft) || (pos == DockPositionsLeftRight))
        return getSourceItem()->getLeftDockingPoint();
    else
        return getSourceItem()->getRightDockingPoint();
}

QPointF LinkArrowGraphicsItem::getDestinationPoint() const {
    DockPositions pos = getDockPositions();
    if ((pos == DockPositionsLeftLeft) || (pos == DockPositionsRightLeft))
        return getDestinationItem()->getLeftDockingPoint();
    else
        return getDestinationItem()->getRightDockingPoint();
}

QPainterPath LinkArrowGraphicsItem::shape() const {
    QPointF sourcePoint = getSourcePoint();
    QPointF destinationPoint = getDestinationPoint();
    switch (getState()) {
    case LinkArrowStateProcessorToProcessor:
        if (sourceItem_->parentItem() == destinationItem_->parentItem())
            return QPainterPath();
        else {
            sourcePoint = getSourceItem()->getRootGraphicsItem()->linkArrowDockingPoint();
            destinationPoint = getDestinationItem()->getRootGraphicsItem()->linkArrowDockingPoint();
        }
        break;
    case LinkArrowStateProcessorToProperty:
        sourcePoint = getSourceItem()->getRootGraphicsItem()->linkArrowDockingPoint();
        break;
    case LinkArrowStatePropertyToProcessor:
        destinationPoint = getDestinationItem()->getRootGraphicsItem()->linkArrowDockingPoint();
        break;
    case LinkArrowStatePropertyToProperty:
        break;
    }

    QPainterPath path(sourcePoint);
    qreal defl = fabs(sourcePoint.y() - destinationPoint.y()) / 2.0 + fabs(sourcePoint.x() - destinationPoint.x()) / 2.0;

    if (getState() == LinkArrowStateProcessorToProcessor) {
        path.lineTo(QPointF(sourcePoint.x(), sourcePoint.y() - 10.f));
        path.cubicTo(QPointF(sourcePoint) - QPointF(0, defl / 2.0),
                         QPointF(destinationPoint) - QPointF(0, defl / 2.0),
                         QPointF(destinationPoint) - QPointF(0, 10));
        path.lineTo(destinationPoint);
        path.lineTo(destinationPoint - QPointF(0, 10));
        path.cubicTo( QPointF(destinationPoint) - QPointF(0, defl / 2.0),
                         QPointF(sourcePoint) - QPointF(0, defl / 2.0),
                         QPointF(sourcePoint.x(), sourcePoint.y() - 10.f));
        path.lineTo(sourcePoint);

        sourceHeadDirection_ = ArrowHeadDirectionNone;
        destinationHeadDirection_ = ArrowHeadDirectionNone;

        return path;
    }
    defl = defl / 2.5;

    if (getSourceItem()->getRootGraphicsItem() != getDestinationItem()->getRootGraphicsItem()) {
        switch (getDockPositions()) {
        case DockPositionsLeftLeft:
            path.lineTo(QPointF(sourcePoint.x() - 10, sourcePoint.y()));
            path.cubicTo(QPointF(sourcePoint) - QPointF(defl, 0),
                             QPointF(destinationPoint) - QPointF(defl, 0),
                             QPointF(destinationPoint) - QPointF(10, 0));
            path.lineTo(destinationPoint);
            path.lineTo(QPointF(destinationPoint) - QPointF(10, 0));
            path.cubicTo(QPointF(destinationPoint) - QPointF(defl, 0),
                         QPointF(sourcePoint) - QPointF(defl, 0),
                         QPointF(sourcePoint) - QPointF(10, 0));
            path.lineTo(sourcePoint);
            break;
        case DockPositionsLeftRight:
            path.lineTo(QPointF(sourcePoint.x() - 10, sourcePoint.y()));
            path.cubicTo(QPointF(sourcePoint) - QPointF(defl, 0),
                         QPointF(destinationPoint) + QPointF(defl, 0),
                         QPointF(destinationPoint) + QPointF(10, 0));
            path.lineTo(destinationPoint);
            path.lineTo(QPointF(destinationPoint) + QPointF(10, 0));
            path.cubicTo(QPointF(destinationPoint) + QPointF(defl, 0),
                         QPointF(sourcePoint) - QPointF(defl, 0),
                         QPointF(sourcePoint) - QPointF(10, 0));
            path.lineTo(sourcePoint);
            break;
        case DockPositionsRightLeft:
            path.lineTo(QPointF(sourcePoint.x() + 10, sourcePoint.y()));
            path.cubicTo(QPointF(sourcePoint) + QPointF(defl, 0),
                         QPointF(destinationPoint) - QPointF(defl, 0),
                         QPointF(destinationPoint) - QPointF(10, 0));
            path.lineTo(destinationPoint);
            path.lineTo(QPointF(destinationPoint) - QPointF(10, 0));
            path.cubicTo(QPointF(destinationPoint) - QPointF(defl, 0),
                         QPointF(sourcePoint) + QPointF(defl, 0),
                         QPointF(sourcePoint) + QPointF(10, 0));
            path.lineTo(sourcePoint);
            break;
        case DockPositionsRightRight:
            path.lineTo(QPointF(sourcePoint.x() + 10, sourcePoint.y()));
            path.cubicTo(QPointF(sourcePoint) + QPointF(defl, 0),
                         QPointF(destinationPoint) + QPointF(defl, 0),
                         QPointF(destinationPoint) + QPointF(10, 0));
            path.lineTo(destinationPoint);
            path.lineTo(QPointF(destinationPoint) + QPointF(10, 0));
            path.cubicTo(QPointF(destinationPoint) + QPointF(defl, 0),
                         QPointF(sourcePoint) + QPointF(defl, 0),
                         QPointF(sourcePoint) + QPointF(10, 0));
            path.lineTo(sourcePoint);
            break;
        default:
            tgtAssert(false, "shouldn't get here");
        }
    }
    else {
        path.lineTo(sourcePoint - QPointF(10.0, 0.0));
        path.cubicTo(sourcePoint - QPointF(25.0, 0.0), destinationPoint - QPointF(25.0, 0.0), destinationPoint - QPointF(10.0, 0.0));
        path.lineTo(destinationPoint);
        path.lineTo(destinationPoint - QPointF(10, 0));
        path.cubicTo(destinationPoint - QPointF(25.0, 0.0), sourcePoint - QPointF(25.0, 0.0), sourcePoint - QPointF(10.0, 0.0));
        path.lineTo(sourcePoint);
    }


    if (showSourceArrowHead_ && getSourceItem()->isVisible()) {
        DockPositions pos = getDockPositions();
        if ((pos == DockPositionsLeftLeft) || (pos == DockPositionsLeftRight))
            sourceHeadDirection_ = ArrowHeadDirectionWE;
        else
            sourceHeadDirection_ = ArrowHeadDirectionEW;
    }
    else
        sourceHeadDirection_ = ArrowHeadDirectionNone;

    if (showDestinationArrowHead_ && getDestinationItem()->isVisible()) {
        DockPositions pos = getDockPositions();
        if ((pos == DockPositionsLeftLeft) || (pos == DockPositionsRightLeft))
            destinationHeadDirection_ = ArrowHeadDirectionWE;
        else
            destinationHeadDirection_ = ArrowHeadDirectionEW;
    }
    else
        destinationHeadDirection_ = ArrowHeadDirectionNone;

    return path;
}

} // namespace
