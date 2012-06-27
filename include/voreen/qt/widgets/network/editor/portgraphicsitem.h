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

#ifndef VRN_PORTGRAPHICSITEM_H
#define VRN_PORTGRAPHICSITEM_H

#include <QGraphicsItem>
#include "voreen/qt/voreenqtglobal.h"
#include "voreen/qt/widgets/network/editor/networkeditor.h"

namespace voreen {

class ArrowGraphicsItem;
class Port;
class ProcessorGraphicsItem;

class PortGraphicsItem : public QGraphicsItem {
public:
    PortGraphicsItem(Port* port, ProcessorGraphicsItem* parent);

    enum { Type = UserType + UserTypesPortGraphicsItem };

    int type() const;

    Port* getPort();

    ProcessorGraphicsItem* getParent();

    QList<PortGraphicsItem*>& getConnected();

    void addConnection(PortGraphicsItem* port);

    void disconnect(PortGraphicsItem* inport);

    bool isOutport();

    QList<ArrowGraphicsItem*>& getArrowList();

    void adjustArrows();

    bool doesArrowExist(PortGraphicsItem* item);

    QRectF boundingRect() const;

    void setLayer(NetworkEditorLayer layer);

protected:
    QColor getColor();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

private:
    // list of outgoing arrows
    QList<ArrowGraphicsItem*> arrowList_;
    // list of gui items that are connected to this port.
    QList<PortGraphicsItem*> connectedPorts_;
    ProcessorGraphicsItem* parent_;
    // represented port
    Port* port_;

    ArrowGraphicsItem* currentArrow_;
};

} // namespace

#endif // VRN_PORTGRAPHICSITEM_H
