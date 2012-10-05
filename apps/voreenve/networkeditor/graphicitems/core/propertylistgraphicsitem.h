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

#ifndef VRN_PROPERTYLISTGRAPHICSITEM_H
#define VRN_PROPERTYLISTGRAPHICSITEM_H

#include "../nwebasegraphicsitem.h"

namespace voreen {

class Property;
class PropertyGraphicsItem;
class PropertyOwnerGraphicsItem;

class PropertyListGraphicsItem : public NWEBaseGraphicsItem {
Q_OBJECT

public:
    enum HeaderMode{        ///value equals size
        NO_HEADER = 0,
        SMALL_HEADER = 20,
        BIG_HEADER = 40
    };

    enum PropertyVisibleMode{
        SHOW_ALL_PROPERTIES,        ///show all properties
        SHOW_LINKED_PROPERTIES,     ///show just linked properties
        SHOW_CAMERA_PROPERTIES
    };

    enum PropertyVisibleModeModifier{
        HIDE_NO_PROPERTIES,
        HIDE_CAMERA_PROPERTIES,
        HIDE_SIZE_PROPERTIES,
        HIDE_CAMERA_AND_SIZE_PROPERTIES
    };

    enum LinkArrowVisibleMode{
        SHOW_NO_ARROWS,
        SHOW_ALL_ARROWS,
        SHOW_FRIEND_LIST_ARROWS
    };

    //constructor + destructor
    PropertyListGraphicsItem(PropertyOwnerGraphicsItem* parent);
    ~PropertyListGraphicsItem();

    //---------------------------------------------------------------------------------
    //      nwebasegraphicsitem functions                                                       
    //---------------------------------------------------------------------------------
    void updateNWELayerAndCursor() {};
    int type() const {return UserTypesPropertyListGraphicsItem;}

    //style
    QRectF boundingRect() const;
    QPainterPath shape() const;
protected:
    void initializePaintSettings();
    void prePaint(QPainter* painter, const QStyleOptionGraphicsItem* option);
    void mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

public:
    //handling child elements
    void layoutChildItems();
protected:
    void createChildItems();
    void deleteChildItems();

    //---------------------------------------------------------------------------------
    //      propertylistgraphicsitem functions                                                       
    //---------------------------------------------------------------------------------
public:
    //getter and setter
    PropertyOwnerGraphicsItem* getParent() const;

    PropertyGraphicsItem* getPropertyGraphicsItem(const Property* prop);
    QList<PropertyGraphicsItem*> getPropertyGraphicsItems();
    const QList<PropertyGraphicsItem*> getPropertyGraphicsItems() const;
    QList<PropertyGraphicsItem*> getVisiblePropertyGraphicsItems();
    const QList<PropertyGraphicsItem*> getVisiblePropertyGraphicsItems() const;
    QList<PropertyGraphicsItem*> getAllPropertyGraphicsItems();
    const QList<PropertyGraphicsItem*> getAllPropertyGraphicsItems() const;
    QList<PropertyGraphicsItem*> getAllVisiblePropertyGraphicsItems();
    const QList<PropertyGraphicsItem*> getAllVisiblePropertyGraphicsItems() const;

    void setPropertyVisibleMode(PropertyVisibleMode pvm);
    PropertyVisibleMode getPropertyVisibleMode() const;
    void setPropertyVisibleModifier(PropertyVisibleModeModifier pvmm);
    PropertyVisibleModeModifier getPropertyVisibleModeModifier() const;
    void setLinkArrowMode(LinkArrowVisibleMode lavm, PropertyListGraphicsItem* friendList = 0);
    LinkArrowVisibleMode getLinkArrowVisibleMode() const;
    void setHeaderMode(HeaderMode hm);
    HeaderMode getHeaderMode() const;
    
    const QGraphicsTextItem* getParentLabel() const;
    QGraphicsTextItem* getParentLabel();
    QList<PropertyListGraphicsItem*> getSubPropertyLists();
    const QList<PropertyListGraphicsItem*> getSubPropertyLists() const;
    void setScene(QGraphicsScene* scene);
    bool getIsVisibleInEditor() const;
    void setIsVisibleInEditor(bool b);
    void updateParentLabel();

    //modifications
    void setPosition(const QPointF& pos);
    void setPosition(qreal x, qreal y);
    void prepareGeometryChange();

    //property management
    void addProperties(const QList<Property*>& items);
    void addChildPropertyList(PropertyListGraphicsItem* list);
    float setChildListPosition(QPointF point);
    void setVisible(bool visible);

    //width management
    void findNewListWidth();
    void resizeListWidth(qreal width);
    qreal getCurrentWidth() const;

signals:
    void linkProperties(PropertyGraphicsItem* src, PropertyGraphicsItem* dst);

    //member
private:
    PropertyOwnerGraphicsItem* parent_;
    QList<PropertyGraphicsItem*> propertyItems_;
    QList<PropertyListGraphicsItem*> subPropertyLists_;    ///list of sub-property-lists
    QGraphicsTextItem* parentLabel_;

    HeaderMode currentHeaderMode_;
    PropertyVisibleMode currentPropertyMode_;
    PropertyVisibleModeModifier currentPropertyModifier_;
    LinkArrowVisibleMode currentLinkArrowMode_;
    PropertyListGraphicsItem* friendList_;
    bool isVisibleInEditor_;

    qreal currentHeight_;                           ///height of boundingbox
    qreal currentWidth_;
};

} // namespace

#endif // VRN_PROPERTYLISTGRAPHICSITEM_H
