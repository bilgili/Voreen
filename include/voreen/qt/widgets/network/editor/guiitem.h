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

#ifndef VRN_GUIITEM_H
#define VRN_GUIITEM_H

#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QObject>
#include <QString>

#include <vector>

#include "voreen/core/vis/identifier.h"

namespace voreen {

class ProcessorGraphicsItem;
class ArrowGraphicsItem;
class PortGraphicsItem;
class TextGraphicsItem;
class Processor;
class Port;

//---------------------------------------------------------------------------

/**
 * Representation of a Port in the scene.
 */
class PortGraphicsItem : public QGraphicsItem {

public:
    /**
     * Constructor.
     * @param type type of represented processor
     * @param port represented port
     */
    PortGraphicsItem(Identifier type, Port* port, ProcessorGraphicsItem* parent);

    /**
     * Used to identfy an item in the scene as PortGraphicsItem
     */
    enum { Type = UserType + 1 };

    /**
     * Returns the type that identifies this item as a PortGraphicsItem.
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
     * Returns the parentItem casted to PortGraphicsItem*
     */
    ProcessorGraphicsItem* getParent();

    /**
     * Returns the ProcessorItems that are connected to this PortItem.
     */
    std::vector<PortGraphicsItem*>& getConnected();

    /**
     * Connects given port to this PortItem.
     */
    void addConnection(PortGraphicsItem* port);

    /**
     * Disconnects given inport from this port item.
     */
    void disconnect(PortGraphicsItem* inport);

    /**
     * Returns true, if this port is an Outport.
     */
    bool isOutport();

    /**
     * Returns the outgoing arrows of this port.
     */
    std::vector<ArrowGraphicsItem*>& getArrowList() { return arrowList_; }

    /**
     * Adjusts and updates the arrows.
     */
    void adjustArrows();

    /**
     * Returns true, if an arrow to the given PortItem exists.
     */
    bool doesArrowExist(PortGraphicsItem* item);

    QRectF boundingRect() const;

protected:
    // sets the color depending on port type
    void setColor();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
    // list of outgoing arrows
    std::vector<ArrowGraphicsItem*> arrowList_;
    // list of gui items that are connected to this port.
    std::vector<PortGraphicsItem*> connectedPorts_;
    // port type
    Identifier type_;
    // represented port
    Port* port_;
    // color
    QColor color_;
    QColor sunkenColor_;
};

//---------------------------------------------------------------------------

class TextGraphicsItem : public QGraphicsTextItem {
    Q_OBJECT
public:
    TextGraphicsItem(const QString& text, ProcessorGraphicsItem* parent = 0, QGraphicsScene* scene = 0);

    ~TextGraphicsItem();

    /**
     * Used to identfy an item in the scene as PortGraphicsItem
     */
    enum { Type = UserType + 9 };

    /**
     * Returns the type that identifies this item as a PortGraphicsItem.
     */
    int type() const { return Type; }
    
    void setFocus(Qt::FocusReason focusReason = Qt::OtherFocusReason);
    void setPlainText(const QString& text);

protected:
    void keyPressEvent(QKeyEvent* event);

signals:
    void renameFinished();
    void textChanged();

private:
    QString previousText_;
};

} //namespace voreen

#endif // VRN_GUIITEM_H

