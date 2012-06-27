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

#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/arrowgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/guiitem.h"

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/processorfactory.h"

#include <iostream>
#include <typeinfo>

namespace voreen {

ProcessorGraphicsItem::ProcessorGraphicsItem(Identifier type, QGraphicsItem* parent)
    : QGraphicsItem(parent),
      type_(type)
{
    processor_ = ProcessorFactory::getInstance()->create(type_);

    textItem_ = new TextGraphicsItem(type.getName().c_str(), this);
    textItem_->moveBy(3, boundingRect().height()/2 - textItem_->boundingRect().height()/2 - 3);
    QObject::connect(textItem_, SIGNAL(renameFinished()), this, SLOT(renameFinished()));
    QObject::connect(textItem_, SIGNAL(textChanged()), this, SLOT(nameChanged()));

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);
    setZValue(1);

    setColor();
    createIO();
    setAcceptDrops(true);
}

ProcessorGraphicsItem::ProcessorGraphicsItem(Processor* processor, QGraphicsItem* parent)
    : QGraphicsItem(parent),
      type_(processor->getClassName().getSubString(1))
{
    processor_ = processor;

    textItem_ = new TextGraphicsItem(processor->getClassName().getSubString(1).c_str(), this);
    textItem_->moveBy(3, boundingRect().height()/2 - textItem_->boundingRect().height()/2 - 3);
    QObject::connect(textItem_, SIGNAL(renameFinished()), this, SLOT(renameFinished()));
    QObject::connect(textItem_, SIGNAL(textChanged()), this, SLOT(nameChanged()));

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);
    setZValue(1);

    setColor();
    createIO();
    setAcceptDrops(true);
}

ProcessorGraphicsItem::~ProcessorGraphicsItem() {
    
}

void ProcessorGraphicsItem::setName(const std::string& name) {
    processor_->setName(name);
    textItem_->setPlainText(name.c_str());
    nameChanged();
}

void ProcessorGraphicsItem::repositionPorts() {
    qreal w = boundingRect().width();
    qreal h = boundingRect().height();

    for (size_t i=0; i<inports_.size(); i++) {
        inports_.at(i)->setPos(0,0);
        inports_.at(i)->moveBy((i+1) * w/(inports_.size()+1) - inports_.at(i)->boundingRect().width()/2 - 15, 0);
    }
    for (size_t i=0; i<outports_.size(); i++) {
        outports_.at(i)->setPos(0,0);
        outports_.at(i)->moveBy((i+1) * w/(outports_.size()+1) - outports_.at(i)->boundingRect().width()/2 - 15, h - outports_.at(i)->boundingRect().height() - 4);
    }
    for (size_t i=0; i<coProcessorInports_.size(); i++) {
        coProcessorInports_.at(i)->setPos(0,0);
        coProcessorInports_.at(i)->moveBy(0-15, (i+1) * h/(coProcessorInports_.size()+1) - coProcessorInports_.at(i)->boundingRect().height()/2);
    }
    for (size_t i=0; i<coProcessorOutports_.size(); i++) {
        coProcessorOutports_.at(i)->setPos(0,0);
        coProcessorOutports_.at(i)->moveBy(w - coProcessorOutports_.at(i)->boundingRect().width() - 15, (i+1) * h/(coProcessorOutports_.size()+1) - coProcessorOutports_.at(i)->boundingRect().height()/2);
    }
}

const std::string ProcessorGraphicsItem::getName() const {
    return textItem_->toPlainText().toStdString();
}

PortGraphicsItem* ProcessorGraphicsItem::getInport(Identifier type) {
    if (type.getSubString(0) == "coprocessor") {
        for (size_t i=0; i<coProcessorInports_.size(); i++) {
            if (coProcessorInports_[i]->getPortType() == type && coProcessorInports_[i]->getConnected().size() == 0) {
                return coProcessorInports_.at(i);
            }
        }
    }
    else {
        for (size_t i=0; i<inports_.size(); i++) {
            if (inports_[i]->getPortType() == type && inports_[i]->getConnected().size() == 0) {
                return inports_.at(i);
            }
        }
    }
    return 0;
}


PortGraphicsItem* ProcessorGraphicsItem::getOutport(Identifier type) {
    if (type.getSubString(0) == "coprocessor") {
        for (size_t i=0; i<coProcessorOutports_.size(); i++) {
            if (coProcessorOutports_[i]->getPortType() == type) {
                return coProcessorOutports_.at(i);
            }
        }
    }
    else {
        for (size_t i=0; i<outports_.size(); i++) {
            if (outports_[i]->getPortType() == type) {
                return outports_.at(i);
            }
        }
    }
    return 0;
}

