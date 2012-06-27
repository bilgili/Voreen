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

#include "voreen/qt/widgets/network/editor/guiitem.h"

#include "voreen/core/vis/processors/port.h"
#include "voreen/core/vis/processors/processor.h"

#include "voreen/qt/widgets/network/editor/arrowgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"

namespace voreen {

PortGraphicsItem::PortGraphicsItem(Identifier type, Port* port, ProcessorGraphicsItem* parent)
    : QGraphicsItem(parent)
    , type_(type)
    , port_(port)
{
    setToolTip(type.getName().c_str());
    setColor();
    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);
}

void PortGraphicsItem::setColor() {
    switch (port_->getType()) {
        case Port::PORT_TYPE_VOLUMESET:
            color_ = Qt::magenta;
            sunkenColor_ = Qt::darkMagenta;
            break;
        case Port::PORT_TYPE_VOLUMEHANDLE:
            color_ = Qt::red;
            sunkenColor_ = Qt::darkRed;
            break;
        case Port::PORT_TYPE_IMAGE:
            color_ = Qt::blue;
            sunkenColor_ = Qt::darkBlue;
            break;
        case Port::PORT_TYPE_GEOMETRY:
            color_ = Qt::yellow;
            sunkenColor_ = Qt::darkYellow;
            break;
        case Port::PORT_TYPE_COPROCESSOR:
            color_ = Qt::green;
            sunkenColor_ = Qt::darkGreen;
            break;
        default:
            color_ = Qt::gray;
            sunkenColor_ = Qt::darkGray;
            break;
    }   // switch
    color_.setAlpha(100);
}

bool PortGraphicsItem::isOutport() {
    return port_->isOutport();
}

void PortGraphicsItem::disconnect(PortGraphicsItem* inport) {
    if (inport == 0) {
        return;
    }

    if (!isOutport())
        return;

    // remove this from inport's connectedPorts
    std::vector<PortGraphicsItem*>& connected = inport->getConnected();
    for (size_t i=0; i<connected.size(); i++) {
        if (connected[i] == this)
            connected.erase(connected.begin() + i);
    }

    // remove inport from this' connectedPorts
    for (size_t i=0; i<connectedPorts_.size(); i++) {
        if (connectedPorts_[i] == inport)
            connectedPorts_.erase(connectedPorts_.begin() + i);
    }

    // delete arrow
    for (size_t i=0; i<arrowList_.size(); i++) {
        if (arrowList_[i]->getDestNode() == inport) {
            delete arrowList_[i];
            arrowList_[i] = 0;
            arrowList_.erase(arrowList_.begin() + i);
        }
    }

}

std::vector<PortGraphicsItem*>& PortGraphicsItem::getConnected() {
   return connectedPorts_;
}

void PortGraphicsItem::addConnection(PortGraphicsItem* port) {
    connectedPorts_.push_back(port);
}

void PortGraphicsItem::adjustArrows() {
    if (!isOutport()) {
        for (size_t i=0; i<connectedPorts_.size(); i++) {
            connectedPorts_[i]->adjustArrows();
        }
    }
    else {
        for (size_t i=0; i<arrowList_.size(); i++) {
            arrowList_[i]->adjust();
        }
    }
}

QRectF PortGraphicsItem::boundingRect() const {
    QRectF rect(0,0,10,10);
    if (port_->allowMultipleConnections() && !port_->isOutport()) {
        if (port_->getType() == Port::PORT_TYPE_COPROCESSOR)
            rect = QRectF(0,0,10,20);
        else
            rect = QRectF(0,0,25,10);
    }
    return rect;
}

void PortGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    painter->setPen(QPen(color_, 2));
    painter->setBrush(QColor(255,255,255,150));

    if (option->state & QStyle::State_Sunken) {
        painter->setBrush(sunkenColor_);
    }
    if (option->state & QStyle::State_MouseOver) {
        painter->setBrush(color_);
    }
    painter->drawRect(boundingRect());

}

void PortGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (isOutport()) {
        arrowList_.push_back(new ArrowGraphicsItem(this));
        //TODO: This line produces a warning
        scene()->addItem(arrowList_.at(arrowList_.size()-1));
        QPointF scenePos = event->scenePos();
        arrowList_.at(arrowList_.size() - 1)->adjust(scenePos);
    }

    QGraphicsItem::mousePressEvent(event);
}

void PortGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (isOutport()) {
        QPointF scenePos = event->scenePos();
        arrowList_.at(arrowList_.size()-1)->adjust(scenePos);
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void PortGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsItem* item = scene()->itemAt(event->scenePos());

    if (isOutport()) {
        if (item && item != this) {
            // released over port item
            if (item->type() == PortGraphicsItem::Type) {
                PortGraphicsItem* portItem = static_cast<PortGraphicsItem*>(item);
                if (portItem->getParent() != getParent() && !portItem->isOutport()) {
                    getParent()->connect(this, portItem);
                    getParent()->contentChanged();
                }
                else {
                    delete arrowList_.at(arrowList_.size()-1);
                    arrowList_.pop_back();
                }
            }

            // released over gui item
            else if (item->type() == ProcessorGraphicsItem::Type   ||
                     item->type() == TextGraphicsItem::Type)
            {
                ProcessorGraphicsItem* guiItem = (item->type() == TextGraphicsItem::Type) ? static_cast<ProcessorGraphicsItem*>(item->parentItem()) : static_cast<ProcessorGraphicsItem*>(item);
                getParent()->connect(this, guiItem);
                getParent()->contentChanged();
            }
            else {
                delete arrowList_.at(arrowList_.size()-1);
                arrowList_.pop_back();
            }
        }
        else {
            delete arrowList_.at(arrowList_.size()-1);
            arrowList_.pop_back();
        }
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

bool PortGraphicsItem::doesArrowExist(PortGraphicsItem* destItem) {
    for (size_t i=0;i<arrowList_.size();i++) {
        if (arrowList_.at(i)->getDestNode() == destItem)
            return true;
    }
    return false;
}

ProcessorGraphicsItem* PortGraphicsItem::getParent() {
    return static_cast<ProcessorGraphicsItem*>(parentItem());
}

//---------------------------------------------------------------------------

TextGraphicsItem::TextGraphicsItem(const QString& text, ProcessorGraphicsItem* parent, QGraphicsScene* scene)
    : QGraphicsTextItem(text, parent, scene)
{
    setFlag(ItemIsSelectable, false);
    setDefaultTextColor(Qt::white);
}

TextGraphicsItem::~TextGraphicsItem() {
}

void TextGraphicsItem::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        // restore saved text
        setPlainText(previousText_);
        emit renameFinished();
    }
    else if ((event->key() == Qt::Key_Return) && (event->modifiers() == Qt::NoModifier)) {
        previousText_ = toPlainText();
        emit renameFinished();
    }
    else {
        QGraphicsTextItem::keyPressEvent(event);
        emit textChanged();
    }
}

void TextGraphicsItem::setFocus(Qt::FocusReason focusReason) {
    // save old text
    previousText_ = toPlainText();
    QGraphicsTextItem::setFocus(focusReason);
}

void TextGraphicsItem::setPlainText(const QString& text) {
    previousText_ = text;
    QGraphicsTextItem::setPlainText(text);
}

} // namespace voreen
