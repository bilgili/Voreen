/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/qt/networkeditor/graphicitems/core/aggregationgraphicsitem.h"

#include "voreen/qt/networkeditor/editor_settings.h"
#include "voreen/qt/networkeditor/styles/nwestyle_base.h"

#include "voreen/core/datastructures/meta/aggregationmetadata.h"

//core
#include "voreen/core/ports/port.h"
#include "voreen/core/ports/coprocessorport.h"

#include "voreen/core/processors/processor.h"
//gi
#include "voreen/qt/networkeditor/graphicitems/core/portgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/core/processorgraphicsitem.h"
#include "voreen/qt/networkeditor/graphicitems/connections/portarrowgraphicsitem.h"

namespace voreen {
AggregationGraphicsItem::AggregationGraphicsItem(Processor* aggregation, QList<PortOwnerGraphicsItem*>* items, voreen::NetworkEditor* networkEditor)
    : ProcessorGraphicsItem(aggregation ,networkEditor)
    , portOwnerItems_(items)
    , metaData_(0)
{
    /*networkEditor->scene()->addItem(this);

    setGuiName("Aggregation");
    //set text
    initializePaintSettings();
    paintHasBeenInitialized_ = true;
    getPropertyList()->updateParentLabel();

    setZValue(ZValuesPortOwnerGraphicsItem);
    setFlag(ItemIsMovable);
    setAcceptsHoverEvents(true);

    QPointF pos;
    foreach (PortOwnerGraphicsItem* item, *items) {
        pos += item->pos();
    }
    pos.rx() /= items->count();
    pos.ry() /= items->count();
    setPos(pos);

    registerPorts();
    updateNWELayerAndCursor();

    QList<ProcessorWidget*> widgets;
    foreach (Processor* proc, getProcessors()) {
        dynamic_cast<Observable<ProcessorObserver>* >(proc)->addObserver(this);
        if (proc->getProcessorWidget())
            widgets.push_back(proc->getProcessorWidget());
    }
    //set widgets
    if (widgets.size() > 0) {
        widgetToggleButton_.setProcessorWidgets(widgets);
        widgetToggleButton_.show();
    }
    //memorize old item positions
    foreach (PortOwnerGraphicsItem* po, *items) {
        QPointF relPos;
        relPos.rx() = po->pos().x() - pos.x();
        relPos.ry() = po->pos().y() - pos.y();

        relativePositionMap_.insert(po, relPos);
    }*/
}

AggregationGraphicsItem::~AggregationGraphicsItem() {}

//---------------------------------------------------------------------------------------------------------------
//                  nwebasegraphicsitem functions
//---------------------------------------------------------------------------------------------------------------
QRectF AggregationGraphicsItem::boundingRect() const {
    return currentStyle()->ProcessorGI_boundingRect(this);
}

QPainterPath AggregationGraphicsItem::shape() const {
    return currentStyle()->ProcessorGI_shape(this);
}

void AggregationGraphicsItem::initializePaintSettings() {
    currentStyle()->ProcessorGI_initializePaintSettings(this);
}

void AggregationGraphicsItem::mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget, NWEItemSettings& setting) {
    currentStyle()->ProcessorGI_paint(this,painter,option,widget,setting);
}

//---------------------------------------------------------------------------------------------------------------
//                  portownerfunctions
//---------------------------------------------------------------------------------------------------------------
QList<Processor*> AggregationGraphicsItem::getProcessors() const {
    QList<Processor*> list;
    foreach(PortOwnerGraphicsItem* poItem, *portOwnerItems_)
        list.append(poItem->getProcessors());
    return list;
}

void AggregationGraphicsItem::saveMeta(){}
void AggregationGraphicsItem::loadMeta(){}

