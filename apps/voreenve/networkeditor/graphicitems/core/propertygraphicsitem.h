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

#ifndef VRN_PROPERTYGRAPHICSITEM_H
#define VRN_PROPERTYGRAPHICSITEM_H

#include "../nwebasegraphicsitem.h"

namespace voreen {

class Property;
class PropertyListGraphicsItem;
class PropertyOwnerGraphicsItem;
class PropertyLinkArrowGraphicsItem;

class PropertyGraphicsItem : public NWEBaseGraphicsItem {
Q_OBJECT

public:
    //constructor + destructor
    PropertyGraphicsItem(const Property* prop, PropertyListGraphicsItem* parent);
    ~PropertyGraphicsItem();

    //---------------------------------------------------------------------------------
    //      nwebasegraphicsitem functions                                                       
    //---------------------------------------------------------------------------------
    int type() const {return UserTypesPropertyGraphicsItem;}
    void updateNWELayerAndCursor() {};
    void prepareGeometryChange();

    //style functions
    QRectF boundingRect() const;
    QPainterPath shape() const;
protected:
    void initializePaintSettings();
    void prePaint(QPainter* painter, const QStyleOptionGraphicsItem* option);
    void mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

public:
    //handling child items
    void layoutChildItems(){};
protected:
    void createChildItems(){};
    void deleteChildItems();

    //---------------------------------------------------------------------------------
    //      propertygraphicsitem functions                                                      
    //---------------------------------------------------------------------------------
public:
    //getter and setter
    const Property* getProperty() const;
    PropertyListGraphicsItem* getPropertyList() const;
    QGraphicsTextItem* getPropertyLabel();
    const QGraphicsTextItem* getPropertyLabel() const;
    PropertyOwnerGraphicsItem* getPropertyOwnerGraphicsItem() const;
    QList<PropertyLinkArrowGraphicsItem*> getSourceLinkList();
    QList<PropertyLinkArrowGraphicsItem*> getDestinationLinkList();

    QPointF getLeftDockingPoint() const;
    QPointF getRightDockingPoint() const;

    void setVisible(bool visible);
    void setShowLinkArrows(bool b);
    void setPropertyListLinkedTo(PropertyListGraphicsItem* list);

    //size management
    void resetSize(qreal width);
    qreal getBoundingWidth() const;

    //links
    PropertyLinkArrowGraphicsItem* addGraphicalLink(PropertyGraphicsItem* prop);
    void removeGraphicalLink(PropertyGraphicsItem* prop);
protected:
    void addDestinationLink(PropertyGraphicsItem* prop);
    void removeDestinationLink(PropertyGraphicsItem* prop);

public:
    //events
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

signals:
    void linkProperties(PropertyGraphicsItem* src, PropertyGraphicsItem* dst);

    //members
private:
    const Property* property_;
    PropertyListGraphicsItem* propertyListItem_;

    QGraphicsTextItem* propertyLabel_;

    QList<PropertyLinkArrowGraphicsItem*> sourceLinkList_;
    QList<PropertyLinkArrowGraphicsItem*> destinationLinkList_;

    bool showLinkArrows_;
    PropertyListGraphicsItem* propertyListLinkedTo_; 

    qreal boundingWidth_;
};

} //namespace voreen

#endif // VRN_PROPERTYGRAPHICSITEM_H
