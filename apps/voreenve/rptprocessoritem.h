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

#ifndef VRN_RPTPROCESSORITEM_H
#define VRN_RPTPROCESSORITEM_H

#include <QtGui>
#include "rptguiitem.h"

#include "voreen/core/vis/processors/volumesetsourceprocessor.h"

namespace voreen {

class Processor;
class Port;

/**
 * Representation of a Processor in the scene. The interface to connect with other processor- or port-items
 * corresponds to the interface of the original processor- and port-classes.
 */
class RptProcessorItem : public RptGuiItem {

    Q_OBJECT

public:
    /**
     * Constructor.
     * @param type type of represented processor
     */
    RptProcessorItem(Identifier type, QGraphicsItem* parent=0);
    RptProcessorItem(Processor* processor, QGraphicsItem* parent=0);
    ~RptProcessorItem();

    /** Enables or disables the context menu entry "aggregate". This is used
     * by the RptGraphWidget class in case of only one processore item being
     * selected.
     * @param   newState    the new state of the context menu entry
     * @return  previous state of the context menu entry
     */
    bool enableAggregateContextMenuEntry(const bool newState);

    /**
     * Creates and adjusts the port items
     */
    void createIO();

    Identifier getType() { return type_; }

    virtual void setName(std::string name);

    /**
     * Returns the type that identifies this item's class.
     */
    int type() const { return Type; }

    /**
     * @param id used to identify specific object
     * Returns the represented 
     */
    Processor* getProcessor(RptPortItem* /*port*/=0) { return processor_; }
    
    RptProcessorItem& saveMeta();
    RptProcessorItem& loadMeta();

    /**
     * Used to identfy an item in the scene as RptProcessorItem
     */     
    enum { Type = UserType + 2 };

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	// Constructs a QRectF for round boxes. 
	QRectF boundingRect() const;
    // Constructs a QRectF for drawing processors.
	QRectF drawingRect() const;
	// Constructs a QPainterPath for round buttons. 
    QPainterPath coprocessorBoundingPath(QRectF) const;
	QPainterPath canvasBoundingPath(QRectF) const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /** DropEvent for dropping VolumeSets onto VolumeSetSourceProcessors.
     * This method is called by RptGraphWidget, when the dropping target is
     * a RptProcessorItem and the event is delegated to this class.
     * Any RptProcessorItem which does not hold a VolumeSetSourceProcessor*
     * in its member processor_ and has nothing to do with VolumeSet
     * should ignore this event.
     */
    void dropEvent(QDropEvent* event);

public slots:
    void aggregateActionSlot();

signals:
    void aggregateSignal();

protected:
    void createContextMenu();

private:
    // sets the color depending on processor type
    void setColor();

    // color
    QColor color_;

    Identifier type_;
    // represented processor
    Processor* processor_;

    QAction* aggregateAction_;

};

} //namespace voreen

#endif // VRN_RPTPROCESSORITEM_H
