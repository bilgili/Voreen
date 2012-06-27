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

#include "rptaggregationitem.h"
#include "rptpropertysetitem.h"
#include "rptprocessoritem.h"
#include "voreen/core/vis/processors/processor.h"


namespace voreen {

RptAggregationItem::RptAggregationItem(std::vector<RptProcessorItem*> processors, QGraphicsScene* scene,
                                       std::string name, QGraphicsItem* parent)
    : RptGuiItem(name, parent)
    , processors_(processors)
{
    std::string s;
    for (size_t i=0; i<processors_.size(); i++) {
        if (i!=0)
            s.append("\n");
        s.append(processors_[i]->getType().getName());

        for (size_t j=0; j<processors_[i]->getPropertySets().size(); j++) {
            bool found = false;
            for (size_t k=0; k<propertySets_.size(); k++) {
                if (processors_[i]->getPropertySets()[j] == propertySets_[k])
                    found = true;
            }
            if (!found) {
                propertySets_.push_back(processors_[i]->getPropertySets()[j]);
            }
        }
    }
    setToolTip(QString(s.c_str()));
    
    scene->addItem(this);

    // correct position
    QPoint newPos = QPoint(0,0);
    for (size_t i=0; i<processors_.size(); i++) {
        newPos += processors_[i]->scenePos().toPoint();
    }
    newPos /= processors_.size();
    setPos(newPos);

    color_ = QColor(233,218,176);

    showContent_ = true;
    showContent(false);

    createContextMenu();

    setZValue(0.1);
}

RptAggregationItem::RptAggregationItem(std::vector<RptProcessorItem*> processors, std::string name, QGraphicsItem* parent)
    : RptGuiItem(name, parent)
    , processors_(processors)
{
    std::string s;
    for (size_t i=0; i<processors_.size(); i++) {
        if (i!=0)
            s.append("\n");
        s.append(processors_[i]->getType().getName());

        for (size_t j=0; j<processors_[i]->getPropertySets().size(); j++) {
            bool found = false;
            for (size_t k=0; k<propertySets_.size(); k++) {
                if (processors_[i]->getPropertySets()[j] == propertySets_[k])
                    found = true;
            }
            if (!found) {
                propertySets_.push_back(processors_[i]->getPropertySets()[j]);
            }
        }
    }
    setToolTip(QString(s.c_str()));
    color_ = QColor(233,218,176);
    showContent_ = false;

    createContextMenu();

    setZValue(0.1);
}

RptAggregationItem::~RptAggregationItem() {
    disconnectAll();
    for (size_t i=0; i<processors_.size(); i++) {
        delete processors_[i];
        processors_[i] = 0;
    }
}

void RptAggregationItem::initialize() {
    showContent_ = true;
    showContent(false);
}

std::vector<RptPortItem*> RptAggregationItem::getUnconnectedPortItems() {
    std::vector<RptPortItem*> ports;
    for (size_t i=0; i<processors_.size(); i++) {
        std::vector<RptPortItem*> portVector = processors_.at(i)->getInports();
        for (size_t j=0; j<portVector.size(); j++) {
            if (portVector[j]->getConnected().size() == 0)
                ports.push_back(portVector[j]);
        }
        portVector = processors_.at(i)->getOutports();
        for (size_t j=0; j<portVector.size(); j++) {
            if (portVector[j]->getConnected().size() == 0)
                ports.push_back(portVector[j]);
        }
    }
    return ports;
}

std::vector<RptPortItem*> RptAggregationItem::getUnconnectedCoProcessorPortItems() {
    std::vector<RptPortItem*> ports;
    for (size_t i=0; i<processors_.size(); i++) {
        std::vector<RptPortItem*> portVector = processors_.at(i)->getCoProcessorInports();
        for (size_t j=0; j<portVector.size(); j++) {
            if (portVector[j]->getConnected().size() == 0)
                ports.push_back(portVector[j]);
        }
        portVector = processors_.at(i)->getCoProcessorOutports();
        for (size_t j=0; j<portVector.size(); j++) {
            if (portVector[j]->getConnected().size() == 0)
                ports.push_back(portVector[j]);
        }
    }
    return ports;
}

std::vector<RptPortItem*> RptAggregationItem::getOutwardsConnectedPortItems() {
    std::vector<RptPortItem*> ports;

    for (size_t i=0; i<processors_.size(); i++) {
        // go through all inports
        std::vector<RptPortItem*> portVector = processors_.at(i)->getInports();
        for (size_t j=0; j<portVector.size(); j++) {
            // check if connectedPorts' parentItems are this aggregation
            for (size_t k=0; k<portVector[j]->getConnected().size(); k++) {
                if (!contains(portVector[j]->getConnected()[k]->getParent())) {
                    ports.push_back(portVector[j]);
                    break;  // so don't push_back a port twice
                }
            }
        }
        // go through all outports
        portVector = processors_.at(i)->getOutports();
        for (size_t j=0; j<portVector.size(); j++) {
            // check if connectedPorts' parentItems are in this aggregation
            for (size_t k=0; k<portVector[j]->getConnected().size(); k++) {
                if (!contains(portVector[j]->getConnected()[k]->getParent())) {
                    ports.push_back(portVector[j]);
                    break;  // so don't push_back a port twice
                }
            }
        }
    }

    return ports;
}

std::vector<RptPortItem*> RptAggregationItem::getOutwardsConnectedCoProcessorPortItems() {
    std::vector<RptPortItem*> ports;

    for (size_t i=0; i<processors_.size(); i++) {
        // go through all inports
        std::vector<RptPortItem*> portVector = processors_.at(i)->getCoProcessorInports();
        for (size_t j=0; j<portVector.size(); j++) {
            // check if connectedPorts' parentItems are in this aggregation
            for (size_t k=0; k<portVector[j]->getConnected().size(); k++) {
                if (!contains(portVector[j]->getConnected()[k]->getParent())) {
                    ports.push_back(portVector[j]);
                    break;  // so don't push_back a port twice
                }
            }
        }
        // go through all outports
        portVector = processors_.at(i)->getCoProcessorOutports();
        for (size_t j=0; j<portVector.size(); j++) {
            // check if connectedPorts' parentItems are in this aggregation
            for (size_t k=0; k<portVector[j]->getConnected().size(); k++) {
                if (!contains(portVector[j]->getConnected()[k]->getParent())) {
                    ports.push_back(portVector[j]);
                    break;  // so don't push_back a port twice
                }
            }
        }
    }

    return ports;
}

bool RptAggregationItem::contains(RptGuiItem* item) {
    for (size_t i=0; i<processors_.size(); i++) {
        if (processors_[i] == item) {
            return true;
        }
    }
    return false;
}

void RptAggregationItem::createContextMenu() {
    RptGuiItem::createContextMenu();
    
    // createActions
    showContentAction_ = new QAction(tr("Show Content (double click)"), this);
    deaggregateAction_ = new QAction(tr("Deaggregate"), this);
    QAction* saveAction = new QAction(tr("Save"),this);

    // add actions to context menu
    contextMenu_.addAction(showContentAction_);
    contextMenu_.addAction(deaggregateAction_);
    contextMenu_.addAction(saveAction);

    // connect actions
    QObject::connect(showContentAction_, SIGNAL(triggered()), this, SLOT(showContentActionSlot()));
    QObject::connect(deaggregateAction_, SIGNAL(triggered()), this, SLOT(deaggregateActionSlot()));
    QObject::connect(saveAction, SIGNAL(triggered()), this, SLOT(saveActionSlot()));
}

void RptAggregationItem::showContentActionSlot() {
    showContent(!showContent_);
}

void RptAggregationItem::deaggregateActionSlot() {
    emit deaggregateSignal();
}
void RptAggregationItem::saveActionSlot() {
    emit saveSignal(this);
}

void RptAggregationItem::createAndConnectPorts() {
    inports_.clear();
    outports_.clear();

    // get the unconnected ports
    std::vector<RptPortItem*> items = getUnconnectedPortItems();
    for (size_t i=0; i<items.size(); i++) {
        if (items[i]->isOutport()) 
            outports_.push_back(new RptPortItem(items[i]->getPortType(), items[i]->getPort(), this));
        else
            inports_.push_back(new RptPortItem(items[i]->getPortType(), items[i]->getPort(), this));
    }

    // get ports connected to ports outside the aggregation
    items = getOutwardsConnectedPortItems();
    for (size_t i=0; i<items.size(); i++) {
        // go through outports
        if (items[i]->isOutport()) {
            // if connected item is outside the aggregation
            for (size_t j=0; j<items[i]->getConnected().size(); j++) {
                if (!contains(items[i]->getConnected()[j]->getParent())) {
                    RptPortItem* outwardItem = items[i]->getConnected()[j];
                    // disconnect from processor port item
                    items[i]->getParent()->disconnect(items[i], outwardItem);
                    // and therefore connect it to aggregation port item
                    outports_.push_back(new RptPortItem(items[i]->getPortType(), items[i]->getPort(), this));
                    outports_.at(outports_.size()-1)->getParent()->connectAndCreateArrow(outports_.at(outports_.size()-1), outwardItem);
                    j--;
                }
            }
        }

        // go through inports
        else {
            // if connected item is outside the aggregation
            RptPortItem* inport = new RptPortItem(items[i]->getPortType(), items[i]->getPort(), this);
            for (size_t j=0; j<items[i]->getConnected().size(); j++) {
                if (!contains(items[i]->getConnected()[j]->getParent())) {
                    RptPortItem* outwardItem = items[i]->getConnected()[j];
                    // disconnect from processor port item
                    outwardItem->getParent()->disconnect(outwardItem, items[i]);
                    // and therefore connect it to aggregation port item
                    //inports_.push_back(new RptPortItem(items[i]->getPortType(), items[i]->getPort(), this));
                    outwardItem->getParent()->connectAndCreateArrow(outwardItem, inport);
                    j--;
                }
            }
            if (inport->getConnected().size() > 0)
                inports_.push_back(inport);
            else 
                delete(inport);
        }

    }
}

void RptAggregationItem::createAndConnectCoProcessorPorts() {
    coProcessorInports_.clear();
    coProcessorOutports_.clear();

    // get the unconnected ports
    std::vector<RptPortItem*> items = getUnconnectedCoProcessorPortItems();
    for (size_t i=0; i<items.size(); i++) {
        if (items[i]->isOutport()) 
            coProcessorOutports_.push_back(new RptPortItem(items[i]->getPortType(), items[i]->getPort(), this));
        else
            coProcessorInports_.push_back(new RptPortItem(items[i]->getPortType(), items[i]->getPort(), this));
    }

    // get ports connected to ports outside the aggregation
    items = getOutwardsConnectedCoProcessorPortItems();
    for (size_t i=0; i<items.size(); i++) {
        // go through outports
        if (items[i]->isOutport()) {
            // if connected item is outside the aggregation
            for (size_t j=0; j<items[i]->getConnected().size(); j++) {
                if (!contains(items[i]->getConnected()[j]->getParent())) {
                    RptPortItem* outwardItem = items[i]->getConnected()[j];
                    // disconnect from processor port item
                    items[i]->getParent()->disconnect(items[i], outwardItem);
                    // and therefore connect it to aggregation port item
                    coProcessorOutports_.push_back(new RptPortItem(items[i]->getPortType(), items[i]->getPort(), this));
                    coProcessorOutports_.at(coProcessorOutports_.size()-1)->getParent()->connectAndCreateArrow(coProcessorOutports_.at(coProcessorOutports_.size()-1), outwardItem);
                    j--;
                }
            }
        }

        // go through inports
        else {
            // if connected item is outside the aggregation
            RptPortItem* inport = new RptPortItem(items[i]->getPortType(), items[i]->getPort(), this);
            for (size_t j=0; j<items[i]->getConnected().size(); j++) {
                if (!contains(items[i]->getConnected()[j]->getParent())) {
                    RptPortItem* outwardItem = items[i]->getConnected()[j];
                    // disconnect from processor port item
                    outwardItem->getParent()->disconnect(outwardItem, items[i]);
                    // and therefore connect it to aggregation port item
                    outwardItem->getParent()->connectAndCreateArrow(outwardItem, inport);
                    j--;
                }
            }
            if (inport->getConnected().size() > 0)
                coProcessorInports_.push_back(inport);
            else 
                delete(inport);
        }
    }
}

void RptAggregationItem::showContent(bool show) {
    if (show && !showContent_) {
        showContent_ = true;
        setZValue(0.1);

        deaggregate();

        for (size_t i=0; i<processors_.size(); i++) {
            QPointF mov = scenePos() - oldPos_;
            processors_[i]->moveBy(mov.x(), mov.y());

            QPointF tmp = processors_[i]->scenePos();

            processors_[i]->setParentItem(this);

            QPointF tmp2 = processors_[i]->scenePos();
            QPointF correction = tmp - tmp2;
            processors_[i]->moveBy(correction.x(), correction.y());

            processors_[i]->adjustArrows();
        }
        updateGeometry();
    }
    else if (!show && showContent_) {
        showContent_ = false;
        setZValue(1);

        for (size_t i=0; i<processors_.size(); i++) {
            if (scene() && scene() == processors_[i]->scene()) {
                processors_[i]->removeAllArrows();
                QPointF tmp = processors_[i]->scenePos();
                
                processors_[i]->setParent(0);
                scene()->removeItem(processors_[i]);
                
                QPointF tmp2 = processors_[i]->scenePos();
                QPointF correction = tmp - tmp2;
                processors_[i]->moveBy(correction.x(), correction.y());
            }
        }
        oldPos_ = scenePos();

        updateGeometry();

        createAndConnectPorts();
        createAndConnectCoProcessorPorts();
        repositionPorts();

        QGraphicsScene* s = scene();
        s->removeItem(this);
        s->addItem(this);

        for (size_t i=0; i<propertySets_.size(); i++) {
            propertySets_[i]->aggregate(this);
        }

        adjustArrows();
    }
    updateGeometry();
}

std::vector<RptProcessorItem*> RptAggregationItem::deaggregate() {
    for (size_t i=0; i<processors_.size(); i++) {
        scene()->addItem(processors_[i]);
        processors_[i]->showAllArrows();

        RptProcessorItem* guiItem = processors_[i];
        guiItem->setParentItem(0);

        for (size_t j=0; j < inports_.size(); j++) {
            if (guiItem->getInport(inports_[j]->getPort()) != 0) {
                for (size_t k=0; k<inports_[j]->getConnected().size(); k++) {
                    RptPortItem* outwardPort = inports_[j]->getConnected()[k];
                    if (!contains(outwardPort->getParent())) {
                        outwardPort->getParent()->disconnect(outwardPort, inports_[j]);
                        outwardPort->getParent()->connectAndCreateArrow(outwardPort, guiItem->getInport(inports_[j]->getPort()));
                        k--;
                    }
                }
            }
        }

        for (size_t j=0; j < outports_.size(); j++) {
            if (guiItem->getOutport(outports_[j]->getPort()) != 0) {
                for (size_t k=0; k<outports_[j]->getConnected().size(); k++) {
                    RptPortItem* outwardPort = outports_[j]->getConnected()[k];
                    if (!contains(outwardPort->getParent())) {
                        outports_[j]->getParent()->disconnect(outports_[j],outwardPort);
                        guiItem->getOutport(outports_[j]->getPort())->getParent()->connectAndCreateArrow(guiItem->getOutport(outports_[j]->getPort()), outwardPort);
                        k--;
                    }
                }
            }
        }

        // --- coprocessor ports ---
        for (size_t j=0; j < coProcessorInports_.size(); j++) {
            if (guiItem->getInport(coProcessorInports_[j]->getPort()) != 0) {
                for (size_t k=0; k<coProcessorInports_[j]->getConnected().size(); k++) {
                    RptPortItem* outwardPort = coProcessorInports_[j]->getConnected()[k];
                    if (!contains(outwardPort->getParent())) {
                        outwardPort->getParent()->disconnect(outwardPort, coProcessorInports_[j]);
                        outwardPort->getParent()->connectAndCreateArrow(outwardPort, guiItem->getInport(coProcessorInports_[j]->getPort()));
                        k--;
                    }
                }
            }
        }

        for (size_t j=0; j < coProcessorOutports_.size(); j++) {
            if (guiItem->getOutport(coProcessorOutports_[j]->getPort()) != 0) {
                for (size_t k=0; k<coProcessorOutports_[j]->getConnected().size(); k++) {
                    RptPortItem* outwardPort = coProcessorOutports_[j]->getConnected()[k];
                    if (!contains(outwardPort->getParent())) {
                        coProcessorOutports_[j]->getParent()->disconnect(coProcessorOutports_[j],outwardPort);
                        guiItem->getOutport(coProcessorOutports_[j]->getPort())->getParent()->connectAndCreateArrow(guiItem->getOutport(coProcessorOutports_[j]->getPort()), outwardPort);
                        k--;
                    }
                }
            }
        }
    }

    for (size_t i=0; i < inports_.size(); i++) {
        delete(inports_[i]);
    }
    inports_.clear();

    for (size_t i=0; i<outports_.size(); i++) {
        delete(outports_[i]);
    }
    outports_.clear();

    // --- coprocessor ports ---
    for (size_t i=0; i < coProcessorInports_.size(); i++) {
        delete(coProcessorInports_[i]);
    }
    coProcessorInports_.clear();

    for (size_t i=0; i < coProcessorOutports_.size(); i++) {
        delete(coProcessorOutports_[i]);
    }
    coProcessorOutports_.clear();

    for (size_t i=0; i < propertySets_.size(); i++) {
        propertySets_[i]->deaggregate(this);
    }

    return processors_;
}

Processor* RptAggregationItem::getProcessor(RptPortItem* port) {
    return port->getPort()->getProcessor();
}

QVariant RptAggregationItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    return RptGuiItem::itemChange(change, value);
}

