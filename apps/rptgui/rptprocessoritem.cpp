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

#include "voreen/core/vis/processors/processor.h"
#include "rptprocessoritem.h"
#include "rptaggregationitem.h"
#include "rptpropertysetitem.h"
#include "voreen/core/vis/processors/processorfactory.h"
#include "rptarrow.h"
#include <iostream>
#include <typeinfo>

namespace voreen {

RptProcessorItem::RptProcessorItem(Identifier type, QGraphicsItem* parent)
    : RptGuiItem(type.getName(),parent)
    , type_(type), aggregateAction_(0)
{
    processor_ = ProcessorFactory::getInstance()->create(type_);
    if ( !processor_)
        return;
    //textItem_->setPlainText(QString(type.getName().c_str()));
    setColor();
    createIO();
    createContextMenu();
    setAcceptDrops(true);
}

RptProcessorItem::RptProcessorItem(Processor* processor, QGraphicsItem* parent)
    : RptGuiItem(processor->getName(),parent)
    , type_(processor->getClassName().getSubString(1))
    , aggregateAction_(0)
{
    processor_ = processor;
    if ( !processor_)
        return;
    setColor();
    createIO();
    createContextMenu();
    setAcceptDrops(true);
}

RptProcessorItem::~RptProcessorItem() {
    disconnectAll();
    delete processor_;
    processor_ = 0;
    QVector<int> dummy;
    emit sendProcessor(0, dummy);
}

bool RptProcessorItem::enableAggregateContextMenuEntry(const bool newState)
{
    const bool prevState = aggregateAction_->isEnabled();
    aggregateAction_->setEnabled(newState);
    return prevState;
}

void RptProcessorItem::setName(std::string name) {
    //getProcessor()->setName(name);
    RptGuiItem::setName(name);
}

void RptProcessorItem::setColor() {
        color_ = QColor(233,218,176);
}

void RptProcessorItem::createContextMenu() {
    RptGuiItem::createContextMenu();

    // createActions
    aggregateAction_ = new QAction(tr("Aggregate"), this);

    // add actions to context menu
    contextMenu_.addAction(aggregateAction_);

    // connect actions
    QObject::connect(aggregateAction_, SIGNAL(triggered()), this, SLOT(aggregateActionSlot()));
}

void RptProcessorItem::aggregateActionSlot() {
    emit aggregateSignal();
}

void RptProcessorItem::createIO() {
	/*qreal w = boundingRect().width();
    qreal h = boundingRect().height();*/
    
	std::vector<Port*> inports = processor_->getInports();
    std::vector<Port*> outports = processor_->getOutports();

	for (size_t i=0;i<inports.size();i++) {
		inports_.push_back(new RptPortItem(inports.at(i)->getType(), inports.at(i), this));
	}
	for (size_t i=0;i<outports.size();i++) {
		outports_.push_back(new RptPortItem(outports.at(i)->getType(), outports.at(i), this));
	}

    //for (size_t i=0; i<inports_.size(); i++) {
    //    inports_.at(i)->moveBy((i+1) * w/(inports_.size()+1) - inports_.at(i)->boundingRect().width()/2 - 15, 0);
    //    //inports_.at(i)->setParentItem(this);
    //}
    //for (size_t i=0; i<outports_.size(); i++) {
    //    outports_.at(i)->moveBy((i+1) * w/(outports_.size()+1) - outports_.at(i)->boundingRect().width()/2 - 15, h - outports_.at(i)->boundingRect().height());
    //    //outports_.at(i)->setParentItem(this);
    //}

    // coProcessorPorts
    inports = processor_->getCoProcessorInports();
    outports = processor_->getCoProcessorOutports();

    for (size_t i=0;i<inports.size();i++) {
		coProcessorInports_.push_back(new RptPortItem(inports.at(i)->getType(), inports.at(i), this));
	}
	for (size_t i=0;i<outports.size();i++) {
		coProcessorOutports_.push_back(new RptPortItem(outports.at(i)->getType(), outports.at(i), this));
	}

    //for (size_t i=0; i<coProcessorInports_.size(); i++) {
    //    coProcessorInports_.at(i)->moveBy(0-15, (i+1) * h/(coProcessorInports_.size()+1) - coProcessorInports_.at(i)->boundingRect().height()/2);
    //    //inports_.at(i)->setParentItem(this);
    //}
    //for (size_t i=0; i<coProcessorOutports_.size(); i++) {
    //    coProcessorOutports_.at(i)->moveBy(w - coProcessorOutports_.at(i)->boundingRect().width() - 15, (i+1) * h/(coProcessorOutports_.size()+1) - coProcessorOutports_.at(i)->boundingRect().height()/2);
    //    //outports_.at(i)->setParentItem(this);
    //}

    repositionPorts();

}

QVector<int> RptProcessorItem::getUnequalEntries() {
    QVector<int> unequalEntries;

    // TODO: fix RptPropertySetItem->getUnequalEntries first
    /*for (size_t i=0; i<propertySets_.size(); i++) {
        unequalEntries += propertySets_[i]->getUnequalEntries(this);
    }*/

    return unequalEntries;
}

QVariant RptProcessorItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange) {
        if (parentItem() && parentItem()->type() == RptAggregationItem::Type) {
            static_cast<RptAggregationItem*>(parentItem())->updateGeometry();
        }
    }

    return RptGuiItem::itemChange(change, value);
}

