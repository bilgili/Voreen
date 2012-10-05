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

#include "propertygraphicsitem.h"
#include "../../styles/nwestyle_base.h"

//core
#include "voreen/core/properties/property.h"
//gi
#include "propertylistgraphicsitem.h"
#include "propertyownergraphicsitem.h"
#include "../connections/propertylinkarrowgraphicsitem.h"

#include <QGraphicsSceneMouseEvent>

namespace voreen {

PropertyGraphicsItem::PropertyGraphicsItem(const Property* prop, PropertyListGraphicsItem* parent)
    : NWEBaseGraphicsItem(parent->getNetworkEditor())
    , property_(prop)
    , propertyListItem_(parent)
    , showLinkArrows_(true)
    , propertyListLinkedTo_(0)
    , boundingWidth_(0.f)
{
    tgtAssert(prop != 0, "passed null pointer");
    setParent(parent);
    setParentItem(parent);

    setFlag(ItemIsSelectable);
    setFlag(ItemIsPanel);
    setAcceptsHoverEvents(true);
    setZValue(ZValuesPropertyGraphicsItem);

    propertyLabel_ = new QGraphicsTextItem(this);
    propertyLabel_->setParentItem(this);

    //set text
    initializePaintSettings();
    paintHasBeenInitialized_ = true;
    resetSize(propertyLabel_->boundingRect().width());
}

PropertyGraphicsItem::~PropertyGraphicsItem() {
    deleteChildItems();
}

//---------------------------------------------------------------------------------------------------------------
//                  getter and setter                                                                            
//---------------------------------------------------------------------------------------------------------------
const Property* PropertyGraphicsItem::getProperty() const {
    return property_;
}

PropertyListGraphicsItem* PropertyGraphicsItem::getPropertyList() const {
    return propertyListItem_;
}

QGraphicsTextItem* PropertyGraphicsItem::getPropertyLabel() {
    return propertyLabel_;
}

const QGraphicsTextItem* PropertyGraphicsItem::getPropertyLabel() const{
    return propertyLabel_;
}

PropertyOwnerGraphicsItem* PropertyGraphicsItem::getPropertyOwnerGraphicsItem() const {
    return propertyListItem_->getParent();
}

qreal PropertyGraphicsItem::getBoundingWidth() const {
    return boundingWidth_;
}

QList<PropertyLinkArrowGraphicsItem*> PropertyGraphicsItem::getSourceLinkList() {
    return sourceLinkList_;
}

QList<PropertyLinkArrowGraphicsItem*> PropertyGraphicsItem::getDestinationLinkList() {
    return destinationLinkList_;
}

QPointF PropertyGraphicsItem::getLeftDockingPoint() const {
    return mapToScene(QPointF(boundingRect().x(), boundingRect().height() / 2.f));
}

QPointF PropertyGraphicsItem::getRightDockingPoint() const {
    return mapToScene(QPointF(boundingRect().width(), boundingRect().height() / 2.f));
}

//---------------------------------------------------------------------------------------------------------------
//                  visibility of arrows                                                                         
//---------------------------------------------------------------------------------------------------------------
void PropertyGraphicsItem::setVisible(bool visible){
    QGraphicsItem::setVisible(visible);
    if(!showLinkArrows_){
        foreach(PropertyLinkArrowGraphicsItem* item, sourceLinkList_) {
            item->setVisible(false);
        }
        foreach (PropertyLinkArrowGraphicsItem* item, destinationLinkList_) {
            item->setVisible(false);
        }
    } else
    if(visible) {
        if(!propertyListLinkedTo_){
            foreach(PropertyLinkArrowGraphicsItem* item, sourceLinkList_) {
                item->setVisible(true);
            }
            foreach (PropertyLinkArrowGraphicsItem* item, destinationLinkList_) {
                item->setVisible(true);
            }
        } else {
            QList<PropertyGraphicsItem*> list = propertyListLinkedTo_->getAllPropertyGraphicsItems();
            foreach(PropertyLinkArrowGraphicsItem* item, sourceLinkList_) {
                if(list.contains(item->getDestinationItem()))
                    item->setVisible(true);
                else
                    item->setVisible(false);
            }
            foreach (PropertyLinkArrowGraphicsItem* item, destinationLinkList_) {
                if(list.contains(item->getSourceItem()))
                    item->setVisible(true);
                else
                    item->setVisible(false);
            }
        }
    } else {
        foreach(PropertyLinkArrowGraphicsItem* item, sourceLinkList_) {
            item->setVisible(false);
        }
        foreach (PropertyLinkArrowGraphicsItem* item, destinationLinkList_) {
            item->setVisible(false);
        }
        if(!propertyListLinkedTo_){
            foreach(PropertyLinkArrowGraphicsItem* item, sourceLinkList_) {
                if(item->getDestinationItem()->isVisible())
                    item->getDestinationItem()->setVisible(true);
            }
            foreach (PropertyLinkArrowGraphicsItem* item, destinationLinkList_) {
                if(item->getSourceItem()->isVisible())
                    item->getSourceItem()->setVisible(true);
            }
        } else {

        }
    }
}

void PropertyGraphicsItem::setShowLinkArrows(bool b) {
    showLinkArrows_ = b;
}

void PropertyGraphicsItem::setPropertyListLinkedTo(PropertyListGraphicsItem* list) {
    propertyListLinkedTo_ = list;
}

//---------------------------------------------------------------------------------------------------------------
//                  size management                                                                              
//---------------------------------------------------------------------------------------------------------------
void PropertyGraphicsItem::resetSize(qreal width) {
    boundingWidth_ = width;
}

//---------------------------------------------------------------------------------------------------------------
//                  nwebasegraphicsitem functions                                                                         
//---------------------------------------------------------------------------------------------------------------
QRectF PropertyGraphicsItem::boundingRect() const {
    return currentStyle()->PropertyGI_boundingRect(this);
}

QPainterPath PropertyGraphicsItem::shape() const{
    return currentStyle()->PropertyGI_shape(this);
}

void PropertyGraphicsItem::initializePaintSettings() {
    currentStyle()->PropertyGI_initializePaintSettings(this);
}

void PropertyGraphicsItem::prePaint(QPainter* painter, const QStyleOptionGraphicsItem* option) {}

void PropertyGraphicsItem::mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    currentStyle()->PropertyGI_paint(this, painter, option, widget);
}

