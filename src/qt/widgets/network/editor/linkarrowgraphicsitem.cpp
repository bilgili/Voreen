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

#include "voreen/qt/widgets/network/editor/linkarrowgraphicsitem.h"

#include "voreen/core/vis/properties/link/linkevaluatorfactory.h"
#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/propertygraphicsitem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace {
    qreal arrowSize = 10.f;
}

namespace voreen {

LinkArrowGraphicsItem::LinkArrowGraphicsItem(PropertyGraphicsItem* propertyItem1, PropertyGraphicsItem* propertyItem2, const PropertyLink* link)
    : QGraphicsItem()
    , propertyItem1_(propertyItem1)
    , propertyItem2_(propertyItem2)
    , link_(link)
    , secondLink_(0)
    , color_(Qt::lightGray)
{
    tgtAssert(propertyItem1_ != 0, "passed null pointer");
    tgtAssert(propertyItem2_ != 0, "passed null pointer");

    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);
    setZValue(ZValueLinkArrowGraphicsItem);
}

int LinkArrowGraphicsItem::type() const {
    return Type;
}

void LinkArrowGraphicsItem::adjust() {
    prepareGeometryChange();
}

PropertyGraphicsItem* LinkArrowGraphicsItem::firstPropertyItem() const {
    return propertyItem1_;
}

PropertyGraphicsItem* LinkArrowGraphicsItem::secondPropertyItem() const {
    return propertyItem2_;
}

const PropertyLink* LinkArrowGraphicsItem::getLink() const {
    return link_;
}

const PropertyLink* LinkArrowGraphicsItem::getSecondLink() const {
    return secondLink_;
}

void LinkArrowGraphicsItem::setSecondLink(const PropertyLink* link) {
    secondLink_ = link;
}

bool LinkArrowGraphicsItem::hasSecondLink() const {
    return secondLink_ != 0;
}

LinkArrowGraphicsItem::LinkArrowState LinkArrowGraphicsItem::getState() const {
    if ((propertyItem1_->isVisible()) && (propertyItem2_->isVisible())) {
        return LinkArrowStatePropertyToProperty;
    } else if ((propertyItem1_->isVisible()) && (!propertyItem2_->isVisible())) {
        return LinkArrowStatePropertyToProcessor;
    } else if ((!propertyItem1_->isVisible()) && (propertyItem2_->isVisible())) {
        return LinkArrowStateProcessorToProperty;
    } else {
        return LinkArrowStateProcessorToProcessor;
    }
}