PortGraphicsItem* ProcessorGraphicsItem::getPortItem(Identifier ident) {
    for (size_t i=0; i <inports_.size(); i++) {
        if (inports_.at(i)->getPortType() == ident)
            return inports_.at(i);
    }

    for (size_t i=0; i <outports_.size(); i++) {
        if (outports_.at(i)->getPortType() == ident)
            return outports_.at(i);
    }

    for (size_t i=0; i <coProcessorInports_.size(); i++) {
        if (coProcessorInports_.at(i)->getPortType() == ident)
            return coProcessorInports_.at(i);
    }

    for (size_t i=0; i <coProcessorOutports_.size(); i++) {
        if (coProcessorOutports_.at(i)->getPortType() == ident)
            return coProcessorOutports_.at(i);
    }

    return 0;
}


PortGraphicsItem* ProcessorGraphicsItem::getInport(Port* port) {
    if (port->getType() == Port::PORT_TYPE_COPROCESSOR) {
        for (size_t i=0; i<coProcessorInports_.size(); i++) {
            if (coProcessorInports_[i]->getPort() == port)
                return coProcessorInports_[i];
        }
    }
    else {
        for (size_t i=0; i<inports_.size(); i++) {
            if (inports_[i]->getPort() == port)
                return inports_[i];
        }
    }
    return 0;
}

PortGraphicsItem* ProcessorGraphicsItem::getOutport(Port* port) {
    if (port->getType() == Port::PORT_TYPE_COPROCESSOR) {
        for (size_t i=0; i<coProcessorOutports_.size(); i++) {
            if (coProcessorOutports_[i]->getPort() == port)
                return coProcessorOutports_[i];
        }
    }
    else {
        for (size_t i=0; i<outports_.size(); i++) {
            if (outports_[i]->getPort() == port)
                return outports_[i];
        }
    }
    return 0;
}

bool ProcessorGraphicsItem::connect(ProcessorGraphicsItem* dest) {
    bool success = false;

    std::vector<PortGraphicsItem*> outports = getOutports();
    std::vector<PortGraphicsItem*> inports = dest->getInports();

    // find matching ports
    for (size_t j=0; j<inports.size(); j++) {
        for (size_t i=0; i<outports.size(); i++) {
            if (outports[i]->getPortType().getSubString(0) == inports[j]->getPortType().getSubString(0)) {
                success = connectAndCreateArrow(outports[i], inports[j]);
                outports.erase(outports.begin() + i);
                break;
            }
        }
    }

    // coprocessor ports
    outports = getCoProcessorOutports();
    inports = dest->getCoProcessorInports();

    // find matching ports
    for (size_t j=0; j<inports.size(); j++) {
        for (size_t i=0; i<outports.size(); i++) {
            if (outports[i]->getPortType().getSubString(0) == inports[j]->getPortType().getSubString(0)) {
                bool success2 = connectAndCreateArrow(outports[i], inports[j]);
                if (!success)
                    success = success2;
                outports.erase(outports.begin() + i);
                break;
            }
        }
    }
    return success;
}

bool ProcessorGraphicsItem::testConnect(ProcessorGraphicsItem* dest) {
    bool success = false;

    std::vector<PortGraphicsItem*> outports = getOutports();
    std::vector<PortGraphicsItem*> inports = dest->getInports();

    // find matching ports
    for (size_t j=0; j<inports.size(); j++) {
        for (size_t i=0; i<outports.size(); i++) {
            if (outports[i]->getPortType().getSubString(0) == inports[j]->getPortType().getSubString(0)) {
                success = testConnect(outports[i], inports[j]);
                outports.erase(outports.begin() + i);
                break;
            }
        }
    }

    // coprocessor ports
    outports = getCoProcessorOutports();
    inports = dest->getCoProcessorInports();

    // find matching ports
    for (size_t j=0; j<inports.size(); j++) {
        for (size_t i=0; i<outports.size(); i++) {
            if (outports[i]->getPortType().getSubString(0) == inports[j]->getPortType().getSubString(0)) {
                bool success2 = testConnect(outports[i], inports[j]);
                if (!success)
                    success = success2;
                outports.erase(outports.begin() + i);
                break;
            }
        }
    }
    return success;
}