QRectF RptProcessorItem::drawingRect() const {
    QRectF rect = textItem_->boundingRect();
    QRectF newRect(
        rect.left()-15,
        rect.top(),
        (rect.width()>59) ? rect.width()+30 : 90,
        rect.height()+40
    );
    return newRect;
}

QRectF RptProcessorItem::boundingRect() const {
    QRectF rect = drawingRect();
    QRectF newRect(
        rect.left()-3,
        rect.top()-3,
        rect.width() + 6,
        rect.height() + 6
    );
    return newRect;
}

QPainterPath RptProcessorItem::coprocessorBoundingPath(QRectF rect) const {
    // taken from http://www.bessrc.aps.anl.gov/software/qt4-x11-4.2.2/widgets-styles.html
    int radius = qMin(rect.width(), rect.height()) / 2;
    int diam = 2 * radius;

    qreal x1, y1, x2, y2;
    rect.getCoords(&x1, &y1, &x2, &y2);

    QPainterPath path;
    path.moveTo(x2, y1 + radius);
    path.arcTo(QRect(x2 - diam, y1, diam, diam), 0.0, +90.0);
    path.lineTo(x1 + radius, y1);
    path.arcTo(QRect(x1, y1, diam, diam), 90.0, +90.0);
    path.lineTo(x1, y2 - radius);
    path.arcTo(QRect(x1, y2 - diam, diam, diam), 180.0, +90.0);
    path.lineTo(x1 + radius, y2);
    path.arcTo(QRect(x2 - diam, y2 - diam, diam, diam), 270.0, +90.0);
    path.closeSubpath();
    return path;
}

QPainterPath RptProcessorItem::canvasBoundingPath(QRectF rect) const {
    QPainterPath path;
    path.addEllipse(rect);
    return path;
}

void RptProcessorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {

	painter->setPen(Qt::NoPen);
	// draw shadow
	painter->setBrush(Qt::darkGray);
    QRectF shadowRect(drawingRect().left()+3,drawingRect().top()+3,drawingRect().width(), drawingRect().height());
	if (processor_->getIsCoprocessor())
		painter->drawPath(coprocessorBoundingPath(shadowRect));
	else if (processor_->getClassName().getSubString(1) == "Canvas")
        painter->drawPath(canvasBoundingPath(shadowRect));
	else
		painter->drawRoundRect(shadowRect);

    // draw linear gradient for top highlight and bottom darkening
	QLinearGradient linGradient(0, 0, 0, drawingRect().height());
	linGradient.setColorAt(0.0, color_.light(220));
	linGradient.setColorAt(0.15, color_);
	linGradient.setColorAt(0.9, color_.dark(200));
	linGradient.setColorAt(1, Qt::black);
    painter->setBrush(linGradient);
	if (processor_->getIsCoprocessor())
		painter->drawPath(coprocessorBoundingPath(drawingRect()));
	else if (processor_->getClassName().getSubString(1) == "Canvas")
        painter->drawPath(canvasBoundingPath(drawingRect()));
	else
        painter->drawRoundRect(drawingRect());

	painter->setBrush(Qt::NoBrush);
	// hover effect
	if (option->state & QStyle::State_MouseOver)
		painter->setBrush(QColor(255, 255, 255, 70));
	// frame indicates selected process
	if (option->state & QStyle::State_Selected)
		painter->setPen(QPen(Qt::green, 3));
	// draw frame / hover effect
	if (processor_->getIsCoprocessor())
		painter->drawPath(coprocessorBoundingPath(drawingRect()));
	else if (processor_->getClassName().getSubString(1) == "Canvas")
        painter->drawPath(canvasBoundingPath(drawingRect()));
	else
		painter->drawRoundRect(drawingRect());

    //DEBUG
    /*painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());*/
}

void RptProcessorItem::dropEvent(QDropEvent *event)
{
    // Determine, whether the target was a VolumeSetSourceProcessor
    // and the mime data matches the requirements.
    //
    VolumeSetSourceProcessor* vssp = dynamic_cast<VolumeSetSourceProcessor*>(processor_);
    if( (vssp != 0) )
    {
        // Set the dropped data (a pointer to a VolumeSet object) for the processor.
        // The data have been converted from VolumeSet* to qulonglong in order to
        // be passed by a QByteArray. I know it's ugly and dangerous but I see no easier
        // possibilities.... (dirk)
        //
        QByteArray itemData = event->mimeData()->data("application/x-voreenvolumesetpointer");
        qulonglong data = itemData.toULongLong();
        VolumeSet* volumeset = reinterpret_cast<VolumeSet*>(data);
        vssp->setVolumeSet(volumeset);
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

RptProcessorItem& RptProcessorItem::saveMeta() {
    //processor_->clearMeta();
    TiXmlElement* meta = new TiXmlElement("RptProcessorItem");
    meta->SetAttribute("x", x());
    meta->SetAttribute("y", y());
    meta->SetAttribute("name", getName());
    processor_->addToMeta(meta);
    return *this;
}

RptProcessorItem& RptProcessorItem::loadMeta() {
    TiXmlElement* meta = processor_->getFromMeta("RptProcessorItem");
    float x,y;
    if (meta->QueryFloatAttribute("x",&x) != TIXML_SUCCESS || meta->QueryFloatAttribute("y",&y) != TIXML_SUCCESS)
        throw XmlAttributeException("The Position of a ProcessorItem remains unknown!");
    setPos(x,y);
    if (meta->Attribute("name"))
        setName(meta->Attribute("name"));
    return *this;
}

//void RptProcessorItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
//    contextMenu_.exec(event->screenPos());
//}

} //namespace voreen
