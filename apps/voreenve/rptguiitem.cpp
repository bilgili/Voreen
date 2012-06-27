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

#include "rptguiitem.h"
#include "rptarrow.h"
#include "voreen/core/vis/processors/processor.h"
#include <iostream>
#include <typeinfo>
#include <time.h>

#include "rptprocessoritem.h"
#include "rptaggregationitem.h"
#include "rptpropertysetitem.h"


namespace voreen {

RptGuiItem::RptGuiItem(std::string name, QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    textItem_ = new RptTextItem("");
    textItem_->setPlainText(QString(name.c_str()));
    textItem_->setParentItem(this);
    textItem_->moveBy(3, boundingRect().height()/2 - textItem_->boundingRect().height()/2);

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);
    setZValue(1);
    textItem_->setFlag(QGraphicsItem::ItemIsSelectable, false);

    QObject::connect(textItem_, SIGNAL(sendText(std::string)), this, SLOT(renameSlot(std::string)));

    collisionPriority_ = -1;
}

RptGuiItem::~RptGuiItem() {
    contextMenu_.disconnect();

    //the textitem_ and all RptPortItems in the vectors are automatically
    //deleted by qt because they are children of the RptGuiItem. So we must
    //not delete them here.
}

void RptGuiItem::repositionPorts() {
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

std::string RptGuiItem::getName() const {
    return textItem_->toPlainText().toStdString();
}

void RptGuiItem::setName(std::string name) {
    prepareGeometryChange();
    textItem_->setPlainText(QString(name.c_str()));
    update();
    repositionPorts();
}

void RptGuiItem::renameSlot(std::string name) {
    setName(name);
    textItem_->setTextInteractionFlags(Qt::NoTextInteraction);
    textItem_->setFlag(QGraphicsItem::ItemIsSelectable,false);
    repositionPorts();
    adjustArrows();
    scene()->invalidate();
}

RptPortItem* RptGuiItem::getInport(Identifier type) {
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


RptPortItem* RptGuiItem::getOutport(Identifier type) {
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

RptPortItem* RptGuiItem::getPortItem(Identifier ident) {
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


RptPortItem* RptGuiItem::getInport(Port* port) {
    if (port->getType().getSubString(0) == "coprocessor") {
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

RptPortItem* RptGuiItem::getOutport(Port* port) {
    if (port->getType().getSubString(0) == "coprocessor") {
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

bool RptGuiItem::connect(RptGuiItem* dest) {
    bool success = false;

    std::vector<RptPortItem*> outports = getOutports();
    std::vector<RptPortItem*> inports = dest->getInports();

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

bool RptGuiItem::testConnect(RptGuiItem* dest) {
    bool success = false;

    std::vector<RptPortItem*> outports = getOutports();
    std::vector<RptPortItem*> inports = dest->getInports();

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

bool RptGuiItem::connect(RptPortItem* outport, RptGuiItem* dest) {
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

    std::vector<RptArrow*> &arrows = outport->getArrowList();
    delete(arrows.at(arrows.size()-1));
    arrows.pop_back();
    return false;
}

bool RptGuiItem::testConnect(RptPortItem* outport, RptGuiItem* dest) {
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


bool RptGuiItem::connect(RptPortItem* outport, RptPortItem* inport) {
    if (getProcessor(outport)->connect(outport->getPort(), inport->getPort())) {
        outport->addConnection(inport);
        inport->addConnection(outport);
        outport->getArrowList().at(outport->getArrowList().size()-1)->setDestNode(inport);
        contentChanged();

        return true;
    }

    std::vector<RptArrow*> &arrows = outport->getArrowList();
    delete(arrows.at(arrows.size()-1));
    arrows.pop_back();
    return false;
}

bool RptGuiItem::testConnect(RptPortItem* outport, RptPortItem* inport) {
    if (getProcessor(outport)->testConnect(outport->getPort(), inport->getPort())) {
        return true;
    }
    return false;
}

bool RptGuiItem::connectAndCreateArrow(RptPortItem* outport, RptPortItem* inport) {
    if (getProcessor(outport)->connect(outport->getPort(), inport->getPort())) {
        std::vector<RptArrow*> &arrows = outport->getArrowList();
        arrows.push_back(new RptArrow(outport));
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

void RptGuiItem::connectGuionly(RptPortItem* outport, RptPortItem* inport) {
        std::vector<RptArrow*> &arrows = outport->getArrowList();
        arrows.push_back(new RptArrow(outport));
        outport->addConnection(inport);
        inport->addConnection(outport);
        outport->getArrowList().back()->setDestNode(inport);
        outport->getArrowList().back()->adjust();
        contentChanged();
}

bool RptGuiItem::disconnect(RptPortItem* outport, RptPortItem* inport) {
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

bool RptGuiItem::disconnectAll() {
    for (size_t i=0; i<inports_.size(); i++) {
        if (inports_[i] != 0) {
            while (inports_[i]->getConnected().size() > 0) {
                if (inports_[i]->getConnected()[0] != 0) {
                    RptGuiItem* parentGuiItem = inports_[i]->getConnected()[0]->getParent();
                    if (parentGuiItem != 0) {
                        parentGuiItem->disconnect(inports_[i]->getConnected()[0], inports_[i]);
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
                RptPortItem* connectedPort = coProcessorInports_[i]->getConnected()[0];
                if (connectedPort != 0) {
                    RptGuiItem* parent = connectedPort->getParent();
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
                RptGuiItem* parent = coProcessorOutports_[i]->getParent();
                if (parent != 0) {
                    parent->disconnect(coProcessorOutports_[i], coProcessorOutports_[i]->getConnected()[0]);
                }
            }
        }
    }
    // disconnect from property sets

    // needed 'cause disconnectGuiItem() calls RptGuiItem::removePropertySet() and there propertySets_ is undesirably modified
    // disconnect this from propertysets - one at a time
    while (propertySets_.size() > 0) {
        if (propertySets_.back()) {
            // This also removes the Entry from the Vector
            propertySets_.back()->disconnectGuiItem(this);
        }
        else {
            // nullpointer? how did it get there? Get rid of it!
            propertySets_.pop_back();
        }
    }
    contentChanged();
    return true;
}

void RptGuiItem::addPropertySet(RptPropertySetItem* propSet) {
    propertySets_.push_back(propSet);
}

void RptGuiItem::removePropertySet(RptPropertySetItem* propSet) {
    for (size_t i = 0; i < propertySets_.size(); i++) {
        if (propertySets_[i] == propSet)
            propertySets_.erase(propertySets_.begin() + i);
    }
}

void RptGuiItem::removeFromScene() {
    // remove item itself from scene
    if (scene())
        scene()->removeItem(this);
    contentChanged();
}

void RptGuiItem::removeArrows(RptGuiItem* item) {
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

void RptGuiItem::removeAllArrows() {
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

void RptGuiItem::showAllArrows() {
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

QRectF RptGuiItem::boundingRect() const {
    QRectF rect = textItem_->boundingRect();
    QRectF newRect(rect.left(), rect.top(), rect.width()+6, rect.height()+40);
    return newRect;
}

void RptGuiItem::paint(QPainter* /*painter*/, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
    //painter->setPen(Qt::NoPen);
    //painter->setBrush(Qt::darkGray);
    //QRectF shadowRect(boundingRect().left()+3,boundingRect().top()+3,boundingRect().width(), boundingRect().height());
    //painter->drawRect(shadowRect);
    //
    //painter->setBrush(Qt::white);
    //painter->setPen(QPen(Qt::black, 0));

    ///*if (option->state & QStyle::State_Sunken) {
    //newCursor.setShape(Qt::CursorShape::ClosedHandCursor);
    //setCursor(newCursor);
    //}*/
    //
    //if (option->state & QStyle::State_MouseOver) {
    //    painter->setPen(QPen(Qt::red, 0));
    //    //std::cout << "Hover" << std::endl;  
    //}
    //if (option->state & QStyle::State_Selected) {
    //    painter->setBrush(Qt::yellow);
    //}
    //    
    //painter->drawRect(boundingRect());

}

void RptGuiItem::adjustArrows() {
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

    // property sets
    for (size_t i=0; i<propertySets_.size(); i++) {
        propertySets_[i]->adjustArrows();
    }

}

QVariant RptGuiItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange) {
        adjustArrows();
    }
    return QGraphicsItem::itemChange(change, value);
}

void RptGuiItem::createContextMenu() {
    QAction* deleteAction = new QAction(tr("Delete (Del)"), this);
    QAction* renameAction = new QAction(tr("Rename"), this);

    // add actions to context menu
    contextMenu_.addAction(renameAction);
    contextMenu_.addAction(deleteAction);

    // connect actions
    QObject::connect(renameAction, SIGNAL(triggered()), this, SLOT(renameActionSlot()));
    QObject::connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteActionSlot()));

    textItem_->setContextMenu(&contextMenu_);
}

void RptGuiItem::deleteActionSlot() {
    emit deleteSignal();
}

void RptGuiItem::renameActionSlot() {
    textItem_->setTextInteractionFlags(Qt::TextEditorInteraction);
    textItem_->setEnabled(true);
    textItem_->setFlag(QGraphicsItem::ItemIsSelectable);
    textItem_->setSelected(true);
    textItem_->setFlag(QGraphicsItem::ItemIsFocusable, true);
    textItem_->setFocus();
}

void RptGuiItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsItem::mousePressEvent(event);
}

void RptGuiItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsItem::mouseMoveEvent(event);
}

void RptGuiItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    adjustArrows();
    QGraphicsItem::mouseReleaseEvent(event);
}

void RptGuiItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    setSelected(true);
    contextMenu_.exec(event->screenPos());
}

void RptGuiItem::contentChanged() {
    emit changed();
}

// ----------------------------------------------------------------------------


RptPortItem::RptPortItem(Identifier type, Port* port, RptGuiItem* parent)
    : QGraphicsItem(parent)
    , type_(type)
    , port_(port)        
{  
    setToolTip(type.getName().c_str());
    setColor();
    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);
}

void RptPortItem::setColor() {
    if (port_->getType().getSubString(0) == "coprocessor") {
        color_ = Qt::green;
        sunkenColor_ = Qt::darkGreen;
    }
    else if (port_->getType().getSubString(0) == "volumeset") {
        color_ = Qt::magenta;
        sunkenColor_ = Qt::darkMagenta;
    }
    else if (port_->getType().getSubString(0) == "volumehandle") {
        color_ = Qt::red;
        sunkenColor_ = Qt::darkRed;
    }
    else if (port_->getType().getSubString(0) == "image") {
        color_ = Qt::blue;
        sunkenColor_ = Qt::darkBlue;
    }
    else if (port_->getType().getSubString(0) == "geometry") {
        color_ = Qt::yellow;
        sunkenColor_ = Qt::darkYellow;
    }
    else {
        color_ = Qt::gray;
        sunkenColor_ = Qt::darkGray;
    }
	color_.setAlpha(100);
}

bool RptPortItem::isOutport() {
    return port_->isOutport();
}

void RptPortItem::disconnect(RptPortItem* inport) {
    if (inport == 0) {
        return;
    }

    if (!isOutport())
        return;

    // remove this from inport's connectedPorts
    std::vector<RptPortItem*>& connected = inport->getConnected();
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

std::vector<RptPortItem*>& RptPortItem::getConnected() {
   return connectedPorts_;
}

void RptPortItem::addConnection(RptPortItem* port) {
    connectedPorts_.push_back(port);
}

void RptPortItem::adjustArrows() {
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

QRectF RptPortItem::boundingRect() const {
    QRectF rect(0,0,10,10);
    if (port_->allowMultipleConnections() && !port_->isOutport()) {
        if (port_->getType().getSubString(0) == "coprocessor")
            rect = QRectF(0,0,10,20);
        else
            rect = QRectF(0,0,25,10);
    }
    return rect;
}

void RptPortItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {    
    painter->setPen(QPen(color_, 2));
	painter->setBrush(QColor(255,255,255,150));

    if (option->state & QStyle::State_Sunken) {
        painter->setBrush(QColor(0,0,0,150));
    }
    if (option->state & QStyle::State_MouseOver) {
        painter->setBrush(color_);
    }
    painter->drawRect(boundingRect());

}

QVariant RptPortItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    return QGraphicsItem::itemChange(change, value);
}

void RptPortItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (isOutport()) {
        arrowList_.push_back(new RptArrow(this));
        //TODO: This line produces a warning
	    scene()->addItem(arrowList_.at(arrowList_.size()-1));
		arrowList_.at(arrowList_.size()-1)->adjust(event->scenePos());
    }

    QGraphicsItem::mousePressEvent(event);
}

void RptPortItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (isOutport()) {
        arrowList_.at(arrowList_.size()-1)->adjust(event->scenePos());
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void RptPortItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsItem* item = scene()->itemAt(event->scenePos());

    if (isOutport()) {
        if (item && item != this) {
            // released over port item
            if (item->type() == RptPortItem::Type) {
                RptPortItem* portItem = static_cast<RptPortItem*>(item);
                if (portItem->getParent() != getParent() && !portItem->isOutport()) {
                    getParent()->connect(this, portItem);
                    getParent()->contentChanged();
                }
                else {
                    delete(arrowList_.at(arrowList_.size()-1));
	                arrowList_.pop_back();
                }
            }

            // released over gui item
            else if (item->type() == RptProcessorItem::Type
                || item->type() == RptAggregationItem::Type
                || item->type() == RptTextItem::Type) { // text item
                    RptGuiItem* guiItem = (item->type() == RptTextItem::Type) ? static_cast<RptGuiItem*>(item->parentItem()) : static_cast<RptGuiItem*>(item);
                    getParent()->connect(this, guiItem);
                    getParent()->contentChanged();
            }
            else {
                delete(arrowList_.at(arrowList_.size()-1));
	            arrowList_.pop_back();
            }
        }
        else {
            delete(arrowList_.at(arrowList_.size()-1));
	        arrowList_.pop_back();
        }
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

bool RptPortItem::doesArrowExist(RptPortItem* destItem) {
	for (size_t i=0;i<arrowList_.size();i++) {
		if (arrowList_.at(i)->getDestNode() == destItem)
			return true;
	}
	return false;
}

//---------------------------------------------------------------------------

RptTextItem::RptTextItem(const QString& text, RptGuiItem* parent, QGraphicsScene* scene)
    : QGraphicsTextItem(text, parent, scene)
{
    setFlag(ItemIsSelectable, false);
	setDefaultTextColor(Qt::white);
}

QVariant RptTextItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemSelectedChange) {
        if (isSelected())
            emit sendText(toPlainText().toStdString());
    }
    return QGraphicsTextItem::itemChange(change, value);
}

void RptTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    setTextInteractionFlags(Qt::TextEditable);
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

void RptTextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    parentItem()->setSelected(true);
    if (contextMenu_)
        contextMenu_->exec(event->screenPos());
}

void RptTextItem::setContextMenu(QMenu* contextMenu) {
    contextMenu_ = contextMenu;
}


} //namespace voreen
