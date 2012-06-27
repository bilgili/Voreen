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

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/image/scale.h"
#include "voreen/core/vis/properties/property.h"
#include "voreen/core/vis/processors/processorfactory.h"
#include "voreen/core/vis/processors/processorwidget.h"
#include "voreen/qt/widgets/network/editor/arrowgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/openpropertylistbutton.h"
#include "voreen/qt/widgets/network/editor/propertygraphicsitem.h"
#include "voreen/qt/widgets/network/editor/networkeditor.h"
#include "voreen/qt/widgets/network/editor/portgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/textgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/propertylistgraphicsitem.h"

#include <iostream>
#include <typeinfo>
#include <vector>

namespace voreen {

namespace {

const qreal drawingRectMinimumWidth = 100.f;
const qreal drawingRectMinimumHeight = 60.f;

const qreal drawingRectWidthOffset = 30.f;
const qreal drawingRectHeightOffset = 10.f;

const qreal boundingRectSpacing = 1.f;

const qreal openPropertyListButtonOffsetX = 5.f;
const qreal openPropertyListButtonOffsetY = 5.f;

} // namespace

ProcessorGraphicsItem::ProcessorGraphicsItem(Processor* processor, NetworkEditor* const networkEditor)
    : QGraphicsItem(0)
    , networkEditor_(networkEditor)
    , processor_(processor)
    , linkExists_(false)
    , textItem_(processor->getName().c_str(), this)
    , propertyListItem_(this)
    , openPropertyListButton_(this)
{
    tgtAssert(processor_ != 0, "passed null pointer");
    tgtAssert(networkEditor != 0, "passed null pointer");

    QObject::connect(&textItem_, SIGNAL(renameFinished()), this, SLOT(renameFinished()));
    QObject::connect(&textItem_, SIGNAL(textChanged()), this, SLOT(nameChanged()));

    QObject::connect(&openPropertyListButton_, SIGNAL(pressed()), this, SLOT(showPropertyList()));

    QObject::connect(&propertyListItem_, SIGNAL(pressedPropertyGraphicsItem(PropertyGraphicsItem*)), this, SIGNAL(pressedPropertyGraphicsItem(PropertyGraphicsItem*)));

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);
    setZValue(ZValueProcessorGraphicsItemNormal);

    setColor();
    createChildItems();
}

ProcessorGraphicsItem::~ProcessorGraphicsItem() {}

ProcessorGraphicsItem* ProcessorGraphicsItem::clone() {
    Processor* copyProcessor = processor_->clone();
    NetworkEditor* copyNetworkEditor = networkEditor_;

    ProcessorGraphicsItem* copy = new ProcessorGraphicsItem(copyProcessor, copyNetworkEditor);

    return copy;
}

void ProcessorGraphicsItem::setName(const QString& name) {
    setName(name.toStdString());
}

void ProcessorGraphicsItem::setName(const std::string& name) {

    if (!networkEditor_ || !networkEditor_->getProcessorNetwork())
        return;

    try {
        networkEditor_->getProcessorNetwork()->setProcessorName(processor_, name);
        textItem_.setPlainText(QString::fromStdString(processor_->getName()));
        nameChanged();
    }
    catch (VoreenException& e) {
        LWARNINGC("voreen.qt.ProcessorGraphicsItem", e.what());
    }

    textItem_.setPlainText(QString::fromStdString(processor_->getName()));
}

QPointF ProcessorGraphicsItem::dockingPoint() const {
    return openPropertyListButton_.dockingPoint();
}

int ProcessorGraphicsItem::type() const {
    return Type;
}

bool ProcessorGraphicsItem::isExpanded() {
    return openPropertyListButton_.isVisible();
}

QList<PortGraphicsItem*> ProcessorGraphicsItem::getInports() const {
    return inports_;
}

QList<PortGraphicsItem*> ProcessorGraphicsItem::getOutports() const {
    return outports_;
}

QList<PortGraphicsItem*> ProcessorGraphicsItem::getCoProcessorInports() const {
    return coProcessorInports_;
}

QList<PortGraphicsItem*> ProcessorGraphicsItem::getCoProcessorOutports() const {
    return coProcessorOutports_;
}

