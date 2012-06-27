/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "processorgraphicsitem.h"

#include "voreen/core/processors/processor.h"
#include "modules/base/processors/utility/scale.h"
#include "voreen/core/properties/property.h"
#include "voreen/core/processors/processorfactory.h"
#include "voreen/core/processors/processorwidget.h"
#include "openpropertylistbutton.h"
#include "portarrowgraphicsitem.h"
#include "propertygraphicsitem.h"
#include "networkeditor.h"
#include "portgraphicsitem.h"
#include "textgraphicsitem.h"
#include "progressbargraphicsitem.h"

#include <iostream>
#include <typeinfo>
#include <vector>

#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

namespace voreen {

namespace {

const qreal utilityOpacityValue = 0.5;

const QColor baseColor = QColor(50, 50, 50, 255);
const QColor selectedColor = Qt::red;
const QColor highlightColor = Qt::blue;
const QColor shadowColor = Qt::black;

} // namespace

ProcessorGraphicsItem::ProcessorGraphicsItem(Processor* processor, NetworkEditor* networkEditor)
    : RootGraphicsItem(networkEditor)
    , processor_(processor)
{
    tgtAssert(processor_ != 0, "passed null pointer");

    setName(QString::fromStdString(processor_->getName()));

    if (processor_->isUtility())
        setOpacity(utilityOpacityValue);

    initializePorts();

    if (processor_->usesExpensiveComputation()) {
        QPointF center(boundingRect().x() + boundingRect().width() / 2.0, boundingRect().y() + boundingRect().height() * 0.775);
        qreal width = boundingRect().width() * 0.8;
        qreal height = 8;
        progressBar_ = new ProgressBarGraphicsItem(this, center, width, height);
        processor_->setProgressBar(progressBar_);
    }

    dynamic_cast<Observable<ProcessorObserver>* >(processor_)->addObserver(this);
}

void ProcessorGraphicsItem::initializePorts() {
    inports_ = stdVectorToQList<Port*>(processor_->getInports());
    outports_ = stdVectorToQList<Port*>(processor_->getOutports());
    coInports_ = stdVectorToQList<CoProcessorPort*>(processor_->getCoProcessorInports());
    coOutports_ = stdVectorToQList<CoProcessorPort*>(processor_->getCoProcessorOutports());

    createChildItems();
}

QList<Port*> ProcessorGraphicsItem::getInports() const {
    return inports_;
}

QList<Port*> ProcessorGraphicsItem::getOutports() const {
    return outports_;
}

QList<CoProcessorPort*> ProcessorGraphicsItem::getCoProcessorInports() const {
    return coInports_;
}

QList<CoProcessorPort*> ProcessorGraphicsItem::getCoProcessorOutports() const {
    return coOutports_;
}

int ProcessorGraphicsItem::type() const {
    return Type;
}

RootGraphicsItem* ProcessorGraphicsItem::clone() const {
    Processor* copyProcessor = processor_->clone();
    NetworkEditor* copyNetworkEditor = networkEditor_;

    ProcessorGraphicsItem* copy = new ProcessorGraphicsItem(copyProcessor, copyNetworkEditor);

    return copy;
}

void ProcessorGraphicsItem::layoutChildItems() {
    if (progressBar_) {
        QPointF center(boundingRect().x() + boundingRect().width() / 2.0, boundingRect().y() + boundingRect().height() * 0.775);
        qreal width = boundingRect().width() * 0.8;
        qreal height = 8;

        progressBar_->resize(center, width, height);
    }
    RootGraphicsItem::layoutChildItems();
}

bool ProcessorGraphicsItem::contains(RootGraphicsItem* rootItem) const {
    return rootItem == this;
}

bool ProcessorGraphicsItem::contains(Processor* processor) const {
    return (processor == processor_);
}

Processor* ProcessorGraphicsItem::getProcessor() const {
    return processor_;
}

QList<Processor*> ProcessorGraphicsItem::getProcessors() const {
    QList<Processor*> result;
    result.append(processor_);
    return result;
}

bool ProcessorGraphicsItem::hasProperty(const Property* prop) const {
    const std::vector<Property*> properties = processor_->getProperties();
    for (size_t i = 0; i < properties.size(); ++i)
        if (properties[i] == prop)
            return true;

    return false;
}

void ProcessorGraphicsItem::renameFinished(bool) {
    textItem_.setTextInteractionFlags(Qt::NoTextInteraction);
    textItem_.setFlag(QGraphicsItem::ItemIsFocusable, false);
    textItem_.setFlag(QGraphicsItem::ItemIsSelectable, false);

    // name unchanged
    if (textItem_.toPlainText().toStdString() == processor_->getName())
        return;

    try {
        ProcessorNetwork* network = networkEditor_->getProcessorNetwork();
        network->setProcessorName(processor_, network->generateUniqueProcessorName(textItem_.toPlainText().toStdString()));
    }
    catch (VoreenException& e) {
        LWARNINGC("voreen.qt.ProcessorGraphicsItem", e.what());
        textItem_.setPlainText(QString::fromStdString(processor_->getName()));
    }

    nameChanged();
}

void ProcessorGraphicsItem::saveMeta() {
    PositionMetaData* meta = new PositionMetaData(static_cast<int>(x()), static_cast<int>(y()));

    processor_->getMetaDataContainer().addMetaData("ProcessorGraphicsItem", meta);
}

void ProcessorGraphicsItem::loadMeta() {
    if (processor_->getMetaDataContainer().hasMetaData("ProcessorGraphicsItem")) {
        PositionMetaData* meta = dynamic_cast<PositionMetaData*>(processor_->getMetaDataContainer().getMetaData("ProcessorGraphicsItem"));
        setPos(meta->getX(), meta->getY());
    }
}

void ProcessorGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    if (currentLayer() == NetworkEditorLayerDataflow) {
        // toggle visibility of processor widget
        if (processor_->getProcessorWidget())
            toggleProcessorWidget();
    }
    else {
        togglePropertyList();
    }
    event->accept();
}

