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

#include "voreen/qt/widgets/network/editor/portgraphicsitem.h"

#include "voreen/core/vis/processors/ports/allports.h"
#include "voreen/qt/widgets/network/editor/arrowgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/textgraphicsitem.h"

namespace {
    QColor colorVolumeCollectionPort = Qt::magenta;
    QColor colorVolumePort = Qt::red;
    QColor colorRenderPort = Qt::blue;
    QColor colorGeometryPort = Qt::yellow;
    QColor colorCoProcessorPort = Qt::green;
    QColor colorUnknownPort = Qt::gray;

    QColor colorBrush = Qt::lightGray;

    QSize sizeNormal = QSize(10, 10);
    qreal sizeMultipleConnectionStrechFactor = 2.f;
}

namespace voreen {

PortGraphicsItem::PortGraphicsItem(Port* port, ProcessorGraphicsItem* parent)
    : QGraphicsItem(parent)
    , parent_(parent)
    , port_(port)
    , currentArrow_(0)
{
    tgtAssert(port != 0, "passed null pointer");
    tgtAssert(parent != 0, "passed null pointer");

    setToolTip(port->getName().c_str());
    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);
}

void PortGraphicsItem::setLayer(NetworkEditorLayer layer) {
    switch (layer) {
    case NetworkEditorLayerDataflow:
        setFlag(ItemIsSelectable);
        setAcceptsHoverEvents(true);
    #if QT_VERSION >= 0x040600
        setGraphicsEffect(0);
#else
        setVisible(true);
#endif
            break;
    case NetworkEditorLayerLinking:
        setFlag(ItemIsSelectable, false);
        setAcceptsHoverEvents(false);
#if QT_VERSION >= 0x040600
        {
        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect;
        effect->setOpacity(0.2);
        setGraphicsEffect(effect);
        }
#else
        setVisible(false);
#endif
        break;
    default:
        tgtAssert(false, "shouldn't get here");
    }
}

QColor PortGraphicsItem::getColor() {
    QColor result;

    if (dynamic_cast<VolumeCollectionPort*>(port_)) {
        result = colorVolumeCollectionPort;
    }
    else if (dynamic_cast<VolumePort*>(port_)) {
        result = colorVolumePort;
    }
    else if(dynamic_cast<RenderPort*>(port_)) {
        result = colorRenderPort;
    }
    else if(dynamic_cast<GeometryPort*>(port_)) {
        result = colorGeometryPort;
    }
    else if(dynamic_cast<CoProcessorPort*>(port_)) {
        result = colorCoProcessorPort;
    }
    else {
        result = colorUnknownPort;
    }
    result.setAlpha(100);

    return result;
}

bool PortGraphicsItem::isOutport() {
    return port_->isOutport();
}

int PortGraphicsItem::type() const {
    return Type;
}

Port* PortGraphicsItem::getPort() {
    return port_;
}

QList<ArrowGraphicsItem*>& PortGraphicsItem::getArrowList() {
    return arrowList_;
}

void PortGraphicsItem::disconnect(PortGraphicsItem* inport) {
    if (inport == 0) {
        return;
    }

    if (!isOutport())
        return;

    // remove this from inport's connectedPorts
    QList<PortGraphicsItem*>& connected = inport->getConnected();
    for (int i = 0; i < connected.size(); ++i) {
        if (connected[i] == this)
            connected.erase(connected.begin() + i);
    }

    // remove inport from this' connectedPorts
    for (int i = 0; i < connectedPorts_.size(); ++i) {
        if (connectedPorts_[i] == inport)
            connectedPorts_.erase(connectedPorts_.begin() + i);
    }

    // delete arrow
    for (int i = 0; i < arrowList_.size(); ++i) {
        if (arrowList_[i]->getDestNode() == inport) {
            delete arrowList_[i];
            arrowList_[i] = 0;
            arrowList_.erase(arrowList_.begin() + i);
        }
    }

}

