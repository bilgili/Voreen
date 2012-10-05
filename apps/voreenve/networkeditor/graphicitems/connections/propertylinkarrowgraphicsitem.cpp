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

#include "propertylinkarrowgraphicsitem.h"
#include "../../styles/nwestyle_base.h"

//core

//gi
#include "../core/propertygraphicsitem.h"
#include "../core/portownergraphicsitem.h"
#include "../core/portgraphicsitem.h"

#include <QGraphicsSceneEvent>

namespace voreen {

PropertyLinkArrowGraphicsItem::PropertyLinkArrowGraphicsItem(PropertyGraphicsItem* sourceItem, PropertyGraphicsItem* destinationItem)
    : ConnectionBaseGraphicsItem(sourceItem, destinationItem)
{
    sourceItem->scene()->addItem(this);

    setZValue(ZValuesPropertyLinkArrowGraphicsItem);
    destinationArrowHeadDirection_ = ArrowHeadDirectionWE;
    setFlag(ItemIsSelectable);
    setAcceptHoverEvents(true);
    setFlag(ItemIsPanel,true);

    hide();
}

PropertyLinkArrowGraphicsItem::~PropertyLinkArrowGraphicsItem() {}

//---------------------------------------------------------------------------------------------------------------
//                  getter and setter                                                                            
//---------------------------------------------------------------------------------------------------------------
PropertyGraphicsItem* PropertyLinkArrowGraphicsItem::getSourceItem() const {
    if (sourceItem_) {
        PropertyGraphicsItem* i = static_cast<PropertyGraphicsItem*>(sourceItem_);
        tgtAssert(i, "sourceItem was of wrong type");
        return i;
    } else
        return 0;
}

PropertyGraphicsItem* PropertyLinkArrowGraphicsItem::getDestinationItem() const {
    if (destinationItem_) {
        PropertyGraphicsItem* i = static_cast<PropertyGraphicsItem*>(destinationItem_);
        tgtAssert(i, "destinationItem was of wrong type");
        return i;
    }
    else
        return 0;
}

QPointF PropertyLinkArrowGraphicsItem::getSourcePoint() const {
    if(!getSourceItem())
        return mapFromScene(sourcePoint_);
    if(getSourceItem()->isVisible()){
        DockPositions dp = getDockPositions();
        if(dp == DockPositionsLeftLeft || dp == DockPositionsLeftRight)
            return mapFromScene(getSourceItem()->getLeftDockingPoint());
        else
            return mapFromScene(getSourceItem()->getRightDockingPoint());
    } else {
        if(dynamic_cast<PortOwnerGraphicsItem*>(getSourceItem()->getPropertyOwnerGraphicsItem()))
            return mapFromScene(dynamic_cast<PortOwnerGraphicsItem*>(getSourceItem()->getPropertyOwnerGraphicsItem())->getPropertyListButton().dockingPoint());
        else
            return mapFromScene(dynamic_cast<PortGraphicsItem*>(getSourceItem()->getPropertyOwnerGraphicsItem())->getPortOwner()->getPropertyListButton().dockingPoint());
    }
}

QPointF PropertyLinkArrowGraphicsItem::getDestinationPoint() const {
    if(!getDestinationItem())
        return mapFromScene(destinationPoint_);
    if(getDestinationItem()->isVisible()){
        DockPositions dp = getDockPositions();
        if(dp == DockPositionsLeftLeft || dp == DockPositionsRightLeft)
            return mapFromScene(getDestinationItem()->getLeftDockingPoint());
        else
            return mapFromScene(getDestinationItem()->getRightDockingPoint());
    } else {
        if(dynamic_cast<PortOwnerGraphicsItem*>(getDestinationItem()->getPropertyOwnerGraphicsItem()))
            return mapFromScene(dynamic_cast<PortOwnerGraphicsItem*>(getDestinationItem()->getPropertyOwnerGraphicsItem())->getPropertyListButton().dockingPoint());
        else
            return mapFromScene(dynamic_cast<PortGraphicsItem*>(getDestinationItem()->getPropertyOwnerGraphicsItem())->getPortOwner()->getPropertyListButton().dockingPoint());
    }
}

PropertyLinkArrowGraphicsItem::DockPositions PropertyLinkArrowGraphicsItem::getDockPositions() const {
    QPointF sourcePropLeftPoint, sourcePropMiddlePoint, sourcePropRightPoint;
    QPointF destinationPropLeftPoint, destinationPropMiddlePoint, destinationPropRightPoint;
    bool sourceMiddle = false;
    bool destinationMiddle = false;
    if(!getSourceItem()) {
        sourcePropMiddlePoint = sourcePoint_;
        sourceMiddle = true;
    } else
    if(getSourceItem()->isVisible()) {
        sourcePropLeftPoint = getSourceItem()->getLeftDockingPoint();
        sourcePropRightPoint = getSourceItem()->getRightDockingPoint();
    } else {
        if(dynamic_cast<PortOwnerGraphicsItem*>(getSourceItem()->getPropertyOwnerGraphicsItem()))
            sourcePropMiddlePoint = dynamic_cast<PortOwnerGraphicsItem*>(getSourceItem()->getPropertyOwnerGraphicsItem())->getPropertyListButton().dockingPoint();
        else
            sourcePropMiddlePoint = dynamic_cast<PortGraphicsItem*>(getSourceItem()->getPropertyOwnerGraphicsItem())->getPortOwner()->getPropertyListButton().dockingPoint();
        sourceMiddle = true;
    }

    if(!getDestinationItem()) {
        destinationPropMiddlePoint = destinationPoint_;
        destinationMiddle = true;
    } else
    if(getDestinationItem()->isVisible()){
        destinationPropLeftPoint = getDestinationItem()->getLeftDockingPoint();
        destinationPropRightPoint = getDestinationItem()->getRightDockingPoint();
    } else {
        if(dynamic_cast<PortOwnerGraphicsItem*>(getDestinationItem()->getPropertyOwnerGraphicsItem()))
            destinationPropMiddlePoint = dynamic_cast<PortOwnerGraphicsItem*>(getDestinationItem()->getPropertyOwnerGraphicsItem())->getPropertyListButton().dockingPoint();
        else
            destinationPropMiddlePoint = dynamic_cast<PortGraphicsItem*>(getDestinationItem()->getPropertyOwnerGraphicsItem())->getPortOwner()->getPropertyListButton().dockingPoint();
        destinationMiddle = true;
    }

    if(!destinationMiddle && !sourceMiddle){
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
    } else
        if (destinationMiddle) {
            qreal leftMiddle = QLineF(sourcePropLeftPoint, destinationPropMiddlePoint).length();
            qreal rightMiddle = QLineF(sourcePropRightPoint, destinationPropMiddlePoint).length();

            if (leftMiddle < rightMiddle)
                return DockPositionsLeftRight;
            else
                return DockPositionsRightLeft;
        } else {
            qreal middleLeft = QLineF(sourcePropMiddlePoint, destinationPropLeftPoint).length();
            qreal middleRight = QLineF(sourcePropMiddlePoint, destinationPropRightPoint).length();

            if (middleLeft < middleRight)
                return DockPositionsRightLeft;
            else
                return DockPositionsLeftRight;
        }

}

//---------------------------------------------------------------------------------------------------------------
//                  nwebasegraphicsitem functions                                                                         
//---------------------------------------------------------------------------------------------------------------
QRectF PropertyLinkArrowGraphicsItem::boundingRect() const {
    return currentStyle()->PropertyLinkArrowGI_boundingRect(this);
}

QPainterPath PropertyLinkArrowGraphicsItem::shape() const {
    return currentStyle()->PropertyLinkArrowGI_shape(this);
}

void PropertyLinkArrowGraphicsItem::initializePaintSettings(){
    currentStyle()->PropertyLinkArrowGI_initializePaintSettings(this);
}

void PropertyLinkArrowGraphicsItem::prePaint(QPainter* painter, const QStyleOptionGraphicsItem* option) {}

void PropertyLinkArrowGraphicsItem::mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    currentStyle()->PropertyLinkArrowGI_paint(this, painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------
//                  events                                                                                       
//---------------------------------------------------------------------------------------------------------------
void PropertyLinkArrowGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    NWEBaseGraphicsItem::mousePressEvent(event);
}

void PropertyLinkArrowGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    NWEBaseGraphicsItem::mouseMoveEvent(event);
}

void PropertyLinkArrowGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    NWEBaseGraphicsItem::mouseReleaseEvent(event);
}

} // voreen namespace

