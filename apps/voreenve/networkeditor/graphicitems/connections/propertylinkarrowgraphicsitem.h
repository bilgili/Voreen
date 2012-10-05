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

#ifndef VRN_PROPERTYLINKARROWGRAPHICSITEM_H
#define VRN_PROPERTYLINKARROWGRAPHICSITEM_H

#include "connectionbasegraphicsitem.h"

namespace voreen {

class PropertyGraphicsItem;

class PropertyLinkArrowGraphicsItem : public ConnectionBaseGraphicsItem {
public:
    enum DockPositions {
        DockPositionsLeftLeft,
        DockPositionsLeftRight,
        DockPositionsRightLeft,
        DockPositionsRightRight
    };

    //constructor and destructor
    PropertyLinkArrowGraphicsItem(PropertyGraphicsItem* sourceItem = 0, PropertyGraphicsItem* destinationItem = 0);
    ~PropertyLinkArrowGraphicsItem();

    //---------------------------------------------------------------------------------
    //      nwebasegraphicsitem functions                                                       
    //---------------------------------------------------------------------------------
    int type() const {return UserTypesPropertyLinkArrowGraphicsItem;}

    //style
    QRectF boundingRect() const;
    QPainterPath shape() const;
protected:
    void initializePaintSettings();
    void prePaint(QPainter* painter, const QStyleOptionGraphicsItem* option);
    void mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    //---------------------------------------------------------------------------------
    //      propertylinkarrowgraphicsitem functions                                             
    //---------------------------------------------------------------------------------
public:
    //getter and setter
    PropertyGraphicsItem* getSourceItem() const;
    PropertyGraphicsItem* getDestinationItem() const;
    DockPositions getDockPositions() const;

    QPointF getSourcePoint() const;
    QPointF getDestinationPoint() const;

    //events
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

protected:
    //member
    QPointF clickedPoint_; 
    bool initCurrentArrow_;
    bool movedAwayInEvent_;
};

} // namespace

#endif // VRN_PROPERTYLINKARROWGRAPHICSITEM_H
