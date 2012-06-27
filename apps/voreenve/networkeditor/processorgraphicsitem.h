/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "rootgraphicsitem.h"
#include "voreen/core/io/ioprogress.h"
#include "networkeditor_common.h"

#include <QProgressDialog>
#include <QTime>

namespace voreen {

/**
 * Representation of a Processor in the scene. The interface to connect with other processor- or port-items
 * corresponds to the interface of the original processor- and port-classes.
 */
class ProcessorGraphicsItem : public RootGraphicsItem, IOProgress {
Q_OBJECT
public:
    ProcessorGraphicsItem(Processor* processor, NetworkEditor* networkEditor);
    RootGraphicsItem* clone() const;

    enum { Type = UserType + UserTypesProcessorGraphicsItem };
    int type() const;

    bool hasProperty(const Property* prop) const;
    bool contains(RootGraphicsItem* rootItem) const;
    bool contains(Processor* processor) const;
    Processor* getProcessor() const;
    QList<Processor*> getProcessors() const;

    void saveMeta();
    void loadMeta();

    QList<Port*> getInports() const;
    QList<Port*> getOutports() const;
    QList<CoProcessorPort*> getCoProcessorInports() const;
    QList<CoProcessorPort*> getCoProcessorOutports() const;

    // progress bar related stuff
    virtual void update();
    virtual void setTotalSteps(int numSteps);
    virtual void show();
    virtual void hide();
    virtual void forceUpdate();

protected:

    QTime* time_; // progress bar related
    unsigned int progress_; // progress bar related
    unsigned int maxProgress_; // progress bar related

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);

public slots:
    void renameFinished(bool changeChildItems = true);

private:
    Processor* processor_;

    QList<Port*> inports_;
    QList<Port*> outports_;
    QList<CoProcessorPort*> coInports_;
    QList<CoProcessorPort*> coOutports_;
};

    //PropertyGraphicsItem* createPropertyGraphicsItem(const Property* property);
    //void removePropertyGraphicsItem(PropertyGraphicsItem* item);

    //void setName(const QString& name);

    //void updateName();


    //QPointF dockingPoint() const;

    //QList<PortGraphicsItem*> getPorts() const;

    //PortGraphicsItem* getPort(const Port* port);
    //QPointF getPositionForPort(PortGraphicsItem* portItem);

    //PropertyGraphicsItem* getProperty(Property* property);

    /**
    * Connects outport to the matching ports of dest.
    */
    //bool connect(PortGraphicsItem* outport, ProcessorGraphicsItem* dest, bool testing = false);

    /**
    * Connect to a port to the port of another processor item.
    * @param sourcePort outport of this item
    * @param dest processor item to connect with
    * @param destPort inport of dest to connect with
    * @return true if connect succeeds
    */
    //bool connect(PortGraphicsItem* outport, PortGraphicsItem* dest, bool testing = false);

    /**
    * Disconnects the given ports.
    */
    //void disconnect(PortGraphicsItem* outport, PortGraphicsItem* inport);

    /**
    * Refresh source and dest points of connected arrows and repaint them.
    */
    //void adjustArrows();

    //void enterRenameMode();

    /**
     * @param id used to identify specific object
     * Returns the represented
     */
    //Processor* getProcessor();

    //void saveMeta();
    //void /*loadMeta*/();

    // Constructs a QRectF for round boxes.
    //QRectF boundingRect() const;
    // Constructs a QRectF for drawing processors.
    //QRectF drawingRect() const;
    //void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    //void setLayer(NetworkEditorLayer layer);
    //NetworkEditorLayer currentLayer() const;

//public slots:
//    virtual void nameChanged();
//    void renameFinished();
//    void togglePropertyList();

//signals:
//    void processorNameChanged(Processor*);
//    void createdLinkArrow(LinkArrowGraphicsItem*);
//    void createLink(ProcessorGraphicsItem* src, ProcessorGraphicsItem* dest);
    //void hasChangedVisibilityOfPropertyList();
    //void managePropertyLinking(PropertyGraphicsItem* propertyGraphicsItem);

    /**
     * Informs the parent NetworkEditor widget about changes of the connections
     * of the ports. This method is intended to be called by methods which change
     * the connections between the ports.
     */
    //void portConnectionsChanged();

//protected:

    /**
     * Does what has to be done on a state change (like a position change),
     * e.g. repaint the connected arrows.
     */
    //QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    //void createChildItems();
    //void layoutChildItems();

    //void mousePressEvent(QGraphicsSceneMouseEvent* event);

//private:
    //void setLayerForArrow(PortArrowGraphicsItem* arrow, NetworkEditorLayer layer) const;

    //NetworkEditor* networkEditor_;
    // represented processor
    //Processor* processor_;

    //// inports, outports
    //QList<PortGraphicsItem*> inports_;
    //QList<PortGraphicsItem*> outports_;

    //// coprocessor ports
    //QList<PortGraphicsItem*> coProcessorInports_;
    //QList<PortGraphicsItem*> coProcessorOutports_;

    //// item that shows the name of type_
    //TextGraphicsItem textItem_;
    //PropertyListGraphicsItem propertyListItem_;

    //OpenPropertyListButton openPropertyListButton_;

    //LinkArrowGraphicsItemStub* currentArrow_;
//};

} //namespace voreen

#endif // VRN_PROCESSORGRAPHICSITEM_H
