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

#include "propertylistgraphicsitem.h"
#include "../../styles/nwestyle_classic.h"

//core
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/ports/renderport.h"
//gi
#include "propertyownergraphicsitem.h"
#include "propertygraphicsitem.h"
#include "../connections/propertylinkarrowgraphicsitem.h"

#include <QBrush>



namespace voreen {

PropertyListGraphicsItem::PropertyListGraphicsItem(PropertyOwnerGraphicsItem* parent)
    : NWEBaseGraphicsItem(parent->getNetworkEditor())
    , parent_(parent)
    , currentHeaderMode_(NO_HEADER)
    , currentPropertyMode_(SHOW_LINKED_PROPERTIES)
    , currentPropertyModifier_(HIDE_NO_PROPERTIES)
    , currentLinkArrowMode_(SHOW_NO_ARROWS)
    , friendList_(0)
    , isVisibleInEditor_(false)
    , currentHeight_(0.f)
    , currentWidth_(0.f)
{
    tgtAssert(parent != 0, "passed null pointer");
    setZValue(ZValuesPropertyListGraphicsItem);
    setParent(parent);

    parent->getNetworkEditor()->scene()->addItem(this);

    parentLabel_ = new QGraphicsTextItem(this);
    parentLabel_->setParentItem(this);
    currentWidth_ = getParentLabel()->boundingRect().width();
    paint(0,0,0);
    createChildItems();
}

PropertyListGraphicsItem::~PropertyListGraphicsItem() {
    deleteChildItems();
}

//---------------------------------------------------------------------------------------------------------------
//                  getter and setter                                                                            
//---------------------------------------------------------------------------------------------------------------
PropertyOwnerGraphicsItem* PropertyListGraphicsItem::getParent() const {
    return parent_;
}

void PropertyListGraphicsItem::setLinkArrowMode(LinkArrowVisibleMode lavm, PropertyListGraphicsItem* friendList){
    currentLinkArrowMode_= lavm;
    friendList_ = friendList;

    foreach(PropertyGraphicsItem* item, propertyItems_) {
        switch(lavm) {
        case SHOW_NO_ARROWS:
            item->setShowLinkArrows(false);
            break;
        case SHOW_ALL_ARROWS:
        case SHOW_FRIEND_LIST_ARROWS:
            item->setShowLinkArrows(true);
            break;
        }
        item->setPropertyListLinkedTo(friendList);
    }

    foreach(PropertyListGraphicsItem* listItem, subPropertyLists_){
        listItem->setLinkArrowMode(lavm, friendList);
    }

    layoutChildItems();
}

void PropertyListGraphicsItem::setPropertyVisibleMode(PropertyVisibleMode pvm){
    currentPropertyMode_= pvm;

    foreach(PropertyListGraphicsItem* listItem, subPropertyLists_){
        listItem->setPropertyVisibleMode(pvm);
    }

    layoutChildItems();
}

void PropertyListGraphicsItem::setPropertyVisibleModifier(PropertyVisibleModeModifier pvmm){
    currentPropertyModifier_= pvmm;

    foreach(PropertyListGraphicsItem* listItem, subPropertyLists_){
        listItem->setPropertyVisibleModifier(pvmm);
    }

    layoutChildItems();
}

void PropertyListGraphicsItem::setHeaderMode(HeaderMode hm){
    if(hm == currentHeaderMode_)
        return;

    currentHeaderMode_= hm;

    layoutChildItems();
}


PropertyGraphicsItem* PropertyListGraphicsItem::getPropertyGraphicsItem(const Property* prop) {
    foreach(PropertyGraphicsItem* item, propertyItems_){
        if(item->getProperty() == prop)
            return item;
    }
    return 0;
}

QList<PropertyGraphicsItem*> PropertyListGraphicsItem::getPropertyGraphicsItems() {
    return propertyItems_;
}

const QList<PropertyGraphicsItem*> PropertyListGraphicsItem::getPropertyGraphicsItems() const{
    return propertyItems_;
}

QList<PropertyGraphicsItem*> PropertyListGraphicsItem::getVisiblePropertyGraphicsItems() {
    QList<PropertyGraphicsItem*> list;
    foreach(PropertyGraphicsItem* item, propertyItems_)
        if(item->isVisible())
            list.append(item);
    return list;
}

const QList<PropertyGraphicsItem*> PropertyListGraphicsItem::getVisiblePropertyGraphicsItems() const{
    QList<PropertyGraphicsItem*> list;
    foreach(PropertyGraphicsItem* item, propertyItems_)
        if(item->isVisible())
            list.append(item);
    return list;
}

QList<PropertyGraphicsItem*> PropertyListGraphicsItem::getAllPropertyGraphicsItems() {
    QList<PropertyGraphicsItem*> list(propertyItems_);
    foreach(PropertyListGraphicsItem* item, getSubPropertyLists())
        list.append(item->getPropertyGraphicsItems());
    return list;
}

const QList<PropertyGraphicsItem*> PropertyListGraphicsItem::getAllPropertyGraphicsItems() const{
    QList<PropertyGraphicsItem*> list(propertyItems_);
    foreach(PropertyListGraphicsItem* item, getSubPropertyLists())
        list.append(item->getPropertyGraphicsItems());
    return list;
}

QList<PropertyGraphicsItem*> PropertyListGraphicsItem::getAllVisiblePropertyGraphicsItems() {
    QList<PropertyGraphicsItem*> list = getVisiblePropertyGraphicsItems();
    foreach(PropertyListGraphicsItem* item, getSubPropertyLists())
        list.append(item->getVisiblePropertyGraphicsItems());
    return list;
}

const QList<PropertyGraphicsItem*> PropertyListGraphicsItem::getAllVisiblePropertyGraphicsItems() const {
    QList<PropertyGraphicsItem*> list = getVisiblePropertyGraphicsItems();
    foreach(PropertyListGraphicsItem* item, getSubPropertyLists())
        list.append(item->getVisiblePropertyGraphicsItems());
    return list;
}

const QGraphicsTextItem* PropertyListGraphicsItem::getParentLabel() const {
    return parentLabel_;
}

QGraphicsTextItem* PropertyListGraphicsItem::getParentLabel() {
    return parentLabel_;
}

PropertyListGraphicsItem::LinkArrowVisibleMode PropertyListGraphicsItem::getLinkArrowVisibleMode() const {
    return currentLinkArrowMode_;
}

PropertyListGraphicsItem::PropertyVisibleMode PropertyListGraphicsItem::getPropertyVisibleMode() const {
    return currentPropertyMode_;
}

PropertyListGraphicsItem::PropertyVisibleModeModifier PropertyListGraphicsItem::getPropertyVisibleModeModifier() const {
    return currentPropertyModifier_;
}
PropertyListGraphicsItem::HeaderMode PropertyListGraphicsItem::getHeaderMode() const {
    return currentHeaderMode_;
}

QList<PropertyListGraphicsItem*> PropertyListGraphicsItem::getSubPropertyLists() {
    return subPropertyLists_;
}

const QList<PropertyListGraphicsItem*> PropertyListGraphicsItem::getSubPropertyLists() const {
    return subPropertyLists_;
}

void PropertyListGraphicsItem::setScene(QGraphicsScene* scene) {
    this->scene()->removeItem(this);
    scene->addItem(this);

    foreach(PropertyGraphicsItem* propItem, getPropertyGraphicsItems()){
        foreach(PropertyLinkArrowGraphicsItem* arrow, propItem->getSourceLinkList()) {
            arrow->scene()->removeItem(arrow);
            scene->addItem(arrow);
        }
        foreach(PropertyLinkArrowGraphicsItem* arrow, propItem->getDestinationLinkList()) {
            arrow->scene()->removeItem(arrow);
            scene->addItem(arrow);
        }
    }

    foreach(PropertyListGraphicsItem* item, subPropertyLists_){
        item->setScene(scene);
    }
   
};

bool PropertyListGraphicsItem::getIsVisibleInEditor() const {
    return isVisibleInEditor_;
}

void PropertyListGraphicsItem::setIsVisibleInEditor(bool b) {
    isVisibleInEditor_ = b;
}

void PropertyListGraphicsItem::updateParentLabel() {
    QString labelText = getParent()->getGuiName();
    getParentLabel()->setHtml(labelText);
    findNewListWidth();
}


//---------------------------------------------------------------------------------------------------------------
//                  nwebasegraphicsitem functions                                                                         
//---------------------------------------------------------------------------------------------------------------
QRectF PropertyListGraphicsItem::boundingRect() const {
    return currentStyle()->PropertyListGI_boundingRect(this);
}

QPainterPath PropertyListGraphicsItem::shape() const{
    return currentStyle()->PropertyListGI_shape(this);
}

void PropertyListGraphicsItem::initializePaintSettings() {
    currentStyle()->PropertyListGI_initializePaintSettings(this);
}

void PropertyListGraphicsItem::prePaint(QPainter* painter, const QStyleOptionGraphicsItem* option) {}

void PropertyListGraphicsItem::mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    currentStyle()->PropertyListGI_paint(this,painter, option, widget);
}