void AggregationGraphicsItem::registerPorts(){
    foreach (PortOwnerGraphicsItem* item, *portOwnerItems_) {
        foreach (PortGraphicsItem* itemPort, item->getPortGraphicsItems()) {
            if (itemPort->isOutport()) {
                if(dynamic_cast<CoProcessorPort*>(itemPort->getPort()))
                    coOutports_.append(dynamic_cast<CoProcessorPort*>(itemPort->getPort()));
                else
                    outports_.append(itemPort->getPort());
            } else {
                bool outside = false;
                foreach(PortGraphicsItem* port, itemPort->getConnectedOutportGraphicsItems())
                    if(!contains(port->getPortOwner())){
                        foreach(PortArrowGraphicsItem* arrow, port->getArrowList())
                            if(arrow->getDestinationItem() == itemPort)
                                arrow->setVisible(false);
                        outside = true;
                        break;
                    }
                if((itemPort->getPort()->allowMultipleConnections() || !itemPort->getPort()->isConnected()) ||
                    (outside))
                    if(dynamic_cast<CoProcessorPort*>(itemPort->getPort()))
                        inports_.append(dynamic_cast<CoProcessorPort*>(itemPort->getPort()));
                    else
                        inports_.append(itemPort->getPort());
            }
        }
    }

    createChildItems();
    registerConnections();
    layoutChildItems();
}

void AggregationGraphicsItem::registerConnections() {
    foreach (PortOwnerGraphicsItem* item, *portOwnerItems_) {
        foreach (PortGraphicsItem* itemPort, item->getPortGraphicsItems()) {
            PortGraphicsItem* aggregationPort = getPortGraphicsItem(itemPort->getPort());
            if(aggregationPort) {
                if (itemPort->isOutport()) {
                    foreach (PortArrowGraphicsItem* arrow, itemPort->getArrowList()) {
                        bool arrowSourceItemInAggregation = contains(arrow->getSourceItem()->getPortOwner());
                        bool arrowDestinationItemInAggregation = contains(arrow->getDestinationItem()->getPortOwner());
                        bool correctPortItem = aggregationPort->getPort() == arrow->getSourceItem()->getPort();

                        if (!arrowDestinationItemInAggregation && arrowSourceItemInAggregation && correctPortItem) {
                            aggregationPort->addGraphicalConnection(arrow->getDestinationItem());
                        }
                    }
                } else {
                    foreach (PortGraphicsItem* connectedPort, itemPort->getConnectedOutportGraphicsItems()) {
                        foreach (PortArrowGraphicsItem* arrow, connectedPort->getArrowList()) {
                            bool arrowSourceItemInAggregation = contains(arrow->getSourceItem()->getPortOwner());
                            bool arrowDestinationItemInAggregation = contains(arrow->getDestinationItem()->getPortOwner());
                            bool correctPortItem = aggregationPort->getPort() == arrow->getDestinationItem()->getPort();

                            if (arrowDestinationItemInAggregation && !arrowSourceItemInAggregation && correctPortItem) {
                                connectedPort->addGraphicalConnection(aggregationPort);
                            }
                        }
                    }
                }
            }
        }
    }
}

void AggregationGraphicsItem::deregisterConnections(){
    foreach (PortGraphicsItem* aggregationPortItem, getPortGraphicsItems()) {
        if (aggregationPortItem->isOutport()) {
                /*foreach (PortArrowGraphicsItem* arrow, aggregationPortItem->getArrowList()) {
                    if (aggregationItem->getInternalPortArrows().contains(arrow))
                        continue;

                    exchangePortsInConnection(aggregationPortItem, otherPortItem, arrow->getDestinationItem());
                    arrow->setSourceItem(otherPortItem);
                    otherPortItem->addArrow(arrow);
                    aggregationPortItem->removeArrow(arrow);
                }*/
        }
        else {
            foreach (PortGraphicsItem* connectedPort, aggregationPortItem->getConnectedOutportGraphicsItems()) {
                foreach (PortArrowGraphicsItem* arrow, connectedPort->getArrowList()) {
                    bool arrowSourceItemInAggregation = contains(arrow->getSourceItem()->getPortOwner());
                    bool arrowDestinationItemInAggregation = contains(arrow->getDestinationItem()->getPortOwner());
                    bool correctPortItem = aggregationPortItem->getPort() == arrow->getDestinationItem()->getPort();

                    if (arrowDestinationItemInAggregation && !arrowSourceItemInAggregation && correctPortItem) {
                        arrow->setVisible(true);
                    }
                }
            }
        }
    }
}