bool ProcessorGraphicsItem::connect(PortGraphicsItem* outport, ProcessorGraphicsItem* dest) {
    // find first matching port in dest and connect
    if (outport->getPortType().getSubString(0) != "coprocessor")
        for (size_t i=0; i<dest->getInports().size(); i++) {
            if (dest->getInports()[i]->getPortType().getSubString(0) == outport->getPortType().getSubString(0)) {
                if (!(dest->getInports()[i]->isOutport())
                    && (dest->getInports()[i]->getPort()->allowMultipleConnections() || dest->getInports()[i]->getConnected().size() == 0)) {
                        connect(outport, dest->getInports()[i]);
                        return true;
                }
            }
        }

    else
        // look also in coprocessor ports
        for (size_t i=0; i<dest->getCoProcessorInports().size(); i++) {
            if (dest->getCoProcessorInports()[i]->getPortType().getSubString(0) == outport->getPortType().getSubString(0)) {
                if (!(dest->getCoProcessorInports()[i]->isOutport())
                    && (dest->getCoProcessorInports()[i]->getPort()->allowMultipleConnections() || dest->getCoProcessorInports()[i]->getConnected().size() == 0)) {
                        connect(outport, dest->getCoProcessorInports()[i]);
                        return true;
                }
            }
        }

        std::vector<ArrowGraphicsItem*> &arrows = outport->getArrowList();
        delete(arrows.at(arrows.size()-1));
        arrows.pop_back();
        return false;
}

bool ProcessorGraphicsItem::testConnect(PortGraphicsItem* outport, ProcessorGraphicsItem* dest) {
    // find first matching port in dest and connect
    if (outport->getPortType().getSubString(0) != "coprocessor")
        for (size_t i=0; i<dest->getInports().size(); i++) {
            if (dest->getInports()[i]->getPortType().getSubString(0) == outport->getPortType().getSubString(0)) {
                if (!(dest->getInports()[i]->isOutport())
                    && (dest->getInports()[i]->getPort()->allowMultipleConnections() || dest->getInports()[i]->getConnected().size() == 0)) {
                        testConnect(outport, dest->getInports()[i]);
                        return true;
                }
            }
        }

    else
        // look also in coprocessor ports
        for (size_t i=0; i<dest->getCoProcessorInports().size(); i++) {
            if (dest->getCoProcessorInports()[i]->getPortType().getSubString(0) == outport->getPortType().getSubString(0)) {
                if (!(dest->getCoProcessorInports()[i]->isOutport())
                    && (dest->getCoProcessorInports()[i]->getPort()->allowMultipleConnections() || dest->getCoProcessorInports()[i]->getConnected().size() == 0)) {
                        testConnect(outport, dest->getCoProcessorInports()[i]);
                        return true;
                }
            }
        }

        return false;
}


bool ProcessorGraphicsItem::connect(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    if (getProcessor(outport)->connect(outport->getPort(), inport->getPort())) {
        outport->addConnection(inport);
        inport->addConnection(outport);
        outport->getArrowList().at(outport->getArrowList().size()-1)->setDestNode(inport);
        contentChanged();

        return true;
    }

    std::vector<ArrowGraphicsItem*> &arrows = outport->getArrowList();
    delete(arrows.at(arrows.size()-1));
    arrows.pop_back();
    return false;
}

bool ProcessorGraphicsItem::testConnect(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    if (getProcessor(outport)->testConnect(outport->getPort(), inport->getPort())) {
        return true;
    }
    return false;
}

bool ProcessorGraphicsItem::connectAndCreateArrow(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    if (getProcessor(outport)->connect(outport->getPort(), inport->getPort())) {
        std::vector<ArrowGraphicsItem*> &arrows = outport->getArrowList();
        arrows.push_back(new ArrowGraphicsItem(outport));
        scene()->addItem(arrows.at(arrows.size()-1));
        outport->addConnection(inport);
        inport->addConnection(outport);
        outport->getArrowList().at(outport->getArrowList().size()-1)->setDestNode(inport);
        outport->getArrowList().at(outport->getArrowList().size()-1)->adjust();
        contentChanged();
        return true;
    }
    return false;
}

void ProcessorGraphicsItem::connectGuionly(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    std::vector<ArrowGraphicsItem*> &arrows = outport->getArrowList();
    arrows.push_back(new ArrowGraphicsItem(outport));
    outport->addConnection(inport);
    inport->addConnection(outport);
    outport->getArrowList().back()->setDestNode(inport);
    outport->getArrowList().back()->adjust();
    contentChanged();
}