void RptAggregationItem::updateGeometry() {
    prepareGeometryChange();
    update();
    scene()->invalidate();
}

QPainterPath RptAggregationItem::shapePath(bool isShadow, int offset) const {
    QRectF bound = boundingRect();

    qreal x1, y1, x2, y2;
    bound.getCoords(&x1, &y1, &x2, &y2);

    // care about bounding geometry. QRectF's boundary line will be rendered
    // to the right and below the mathematical rectangle's boundary line (size = 3).
    y1 += 3;
    x1 += 3;

    // add parameter for visualization
    int cornerParam = 12;

    QPainterPath path;
    if (isShadow) {
        // adding shadow offset
        x1 += offset;
        y1 += offset;
    }
    else {
        // creating space for shadow
        x2 -= offset;
        y2 -= offset;
    }
    qreal dist = qMax(x2 - x1, y2 - y1);
    dist = dist / cornerParam;
    path.moveTo(x1 + dist, y1);
    path.lineTo(x1, y1 + dist);
    path.lineTo(x1, y2 - dist);
    path.lineTo(x1 + dist, y2);
    path.lineTo(x2 - dist, y2);
    path.lineTo(x2, y2 - dist);
    path.lineTo(x2, y1 + dist);
    path.lineTo(x2 - dist, y1);
    path.lineTo(x1 + dist, y1);
    return path;
}


