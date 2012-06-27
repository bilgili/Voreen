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

#include "processorgraphicsitem.h"

#include "voreen/core/processors/processor.h"
#include "voreen/modules/base/processors/utility/scale.h"
#include "voreen/core/properties/property.h"
#include "voreen/core/processors/processorfactory.h"
#include "voreen/core/processors/processorwidget.h"
#include "openpropertylistbutton.h"
#include "portarrowgraphicsitem.h"
#include "propertygraphicsitem.h"
#include "networkeditor.h"
#include "portgraphicsitem.h"
#include "textgraphicsitem.h"

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

    inports_ = stdVectorToQList<Port*>(processor_->getInports());
    outports_ = stdVectorToQList<Port*>(processor_->getOutports());
    coInports_ = stdVectorToQList<CoProcessorPort*>(processor_->getCoProcessorInports());
    coOutports_ = stdVectorToQList<CoProcessorPort*>(processor_->getCoProcessorOutports());

    setName(QString::fromStdString(processor_->getName()));

    if (processor_->isUtility())
        setOpacity(utilityOpacityValue);

    processor_->setIOProgress(this);

    createChildItems();

    // start timer for progress bar
    time_ = new QTime();
    time_->start();
    maxProgress_ = 0;
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
            processor_->getProcessorWidget()->setVisible(!processor_->getProcessorWidget()->isVisible());
    }
    else {
        togglePropertyList();
    }
    event->accept();
}

void ProcessorGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* ) {
    RootGraphicsItem::paint(painter, option, 0);
    //std::cout << "paint: " << progress_ << "/" << maxProgress_ << std::endl;
    if (maxProgress_ != 0) {
        // TODO: draw progress bar here
        //std::cout << processor_->getName() << "p" << std::endl;
    }
}

void ProcessorGraphicsItem::update() {
    // time to wait between progress bar updates
    const int MINIMAL_UPDATE_WAIT = 50;

    if (time_->elapsed() > MINIMAL_UPDATE_WAIT || progress_ == maxProgress_) {
        time_->restart();
        std::cout << "updating: ";
        // FIXME: somehow the next two lines do not call the paint method
        RootGraphicsItem::update(QRectF());
        if (scene()) scene()->invalidate();
   }
}

void ProcessorGraphicsItem::setTotalSteps(int numSteps) {
    //std::cout << "setTotalSteps" << std::endl;
    maxProgress_ = numSteps - 1;
    time_->restart();
}

void ProcessorGraphicsItem::show() {
    //std::cout << "show" << std::endl;
    setProgress(0);
    if (scene()) scene()->invalidate();
}

void ProcessorGraphicsItem::hide() {
    //std::cout << "hide" << std::endl;
    setProgress(maxProgress_);
    if (scene()) scene()->invalidate();
}

void ProcessorGraphicsItem::forceUpdate() {
    if (scene()) scene()->invalidate();
}

} // namespace voreen