QList<PortGraphicsItem*> ProcessorGraphicsItem::getPorts() const {
    QList<PortGraphicsItem*> result = getInports();
    result += getOutports();
    result += getCoProcessorInports();
    result += getCoProcessorOutports();

    return result;
}

void ProcessorGraphicsItem::setLayer(NetworkEditorLayer layer) {
    if (layer == NetworkEditorLayerDataflow) {
        openPropertyListButton_.setVisible(false);
        propertyListItem_.setVisible(false);

        foreach (PortGraphicsItem* portItem, getPorts()) {
            portItem->setLayer(layer);
            foreach (ArrowGraphicsItem* arrow, portItem->getArrowList()) {
                arrow->setLayer(layer);
            }
        }

    } else if (layer == NetworkEditorLayerLinking) {
        if (processor_->getProperties().size() != 0)
            openPropertyListButton_.setVisible(true);
        //propertyListItem_.setVisible(true);

        foreach (PortGraphicsItem* portItem, getPorts()) {
            portItem->setLayer(layer);
            foreach (ArrowGraphicsItem* arrow, portItem->getArrowList()) {
                arrow->setLayer(layer);
            }
        }
    }
}

NetworkEditorLayer ProcessorGraphicsItem::currentLayer() const {
    return networkEditor_->currentLayer();
}

QList<ArrowGraphicsItem*> ProcessorGraphicsItem::connectionsTo(ProcessorGraphicsItem* processorItem) {
    QList<ArrowGraphicsItem*> result;
    QList<PortGraphicsItem*> ports = getPorts();

    foreach (PortGraphicsItem* port, ports) {
        QList<ArrowGraphicsItem*> arrows = port->getArrowList();

        foreach (ArrowGraphicsItem* arrow, arrows) {
            PortGraphicsItem* destItem = qgraphicsitem_cast<PortGraphicsItem*>(arrow->getDestNode());

            if (destItem->getParent() == processorItem)
                result.append(arrow);
        }
    }

    return result;
}

void ProcessorGraphicsItem::createChildItems() {
    const std::vector<Port*> inports = processor_->getInports();
    const std::vector<Port*> outports = processor_->getOutports();

    foreach (Port* port, inports) {
        PortGraphicsItem* portItem = new PortGraphicsItem(port, this);
        inports_.push_back(portItem);
    }

    foreach (Port* port, outports) {
        PortGraphicsItem* portItem = new PortGraphicsItem(port, this);
        outports_.push_back(portItem);
    }

    // coProcessorPorts
    std::vector<CoProcessorPort*> coInports = processor_->getCoProcessorInports();
    std::vector<CoProcessorPort*> coOutports = processor_->getCoProcessorOutports();

    foreach (CoProcessorPort* port, coInports) {
        PortGraphicsItem* portItem = new PortGraphicsItem(port, this);
        coProcessorInports_.push_back(portItem);
    }

    foreach (CoProcessorPort* port, coOutports) {
        PortGraphicsItem* portItem = new PortGraphicsItem(port, this);
        coProcessorOutports_.push_back(portItem);
    }

    propertyListItem_.hide();

    layoutChildItems();
}

void ProcessorGraphicsItem::layoutChildItems() {
    // ports should be distributed evenly across the side of the processoritem
    foreach (PortGraphicsItem* portItem, inports_) {
        QPointF position = getPositionForPort(portItem);
        portItem->setPos(position);
    }

    foreach (PortGraphicsItem* portItem, outports_) {
        QPointF position = getPositionForPort(portItem);
        portItem->setPos(position);
    }

    foreach (PortGraphicsItem* portItem, coProcessorInports_) {
        QPointF position = getPositionForPort(portItem);
        portItem->setPos(position);
    }

    foreach (PortGraphicsItem* portItem, coProcessorOutports_) {
        QPointF position = getPositionForPort(portItem);
        portItem->setPos(position);
    }


    // the text label should be in the middle of the processoritem
    qreal x = (drawingRect().width() / 2.f) - (textItem_.boundingRect().width() / 2.f);
    qreal y = (drawingRect().height() / 2.f) - (textItem_.boundingRect().height() / 2.f);
    textItem_.setPos(x, y);

    // the propertylist item should be centered on the bottom of the proecssoritem
    // the extra spacing is necessary, because we use drawingRect() as base
    x = (drawingRect().width() / 2.f) - (propertyListItem_.boundingRect().width() / 2.f);
    y = drawingRect().height() + boundingRectSpacing;
    propertyListItem_.setPos(x, y);

    // the openpropertybutton should be located in the upper left area with some small margins
    openPropertyListButton_.setPos(openPropertyListButtonOffsetX, openPropertyListButtonOffsetY);
};