QRectF RptAggregationItem::boundingRect() const {
    if (!showContent_) {
        QRectF rect = textItem_->boundingRect();
        QRectF newRect(rect.left() - 15, rect.top() - 2, rect.width() + 33, rect.height() + 40);
        textItem_->setPos(newRect.topLeft() + QPointF(newRect.left() + 33, newRect.top() + 20));
        return newRect;
    }
    else {
        QPainterPath path = QPainterPath();
        for (size_t i=0; i<processors_.size(); i++) {
            QRectF rect = processors_[i]->boundingRect();
            rect.translate(processors_[i]->scenePos()-scenePos());
            path.addRect(rect);
        }
        
        QRectF rect = path.boundingRect();
        QRectF newRect(rect.left()-5, rect.top()-textItem_->boundingRect().height()-5, rect.width()+10, rect.height()+textItem_->boundingRect().height()+10);
        textItem_->setPos(newRect.topLeft());

        return newRect;
    }
}

void RptAggregationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    QRectF shadowRect(boundingRect().left()+3,boundingRect().top()+3,boundingRect().width(), boundingRect().height());
    if (!showContent_)
        painter->drawPath(shapePath(true, 3));

    // draw linear gradient for top highlight and bottom darkening
    QRectF bbox = boundingRect();
    QLinearGradient linGradient(bbox.left(), bbox.top(), bbox.left(), bbox.bottom());
    linGradient.setColorAt(0, color_);
    linGradient.setColorAt(0.05, color_.light(220));
    linGradient.setColorAt(0.15, color_);
    linGradient.setColorAt(0.9, color_.dark(200));
    linGradient.setColorAt(1, Qt::black);
    painter->setBrush(linGradient);
    painter->drawPath(shapePath(false, 3));

    painter->setBrush(Qt::NoBrush);
    // hover effect
    if (option->state & QStyle::State_MouseOver)
        painter->setBrush(QColor(255, 255, 255, 70));
    // frame indicates selected process
    if (option->state & QStyle::State_Selected)
        painter->setPen(QPen(Qt::green, 3));
    // draw frame / hover effect
    painter->drawPath(shapePath(false, 3));
}

void RptAggregationItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (showContent_) {
        for (size_t i=0; i<processors_.size(); i++) {
            processors_[i]->adjustArrows();
        }
    }
    RptGuiItem::mouseMoveEvent(event);
}

void RptAggregationItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (showContent_) {
        for (size_t i=0; i<processors_.size(); i++) {
            processors_[i]->adjustArrows();
        }
    }
    RptGuiItem::mouseReleaseEvent(event);
}

void RptAggregationItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/) {
    showContent(!showContent_);
}

} //namespace voreen
