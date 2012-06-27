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

#include <QtGui>
//#include "guiitem.h"

#include "voreen/core/vis/processors/volumesetsourceprocessor.h"

namespace voreen {

class Processor;
class PortGraphicsItem;
class TextGraphicsItem;
class ArrowGraphicsItem;
class Port;

/**
 * Representation of a Processor in the scene. The interface to connect with other processor- or port-items
 * corresponds to the interface of the original processor- and port-classes.
 */
class ProcessorGraphicsItem : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param type type of represented processor
     */
    ProcessorGraphicsItem(Identifier type, QGraphicsItem* parent=0);
    ProcessorGraphicsItem(Processor* processor, QGraphicsItem* parent=0);
    virtual ~ProcessorGraphicsItem();

    /**
     * Returns the name of this processor item.
     */
    const std::string getName() const;

    /**
     * Creates and adjusts the port items
     */
    void createIO();

    Identifier getType() { return type_; }

    void setName(const std::string& name);

    /**
     * Returns the type that identifies this item's class.
     */
    int type() const { return Type; }

    virtual PortGraphicsItem* getPortItem(Identifier ident);

    /**
    * Returns the inports of this processor item.
    */
    std::vector<PortGraphicsItem*> getInports() {return inports_;}

    /**
    * Returns the inports of this processor item.
    */
    std::vector<PortGraphicsItem*> getOutports() {return outports_;}

    /**
    * Returns the coprocessor inports of this processor item.
    */
    std::vector<PortGraphicsItem*> getCoProcessorInports() {return coProcessorInports_;}

    /**
    * Returns the coprocessor inports of this processor item.
    */
    std::vector<PortGraphicsItem*> getCoProcessorOutports() {return coProcessorOutports_;}

    /**
    * Returns the inport-item belonging to the given port.
    */
    PortGraphicsItem* getInport(Port* port);

    /**
    * Returns the outport-item belonging to the given port.
    */
    PortGraphicsItem* getOutport(Port* port);

    /**
    * Returns the first inport with the matching type.
    * If no such port exits returns 0.
    */
    PortGraphicsItem* getInport(Identifier type);

    /**
    * Returns the outport with the matching type.
    * If no such port exits returns 0.
    */
    PortGraphicsItem* getOutport(Identifier type);

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
    * TODO: rename parameters like in disconnect(...)
    * @param sourcePort outport of this item
    * @param dest processor item to connect with
    * @param destPort inport of dest to connect with
    * @return true if connect succeeds
    */
    bool connect(PortGraphicsItem* outport, PortGraphicsItem* inport);
    bool testConnect(PortGraphicsItem* outport, PortGraphicsItem* inport);

    /**
    * Same as connect, but creates an arrow.
    */
    bool connectAndCreateArrow(PortGraphicsItem* outport, PortGraphicsItem* inport);

    void connectGuionly(PortGraphicsItem* outport, PortGraphicsItem* inport);

    /**
    * Disconnects the given ports. Returns true if disconnecting successful.
    */
    bool disconnect(PortGraphicsItem* outport, PortGraphicsItem* inport);

    /**
    * Disconnects everything connected to this item. Returns true if disconnecting successful.
    */
    bool disconnectAll();

    void repositionPorts();

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

    void contentChanged();

    void enterRenameMode();

    /**
     * @param id used to identify specific object
     * Returns the represented
     */
    Processor* getProcessor(PortGraphicsItem* /*port*/=0) { return processor_; }

    ProcessorGraphicsItem& saveMeta();
    ProcessorGraphicsItem& loadMeta();

    /**
     * Used to identify an item in the scene as ProcessorGraphicsItem
     */
    enum { Type = UserType + 2 };

//    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    // Constructs a QRectF for round boxes.
    QRectF boundingRect() const;
    // Constructs a QRectF for drawing processors.
    QRectF drawingRect() const;
    // Constructs a QPainterPath for round buttons.
    QPainterPath coprocessorBoundingPath(QRectF) const;
    QPainterPath canvasBoundingPath(QRectF) const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /** DropEvent for dropping VolumeSets onto VolumeSetSourceProcessors.
     * This method is called by NetworkEditor, when the dropping target is
     * a NetworkEditor and the event is delegated to this class.
     * Any NetworkEditor which does not hold a VolumeSetSourceProcessor*
     * in its member processor_ and has nothing to do with VolumeSet
     * should ignore this event.
     */
    void dropEvent(QDropEvent* event);

public slots:
    virtual void nameChanged();
    void renameFinished();

signals:
    void changed();
    void processorNameChanged(Processor*);

protected:
    /**
     * Does what has to be done on a state change (like a position change),
     * e.g. repaint the connected arrows.
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);


    // inports, outports
    std::vector<PortGraphicsItem*> inports_;
    std::vector<PortGraphicsItem*> outports_;

    // coprocessor ports
    std::vector<PortGraphicsItem*> coProcessorInports_;
    std::vector<PortGraphicsItem*> coProcessorOutports_;

    // item that shows the name of type_
    TextGraphicsItem* textItem_;

private:
    // sets the color depending on processor type
    void setColor();

    // color
    QColor color_;

    Identifier type_;
    // represented processor
    Processor* processor_;

};

} //namespace voreen

#endif // VRN_PROCESSORGRAPHICSITEM_H
