/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#ifndef VRN_PROCESSORGRAPHICSITEM_H
#define VRN_PROCESSORGRAPHICSITEM_H

#include <QObject>
#include <QGraphicsItem>

#include "voreen/qt/voreenqtglobal.h"
#include "voreen/qt/widgets/network/editor/networkeditor.h"
#include "voreen/qt/widgets/network/editor/propertylistgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/openpropertylistbutton.h"
#include "voreen/qt/widgets/network/editor/textgraphicsitem.h"

namespace voreen {

class Processor;
class PortGraphicsItem;
class ArrowGraphicsItem;
class Port;
class PropertyGraphicsItem;
class Property;

/**
 * Representation of a Processor in the scene. The interface to connect with other processor- or port-items
 * corresponds to the interface of the original processor- and port-classes.
 */
class ProcessorGraphicsItem : public QObject, public QGraphicsItem {
    Q_OBJECT
#if (QT_VERSION >= 0x040600)
    Q_INTERFACES(QGraphicsItem)
#endif
    friend class PortGraphicsItem;
public:
    ProcessorGraphicsItem(Processor* processor, NetworkEditor* networkEditor);
    virtual ~ProcessorGraphicsItem();

    ProcessorGraphicsItem* clone();

    /**
     * Returns the name of this processor item.
     */
    const QString getName() const;

    void setName(const std::string& name);
    void setName(const QString& name);

    /**
     * Returns the type that identifies this item's class.
     */
    int type() const;

    QPointF dockingPoint() const;

    /**
    * Returns the inports of this processor item.
    */
    QList<PortGraphicsItem*> getInports() const;

    /**
    * Returns the inports of this processor item.
    */
    QList<PortGraphicsItem*> getOutports() const;

    /**
    * Returns the coprocessor inports of this processor item.
    */
    QList<PortGraphicsItem*> getCoProcessorInports() const;

    /**
    * Returns the coprocessor inports of this processor item.
    */
    QList<PortGraphicsItem*> getCoProcessorOutports() const ;

    QList<PortGraphicsItem*> getPorts() const;

    PortGraphicsItem* getPort(Port* port);
    QPointF getPositionForPort(PortGraphicsItem* portItem);

    PropertyGraphicsItem* getProperty(Property* property);

    /**
    * Connects to the matching ports of dest.
    */
    bool connect(ProcessorGraphicsItem* dest);
    bool testConnect(ProcessorGraphicsItem* dest);

    /**
    * Connects outport to the matching ports of dest.
    */
    bool connect(PortGraphicsItem* outport, ProcessorGraphicsItem* dest);
    bool testConnect(PortGraphicsItem* outport, ProcessorGraphicsItem* dest);

    /**
    * Connect to a port to the port of another processor item.
    * @param sourcePort outport of this item
    * @param dest processor item to connect with
    * @param destPort inport of dest to connect with
    * @return true if connect succeeds
    */
    bool connect(PortGraphicsItem* outport, PortGraphicsItem* dest);
    bool testConnect(PortGraphicsItem* outport, PortGraphicsItem* dest);

    ///Test if connection failed because of different size origins:
    bool sizeOriginConnectFailed(PortGraphicsItem* outport, PortGraphicsItem* inport);

    /**
    * Same as connect, but creates an arrow.
    */
    bool connectAndCreateArrow(PortGraphicsItem* outport, PortGraphicsItem* inport);

    void connectGuionly(PortGraphicsItem* outport, PortGraphicsItem* inport);

    /**
    * Disconnects the given ports.
    */
    void disconnect(PortGraphicsItem* outport, PortGraphicsItem* inport);

    /**
    * Disconnects everything connected to this item.
    */
    void disconnectAll();

    /**
    * Removes this item from the scene.
    */
    void removeFromScene();

    /**
    * Removes arrows connected to the given item from the scene.
    */
    void removeArrows(ProcessorGraphicsItem* item);

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

    bool linkExists();
    void setLinkExists(bool b);

    void enterRenameMode();

    virtual void update();

    /**
     * @param id used to identify specific object
     * Returns the represented
     */
    Processor* getProcessor();

    ProcessorGraphicsItem& saveMeta();
    ProcessorGraphicsItem& loadMeta();

    /**
     * Used to identify an item in the scene as ProcessorGraphicsItem
     */
    enum { Type = UserType + UserTypesProcessorGraphicsItem };

//    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    // Constructs a QRectF for round boxes.
    QRectF boundingRect() const;
    // Constructs a QRectF for drawing processors.
    QRectF drawingRect() const;
    // Constructs a QPainterPath for round buttons.
    QPainterPath coprocessorBoundingPath(QRectF) const;
    QPainterPath canvasBoundingPath(QRectF) const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    QGraphicsItem* getPropertyLinkPortGraphicsItem();
    bool isExpanded();

    QList<PropertyGraphicsItem*> getPropertyGraphicsItems();

    void setLayer(NetworkEditorLayer layer);
    NetworkEditorLayer currentLayer() const;

    QList<ArrowGraphicsItem*> connectionsTo(ProcessorGraphicsItem* processorItem);

public slots:
    virtual void nameChanged();
    void renameFinished();
    void showPropertyList();

signals:
    void processorNameChanged(Processor*);
    //void hasChangedVisibilityOfPropertyList();
    //void managePropertyLinking(PropertyGraphicsItem* propertyGraphicsItem);
    void pressedPropertyGraphicsItem(PropertyGraphicsItem* propertyGraphicsItem);

    /**
     * Informs the parent NetworkEditor widget about changes of the connections
     * of the ports. This method is intended to be called by methods which change
     * the connections between the ports.
     */
    void portConnectionsChanged();

protected:
    /**
     * Does what has to be done on a state change (like a position change),
     * e.g. repaint the connected arrows.
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void createChildItems();
    void layoutChildItems();

    //void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event);

private:
    NetworkEditor* networkEditor_;
    // represented processor
    Processor* processor_;

    // inports, outports
    QList<PortGraphicsItem*> inports_;
    QList<PortGraphicsItem*> outports_;

    // coprocessor ports
    QList<PortGraphicsItem*> coProcessorInports_;
    QList<PortGraphicsItem*> coProcessorOutports_;

    // sets the color depending on processor type
    void setColor();

    // color
    QColor color_;

    bool linkExists_;

    // item that shows the name of type_
    TextGraphicsItem textItem_;
    PropertyListGraphicsItem propertyListItem_;

    OpenPropertyListButton openPropertyListButton_;

};

} //namespace voreen

#endif // VRN_PROCESSORGRAPHICSITEM_H