void ProcessorGraphicsItem::processorWidgetCreated(const Processor* processor) {
    widgetIndicatorButton_.show();
    widgetIndicatorButton_.setProcessorWidget(processor->getProcessorWidget());
}

void ProcessorGraphicsItem::processorWidgetDeleted(const Processor*) {
    widgetIndicatorButton_.hide();
    widgetIndicatorButton_.setProcessorWidget(0);
}

void ProcessorGraphicsItem::propertiesChanged(const PropertyOwner*) {
    RootGraphicsItem::propertiesChanged();
}

void ProcessorGraphicsItem::portsChanged(const Processor*) {
    std::vector<std::pair<std::string,PortGraphicsItem*> > connections;
    foreach (PortGraphicsItem* ownPort, portGraphicsItems_) {
        const QList<PortGraphicsItem*> connectedPorts = ownPort->getConnectedPorts();
        foreach (PortGraphicsItem* externalPort, connectedPorts) {
            connections.push_back(std::pair<std::string,PortGraphicsItem*>(ownPort->getPort()->getName(), externalPort));
            if (ownPort->getPort()->isOutport())
                disconnect(ownPort, externalPort);
            else
                disconnect(externalPort, ownPort);
        }
    }

    deleteChildItems();
    initializePorts();
    RootGraphicsItem::portsChanged();
    for (unsigned int i=0; i<connections.size(); i++) {
        foreach (PortGraphicsItem* p, portGraphicsItems_) {
            if (p->getPort()->getName() == connections[i].first) {
                if (p->getPort()->isOutport()) {
                    if (connect(p, connections[i].second, true))
                        connect(p, connections[i].second);
                } else {
                    if (connect(connections[i].second, p, true))
                        connect(connections[i].second, p);
                }
            }
        }
    }
}

void ProcessorGraphicsItem::toggleProcessorWidget() {
    processor_->getProcessorWidget()->setVisible(!processor_->getProcessorWidget()->isVisible());
    widgetIndicatorButton_.update();
}

QList<QAction*> ProcessorGraphicsItem::getProcessorWidgetContextMenuActions() {
    if (processor_->getProcessorWidget()) {
        QAction* action = new QAction(tr("Processor Widget"), this);
        action->setCheckable(true);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(toggleProcessorWidget()));
        if (processor_->getProcessorWidget()->isVisible())
            action->setChecked(true);
        QList<QAction*> result;
        result.append(action);
        return result;
    }
    else
        return RootGraphicsItem::getProcessorWidgetContextMenuActions();
}

} // namespace voreen