void AggregationGraphicsItem::prepareDeaggregation() {
    deregisterConnections();
    foreach(PortOwnerGraphicsItem* po, *portOwnerItems_) {
        po->setVisible(true);
        po->loadMeta();
    }
}

void AggregationGraphicsItem::renameFinished(){
    nameLabel_.setTextInteractionFlags(Qt::NoTextInteraction);
    nameLabel_.setFlag(QGraphicsItem::ItemIsFocusable, false);
    nameLabel_.setFlag(QGraphicsItem::ItemIsSelectable, false);

    if (metaData_)
        metaData_->setName(nameLabel_.toPlainText().toStdString());

    nameChanged();
}

void AggregationGraphicsItem::toggleProcessorWidget(){}

void AggregationGraphicsItem::togglePropertyList(){}

//---------------------------------------------------------------------------------------------------------------
//                  processorobserver functions
//---------------------------------------------------------------------------------------------------------------
void AggregationGraphicsItem::processorWidgetCreated(const Processor* processor){}
void AggregationGraphicsItem::processorWidgetDeleted(const Processor* processor){}
void AggregationGraphicsItem::portsChanged(const Processor* processor){}
void AggregationGraphicsItem::propertiesChanged(const PropertyOwner* owner){}

//---------------------------------------------------------------------------------------------------------------
//                  aggreagtion functions
//---------------------------------------------------------------------------------------------------------------
QList<PortOwnerGraphicsItem*> AggregationGraphicsItem::getContainingItems() const {
    return *portOwnerItems_;
}

bool AggregationGraphicsItem::contains(PortOwnerGraphicsItem* poItem) const {
    foreach (PortOwnerGraphicsItem* i, *portOwnerItems_) {
        if (i == poItem)
            return true;

        if(dynamic_cast<AggregationGraphicsItem*>(i) &&
           dynamic_cast<AggregationGraphicsItem*>(i)->contains(poItem))
            return true;
    }
    return false;
}




//---------------------------------------------------------------------------------------------------------------
//                  getter and setter
//---------------------------------------------------------------------------------------------------------------
void AggregationGraphicsItem::setAggregationMetaData(AggregationMetaData* metaData) {
    metaData_ = metaData;
}

AggregationMetaData* AggregationGraphicsItem::getAggregationMetaData() const {
    return metaData_;
}