void PropertyGraphicsItem::deleteChildItems() {
    foreach (PropertyLinkArrowGraphicsItem* pl, sourceLinkList_){
        removeGraphicalLink(pl->getDestinationItem());
    }
    foreach (PropertyLinkArrowGraphicsItem* pl, destinationLinkList_){
        pl->getSourceItem()->removeGraphicalLink(this);
    }
}

void PropertyGraphicsItem::prepareGeometryChange() {
    NWEBaseGraphicsItem::prepareGeometryChange();
    foreach (PropertyLinkArrowGraphicsItem* pl, sourceLinkList_){
        pl->prepareGeometryChange();
    }
    foreach (PropertyLinkArrowGraphicsItem* pl, destinationLinkList_){
        pl->prepareGeometryChange();
    }
}

//---------------------------------------------------------------------------------------------------------------
//                  linking functions                                                                         
//---------------------------------------------------------------------------------------------------------------
PropertyLinkArrowGraphicsItem* PropertyGraphicsItem::addGraphicalLink(PropertyGraphicsItem* prop) {
    foreach(PropertyLinkArrowGraphicsItem* arrow, getSourceLinkList()){
        if(arrow->getDestinationItem() == prop)
            return arrow;
    }
    PropertyLinkArrowGraphicsItem* link = new PropertyLinkArrowGraphicsItem(this);
    link->setDestinationItem(prop);
    sourceLinkList_.append(link);
    foreach(PropertyLinkArrowGraphicsItem* item, prop->getSourceLinkList()){
        if(item->getDestinationItem() == this){
            item->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
            link->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
            break;
        }
    }
    foreach(PropertyLinkArrowGraphicsItem* item, getDestinationLinkList()){
        if(item->getSourceItem() == prop){
            item->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
            break;
        }
    }
    prop->addDestinationLink(this);
    setVisible(isVisible());
    return link;
}

void PropertyGraphicsItem::addDestinationLink(PropertyGraphicsItem* prop) {
    PropertyLinkArrowGraphicsItem* link = new PropertyLinkArrowGraphicsItem(prop);
    link->setParent(this);
    link->scene()->removeItem(link);
    scene()->addItem(link);
    link->setDestinationItem(this);
    destinationLinkList_.append(link);
    foreach(PropertyLinkArrowGraphicsItem* item, getSourceLinkList()){
        if(item->getDestinationItem() == prop){
            link->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
            break;
        }
    }
}

void PropertyGraphicsItem::removeGraphicalLink(PropertyGraphicsItem* prop) {
    foreach (PropertyLinkArrowGraphicsItem* linkItem, sourceLinkList_) {
        if(linkItem->getDestinationItem() == prop){
            sourceLinkList_.removeOne(linkItem);
            prop->removeDestinationLink(this);
            delete linkItem;
            break;
        } 
    }
    foreach (PropertyLinkArrowGraphicsItem* linkItem, destinationLinkList_) {
        if(linkItem->getSourceItem() == prop){
            linkItem->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionNone);
            return;
        } 
    }

}

void PropertyGraphicsItem::removeDestinationLink(PropertyGraphicsItem* prop) {
    foreach (PropertyLinkArrowGraphicsItem* linkItem, destinationLinkList_) {
        if(linkItem->getSourceItem() == prop){
            destinationLinkList_.removeOne(linkItem);
            delete linkItem;
            break;
        } 
    }
    foreach (PropertyLinkArrowGraphicsItem* linkItem, sourceLinkList_) {
        if(linkItem->getDestinationItem() == prop){
            linkItem->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionNone);
            return;
        } 
    }
}

//---------------------------------------------------------------------------------------------------------------
//                  events                                                                                       
//---------------------------------------------------------------------------------------------------------------
void PropertyGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    NWEBaseGraphicsItem::mousePressEvent(event);
}

void PropertyGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    NWEBaseGraphicsItem::mouseMoveEvent(event);
}

void PropertyGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    NWEBaseGraphicsItem::mouseReleaseEvent(event);
}

} // namespace