bool ProcessorGraphicsItem::disconnect(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    if ((outport == 0) || (inport == 0)) {
        return false;
    }
    if ((getProcessor(outport) == 0) || (outport->getPort() == 0) || (inport->getPort() == 0)) {
        return false;
    }

    if (getProcessor(outport)->disconnect(outport->getPort(), inport->getPort())) {
        outport->disconnect(inport);
        contentChanged();
        return true;
    }
    return false;
}

bool ProcessorGraphicsItem::disconnectAll() {
    for (size_t i=0; i<inports_.size(); i++) {
        if (inports_[i] != 0) {
            while (inports_[i]->getConnected().size() > 0) {
                if (inports_[i]->getConnected()[0] != 0) {
                    ProcessorGraphicsItem* parentProcessorGraphicsItem = inports_[i]->getConnected()[0]->getParent();
                    if (parentProcessorGraphicsItem != 0) {
                        parentProcessorGraphicsItem->disconnect(inports_[i]->getConnected()[0], inports_[i]);
                    }
                }
            }
        }
    }

    for (size_t i=0; i<outports_.size(); i++) {
        if (outports_[i] != 0) {
            while (outports_[i]->getConnected().size() > 0) {
                if (outports_[i]->getParent() != 0) {
                    outports_[i]->getParent()->disconnect(outports_[i], outports_[i]->getConnected()[0]);
                }
            }
        }
    }

    // disconnect coprocessor ports
    for (size_t i=0; i<coProcessorInports_.size(); i++) {
        if (coProcessorInports_[i] != 0) {
            while (coProcessorInports_[i]->getConnected().size() > 0) {
                PortGraphicsItem* connectedPort = coProcessorInports_[i]->getConnected()[0];
                if (connectedPort != 0) {
                    ProcessorGraphicsItem* parent = connectedPort->getParent();
                    if (parent != 0) {
                        parent->disconnect(coProcessorInports_[i]->getConnected()[0], coProcessorInports_[i]);
                    }
                }
            }
        }
    }

    for (size_t i=0; i<coProcessorOutports_.size(); i++) {
        if (coProcessorOutports_[i] != 0) {
            while (coProcessorOutports_[i]->getConnected().size() > 0) {
                ProcessorGraphicsItem* parent = coProcessorOutports_[i]->getParent();
                if (parent != 0) {
                    parent->disconnect(coProcessorOutports_[i], coProcessorOutports_[i]->getConnected()[0]);
                }
            }
        }
    }

    contentChanged();
    return true;
}


void ProcessorGraphicsItem::removeFromScene() {
    // remove item itself from scene
    if (scene())
        scene()->removeItem(this);
    contentChanged();
}

void ProcessorGraphicsItem::removeArrows(ProcessorGraphicsItem* item) {
    for (size_t i=0; i<outports_.size(); i++) {
        for (size_t j=0; j<outports_[i]->getArrowList().size(); j++) {
            if (outports_[i]->getArrowList().at(j)->getDestNode()->parentItem() == item) {
                scene()->removeItem(outports_[i]->getArrowList().at(j));
            }
        }
        contentChanged();
    }

    // --- coprocessor ports ---
    for (size_t i=0; i < coProcessorOutports_.size(); i++) {
        for (size_t j=0; j<coProcessorOutports_[i]->getArrowList().size(); j++) {
            if (coProcessorOutports_[i]->getArrowList().at(j)->getDestNode()->parentItem() == item) {
                scene()->removeItem(coProcessorOutports_[i]->getArrowList().at(j));
            }
        }
    }
}

void ProcessorGraphicsItem::removeAllArrows() {
    for (size_t i=0; i<outports_.size(); i++) {
        for (size_t j=0; j<outports_[i]->getArrowList().size(); j++) {
            scene()->removeItem(outports_[i]->getArrowList().at(j));
        }
    }

    // --- coprocessor ports ---
    for (size_t i=0; i<coProcessorOutports_.size(); i++) {
        for (size_t j=0; j<coProcessorOutports_[i]->getArrowList().size(); j++) {
            scene()->removeItem(coProcessorOutports_[i]->getArrowList().at(j));
        }
    }
    contentChanged();
}

