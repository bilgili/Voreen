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
    : RptGuiItem(type.getName(), parent)
    , type_(type)
    , aggregateAction_(0)
{
    processor_ = ProcessorFactory::getInstance()->create(type_);

    setColor();
    createIO();
    setAcceptDrops(true);
}

RptProcessorItem::RptProcessorItem(Processor* processor, QGraphicsItem* parent)
    : RptGuiItem(processor->getClassName().getSubString(1), parent)
    , type_(processor->getClassName().getSubString(1))
    , aggregateAction_(0)
{
    processor_ = processor;
    setColor();
    createIO();
    setAcceptDrops(true);
}

RptProcessorItem::~RptProcessorItem() {
    //   disconnectAll();
    //FIXME: is this needed here?
    // This leads to crashed if some of the refered objects (ports, other procitems) were
    // already deleted before. joerg
    
    delete processor_;
}

bool RptProcessorItem::enableAggregateContextMenuEntry(const bool newState) {
    bool prevState = aggregateAction_->isEnabled();
    aggregateAction_->setEnabled(newState);
    return prevState;
}

void RptProcessorItem::setName(const std::string& name) {
    processor_->setName(name);
    textItem_->setPlainText(name.c_str());
    RptGuiItem::nameChanged();
}

void RptProcessorItem::nameChanged() {
    if (processor_->getName() != textItem_->toPlainText().toStdString()) {
        processor_->setName(textItem_->toPlainText().toStdString());
        RptGuiItem::nameChanged();

        emit processorNameChanged();
    }
}

void RptProcessorItem::setColor() {
    color_ = QColor(233, 218, 176);
}

void RptProcessorItem::aggregateActionSlot() {
    emit aggregateSignal();
}

void RptProcessorItem::createIO() {

    std::vector<Port*> inports = processor_->getInports();
    std::vector<Port*> outports = processor_->getOutports();

    for (size_t i=0; i<inports.size(); i++)
        inports_.push_back(new RptPortItem(inports.at(i)->getType(), inports.at(i), this));

    for (size_t i=0; i< outports.size(); i++)
        outports_.push_back(new RptPortItem(outports.at(i)->getType(), outports.at(i), this));

    // coProcessorPorts
    inports = processor_->getCoProcessorInports();
    outports = processor_->getCoProcessorOutports();

    for (size_t i=0; i < inports.size(); i++)
        coProcessorInports_.push_back(new RptPortItem(inports.at(i)->getType(), inports.at(i), this));

    for (size_t i=0; i < outports.size(); i++)
        coProcessorOutports_.push_back(new RptPortItem(outports.at(i)->getType(), outports.at(i), this));

    repositionPorts();
}

QVariant RptProcessorItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange) {
        if (parentItem() && parentItem()->type() == RptAggregationItem::Type)
            static_cast<RptAggregationItem*>(parentItem())->updateGeometry();
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

QPainterPath RptProcessorItem::canvasBoundingPath(QRectF rect) const {
    QPainterPath path;
    path.addEllipse(rect);
    return path;
}

void RptProcessorItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* ) {
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

void RptProcessorItem::dropEvent(QDropEvent* event) {
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

RptProcessorItem& RptProcessorItem::saveMeta() {
    TiXmlElement* meta = new TiXmlElement("RptProcessorItem");
    meta->SetAttribute("x", static_cast<int>(x()));
    meta->SetAttribute("y", static_cast<int>(y()));
    processor_->addToMeta(meta);
    return *this;
}

RptProcessorItem& RptProcessorItem::loadMeta() {
    TiXmlElement* meta = processor_->getFromMeta("RptProcessorItem");
    float x, y;
    if (meta->QueryFloatAttribute("x", &x) != TIXML_SUCCESS || meta->QueryFloatAttribute("y", &y) != TIXML_SUCCESS)
        throw XmlAttributeException("The Position of a ProcessorItem remains unknown!");
    setPos(x,y);

    setName(processor_->getName());

    return *this;
}

} // namespace voreen