Processor* ProcessorGraphicsItem::getProcessor() {
    return processor_;
}

QPointF ProcessorGraphicsItem::getPositionForPort(PortGraphicsItem* portItem) {
    Port* port = portItem->getPort();
    qreal x = 0.f;
    qreal y = 0.f;
    int position = 0;
    int numPorts = 0;
    if (dynamic_cast<CoProcessorPort*>(port)) {
        if (port->isInport()) {
            // find the position of port 'port' in the vector
            const std::vector<CoProcessorPort*> inports = processor_->getCoProcessorInports();
            std::vector<CoProcessorPort*>::const_iterator portPosition = std::find(inports.begin(), inports.end(), port);
            tgtAssert(portPosition != inports.end(), "didn't find port");

            position = std::distance(inports.begin(), portPosition);
            numPorts = static_cast<int>(inports.size());

            // set the x value
            x = 0.f;
        } else {
            // find the position of port 'port' in the vector
            const std::vector<CoProcessorPort*> outports = processor_->getCoProcessorOutports();
            std::vector<CoProcessorPort*>::const_iterator portPosition = std::find(outports.begin(), outports.end(), port);
            tgtAssert(portPosition != outports.end(), "didn't find port");

            position = std::distance(outports.begin(), portPosition);
            numPorts = static_cast<int>(outports.size());

            // set the x value
            x = drawingRect().width() - (portItem->boundingRect().width() / 2.f);;
        }

        y = (position+1) * drawingRect().height()/(numPorts+1) - portItem->boundingRect().height()/2;
    } else {
        if (port->isInport()) {
            // find the position of port 'port' in the vector
            const std::vector<Port*> inports = processor_->getInports();
            std::vector<Port*>::const_iterator portPosition = std::find(inports.begin(), inports.end(), port);
            tgtAssert(portPosition != inports.end(), "didn't find port");

            position = std::distance(inports.begin(), portPosition);
            numPorts = static_cast<int>(inports.size());

            // set the y value
            y = 0.f;
        } else {
            const std::vector<Port*> outports = processor_->getOutports();
            std::vector<Port*>::const_iterator portPosition = std::find(outports.begin(), outports.end(), port);
            tgtAssert(portPosition != outports.end(), "didn't find port");

            position = std::distance(outports.begin(), portPosition);
            numPorts = static_cast<int>(outports.size());

            // set the y value
            // /2.f because the outports should leave the processoritem by some pixels
            y = drawingRect().height() - (portItem->boundingRect().height() / 2.f);
        }

        x = (position+1) * drawingRect().width()/(numPorts+1) - portItem->boundingRect().width()/2;
    }
    return QPoint(static_cast<int>(x), static_cast<int>(y));
}

const QString ProcessorGraphicsItem::getName() const {
    return textItem_.toPlainText();
}

PropertyGraphicsItem* ProcessorGraphicsItem::getProperty(Property* property) {
    return propertyListItem_.getProperty(property);
}

PortGraphicsItem* ProcessorGraphicsItem::getPort(Port* port) {
    tgtAssert(port != 0, "passed null pointer");

    if (port->isInport()) {
        if (dynamic_cast<CoProcessorPort*>(port)) {
            foreach (PortGraphicsItem* portGraphicsItem, coProcessorInports_) {
                if (portGraphicsItem->getPort() == port)
                    return portGraphicsItem;
            }
        } else {
            foreach (PortGraphicsItem* portGraphicsItem, inports_) {
                if (portGraphicsItem->getPort() == port)
                    return portGraphicsItem;
            }
        }
    } else {
        if (dynamic_cast<CoProcessorPort*>(port)) {
            foreach (PortGraphicsItem* portGraphicsItem, coProcessorOutports_) {
                if (portGraphicsItem->getPort() == port)
                    return portGraphicsItem;
            }
        }
        else {
            foreach (PortGraphicsItem* portGraphicsItem, outports_) {
                if (portGraphicsItem->getPort() == port)
                    return portGraphicsItem;
            }
        }
    }
    return 0;
}