void ProcessorGraphicsItem::showAllArrows() {
    if (scene()) {
        for (size_t i=0; i<outports_.size(); i++) {
            for (size_t j=0; j<outports_[i]->getArrowList().size(); j++) {
                scene()->addItem(outports_[i]->getArrowList().at(j));
            }
        }

        // --- coprocessor ports ---
        for (size_t i=0; i<coProcessorOutports_.size(); i++) {
            for (size_t j=0; j<coProcessorOutports_[i]->getArrowList().size(); j++) {
                scene()->addItem(coProcessorOutports_[i]->getArrowList().at(j));
            }
        }
    }
    adjustArrows();
}

void ProcessorGraphicsItem::adjustArrows() {
    // incoming arrows
    for (size_t i=0; i<inports_.size(); i++) {
        inports_[i]->adjustArrows();
    }
    // outgoing arrows
    for (size_t i=0; i<outports_.size(); i++) {
        outports_[i]->adjustArrows();
    }

    // --- coprocessor ports ---
    // incoming arrows
    for (size_t i=0; i<coProcessorInports_.size(); i++) {
        coProcessorInports_[i]->adjustArrows();
    }
    // outgoing arrows
    for (size_t i=0; i<coProcessorOutports_.size(); i++) {
        coProcessorOutports_[i]->adjustArrows();
    }

}

QVariant ProcessorGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionChange) {
        adjustArrows();
    }
    else if (change == ItemSelectedChange) {
        if (!value.toBool() && (textItem_->textInteractionFlags() & Qt::TextEditorInteraction))
            // item is deselected and in renaming mode -> finish renaming
            renameFinished();
    }

    return QGraphicsItem::itemChange(change, value);
}

void ProcessorGraphicsItem::enterRenameMode() {
    textItem_->setTextInteractionFlags(Qt::TextEditorInteraction);
    textItem_->setFlag(QGraphicsItem::ItemIsFocusable, true);
    textItem_->setFocus();
}

void ProcessorGraphicsItem::renameFinished() {
    textItem_->setTextInteractionFlags(Qt::NoTextInteraction);
    textItem_->setFlag(QGraphicsItem::ItemIsFocusable, false);
    textItem_->setFlag(QGraphicsItem::ItemIsSelectable, false);
    nameChanged();
}

void ProcessorGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    adjustArrows();
    QGraphicsItem::mouseReleaseEvent(event);
}

void ProcessorGraphicsItem::contentChanged() {
    //FIXME: this signal is not connected to any slot (cdoer)
    emit changed();
}

void ProcessorGraphicsItem::nameChanged() {
	processor_->setName(textItem_->toPlainText().toStdString());
    prepareGeometryChange();
    repositionPorts();
    adjustArrows();
    if (scene())
    	scene()->invalidate();

    emit processorNameChanged(getProcessor());
}

void ProcessorGraphicsItem::setColor() {
    color_ = QColor(233, 218, 176);
}

void ProcessorGraphicsItem::createIO() {

    std::vector<Port*> inports = processor_->getInports();
    std::vector<Port*> outports = processor_->getOutports();

    for (size_t i=0; i<inports.size(); i++)
        inports_.push_back(
            new PortGraphicsItem(inports[i]->getTypeIdentifier(), inports[i], this));

    for (size_t i=0; i< outports.size(); i++)
        outports_.push_back(
            new PortGraphicsItem(outports[i]->getTypeIdentifier(), outports[i], this));

    // coProcessorPorts
    inports = processor_->getCoProcessorInports();
    outports = processor_->getCoProcessorOutports();

    for (size_t i=0; i < inports.size(); i++)
        coProcessorInports_.push_back(
            new PortGraphicsItem(inports[i]->getTypeIdentifier(), inports[i], this));

    for (size_t i=0; i < outports.size(); i++)
        coProcessorOutports_.push_back(
            new PortGraphicsItem(outports[i]->getTypeIdentifier(), outports[i], this));

    repositionPorts();
}

QRectF ProcessorGraphicsItem::drawingRect() const {
    QRectF rect = textItem_->boundingRect();
    QRectF newRect(
        rect.left()-15,
        rect.top(),
        (rect.width()>59) ? rect.width()+30 : 90,
        rect.height()+40
    );
    return newRect;
}

QRectF ProcessorGraphicsItem::boundingRect() const {
    QRectF rect = drawingRect();
    QRectF newRect(
        rect.left()-3,
        rect.top()-3,
        rect.width() + 6,
        rect.height() + 6
    );
    return newRect;
}