/*

bool AggregationGraphicsItem::hasProperty(const Property* prop) const {
    foreach (RootGraphicsItem* i, childItems_) {
        if (i->hasProperty(prop))
            return true;
    }
    return false;
}






void AggregationGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    QPointF newPos = pos();
        foreach (RootGraphicsItem* child, childItems_) {
            const QPointF& relativePos = relativePositionMap_[child];
            child->setPos(newPos + relativePos);
            child->saveMeta();
        }

    RootGraphicsItem::mouseMoveEvent(event);
}


void AggregationGraphicsItem::addInternalPortArrow(PortArrowGraphicsItem* arrow) {
    // we want no duplicates in the list
    if (!internalPortArrows_.contains(arrow)) {
        internalPortArrows_.append(arrow);
        Port* sourcePort = arrow->getSourceItem()->getPort();
        Port* destinationPort = arrow->getDestinationItem()->getPort();

        PortGraphicsItem* sourceItem = getPortGraphicsItem(sourcePort);
        PortGraphicsItem* destinationItem = getPortGraphicsItem(destinationPort);

        if (/*sourcePort->isInport() &&*/ /*!sourcePort->allowMultipleConnections())
            sourceItem->hide();

        if (/*destinationPort->isInport() &&*/ /*!destinationPort->allowMultipleConnections())
            destinationItem->hide();
    }
}

QList<PortArrowGraphicsItem*> AggregationGraphicsItem::getInternalPortArrows() const {
    return internalPortArrows_;
}

void AggregationGraphicsItem::addInternalLinkArrow(LinkArrowGraphicsItem* arrow, const ArrowLinkInformation& linkInformation) {
    tgtAssert(!internalLinkArrows_.contains(qMakePair(arrow, linkInformation)), "arrow is already in the list");

    internalLinkArrows_.append(qMakePair(arrow, linkInformation));
}

QList<QPair<LinkArrowGraphicsItem*, ArrowLinkInformation> > AggregationGraphicsItem::getInternalLinkArrows() const {
    return internalLinkArrows_;
}

QList<Processor*> AggregationGraphicsItem::getProcessors() const {
    QList<Processor*> result;
    foreach (RootGraphicsItem* item, childItems_)
        result += item->getProcessors();

    return result;
}

void AggregationGraphicsItem::renameFinished(bool changeChildItems) {
    textItem_.setTextInteractionFlags(Qt::NoTextInteraction);
    textItem_.setFlag(QGraphicsItem::ItemIsFocusable, false);
    textItem_.setFlag(QGraphicsItem::ItemIsSelectable, false);

    if (changeChildItems) {
        foreach (RootGraphicsItem* childItem, childItems_) {
            childItem->changeAggregationPrefix(textItem_.toPlainText());
        }
    }

    if (metaData_)
        metaData_->setName(getName().toStdString());
}


void AggregationGraphicsItem::toggleProcessorWidget() {
    QList<ProcessorWidget*> widgets;
    foreach (Processor* processor, getProcessors()) {
        if (processor->getProcessorWidget())
            widgets.push_back(processor->getProcessorWidget());
    }

    int visibleWidgetNum = 0;
    foreach (ProcessorWidget* widget, widgets) {
        if (widget->isVisible())
            visibleWidgetNum++;
    }

    if (widgets.size() == visibleWidgetNum) {
        foreach (ProcessorWidget* widget, widgets)
            widget->setVisible(false);
    }
    else {
        foreach (ProcessorWidget* widget, widgets)
            widget->setVisible(true);
    }

    widgetIndicatorButton_.update();
}

void AggregationGraphicsItem::toggleSingleProcessorWidget() {
    QObject* obj = QObject::sender();
    QAction* action = dynamic_cast<QAction*>(obj);
    tgtAssert(action, "This slot must be triggered from a QAction");
    tgtAssert(processorWidgetMap_.contains(action), "The triggering action is not contained in the map");
    ProcessorWidget* widget = processorWidgetMap_[action];
    widget->setVisible(!widget->isVisible());
    widgetIndicatorButton_.update();
}

void AggregationGraphicsItem::processorWidgetCreated(const Processor*) {
    QList<ProcessorWidget*> widgets;
    foreach (Processor* proc, getProcessors()) {
        if (proc->getProcessorWidget())
            widgets.push_back(proc->getProcessorWidget());
    }

    if (widgets.size() > 0) {
        widgetIndicatorButton_.setProcessorWidgets(widgets);
        widgetIndicatorButton_.show();
    }
}

void AggregationGraphicsItem::processorWidgetDeleted(const Processor*) {
    QList<ProcessorWidget*> widgets;
    foreach (Processor* proc, getProcessors()) {
        if (proc->getProcessorWidget())
            widgets.push_back(proc->getProcessorWidget());
    }

    if (widgets.size() > 0) {
        widgetIndicatorButton_.setProcessorWidgets(widgets);
        widgetIndicatorButton_.show();
    }
}

void AggregationGraphicsItem::propertiesChanged(const PropertyOwner*) {}
void AggregationGraphicsItem::portsChanged(const Processor*) {}

*/
} // namespace
