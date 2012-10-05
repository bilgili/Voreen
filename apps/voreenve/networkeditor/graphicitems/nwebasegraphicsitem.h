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

#ifndef VRN_NWEBASEGRAPHICSITEM_H
#define VRN_NWEBASEGRAPHICSITEM_H

#include <QGraphicsItem>

#include "../networkeditor.h"
#include "../networkeditor_settings.h"

class QAction;
class QMenu;

namespace voreen {

class NWEStyle_Base;
class ToolTipBaseGraphicsItem;

class NWEBaseGraphicsItem : public QGraphicsObject {
Q_OBJECT
public:
    //constructor + destructor
    NWEBaseGraphicsItem(NetworkEditor* nwe);
    ~NWEBaseGraphicsItem();
    //getter and setter
    NetworkEditor* getNetworkEditor() const;
    NetworkEditorLayer currentLayer() const;
    NetworkEditorCursorMode currentCursorMode() const;
    NWEStyle_Base* currentStyle() const;
    bool currentToolTipMode() const;
        //paint init
    bool isPaintInitialized() const;
    void resetPaintInitialization();
        //tooltips
    void setToolTipGraphicsItem(ToolTipBaseGraphicsItem* tooltip);
    ToolTipBaseGraphicsItem* getToolTipGraphicsItem() const;
        //just makes this function public
    virtual void prepareGeometryChange();
        //paint called by qt
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
protected:
    //members
    NetworkEditor* networkEditor_;
    ToolTipBaseGraphicsItem* toolTipItem_;
    bool paintHasBeenInitialized_;

    //---------------------------------------------------------------------------------
    //      NEEDS TO BE IMPLEMENTED                                                    
    //---------------------------------------------------------------------------------
public:
    virtual void updateNWELayerAndCursor() = 0;     ///update selectable etc...
    virtual int type() const = 0;                   ///needed fpr select events

    //style
    virtual QRectF boundingRect() const = 0;
    virtual QPainterPath shape() const = 0;   
protected:
    virtual void initializePaintSettings() = 0;     //is called on the first paint and only ones
    virtual void prePaint(QPainter* painter, const QStyleOptionGraphicsItem* option) = 0;
    virtual void mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) = 0;

public:
    //handling child elements
    virtual void layoutChildItems() = 0;
protected:
    virtual void createChildItems() = 0;
    virtual void deleteChildItems() = 0;

    //---------------------------------------------------------------------------------
    //      context menu                                                               
    //---------------------------------------------------------------------------------
public:
    void addActionsToContextMenu(QMenu* menu);
protected:
    virtual void setContextMenuActions();
    //member
    QList<QAction*> contextMenuActionList_;

    //---------------------------------------------------------------------------------
    //      events                                                                     
    //---------------------------------------------------------------------------------
protected: //handling of tooltips
    virtual void hoverEnterEvent (QGraphicsSceneHoverEvent* event);
    virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* event);
    virtual void hoverLeaveEvent (QGraphicsSceneHoverEvent* event);

};

} //namespace voreen

#endif // VRN_NWEBASEGRAPHICSITEM_H