QPainterPath ProcessorGraphicsItem::coprocessorBoundingPath(QRectF rect) const {
    // taken from http://www.bessrc.aps.anl.gov/software/qt4-x11-4.2.2/widgets-styles.html
    int radius = static_cast<int>(qMin(rect.width(), rect.height()) / 2.f);
    int diam = 2 * radius;

    qreal x1, y1, x2, y2;
    rect.getCoords(&x1, &y1, &x2, &y2);

    QPainterPath path;
    path.moveTo(x2, y1 + radius);
    path.arcTo(QRect(static_cast<int>(x2 - diam), static_cast<int>(y1), diam, diam), 0.0, +90.0);
    path.lineTo(x1 + radius, y1);
    path.arcTo(QRect(static_cast<int>(x1), static_cast<int>(y1), diam, diam), 90.0, +90.0);
    path.lineTo(x1, y2 - radius);
    path.arcTo(QRect(static_cast<int>(x1), static_cast<int>(y2 - diam), diam, diam), 180.0, +90.0);
    path.lineTo(x1 + radius, y2);
    path.arcTo(QRect(static_cast<int>(x2 - diam), static_cast<int>(y2 - diam), diam, diam), 270.0, +90.0);
    path.closeSubpath();
    return path;
}

QPainterPath ProcessorGraphicsItem::canvasBoundingPath(QRectF rect) const {
    QPainterPath path;
    path.addEllipse(rect);
    return path;
}

void ProcessorGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* ) {
    QRectF button_rect = drawingRect();

    QColor button_color = QColor(50, 50, 50);

    // hover effect
    if (option->state & QStyle::State_MouseOver)
        button_color = Qt::blue;
    // frame indicates selected process
    if (option->state & QStyle::State_Selected)
        button_color = Qt::red;


    QColor m_shadow = QColor(Qt::black);
    painter->drawRect(button_rect);

    //gradient
    QLinearGradient gradient(0, 0, 0, button_rect.height());
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, button_color);
    gradient.setColorAt(0.4, m_shadow);
    gradient.setColorAt(0.6, m_shadow);
    gradient.setColorAt(1.0, button_color);

    QBrush brush(gradient);
    painter->setBrush(brush);
    painter->setPen(QPen(QBrush(button_color), 2.0));

    painter->setOpacity(1.0);
    painter->drawRect(button_rect);

    //glass highlight
    painter->setBrush(QBrush(Qt::white));
    painter->setPen(QPen(QBrush(Qt::white), 0.01));
    painter->setOpacity(0.30);
    button_rect.setHeight(button_rect.height()/2.0);
    painter->drawRect(button_rect);
}

void ProcessorGraphicsItem::dropEvent(QDropEvent* event) {
    // Determine, whether the target was a VolumeSetSourceProcessor
    // and the mime data matches the requirements.
    VolumeSetSourceProcessor* vssp = dynamic_cast<VolumeSetSourceProcessor*>(processor_);
    if (vssp != 0) {
        // Set the dropped data (a pointer to a VolumeSet object) for the processor.
        // The data have been converted from VolumeSet* to qulonglong in order to
        // be passed by a QByteArray. I know it's ugly and dangerous but I see no easier
        // possibilities.... (dirk)
        QByteArray itemData = event->mimeData()->data("application/x-voreenvolumesetpointer");
        qulonglong data = itemData.toULongLong();
        VolumeSet* volumeset = reinterpret_cast<VolumeSet*>(data);
        vssp->setVolumeSet(volumeset);
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else {
        event->ignore();
    }
}

ProcessorGraphicsItem& ProcessorGraphicsItem::saveMeta() {
    TiXmlElement* meta = new TiXmlElement("RptProcessorItem");
    meta->SetAttribute("x", static_cast<int>(x()));
    meta->SetAttribute("y", static_cast<int>(y()));
    processor_->addToMeta(meta);
    return *this;
}

ProcessorGraphicsItem& ProcessorGraphicsItem::loadMeta() {
    TiXmlElement* meta = processor_->getFromMeta("RptProcessorItem");
    float x, y;
    if (meta->QueryFloatAttribute("x", &x) != TIXML_SUCCESS || meta->QueryFloatAttribute("y", &y) != TIXML_SUCCESS)
        throw XmlAttributeException("The Position of a ProcessorItem remains unknown!");
    setPos(x,y);

    setName(processor_->getName());

    return *this;
}

} // namespace voreen
