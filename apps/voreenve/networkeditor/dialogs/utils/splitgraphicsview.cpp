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

#include "splitgraphicsview.h"

#include "tgt/assert.h"

#include "voreen/core/properties/property.h"

#include "../../graphicitems/core/propertygraphicsitem.h"
#include "../../graphicitems/core/propertylistgraphicsitem.h"
#include "../../graphicitems/connections/propertylinkarrowgraphicsitem.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGraphicsItem>

namespace voreen {

SplitGraphicsView::SplitGraphicsView(QWidget* parent) : QGraphicsView(parent) 
    , buddyView_(0)
    , splitPosition_(UNKNOWN)
    , currentDragedArrow_(0)
{
    setScene(new QGraphicsScene(this));
    setMouseTracking(true);
    setBackgroundBrush(QBrush(Qt::darkGray));
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSceneRect(-50000,-50000,100000,100000);
    setViewportUpdateMode(QGraphicsView::NoViewportUpdate);    
    connect(scene(),SIGNAL(selectionChanged()),this,SLOT(updateBuddySelection()));
}

SplitGraphicsView::~SplitGraphicsView() {
}

//---------------------------------------------------------------------------------------------------------------
//                  getter and setter                                                                            
//---------------------------------------------------------------------------------------------------------------
SplitGraphicsView* SplitGraphicsView::getBuddyView() const{
    return buddyView_;
}

void SplitGraphicsView::setBuddyView(SplitGraphicsView* view) {
    tgtAssert(splitPosition_ == RIGHT || splitPosition_ == LEFT,"No buddy view allowed in this split position");
     tgtAssert((splitPosition_ == RIGHT && view->getSplitPosition() == LEFT) ||
            (splitPosition_ == LEFT && view->getSplitPosition() == RIGHT),"Buddy view has wrong split position");

    buddyView_ = view;
}

SplitGraphicsView::SplitPosition SplitGraphicsView::getSplitPosition() const{
    return splitPosition_;
}

void SplitGraphicsView::setSplitPosition(SplitPosition sp) {
    tgtAssert(sp != UNKNOWN, "Unknown is here not allowed");
    switch(sp) {
    case CENTER:
        setMinimumSize(700, 480);
        setMaximumHeight(480);
        setMaximumWidth(700);
        break;
    case LEFT:
        setMinimumSize(350, 480);
        setMaximumHeight(480);
        setMaximumWidth(350);
        break;
    case RIGHT:
        setMinimumSize(350, 480);
        setMaximumHeight(480);
        setMaximumWidth(350);
        break;
    default:
        tgtAssert(false,"shoudn't get here");
            break;
    }
    splitPosition_ = sp;
}

PropertyListGraphicsItem* SplitGraphicsView::getPropertyList() const{
    return list_;
}

void SplitGraphicsView::setPropertyList(PropertyListGraphicsItem* list) {
    list_ = list;
    list_->setScene(scene());
}

void SplitGraphicsView::updateBuddySelection() {
    if(buddyView_){
        buddyView_->scene()->blockSignals(true);
        buddyView_->scene()->clearSelection();
        QList<QGraphicsItem*> list = scene()->selectedItems();
        foreach(QGraphicsItem* item, list) {
            if(item->type() == UserTypesPropertyLinkArrowGraphicsItem){
                PropertyLinkArrowGraphicsItem* arrow = dynamic_cast<PropertyLinkArrowGraphicsItem*>(item);
                if(arrow->getSourceItem()->scene() == scene()) {
                    foreach(PropertyLinkArrowGraphicsItem* pa, arrow->getDestinationItem()->getDestinationLinkList()){
                        if(pa->getSourceItem() == arrow->getSourceItem()) {
                            pa->setSelected(true);
                            pa->setZValue(ZValuesSelectedPropertyLinkArrowGraphicsItem);
                            break;
                        }
                    }
                } else {
                    foreach(PropertyLinkArrowGraphicsItem* pa, arrow->getSourceItem()->getSourceLinkList()){
                        if(pa->getDestinationItem() == arrow->getDestinationItem()) {
                            pa->setSelected(true);
                            pa->setZValue(ZValuesSelectedPropertyLinkArrowGraphicsItem);
                            break;
                        }
                    }
                }
                break;
            }
        }
        buddyView_->scene()->blockSignals(false);
    }
}

void SplitGraphicsView::updateViewport() {
    viewport()->update();
    if(buddyView_){
        buddyView_->viewport()->update();
    }
}
//---------------------------------------------------------------------------------------------------------------
//                  current draged arrow functions                                                               
//---------------------------------------------------------------------------------------------------------------
PropertyLinkArrowGraphicsItem* SplitGraphicsView::getCurrentDragedArrow() {
    return currentDragedArrow_;
}

void SplitGraphicsView::initCurrentDragedArrow(PropertyGraphicsItem* item, QPointF mousePos) {
    tgtAssert(!currentDragedArrow_,"currentDragedArrow is already benn initilized");
    currentDragedArrow_ = new PropertyLinkArrowGraphicsItem(item);
    currentDragedArrow_->setDestinationPoint(mousePos);
    currentDragedArrow_->setSourceArrowHeadDirection(ConnectionBaseGraphicsItem::ArrowHeadDirectionEW);
    currentDragedArrow_->setIsHovered(true);
    currentDragedArrow_->setVisible(true);
    if(buddyView_ && !buddyView_->getCurrentDragedArrow()){
        buddyView_->initCurrentDragedArrow(item,mousePos);
        buddyView_->getCurrentDragedArrow()->scene()->removeItem(buddyView_->getCurrentDragedArrow());
        buddyView_->scene()->addItem(buddyView_->getCurrentDragedArrow());
    }
}


void SplitGraphicsView::deleteCurrentDragedArrow() {
    delete currentDragedArrow_;
    currentDragedArrow_ = 0;
    if(buddyView_ && buddyView_->getCurrentDragedArrow())
        buddyView_->deleteCurrentDragedArrow();
}

//---------------------------------------------------------------------------------------------------------------
//                  events                                                                                       
//---------------------------------------------------------------------------------------------------------------
void SplitGraphicsView::wheelEvent(QWheelEvent* event) {
    QPointF newPos = list_->pos();
    newPos.setY(newPos.y() + event->delta()/2.0);

    QRectF listBoundingRect = list_->boundingRect();

    qreal visibleAreaTop = mapToScene(0,rect().top()).y();
    qreal visibleAreaBottom = mapToScene(0,rect().bottom()).y();

    if(listBoundingRect.height() <= rect().height()) return;

    if(listBoundingRect.height() <= (visibleAreaBottom - visibleAreaTop)) {// list fits in view
        if(newPos.y() < visibleAreaTop)
            newPos.setY(visibleAreaTop);
        else if(newPos.y() + listBoundingRect.height() > visibleAreaBottom)
            newPos.setY(visibleAreaBottom - listBoundingRect.height());
    } else { //list doesn't fit in view
        if(newPos.y() > visibleAreaTop)
            newPos.setY(visibleAreaTop);
        else if(newPos.y() + listBoundingRect.height() < visibleAreaBottom)
            newPos.setY(visibleAreaBottom - listBoundingRect.height());
    }
    
    list_->setPosition(newPos);
    updateViewport();
}

void SplitGraphicsView::mousePressEvent(QMouseEvent* event) {
    QGraphicsView::mousePressEvent(event);
    if (event->buttons() & Qt::LeftButton){
       QList<QGraphicsItem*> list = scene()->items(mapToScene(event->pos()));
       foreach(QGraphicsItem* item, list){
           if(item->type() == UserTypesPropertyGraphicsItem){
               initCurrentDragedArrow(qgraphicsitem_cast<PropertyGraphicsItem*>(item), mapToScene(event->pos()));
               break;
           }
       }
    } else {
        deleteCurrentDragedArrow();
    }
    updateViewport();
}

void SplitGraphicsView::mouseMoveEvent(QMouseEvent* event) {
    if(currentDragedArrow_){
        currentDragedArrow_->setDestinationPoint(mapToScene(event->pos()));
        currentDragedArrow_->setColorConnectableMode(ConnectionBaseGraphicsItem::CCM_DEFAULT);

        if(buddyView_){
            buddyView_->getCurrentDragedArrow()->setDestinationPoint(mapToScene(event->pos()));
            buddyView_->getCurrentDragedArrow()->setColorConnectableMode(ConnectionBaseGraphicsItem::CCM_DEFAULT);
            QList<QGraphicsItem*> iList = buddyView_->scene()->items(mapToScene(event->pos()));
            foreach(QGraphicsItem* item, iList) {
                if((item->type() == UserTypesPropertyGraphicsItem)){
                    const Property* dstProp = dynamic_cast<const PropertyGraphicsItem*>(item)->getProperty();
                    if (currentDragedArrow_->getSourceItem()->getProperty()->getClassName() == dstProp->getClassName()){
                        currentDragedArrow_->setColorConnectableMode(ConnectionBaseGraphicsItem::CCM_YES);
                        buddyView_->getCurrentDragedArrow()->setColorConnectableMode(ConnectionBaseGraphicsItem::CCM_YES);
                    } else if (currentDragedArrow_->getSourceItem()->getProperty()->isLinkableWith(dstProp)){
                        currentDragedArrow_->setColorConnectableMode(ConnectionBaseGraphicsItem::CCM_MAYBE);
                        buddyView_->getCurrentDragedArrow()->setColorConnectableMode(ConnectionBaseGraphicsItem::CCM_MAYBE);
                    } else {
                        currentDragedArrow_->setColorConnectableMode(ConnectionBaseGraphicsItem::CCM_NO);
                        buddyView_->getCurrentDragedArrow()->setColorConnectableMode(ConnectionBaseGraphicsItem::CCM_NO);
                    }
                break;
                }
            }
        } else {
            QList<QGraphicsItem*> iList = scene()->items(mapToScene(event->pos()));
            foreach(QGraphicsItem* item, iList) {
                if((item != currentDragedArrow_->getSourceItem()) && (item->type() == UserTypesPropertyGraphicsItem)){
                    const Property* dstProp = dynamic_cast<const PropertyGraphicsItem*>(item)->getProperty();
                    if (currentDragedArrow_->getSourceItem()->getProperty()->getClassName() == dstProp->getClassName())
                        currentDragedArrow_->setColorConnectableMode(ConnectionBaseGraphicsItem::CCM_YES);
                    else if (currentDragedArrow_->getSourceItem()->getProperty()->isLinkableWith(dstProp))
                        currentDragedArrow_->setColorConnectableMode(ConnectionBaseGraphicsItem::CCM_MAYBE);
                    else
                        currentDragedArrow_->setColorConnectableMode(ConnectionBaseGraphicsItem::CCM_NO);
                break;
                }
            }
        }
    }
    QGraphicsView::mouseMoveEvent(event);
    updateViewport();
}

void SplitGraphicsView::mouseReleaseEvent(QMouseEvent* event) {
    if (currentDragedArrow_) {
        if(buddyView_) {
            QList<QGraphicsItem*> iList = buddyView_->scene()->items(mapToScene(event->pos()));
            foreach(QGraphicsItem* item, iList) {
                if(item->type() == UserTypesPropertyGraphicsItem){
                    PropertyGraphicsItem* dstPropItem = dynamic_cast<PropertyGraphicsItem*>(item);
                    const Property* dstProp = dstPropItem->getProperty();
                    if (currentDragedArrow_->getSourceItem()->getProperty()->isLinkableWith(dstProp))
                        emit linkProperties(currentDragedArrow_->getSourceItem(),dstPropItem);
                    break;
                }
            }
        } else {
            QList<QGraphicsItem*> iList = scene()->items(mapToScene(event->pos()));
            foreach(QGraphicsItem* item, iList) {
                if((item != currentDragedArrow_->getSourceItem()) && (item->type() == UserTypesPropertyGraphicsItem)){
                    PropertyGraphicsItem* dstPropItem = dynamic_cast<PropertyGraphicsItem*>(item);
                    const Property* dstProp = dstPropItem->getProperty();
                    if (currentDragedArrow_->getSourceItem()->getProperty()->isLinkableWith(dstProp))
                        emit linkProperties(currentDragedArrow_->getSourceItem(),dstPropItem);
                    break;
                }
            }
        }
        deleteCurrentDragedArrow();
    }
    QGraphicsView::mouseReleaseEvent(event);
    updateViewport();
}

void SplitGraphicsView::keyPressEvent(QKeyEvent* event) {
    QGraphicsView::keyPressEvent(event);
}

} // namespace voreen