LinkArrowGraphicsItem::DockPositions LinkArrowGraphicsItem::getDockPositions() const {
    QPointF sourcePropLeftPoint = propertyItem1_->getLeftDockingPoint();
    QPointF sourcePropRightPoint = propertyItem1_->getRightDockingPoint();
    QPointF destinationPropLeftPoint = propertyItem2_->getLeftDockingPoint();
    QPointF destinationPropRightPoint = propertyItem2_->getRightDockingPoint();

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

void LinkArrowGraphicsItem::switchLinks() {
    const PropertyLink* tmp = link_;
    link_ = secondLink_;
    secondLink_ = tmp;
}

QRectF LinkArrowGraphicsItem::boundingRect() const {
    // the bounding box of the bezier path
    QRectF rect = shape().boundingRect();

    // add a few extra pixels for the arrow and the pen
    qreal penWidth = 1;
    qreal extra = (penWidth + arrowSize) / 2.0;
    rect.adjust(-extra, -extra, extra, extra);

    return rect;
}

QPainterPath LinkArrowGraphicsItem::shape() const {
    QPointF sourcePoint;
    QPointF destinationPoint;

    if (getState() == LinkArrowStateProcessorToProcessor) {
        if (propertyItem1_->getProcessorGraphicsItem() == propertyItem2_->getProcessorGraphicsItem())
            return QPainterPath();

        sourcePoint = propertyItem1_->getProcessorGraphicsItem()->dockingPoint();
        destinationPoint = propertyItem2_->getProcessorGraphicsItem()->dockingPoint();
    }
    else {
        switch (getDockPositions()) {
        case DockPositionsLeftLeft:
            sourcePoint = propertyItem1_->getLeftDockingPoint();
            destinationPoint = propertyItem2_->getLeftDockingPoint();
            break;
        case DockPositionsLeftRight:
            sourcePoint = propertyItem1_->getLeftDockingPoint();
            destinationPoint = propertyItem2_->getRightDockingPoint();
            break;
        case DockPositionsRightLeft:
            sourcePoint = propertyItem1_->getRightDockingPoint();
            destinationPoint = propertyItem2_->getLeftDockingPoint();
            break;
        case DockPositionsRightRight:
            sourcePoint = propertyItem1_->getRightDockingPoint();
            destinationPoint = propertyItem2_->getRightDockingPoint();
            break;
        default:
            tgtAssert(false, "shouldn't get here");
        }
    }

    if (getState() == LinkArrowStateProcessorToProperty) {
        sourcePoint = propertyItem1_->getProcessorGraphicsItem()->dockingPoint();
    }
    else if (getState() == LinkArrowStatePropertyToProcessor) {
        destinationPoint = propertyItem2_->getProcessorGraphicsItem()->dockingPoint();
    }

    QPainterPath path(sourcePoint);
    qreal defl = fabs(sourcePoint.y() - destinationPoint.y()) / 2.f + fabs(sourcePoint.x() - destinationPoint.x()) / 2.f;

    if (getState() == LinkArrowStateProcessorToProcessor) {
        path.lineTo(QPointF(sourcePoint.x(), sourcePoint.y() - 10.f));
        path.cubicTo(QPointF(sourcePoint) - QPointF(0, defl),
                         QPointF(destinationPoint) - QPointF(0, defl),
                         QPointF(destinationPoint) - QPointF(0, 10));
        path.lineTo(destinationPoint);        
        path.lineTo(destinationPoint - QPointF(0, 10));        
        path.cubicTo( QPointF(destinationPoint) - QPointF(0, defl),
						 QPointF(sourcePoint) - QPointF(0, defl),
						 QPointF(sourcePoint.x(), sourcePoint.y() - 10.f));
        path.lineTo(sourcePoint);
        return path;
    }
    defl = defl / 2.5;

    if (propertyItem1_->getProcessorGraphicsItem() != propertyItem2_->getProcessorGraphicsItem()) {
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
    else{
        path.lineTo(QPointF(sourcePoint.x()-10, sourcePoint.y()));
        path.quadTo(QPointF(sourcePoint.x()-50,(sourcePoint.y()+destinationPoint.y())/2.f), QPointF(destinationPoint) - QPointF(10, 0));
        path.lineTo(destinationPoint);
        path.lineTo(destinationPoint - QPointF(10, 0));
        path.quadTo(QPointF(sourcePoint.x()-50,(sourcePoint.y()+destinationPoint.y())/2.f), QPointF(sourcePoint) - QPointF(10, 0));
        path.lineTo(sourcePoint);
    }

    return path;
}

void LinkArrowGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) {
    QColor tmpColor;    // needed for hover effect
    if (isSelected())
        tmpColor = Qt::red;
    else
        tmpColor = color_;
    if (option->state & QStyle::State_MouseOver) {
        if (tmpColor == Qt::black)    // Qt is unable to brighten up Qt::black
            tmpColor = Qt::white;
        else
            tmpColor = tmpColor.light();
    }

    painter->setBrush(tmpColor);
    painter->setPen(QPen(tmpColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QPainterPath path = shape();
    painter->drawPath(path);
    painter->setBrush(tmpColor);

    if (!link_->isActive() || (hasSecondLink() && !secondLink_->isActive())) {
        QPointF imageOrigin = path.pointAtPercent(0.5);
        QPixmap pixmap = QPixmap(":/voreenve/icons/cancel.png");
        imageOrigin.rx() -= pixmap.size().width() / 2;
        imageOrigin.ry() -= pixmap.size().height() / 2;
        painter->drawPixmap(imageOrigin, pixmap);
    }

    const float angle = -tgt::PI / 2.f;

    if (getState() == LinkArrowStatePropertyToProperty) {
        switch (getDockPositions()) {
        case DockPositionsLeftLeft:
            {
                QPointF arrowHeadP1 = propertyItem2_->getLeftDockingPoint() + QPointF(cos(angle - tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf / 3.f) * arrowSize);
                QPointF arrowHeadP2 = propertyItem2_->getLeftDockingPoint() + QPointF(cos(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize);
                painter->drawPolygon(QPolygonF() << propertyItem2_->getLeftDockingPoint() << arrowHeadP1 << arrowHeadP2);
                if (hasSecondLink()) {
                    QPointF otherArrowHeadP1 = propertyItem1_->getLeftDockingPoint() + QPointF(cos(angle - tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf / 3.f) * arrowSize);
                    QPointF otherArrowHeadP2 = propertyItem1_->getLeftDockingPoint() + QPointF(cos(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize);
                    painter->drawPolygon(QPolygonF() << propertyItem1_->getLeftDockingPoint() << otherArrowHeadP1 << otherArrowHeadP2);
                }
                break;
            }
        case DockPositionsLeftRight:
            {
                QPointF arrowHeadP1 = propertyItem2_->getRightDockingPoint() - QPointF(cos(angle - tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf / 3.f) * arrowSize);
                QPointF arrowHeadP2 = propertyItem2_->getRightDockingPoint() - QPointF(cos(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize);
                painter->drawPolygon(QPolygonF() << propertyItem2_->getRightDockingPoint() << arrowHeadP1 << arrowHeadP2);
                if (hasSecondLink()) {
                    QPointF otherArrowHeadP1 = propertyItem1_->getLeftDockingPoint() + QPointF(cos(angle - tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf / 3.f) * arrowSize);
                    QPointF otherArrowHeadP2 = propertyItem1_->getLeftDockingPoint() + QPointF(cos(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize);
                    painter->drawPolygon(QPolygonF() << propertyItem1_->getLeftDockingPoint() << otherArrowHeadP1 << otherArrowHeadP2);
                }
                break;
            }
        case DockPositionsRightLeft:
            {
                QPointF arrowHeadP1 = propertyItem2_->getLeftDockingPoint() + QPointF(cos(angle - tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf / 3.f) * arrowSize);
                QPointF arrowHeadP2 = propertyItem2_->getLeftDockingPoint() + QPointF(cos(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize);
                painter->drawPolygon(QPolygonF() << propertyItem2_->getLeftDockingPoint() << arrowHeadP1 << arrowHeadP2);
                if (hasSecondLink()) {
                    QPointF otherArrowHeadP1 = propertyItem1_->getRightDockingPoint() - QPointF(cos(angle - tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf / 3.f) * arrowSize);
                    QPointF otherArrowHeadP2 = propertyItem1_->getRightDockingPoint() - QPointF(cos(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize);
                    painter->drawPolygon(QPolygonF() << propertyItem1_->getRightDockingPoint() << otherArrowHeadP1 << otherArrowHeadP2);
                }
                break;
            }
        case DockPositionsRightRight:
            {
                QPointF arrowHeadP1 = propertyItem2_->getRightDockingPoint() - QPointF(cos(angle - tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf / 3.f) * arrowSize);
                QPointF arrowHeadP2 = propertyItem2_->getRightDockingPoint() - QPointF(cos(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize);
                painter->drawPolygon(QPolygonF() << propertyItem2_->getRightDockingPoint() << arrowHeadP1 << arrowHeadP2);
                if (hasSecondLink()) {
                    QPointF otherArrowHeadP1 = propertyItem1_->getRightDockingPoint() - QPointF(cos(angle - tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf / 3.f) * arrowSize);
                    QPointF otherArrowHeadP2 = propertyItem1_->getRightDockingPoint() - QPointF(cos(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize, sin(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize);
                    painter->drawPolygon(QPolygonF() << propertyItem1_->getRightDockingPoint() << otherArrowHeadP1 << otherArrowHeadP2);
                }
                break;
            }

        default:
            tgtAssert(false, "shouldn't get here");
        }
    }
    // Right now I want a fixed angle for the bezier stuff
    //double angle = -tgt::PIf / 2.0;

    //QPointF sourceArrowP1 = propertyItem1_->getLeftDockingPoint() + QPointF(sin(angle + tgt::PIf / 3.f) * arrowSize,
    //    cos(angle + tgt::PIf / 3.f) * arrowSize);
    //QPointF sourceArrowP2 = propertyItem1_->getLeftDockingPoint() + QPointF(sin(angle + tgt::PIf - tgt::PIf / 3.f) * arrowSize,
    //    cos(angle + tgt::PIf - tgt::PIf / 3.f) * arrowSize);
    //QPointF destArrowP1 = propertyItem2_->getRightDockingPoint() + QPointF(sin(angle - tgt::PIf / 3.f) * arrowSize,
    //    cos(angle - tgt::PIf / 3.f) * arrowSize);
    //QPointF destArrowP2 = propertyItem2_->getRightDockingPoint() + QPointF(sin(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize,
    //    cos(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize);

    //painter->drawPolygon(QPolygonF() << propertyItem2_->getRightDockingPoint() << destArrowP1 << destArrowP2);
    //painter->drawPolygon(QPolygonF() << propertyItem2_->getRightDockingPoint() << destArrowP1 << destArrowP2);

    //if (hasSecondLink()) {
    //    if (propertyItem1_->getProcessorGraphicsItem() != propertyItem2_->getProcessorGraphicsItem()) {
    //        if (sourcePoint_.x() <= destPoint_.x()){
    //            painter->drawPolygon(QPolygonF() << propertyItem2_-> << QPointF(destPoint_.x()-10, destPoint_.y()-5) << QPointF(destPoint_.x()-10, destPoint_.y()+5));
    //            painter->drawPolygon(QPolygonF() << sourcePoint_ << QPointF(sourcePoint_.x()+10, sourcePoint_.y()-5) << QPointF(sourcePoint_.x()+10, sourcePoint_.y()+5));
    //        }
    //        else{
    //            painter->drawPolygon(QPolygonF() << destPoint_ << QPointF(destPoint_.x()+10, destPoint_.y()-5) << QPointF(destPoint_.x()+10, destPoint_.y()+5));
    //            painter->drawPolygon(QPolygonF() << sourcePoint_ << QPointF(sourcePoint_.x()-10, sourcePoint_.y()-5) << QPointF(sourcePoint_.x()-10, sourcePoint_.y()+5));
    //        }
    //    }
    //    else{
    //        painter->drawPolygon(QPolygonF() << destPoint_ << QPointF(destPoint_.x()-10, destPoint_.y()-5) << QPointF(destPoint_.x()-10, destPoint_.y()+5));
    //        painter->drawPolygon(QPolygonF() << sourcePoint_ << QPointF(sourcePoint_.x()-10, sourcePoint_.y()-5) << QPointF(sourcePoint_.x()-10, sourcePoint_.y()+5));
    //    }
    //}
    //else{
    //    if(singleLink_->getSource()->getProcessorGraphicsItem() != singleLink_->getDestination()->getProcessorGraphicsItem()){
    //        if(sourcePoint_.x() <= destPoint_.x()){
    //            painter->drawPolygon(QPolygonF() << destPoint_ << QPointF(destPoint_.x()-10, destPoint_.y()-5) << QPointF(destPoint_.x()-10, destPoint_.y()+5));
    //        }
    //        else{
    //            painter->drawPolygon(QPolygonF() << destPoint_ << QPointF(destPoint_.x()+10, destPoint_.y()-5) << QPointF(destPoint_.x()+10, destPoint_.y()+5));
    //        }
    //    }
    //    else{
    //        painter->drawPolygon(QPolygonF() << destPoint_ << QPointF(destPoint_.x()-10, destPoint_.y()-5) << QPointF(destPoint_.x()-10, destPoint_.y()+5));
    //    }
    //}

    //painter->setBrush(tmpColor);
}

QGraphicsItem* LinkArrowGraphicsItem::tooltip() const {
    std::string functionname = LinkEvaluatorFactory::getInstance()->getFunctionName(link_->getLinkEvaluator());
    QGraphicsSimpleTextItem* tooltipText =
        new QGraphicsSimpleTextItem(functionname.c_str());
    QGraphicsRectItem* tooltip = new QGraphicsRectItem((tooltipText->boundingRect()).adjusted(-4, 0, 4, 0));
    tooltipText->setParentItem(tooltip);
    tooltip->translate(-tooltip->rect().width(), -tooltip->rect().height());
    tooltip->setBrush(QBrush(QColor(253, 237, 212), Qt::SolidPattern));
    return tooltip;
}

} // namespace
