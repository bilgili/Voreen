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

#include "voreen/qt/networkeditor/graphicitems/connections/portarrowgraphicsitem.h"
#include "voreen/qt/networkeditor/editor_settings.h"

//style
#include "voreen/qt/networkeditor/styles/nwestyle_base.h"

//core
#include "voreen/core/ports/coprocessorport.h"
#include "voreen/core/ports/renderport.h"

//gi
#include "voreen/qt/networkeditor/graphicitems/core/portgraphicsitem.h"


#include <QPainterPath>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEffect>

namespace voreen {

PortArrowGraphicsItem::PortArrowGraphicsItem(PortGraphicsItem* sourceItem)
    : ConnectionBaseGraphicsItem(sourceItem, 0)
    , clickedPoint_(), arrowDraged_(false), dragedArrowMoved_(false), currentArrowInitialized_(false)
{
    if(!sourceItem->getPort()->isLoopPort())
        if (dynamic_cast<CoProcessorPort*>(sourceItem->getPort()))
            destinationArrowHeadDirection_ = ArrowHeadDirectionRight;
        else
            destinationArrowHeadDirection_ = ArrowHeadDirectionDown;

    setFlag(ItemIsSelectable);
    setZValue(ZValuesPortArrowGraphicsItem);
    setAcceptsHoverEvents(true);
    setFlag(ItemIsPanel,true);

    sourceItem->scene()->addItem(this);
}

PortArrowGraphicsItem::~PortArrowGraphicsItem() {
}

//---------------------------------------------------------------------------------------------------------------
//                  getter and setter
//---------------------------------------------------------------------------------------------------------------
QPointF PortArrowGraphicsItem::getSourcePoint() const {
    QPointF s = ConnectionBaseGraphicsItem::getSourcePoint();

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

    QPointF d = ConnectionBaseGraphicsItem::getDestinationPoint();

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

//---------------------------------------------------------------------------------------------------------------
//                  nwebasegraphicsitem functions
//---------------------------------------------------------------------------------------------------------------
QRectF PortArrowGraphicsItem::boundingRect() const {
    return currentStyle()->PortArrowGI_boundingRect(this);
}

QPainterPath PortArrowGraphicsItem::shape() const {
    return currentStyle()->PortArrowGI_shape(this);
}

void PortArrowGraphicsItem::initializePaintSettings() {
    currentStyle()->PortArrowGI_initializePaintSettings(this);
}

void PortArrowGraphicsItem::mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget, NWEItemSettings& setting) {
    currentStyle()->PortArrowGI_paint(this, painter, option, widget, setting);
}

void PortArrowGraphicsItem::updateNWELayerAndCursor() {
    switch(currentCursorMode()) {
    case NetworkEditorCursorSelectMode:
        switch (currentLayer()) {
        case NetworkEditorLayerDataFlow:
            setFlag(ItemIsSelectable);
            setAcceptsHoverEvents(true);
            break;
        case NetworkEditorLayerGeneralLinking:
        case NetworkEditorLayerCameraLinking:
            setFlag(ItemIsSelectable, false);
            setAcceptsHoverEvents(false);
            break;
        case NetworkEditorLayerPortSizeLinking:
            setFlag(ItemIsSelectable, false);
            setAcceptsHoverEvents(false);
            break;
        default:
            tgtAssert(false, "shouldn't get here");
            break;
        }
        break;
    case NetworkEditorCursorMoveMode:
        switch (currentLayer()) {
        case NetworkEditorLayerDataFlow:
        case NetworkEditorLayerGeneralLinking:
        case NetworkEditorLayerCameraLinking:
            setFlag(ItemIsSelectable, false);
            setAcceptsHoverEvents(false);
            break;
        case NetworkEditorLayerPortSizeLinking:
            setFlag(ItemIsSelectable, false);
            setAcceptsHoverEvents(false);
             break;
        default:
            tgtAssert(false, "shouldn't get here");
            break;
        }
        break;
    default:
        tgtAssert(false,"should not get here");
        break;
    }
}

//---------------------------------------------------------------------------------------------------------------
//                  events
//---------------------------------------------------------------------------------------------------------------
void PortArrowGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if(event->buttons() == Qt::LeftButton){
        clickedPoint_ = event->scenePos();
        arrowDraged_ = true;
        currentArrowInitialized_ = false;
        dragedArrowMoved_ = false;
    }
    QGraphicsItem::mousePressEvent(event);
}

void PortArrowGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if(arrowDraged_){
        if (!dragedArrowMoved_) {
            QPointF dis = event->scenePos()- clickedPoint_;
            if (sqrt(dis.x()*dis.x() + dis.y()+dis.y()) > paEventRadius){
                dragedArrowMoved_ = true;
            }
        }

        if (dragedArrowMoved_) {
            if(!currentArrowInitialized_){
                getSourceItem()->setCurrentPortArrow(this);
                currentArrowInitialized_ = true;
            }
            getSourceItem()->mouseMoveEvent(event);
        }
    } else
        QGraphicsItem::mouseMoveEvent(event);
}

void PortArrowGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if(dragedArrowMoved_)
        getSourceItem()->mouseReleaseEvent(event);
    else
        QGraphicsItem::mouseReleaseEvent(event);
}

/*
void PortArrowGraphicsItem::setBundleInfo(bool enable, ConnectionBundle* bundle) {
    bundled_ = enable;
    bundle_= bundle;
}

bool PortArrowGraphicsItem::isBundled() const {
    return bundled_;
}*/

} // namespace*/