bool ProcessorGraphicsItem::connect(ProcessorGraphicsItem* dest) {
    bool success = false;

    QList<PortGraphicsItem*> outports = getOutports();
    QList<PortGraphicsItem*> inports = dest->getInports();

    // find matching ports
    for (int j = 0; j < inports.size(); ++j) {
        for (int i = 0; i < outports.size(); ++i) {
            if (testConnect(outports[i], inports[j])) {
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
    for (int j = 0; j < inports.size(); ++j) {
        for (int i = 0; i < outports.size(); ++i) {
            if (testConnect(outports[i], inports[j])) {
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

    QList<PortGraphicsItem*> outports = getOutports();
    QList<PortGraphicsItem*> inports = dest->getInports();

    // find matching ports
    for (int j = 0; j < inports.size(); ++j) {
        for (int i = 0; i < outports.size(); ++i) {
            if (testConnect(outports[i], inports[j])) {
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
    for (int j = 0; j < inports.size(); ++j) {
        for (int i = 0; i < outports.size(); ++i) {
            if (testConnect(outports[i], inports[j])) {
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
    for (int i = 0; i < dest->getInports().size(); ++i) {
        if (testConnect(outport, dest->getInports()[i]))
            return connect(outport, dest->getInports()[i]);
    }
    // look also in coprocessor ports
    for (int i = 0; i < dest->getCoProcessorInports().size(); ++i) {
        if (testConnect(outport, dest->getCoProcessorInports()[i]))
            return connect(outport, dest->getCoProcessorInports()[i]);
    }
    QList<ArrowGraphicsItem*>& arrows = outport->getArrowList();
    delete arrows.back();
    arrows.pop_back();
    return false;
}

bool ProcessorGraphicsItem::testConnect(PortGraphicsItem* outport, ProcessorGraphicsItem* dest) {
    // find first matching port in dest and connect
    for (int i = 0; i < dest->getInports().size(); ++i) {
        if (testConnect(outport, dest->getInports()[i]))
            return true;
    }
    // look also in coprocessor ports
    for (int i = 0; i < dest->getCoProcessorInports().size(); ++i) {
        if (testConnect(outport, dest->getCoProcessorInports()[i]))
            return true;
    }
    return false;
}


bool ProcessorGraphicsItem::connect(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    if (!networkEditor_ || !networkEditor_->getProcessorNetwork()) {
        return false;
    }

    if (networkEditor_->getProcessorNetwork()->connectPorts(inport->getPort(), outport->getPort())) {
        outport->addConnection(inport);
        inport->addConnection(outport);
        outport->getArrowList().at(outport->getArrowList().size()-1)->setDestNode(inport);
        emit portConnectionsChanged();

        return true;
    }

    QList<ArrowGraphicsItem*>& arrows = outport->getArrowList();
    delete(arrows.at(arrows.size()-1));
    arrows.pop_back();

    if(sizeOriginConnectFailed(outport, inport)) {
        Processor* proc = new SingleScale();
        ProcessorGraphicsItem* rgi = new ProcessorGraphicsItem(proc, networkEditor_);

        QPointF pos = (outport->getParent()->pos() + inport->getParent()->pos()) / 2.0f;
        networkEditor_->addProcessor(rgi);
        rgi->setPos(pos);

        PortGraphicsItem* scalein = rgi->getInports()[0];
        PortGraphicsItem* scaleout = rgi->getOutports()[0];

        connectAndCreateArrow(outport, scalein);
        connectAndCreateArrow(scaleout, inport);

        scene()->addItem(outport->getArrowList().back());
        scene()->addItem(scaleout->getArrowList().back());
        return true;
    }
    return false;
}

bool ProcessorGraphicsItem::sizeOriginConnectFailed(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    if(!outport->getPort() || !inport->getPort())
        return false;

    RenderPort* rout = dynamic_cast<RenderPort*>(outport->getPort());
    RenderPort* rin = dynamic_cast<RenderPort*>(inport->getPort());

    if ( (rin && rout) &&
         (rin != rout) &&
         (rout->isOutport() &&  rin->isInport()) &&
         (rin->getProcessor() != rout->getProcessor()) &&
         !rout->isConnectedTo(rin) &&
            (!rin->isConnected() || rin->allowMultipleConnections()) )
        return true;

    return false;
}

bool ProcessorGraphicsItem::testConnect(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    if(!outport->getPort() || !inport->getPort())
        return false;
    return outport->getPort()->testConnectivity(inport->getPort()) ;
}

bool ProcessorGraphicsItem::connectAndCreateArrow(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    if (!networkEditor_ || !networkEditor_->getProcessorNetwork()) {
        return false;
    }
    if (networkEditor_->getProcessorNetwork()->connectPorts(inport->getPort(), outport->getPort())) {
        connectGuionly(outport, inport);
        emit portConnectionsChanged();
        return true;
    }
    return false;
}

void ProcessorGraphicsItem::connectGuionly(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    QList<ArrowGraphicsItem*>& arrows = outport->getArrowList();
    arrows.push_back(new ArrowGraphicsItem(outport));
    outport->addConnection(inport);
    inport->addConnection(outport);
    outport->getArrowList().back()->setDestNode(inport);
    outport->getArrowList().back()->adjust();
}

void ProcessorGraphicsItem::disconnect(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    if ((outport == 0) || (inport == 0)) {
        return;
    }
    if ((outport->getPort() == 0) || (inport->getPort() == 0)) {
        return;
    }
    if (!networkEditor_ || !networkEditor_->getProcessorNetwork()) {
        return;
    }

    networkEditor_->getProcessorNetwork()->disconnectPorts(inport->getPort(), outport->getPort());
    outport->disconnect(inport);
    emit portConnectionsChanged();
}

void ProcessorGraphicsItem::disconnectAll() {
    for (int i = 0; i < inports_.size(); ++i) {
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

    for (int i = 0; i < outports_.size(); ++i) {
        if (outports_[i] != 0) {
            while (outports_[i]->getConnected().size() > 0) {
                if (outports_[i]->getParent() != 0) {
                    outports_[i]->getParent()->disconnect(outports_[i], outports_[i]->getConnected()[0]);
                }
            }
        }
    }

    // disconnect coprocessor ports
    for (int i = 0; i < coProcessorInports_.size(); ++i) {
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

    for (int i = 0; i < coProcessorOutports_.size(); ++i) {
        if (coProcessorOutports_[i] != 0) {
            while (coProcessorOutports_[i]->getConnected().size() > 0) {
                ProcessorGraphicsItem* parent = coProcessorOutports_[i]->getParent();
                if (parent != 0) {
                    parent->disconnect(coProcessorOutports_[i], coProcessorOutports_[i]->getConnected()[0]);
                }
            }
        }
    }

    emit portConnectionsChanged();
}


void ProcessorGraphicsItem::removeFromScene() {
    // remove item itself from scene
    if (scene())
        scene()->removeItem(this);
    emit portConnectionsChanged();
}

void ProcessorGraphicsItem::removeArrows(ProcessorGraphicsItem* item) {
    for (int i = 0; i < outports_.size(); ++i) {
        for (int j = 0; j < outports_[i]->getArrowList().size(); ++j) {
            if (outports_[i]->getArrowList().at(j)->getDestNode()->parentItem() == item) {
                scene()->removeItem(outports_[i]->getArrowList().at(j));
            }
        }
        emit portConnectionsChanged();
    }

    // --- coprocessor ports ---
    for (int i = 0; i < coProcessorOutports_.size(); ++i) {
        for (int j = 0; j < coProcessorOutports_[i]->getArrowList().size(); ++j) {
            if (coProcessorOutports_[i]->getArrowList().at(j)->getDestNode()->parentItem() == item) {
                scene()->removeItem(coProcessorOutports_[i]->getArrowList().at(j));
            }
        }
    }
}

void ProcessorGraphicsItem::removeAllArrows() {
    for (int i = 0; i < outports_.size(); ++i) {
        for (int j = 0; j < outports_[i]->getArrowList().size(); ++j) {
            scene()->removeItem(outports_[i]->getArrowList().at(j));
        }
    }

    // --- coprocessor ports ---
    for (int i = 0; i < coProcessorOutports_.size(); ++i) {
        for (int j = 0; j < coProcessorOutports_[i]->getArrowList().size(); ++j) {
            scene()->removeItem(coProcessorOutports_[i]->getArrowList().at(j));
        }
    }
    emit portConnectionsChanged();
}

void ProcessorGraphicsItem::showAllArrows() {
    adjustArrows();
    if (scene()) {
        for (int i = 0; i < outports_.size(); ++i) {
            for (int j = 0; j < outports_[i]->getArrowList().size(); ++j) {
                //scene()->addRect(outports_[i]->getArrowList().at(j)->boundingRect(), QPen(Qt::green, 1, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin));
                scene()->addItem(outports_[i]->getArrowList().at(j));
            }
        }

        // --- coprocessor ports ---
        for (int i = 0; i < coProcessorOutports_.size(); ++i) {
            for (int j = 0; j < coProcessorOutports_[i]->getArrowList().size(); ++j) {
                //scene()->addRect(coProcessorOutports_[i]->getArrowList().at(j)->boundingRect(), QPen(Qt::red, 1, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin));
                scene()->addItem(coProcessorOutports_[i]->getArrowList().at(j));
            }
        }
    }
}

void ProcessorGraphicsItem::adjustArrows() {
    // incoming arrows
    for (int i = 0; i < inports_.size(); ++i) {
        inports_[i]->adjustArrows();
    }
    // outgoing arrows
    for (int i = 0; i < outports_.size(); ++i) {
        outports_[i]->adjustArrows();
    }

    // --- coprocessor ports ---
    // incoming arrows
    for (int i = 0; i < coProcessorInports_.size(); ++i) {
        coProcessorInports_[i]->adjustArrows();
    }
    // outgoing arrows
    for (int i = 0; i < coProcessorOutports_.size(); ++i) {
        coProcessorOutports_[i]->adjustArrows();
    }
    networkEditor_->adjustLinkArrowGraphicsItems();
}

QVariant ProcessorGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value) {
/*    if (change == ItemPositionChange) {
        adjustArrows();
    }
    else*/ if (change == ItemSelectedChange) {
        if (!value.toBool() && (textItem_.textInteractionFlags() & Qt::TextEditorInteraction))
            // item is deselected and in renaming mode -> finish renaming
            renameFinished();
    }

    return QGraphicsItem::itemChange(change, value);
}

void ProcessorGraphicsItem::enterRenameMode() {
    textItem_.setTextInteractionFlags(Qt::TextEditorInteraction);
    textItem_.setFlag(QGraphicsItem::ItemIsFocusable, true);
    textItem_.setFocus();
}

void ProcessorGraphicsItem::renameFinished() {
    textItem_.setTextInteractionFlags(Qt::NoTextInteraction);
    textItem_.setFlag(QGraphicsItem::ItemIsFocusable, false);
    textItem_.setFlag(QGraphicsItem::ItemIsSelectable, false);
    nameChanged();
}

void ProcessorGraphicsItem::showPropertyList() {
    prepareGeometryChange();
    if (propertyListItem_.isVisible()) {
        propertyListItem_.setVisible(false);
        //setZValue(10);
    }
    else {
        propertyListItem_.setVisible(true);
        //setZValue(11);
    }

    layoutChildItems();
    adjustArrows();

    networkEditor_->resetCachedContent();
}

void ProcessorGraphicsItem::nameChanged() {

    if (!networkEditor_ || !networkEditor_->getProcessorNetwork())
        return;

    try {
        networkEditor_->getProcessorNetwork()->setProcessorName(processor_, textItem_.toPlainText().toStdString());
    }
    catch (VoreenException& e) {
        LWARNINGC("voreen.qt.ProcessorGraphicsItem", e.what());
        textItem_.setPlainText(QString::fromStdString(processor_->getName()));
    }
    
    prepareGeometryChange();
    layoutChildItems();
    adjustArrows();
    if (scene())
        scene()->invalidate();

    emit processorNameChanged(getProcessor());
}

void ProcessorGraphicsItem::setColor() {
    color_ = QColor(233, 218, 176);
}

QRectF ProcessorGraphicsItem::drawingRect() const {
    QRectF rect = textItem_.boundingRect();

    qreal width = rect.width();
    qreal height = rect.height();

    width += drawingRectWidthOffset;
    height += drawingRectHeightOffset;

    if (width < drawingRectMinimumWidth)
        width = drawingRectMinimumWidth;

    if (height < drawingRectMinimumHeight)
        height = drawingRectMinimumHeight;

    rect.setWidth(width);
    rect.setHeight(height);

    return rect;
}

QRectF ProcessorGraphicsItem::boundingRect() const {
    QRectF rect = drawingRect();

    qreal left = rect.left() - boundingRectSpacing;
    qreal top = rect.top() - boundingRectSpacing;
    qreal width = rect.width() + 2.f * boundingRectSpacing;
    qreal height = rect.height() + 2.f * boundingRectSpacing;

    rect = QRectF(left, top, width, height);

    if (propertyListItem_.isVisible()) {
        qreal width = std::max(rect.width(), propertyListItem_.boundingRect().width());
        qreal height = rect.height() + propertyListItem_.boundingRect().height();

        rect.setWidth(width);
        rect.setHeight(height);
    }

    return rect;
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

    // frame indicates selected process
    if (option->state & QStyle::State_Selected) {
        button_color = Qt::red;
#ifndef __GNUC__
        setZValue(ZValueProcessorGraphicsItemMouseOver);
#endif
    }
    // hover effect
    else if (option->state & QStyle::State_MouseOver){
        button_color = Qt::blue;
#ifndef __GNUC__
        setZValue(ZValueProcessorGraphicsItemMouseOver);
#endif
    }
    else {
#ifndef __GNUC__
         setZValue(ZValueProcessorGraphicsItemNormal);
#endif
    }

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

    if(processor_->isUtility())
        painter->setOpacity(0.5);
    else
        painter->setOpacity(1.0);

    painter->drawRect(button_rect);

    //glass highlight
    painter->setBrush(QBrush(Qt::white));
    painter->setPen(QPen(QBrush(Qt::white), 0.01));
    painter->setOpacity(0.30);
    button_rect.setHeight(button_rect.height()/2.0);
    painter->drawRect(button_rect);
}

ProcessorGraphicsItem& ProcessorGraphicsItem::saveMeta() {
    PositionMetaData* meta = new PositionMetaData(
        static_cast<int>(x()),
        static_cast<int>(y()));

    processor_->getMetaDataContainer().addMetaData("ProcessorGraphicsItem", meta);

    return *this;
}

ProcessorGraphicsItem& ProcessorGraphicsItem::loadMeta() {
    if (processor_->getMetaDataContainer().hasMetaData("ProcessorGraphicsItem")) {
        PositionMetaData* meta = dynamic_cast<PositionMetaData*>(processor_->getMetaDataContainer().getMetaData("ProcessorGraphicsItem"));
        setPos(meta->getX(), meta->getY());
    }

    setName(processor_->getName());

    return *this;
}

bool ProcessorGraphicsItem::linkExists(){
    return linkExists_;
}

void ProcessorGraphicsItem::setLinkExists(bool b) {
    //linkExists_ = b;
    openPropertyListButton_.setChecked(b);
}

QGraphicsItem* ProcessorGraphicsItem::getPropertyLinkPortGraphicsItem() {
    return &openPropertyListButton_;
}

QList<PropertyGraphicsItem*> ProcessorGraphicsItem::getPropertyGraphicsItems(){
    return propertyListItem_.getPropertyGraphicsItems();
}

void ProcessorGraphicsItem::update() {
    openPropertyListButton_.update();

    QGraphicsItem::update();
}

void ProcessorGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    // toggle visibility of processor widget
    tgtAssert(processor_, "No processor");
    switch (networkEditor_->currentLayer()) {
    case NetworkEditorLayerDataflow:
        if (processor_->getProcessorWidget()) {
            processor_->getProcessorWidget()->setVisible(!processor_->getProcessorWidget()->isVisible());
        }
        break;
    case NetworkEditorLayerLinking:
        showPropertyList();
        break;
    default:
        tgtAssert(false, "should not get here");
    }
    event->accept();
}

void ProcessorGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    adjustArrows();

    QGraphicsItem::mouseMoveEvent(event);
}

void ProcessorGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    adjustArrows();

    QGraphicsItem::mouseReleaseEvent(event);
}


} // namespace voreen