QList<PortGraphicsItem*>& PortGraphicsItem::getConnected() {
   return connectedPorts_;
}

void PortGraphicsItem::addConnection(PortGraphicsItem* port) {
    connectedPorts_.push_back(port);
}

void PortGraphicsItem::adjustArrows() {
    if (isOutport()) {
        for (int i = 0; i < arrowList_.size(); ++i) {
            arrowList_[i]->adjust();
        }
    }
    else {
        for (int i = 0; i < connectedPorts_.size(); ++i) {
            connectedPorts_[i]->adjustArrows();
        }
    }
}

QRectF PortGraphicsItem::boundingRect() const {
    QRectF rect(0,0,sizeNormal.width(),sizeNormal.height());
    if (port_->allowMultipleConnections() && !port_->isOutport()) {
        if (dynamic_cast<CoProcessorPort*>(port_))
            rect.setHeight(rect.height() * sizeMultipleConnectionStrechFactor);
        else
            rect.setWidth(rect.width() * sizeMultipleConnectionStrechFactor);
    }
    return rect;
}

void PortGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) {
    QColor color = getColor();
    painter->setPen(QPen(color, 2));
    painter->setBrush(colorBrush);

    if (option->state & QStyle::State_Sunken) {
        painter->setBrush(color.darker(150));
    }
    if (option->state & QStyle::State_MouseOver) {
        painter->setBrush(color);
    }

    painter->drawRect(boundingRect());
}

void PortGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    //parent_->mousePressEvent(event);
    if (parent_->currentLayer() == NetworkEditorLayerDataflow) {
        if (isOutport()) {
            currentArrow_ = new ArrowGraphicsItem(this);
            scene()->addItem(currentArrow_);
            QPointF scenePos = event->scenePos();
            currentArrow_->adjust(scenePos);
        }
    }

    QGraphicsItem::mousePressEvent(event);
}

void PortGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    //parent_->mouseMoveEvent(event);
    if (isOutport()) {
        QPointF scenePos = event->scenePos();
        currentArrow_->adjust(scenePos);
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void PortGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    //parent_->mouseReleaseEvent(event);
    QGraphicsItem* item = scene()->itemAt(event->scenePos());

    if (isOutport()) {
        if (item && item != this) {
            // released over port item
            if (item->type() == PortGraphicsItem::Type) {
                PortGraphicsItem* portItem = static_cast<PortGraphicsItem*>(item);
                if (portItem->getParent() != getParent() && !portItem->isOutport()) {
                    arrowList_.push_back(currentArrow_);
                    getParent()->connect(this, portItem);
                    currentArrow_ = 0;
                } else {
                    scene()->removeItem(currentArrow_);
                    delete currentArrow_;
                    currentArrow_ = 0;
                }
            }
            // released over gui item
            else if (item->type() == ProcessorGraphicsItem::Type   ||
                     item->type() == TextGraphicsItem::Type)
            {
                ProcessorGraphicsItem* guiItem = (item->type() == TextGraphicsItem::Type) ? dynamic_cast<ProcessorGraphicsItem*>(item->parentItem()) : dynamic_cast<ProcessorGraphicsItem*>(item);
                arrowList_.push_back(currentArrow_);
                getParent()->connect(this, guiItem);
                currentArrow_ = 0;
            }
            else {
                scene()->removeItem(currentArrow_);
                delete currentArrow_;
                currentArrow_ = 0;
            }
        }
        else {
            scene()->removeItem(currentArrow_);
            delete currentArrow_;
            currentArrow_ = 0;
        }
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

bool PortGraphicsItem::doesArrowExist(PortGraphicsItem* destItem) {
    for (int i = 0; i < arrowList_.size(); ++i) {
        if (arrowList_.at(i)->getDestNode() == destItem)
            return true;
    }
    return false;
}

ProcessorGraphicsItem* PortGraphicsItem::getParent() {
    return parent_;
}

} // namespace
