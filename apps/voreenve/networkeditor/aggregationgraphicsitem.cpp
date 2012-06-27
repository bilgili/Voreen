/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "aggregationgraphicsitem.h"

#include "networkeditor.h"
#include "portarrowgraphicsitem.h"
#include "portgraphicsitem.h"
#include "voreen/core/ports/port.h"
#include "voreen/core/processors/processorwidget.h"
#include "voreen/core/io/serialization/meta/aggregationmetadata.h"

namespace {
    const qreal edgeSpacing = 20.f;
}

namespace voreen {

AggregationGraphicsItem::AggregationGraphicsItem(QList<RootGraphicsItem*> items, NetworkEditor* networkEditor)
    : RootGraphicsItem(networkEditor)
    , childItems_(items)
    , metaData_(0)
{
    tgtAssert(items.size() > 0, "empty items list");

    setName("Aggregation");

    QPointF pos;
    foreach (RootGraphicsItem* item, items) {
        inports_ += item->getInports();
        outports_ += item->getOutports();
        coInports_ += item->getCoProcessorInports();
        coOutports_ += item->getCoProcessorOutports();

        pos += item->pos();
    }

    pos.rx() /= items.count();
    pos.ry() /= items.count();

    setPos(pos);

    setFlag(ItemIsMovable);
    createChildItems();

    QList<ProcessorWidget*> widgets;
    foreach (Processor* proc, getProcessors()) {
        proc->addObserver(this);
        if (proc->getProcessorWidget())
            widgets.push_back(proc->getProcessorWidget());
    }

    if (widgets.size() > 0) {
        widgetIndicatorButton_.setProcessorWidgets(widgets);
        widgetIndicatorButton_.show();
    }

    foreach (RootGraphicsItem* root, items) {
        QPointF relPos;
        relPos.rx() = root->pos().x() - pos.x();
        relPos.ry() = root->pos().y() - pos.y();

        relativePositionMap_.insert(root, relPos);
    }
}

RootGraphicsItem* AggregationGraphicsItem::clone() const {
    return new AggregationGraphicsItem(childItems_, networkEditor_);
}

bool AggregationGraphicsItem::contains(RootGraphicsItem* rootItem) const {
    if (rootItem == this)
        return true;

    foreach (RootGraphicsItem* i, childItems_) {
        if (i->contains(rootItem))
            return true;
    }

    return false;
}

bool AggregationGraphicsItem::contains(Processor* processor) const {
    foreach (RootGraphicsItem* i, childItems_) {
        if (i->contains(processor))
            return true;
    }
    return false;
}

bool AggregationGraphicsItem::hasProperty(const Property* prop) const {
    foreach (RootGraphicsItem* i, childItems_) {
        if (i->hasProperty(prop))
            return true;
    }
    return false;
}

QList<RootGraphicsItem*> AggregationGraphicsItem::getRootGraphicsItems() const {
    return childItems_;
}

int AggregationGraphicsItem::type() const {
    return Type;
}

void AggregationGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
/*
       8_____________________________7
      /                               \
      1                               6
      |                               |
      |                               |
      |                               |
      |                               |
      |                               |
      2                               5
      \3_____________________________4/
*/


    qreal left = boundingRect().left();
    qreal right = boundingRect().right();
    qreal top = boundingRect().top();
    qreal bottom = boundingRect().bottom();
    //qreal width = boundingRect().width();
    //qreal height = boundingRect().height();

    // spacing factor for rounded edges
    //qreal edgeSpacing = edgeCutoffRatio * qMax(width, height);

    QPointF point1(left, top + edgeSpacing);
    QPointF point2(left, bottom - edgeSpacing);
    QPointF point3(left + edgeSpacing, bottom);
    QPointF point4(right - edgeSpacing, bottom);
    QPointF point5(right, bottom - edgeSpacing);
    QPointF point6(right, top + edgeSpacing);
    QPointF point7(right - edgeSpacing, top);
    QPointF point8(left + edgeSpacing, top);

    QPainterPath path;
    path.moveTo(point1);
    path.lineTo(point2);
    path.cubicTo(QPointF(left, bottom), QPointF(left,bottom), point3);
    path.lineTo(point4);
    path.cubicTo(QPointF(right, bottom), QPointF(right, bottom), point5);
    path.lineTo(point6);
    path.cubicTo(QPointF(right, top), QPointF(right, top), point7);
    path.lineTo(point8);
    path.cubicTo(QPointF(left, top), QPointF(left, top), point1);

    painter->setClipPath(path);
    RootGraphicsItem::paint(painter, option, widget);
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

QList<Port*> AggregationGraphicsItem::getInports() const {
    return inports_;
}

QList<Port*> AggregationGraphicsItem::getOutports() const {
    return outports_;
}

QList<CoProcessorPort*> AggregationGraphicsItem::getCoProcessorInports() const {
    return coInports_;
}

QList<CoProcessorPort*> AggregationGraphicsItem::getCoProcessorOutports() const {
    return coOutports_;
}

void AggregationGraphicsItem::addInternalPortArrow(PortArrowGraphicsItem* arrow) {
    // we want no duplicates in the list
    if (!internalPortArrows_.contains(arrow)) {
        internalPortArrows_.append(arrow);
        Port* sourcePort = arrow->getSourceItem()->getPort();
        Port* destinationPort = arrow->getDestinationItem()->getPort();

        PortGraphicsItem* sourceItem = getPortGraphicsItem(sourcePort);
        PortGraphicsItem* destinationItem = getPortGraphicsItem(destinationPort);

        if (/*sourcePort->isInport() &&*/ !sourcePort->allowMultipleConnections())
            sourceItem->hide();

        if (/*destinationPort->isInport() &&*/ !destinationPort->allowMultipleConnections())
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

void AggregationGraphicsItem::setAggregationMetaData(AggregationMetaData* metaData) {
    metaData_ = metaData;
}

AggregationMetaData* AggregationGraphicsItem::getAggregationMetaData() const {
    return metaData_;
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

QList<QAction*> AggregationGraphicsItem::getProcessorWidgetContextMenuActions() {
    QList<QAction*> result;
    foreach (Processor* proc, getProcessors()) {
        if (proc->getProcessorWidget()) {
            QAction* action = new QAction(QString::fromStdString(proc->getName()), this);
            action->setCheckable(true);
            QObject::connect(action, SIGNAL(triggered()), this, SLOT(toggleSingleProcessorWidget()));
            processorWidgetMap_.insert(action, proc->getProcessorWidget());
            if (proc->getProcessorWidget()->isVisible())
                action->setChecked(true);
            result.append(action);
        }
    }
    return result;
}

} // namespace
