/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#ifndef VRN_RPTGUIITEM_H
#define VRN_RPTGUIITEM_H

#include <QtGui>
#include "voreen/core/vis/identifier.h"


namespace voreen {

class RptArrow;
class RptPortItem;
class RptTextItem;
class Processor;
class Port;
class RptPropertySetItem;

/**
 * Superclass for all items in the scene (except arrows). The interface to connect with other processor- or port-items
 * corresponds to the interface of the original processor- and port-classes.
 */
class RptGuiItem : public QObject, public QGraphicsItem {

    Q_OBJECT

public:
    /**
     * Constructor.
     */
    RptGuiItem(std::string name="", QGraphicsItem* parent=0);
    
    ~RptGuiItem();

    /**
     * Returns the name of this gui item.
     */
    std::string getName() const;

    /**
     * Sets the item's name.
     */
    virtual void setName(std::string name);

    /**
     * Returns the represented processor
     */
    virtual Processor* getProcessor(RptPortItem* port) = 0;

	virtual RptPortItem* getPortItem(Identifier ident);

    /**
     * Returns the inports of this processor item.
     */
    std::vector<RptPortItem*> getInports() {return inports_;}
    /**
     * Returns the inports of this processor item.
     */
    std::vector<RptPortItem*> getOutports() {return outports_;}

    /**
     * Returns the coprocessor inports of this processor item.
     */
    std::vector<RptPortItem*> getCoProcessorInports() {return coProcessorInports_;}
    /**
     * Returns the coprocessor inports of this processor item.
     */
    std::vector<RptPortItem*> getCoProcessorOutports() {return coProcessorOutports_;}
    
    /**
     * Returns the inport-item belonging to the given port.
     */
    RptPortItem* getInport(Port* port);
    /**
     * Returns the outport-item belonging to the given port.
     */
    RptPortItem* getOutport(Port* port);

    /**
     * Returns the first inport with the matching type.
     * If no such port exits returns 0.
     */
    RptPortItem* getInport(Identifier type);
    /**
     * Returns the outport with the matching type.
     * If no such port exits returns 0.
     */
    RptPortItem* getOutport(Identifier type);

    /**
     * Connects to the matching ports of dest.
     */
    bool connect(RptGuiItem* dest);
    bool testConnect(RptGuiItem* dest);

    /**
     * Connects outport to the matching ports of dest.
     */
    bool connect(RptPortItem* outport, RptGuiItem* dest);
    bool testConnect(RptPortItem* outport, RptGuiItem* dest);

    /**
     * Connect to a port to the port of another processor item.
     * TODO: rename parameters like in disconnect(...)
     * @param sourcePort outport of this item
     * @param dest processor item to connect with
     * @param destPort inport of dest to connect with
     * @return true if connect succeeds
     */
    bool connect(RptPortItem* outport, RptPortItem* inport);
    bool testConnect(RptPortItem* outport, RptPortItem* inport);

    /**
     * Same as connect, but creates an arrow.
     */
    bool connectAndCreateArrow(RptPortItem* outport, RptPortItem* inport);
    
    void connectGuionly(RptPortItem* outport, RptPortItem* inport);

    /**
     * Disconnects the given ports. Returns true if disconnecting successful.
     */
    bool disconnect(RptPortItem* outport, RptPortItem* inport);
    /**
     * Disconnects everything connected to this item. Returns true if disconnecting successful.
     */
    bool disconnectAll();

    /**
     * Adds the given property set.
     */
    void addPropertySet(RptPropertySetItem* propSet);
    
    /**
     * Returns the property sets this item belongs to.
     */
    std::vector<RptPropertySetItem*> getPropertySets() { return propertySets_; }
    
    /**
     * Removes the given property set.
     */
    void removePropertySet(RptPropertySetItem* propSet);

    void repositionPorts();

    /**
     * Removes this item from the scene.
     */
    void removeFromScene();

    /**
     * Removes arrows connected to the given item from the scene.
     */
    void removeArrows(RptGuiItem* item);
    
    /**
     * Removes all the arrows connected to this item from the scene.
     */
    void removeAllArrows();

    /**
     * Adds the arrows connected to this item to the scene.
     */
    virtual void showAllArrows();