void PropertyListGraphicsItem::createChildItems(){
    if(parent_->getPropertyOwner()) //aggregations have no property owner
        addProperties(stdVectorToQList(parent_->getPropertyOwner()->getProperties()));
}

void PropertyListGraphicsItem::deleteChildItems(){
    foreach (PropertyGraphicsItem* item, propertyItems_)
        delete item;
}

void PropertyListGraphicsItem::layoutChildItems(){
    prepareGeometryChange();
    qreal x = (boundingRect().width() - parentLabel_->boundingRect().width()) / 2.0;
    qreal y = (currentHeaderMode_  - parentLabel_->boundingRect().height()) / 2.0;
    parentLabel_->setPos(x, y);
    
    if(getVisiblePropertyGraphicsItems().empty()){
        parentLabel_->setVisible(false);
        currentHeight_ = 0.f;
    } else {
        if(currentHeaderMode_ != NO_HEADER)
            parentLabel_->setVisible(true);
        currentHeight_ = currentHeaderMode_;
    }

    foreach(PropertyGraphicsItem* propItem, propertyItems_){
        if(propItem->isVisible()){
            propItem->setPos(0,currentHeight_);
            currentHeight_ += propItem->boundingRect().height();
        }
    }

    foreach(PropertyListGraphicsItem* listItem, subPropertyLists_){
            currentHeight_ += listItem->setChildListPosition(mapToItem(listItem->parentItem(),QPointF(0,currentHeight_)));
    }
}


