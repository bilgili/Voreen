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

#include "propertylistbuttongraphicsitem.h"
#include "../../styles/nwestyle_base.h"

#include "../core/portownergraphicsitem.h"
#include <QGraphicsSceneMouseEvent>

namespace voreen {

PropertyListButtonGraphicsItem::PropertyListButtonGraphicsItem(NWEBaseGraphicsItem* parent)
    : NWEBaseGraphicsItem(parent->getNetworkEditor())
{
    setParent(parent);
    setParentItem(parent);
    setZValue(ZValuesPropertyListButtonGraphicsItem);
    setAcceptsHoverEvents(true);
    setToolTip(tr("show/hide list of properties"));
}

//---------------------------------------------------------------------------------------------------------------
//                  getter and setter                                                                            
//---------------------------------------------------------------------------------------------------------------
QPointF PropertyListButtonGraphicsItem::dockingPoint() const {
    QRectF bounds = boundingRect();
    qreal centerX = bounds.x() + bounds.width() / 2.f;
    qreal centerY = bounds.y() + bounds.height() / 2.f;
    return mapToScene(QPointF(centerX, centerY));
}

//---------------------------------------------------------------------------------------------------------------
//                  nwebasegraphicsitem functions                                                                         
//---------------------------------------------------------------------------------------------------------------
QRectF PropertyListButtonGraphicsItem::boundingRect() const {
    return currentStyle()->PropertyListButtonGI_boundingRect(this);
}

QPainterPath PropertyListButtonGraphicsItem::shape() const {
    return currentStyle()->PropertyListButtonGI_shape(this);
}

void PropertyListButtonGraphicsItem::initializePaintSettings() {
    currentStyle()->PropertyListButtonGI_initializePaintSettings(this);
}

void PropertyListButtonGraphicsItem::prePaint(QPainter* painter, const QStyleOptionGraphicsItem* option) {}

void PropertyListButtonGraphicsItem::mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    currentStyle()->PropertyListButtonGI_paint(this, painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------
//                  events                                                                                       
//---------------------------------------------------------------------------------------------------------------
void PropertyListButtonGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton)
        emit pressed();
    else{
        PortOwnerGraphicsItem* parent = static_cast<PortOwnerGraphicsItem*>(parentItem());
        parent->mouseReleaseEvent(event);
    }
}

void PropertyListButtonGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    PortOwnerGraphicsItem* parent = static_cast<PortOwnerGraphicsItem*>(parentItem());
    parent->mouseMoveEvent(event);
}

void PropertyListButtonGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    PortOwnerGraphicsItem* parent = static_cast<PortOwnerGraphicsItem*>(parentItem());
    parent->mouseReleaseEvent(event);
}

} // namespace voreen