    /**
     * Refresh source and dest points of connected arrows and repaint them.
     */
    virtual void adjustArrows();
    
    // TESTING
    int getCollisionPriority() { return collisionPriority_; }
    void setCollisionPriority(int c) { collisionPriority_ = c; }

    void contentChanged();

    QRectF boundingRect() const;
    
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

signals:
    /**
     * Send when delete is clicked in context menu.
     */
    void deleteSignal();
    void changed();


private slots:
    /**
     * Action for context menu. Sends deleteSignal (connected in mainwindow).
     */
    void deleteActionSlot();
    void renameActionSlot();

    /**
     * Changes name of gui item. Called when the text items text gets changed.
     */
    void renameSlot(std::string name);

protected:
    // create ports
    void createIO();

    void createContextMenu();

    /*
     * Does what has to be done on a state change (like a position change),
     * e.g. repaint the connected arrows.
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    // item that shows the name of type_
    RptTextItem* textItem_;

    // inports, outports
    std::vector<RptPortItem*> inports_;
    std::vector<RptPortItem*> outports_;

    // coprocessor ports
    std::vector<RptPortItem*> coProcessorInports_;
    std::vector<RptPortItem*> coProcessorOutports_;

    // property sets
    std::vector<RptPropertySetItem*> propertySets_;

    QMenu contextMenu_;

private:        
    // TESTING
    int collisionPriority_;
};

//---------------------------------------------------------------------------

/**
 * Representation of a Port in the scene.
 */
class RptPortItem : public QGraphicsItem {
    
public:
    /**
     * Constructor.
     * @param type type of represented processor
     * @param port represented port
     */
    RptPortItem(Identifier type, Port* port, RptGuiItem* parent);

    /**
     * Used to identfy an item in the scene as RptPortItem
     */  
    enum { Type = UserType + 1 };

    /**
     * Returns the type that identifies this item as a RptPortItem.
     */
    int type() const { return Type; }
    
    /**
     * Returns the type of the represented port.
     */
    Identifier getPortType() { return type_; }

    /**
     * Returns the represented port.
     */
    Port* getPort() { return port_; }

    /**
     * Returns the parentItem casted to RptGuiItem*
     */
    RptGuiItem* getParent() { return static_cast<RptGuiItem*>(parentItem()); }

    /**
     * Returns the ProcessorItems that are connected to this PortItem.
     */
    std::vector<RptPortItem*>& getConnected();
    
    /**
     * Connects given port to this PortItem.
     */
    void addConnection(RptPortItem* port);

    /**
     * Disconnects given inport from this port item.
     */
    void disconnect(RptPortItem* inport);

    /**
     * Returns true, if this port is an Outport.
     */
    bool isOutport();

    /**
     * Returns the outgoing arrows of this port.
     */
    std::vector<RptArrow*>& getArrowList() { return arrowList_; }

    /**
     * Adjusts and updates the arrows.
     */
    void adjustArrows();
    
    /**
     * Returns true, if an arrow to the given PortItem exists.
     */
    bool doesArrowExist(RptPortItem*);

    QRectF boundingRect() const;

protected:
    // sets the color depending on port type
    void setColor();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:    
    // list of outgoing arrows
    std::vector<RptArrow*> arrowList_;
    // list of gui items that are connected to this port.
    std::vector<RptPortItem*> connectedPorts_;
    // port type
    Identifier type_;
    // represented port
    Port* port_;
    // color
    QColor color_;
    QColor sunkenColor_;
};

//---------------------------------------------------------------------------


class RptTextItem : public QGraphicsTextItem {

    Q_OBJECT

public:
    RptTextItem(const QString& text, RptGuiItem* parent = 0, QGraphicsScene* scene = 0);

    /**
     * Used to identfy an item in the scene as RptPortItem
     */  
    enum { Type = UserType + 9 };

    /**
     * Returns the type that identifies this item as a RptPortItem.
     */
    int type() const { return Type; }

    void setContextMenu(QMenu* contextMenu);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

signals:
    /**
     * Sends the new text.
     */
    void sendText(std::string text);

private:
    QMenu* contextMenu_;
};

} //namespace voreen

#endif // VRN_RPTGUIITEM_H