//---------------------------------------------------------------------------------------------------------------
//                  property management                                                                          
//---------------------------------------------------------------------------------------------------------------
void PropertyListGraphicsItem::addProperties(const QList<Property*>& items) {
    foreach(Property* prop, items) {
        PropertyGraphicsItem* item = new PropertyGraphicsItem(prop,this);
        propertyItems_.append(item);
        connect(item,SIGNAL(linkProperties(PropertyGraphicsItem*,PropertyGraphicsItem*)),this,SIGNAL(linkProperties(PropertyGraphicsItem*,PropertyGraphicsItem*)));
    }
    findNewListWidth();
    layoutChildItems();
}

void PropertyListGraphicsItem::addChildPropertyList(PropertyListGraphicsItem* list) {
    subPropertyLists_.append(list);
    list->setHeaderMode(SMALL_HEADER);
    connect(list,SIGNAL(linkProperties(PropertyGraphicsItem*,PropertyGraphicsItem*)),this,SIGNAL(linkProperties(PropertyGraphicsItem*,PropertyGraphicsItem*)));
    findNewListWidth();
    layoutChildItems();
}

float PropertyListGraphicsItem::setChildListPosition(QPointF point){
    setPos(point);
    layoutChildItems();
    return currentHeight_;
}

void PropertyListGraphicsItem::setVisible(bool visible) {
    switch(currentPropertyMode_){
    case SHOW_ALL_PROPERTIES:
        foreach (PropertyGraphicsItem* propertyItem, propertyItems_) {
            propertyItem->setVisible(visible);
        }
        break;
    case SHOW_LINKED_PROPERTIES:
        foreach (PropertyGraphicsItem* propertyItem, propertyItems_) {
            if(visible == false)
                propertyItem->setVisible(false);
            else {
                if(!propertyItem->getSourceLinkList().empty() || !propertyItem->getDestinationLinkList().empty())
                    propertyItem->setVisible(true);
                else
                    propertyItem->setVisible(false);
            }
        }
        break;
    case SHOW_CAMERA_PROPERTIES:
        foreach (PropertyGraphicsItem* propertyItem, propertyItems_) {
            if(visible == false)
                propertyItem->setVisible(false);
            else {
                if(dynamic_cast<const CameraProperty*>(propertyItem->getProperty()))
                    propertyItem->setVisible(true);
                else
                    propertyItem->setVisible(false);
            }
        }
        break;
    default:
        tgtAssert(false,"should not get here");
        break;
    }

    switch(currentPropertyModifier_) {
    case HIDE_CAMERA_AND_SIZE_PROPERTIES:
    case HIDE_CAMERA_PROPERTIES:
        foreach (PropertyGraphicsItem* propertyItem, propertyItems_) {
            if(dynamic_cast<CameraProperty*>(const_cast<Property*>(propertyItem->getProperty())))
                propertyItem->setVisible(false);
        }
        if(currentPropertyModifier_ == HIDE_CAMERA_PROPERTIES)
            break; //stop, if just camera porperties are hidden
    case HIDE_SIZE_PROPERTIES:
        foreach (PropertyGraphicsItem* propertyItem, propertyItems_) {
            if(dynamic_cast<RenderSizeOriginProperty*>(const_cast<Property*>(propertyItem->getProperty())) ||
               dynamic_cast<RenderSizeReceiveProperty*>(const_cast<Property*>(propertyItem->getProperty())))
                propertyItem->setVisible(false);
        }
        break;
    default:
        break;
    }

    if(currentHeaderMode_ == NO_HEADER || getVisiblePropertyGraphicsItems().empty())
        parentLabel_->setVisible(false);
    else
        parentLabel_->setVisible(visible);

    //go into child lists
    foreach(PropertyListGraphicsItem* listItem, subPropertyLists_){
        listItem->setVisible(visible);
    }
    
    QGraphicsItem::setVisible(visible);
    if(visible == true)
        layoutChildItems();
}


//---------------------------------------------------------------------------------------------------------------
//                  modification management                                                                          
//---------------------------------------------------------------------------------------------------------------
void PropertyListGraphicsItem::setPosition(const QPointF& pos) {
    QGraphicsItem::setPos(pos);
    layoutChildItems();
}

void PropertyListGraphicsItem::setPosition(qreal x, qreal y) {
    QGraphicsItem::setPos(x,y);
    layoutChildItems();
}

void PropertyListGraphicsItem::prepareGeometryChange() {
    NWEBaseGraphicsItem::prepareGeometryChange();
    foreach(PropertyGraphicsItem* item, this->getPropertyGraphicsItems())
        item->prepareGeometryChange();
    foreach(PropertyListGraphicsItem* item, getSubPropertyLists())
        item->prepareGeometryChange();
}

//---------------------------------------------------------------------------------------------------------------
//                  width management                                                                          
//---------------------------------------------------------------------------------------------------------------
void PropertyListGraphicsItem::findNewListWidth() {
    qreal width = getParentLabel()->boundingRect().width();
    foreach (PropertyGraphicsItem* propertyItem, getPropertyGraphicsItems()) {
        width = std::max(width, propertyItem->boundingRect().width());
    }
    foreach(PropertyListGraphicsItem* listItem, getSubPropertyLists()){
        width = std::max(width,listItem->boundingRect().width());
    }
    resizeListWidth(width);
    layoutChildItems();
}

void PropertyListGraphicsItem::resizeListWidth(qreal width){
    currentWidth_ = width;
    foreach(PropertyGraphicsItem* item, propertyItems_){
        item->prepareGeometryChange();
        item->resetSize(width);
    }
    foreach(PropertyListGraphicsItem* item, subPropertyLists_){
        item->prepareGeometryChange();
        item->resizeListWidth(width);
    }
}

qreal PropertyListGraphicsItem::getCurrentWidth() const {
    return currentWidth_;
}

} // namespace
