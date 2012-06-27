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

#include "rptpropertysetitem.h"
#include "rptaggregationitem.h"

namespace voreen {

RptPropertySetItem::RptPropertySetItem(QGraphicsScene* scene, QGraphicsItem* parent)
    : RptGuiItem("PropertySet", parent)
{
    propertySet_ = new PropertySet();
    port_ = new RptPropertyPort(this);
    port_->moveBy(boundingRect().width()/2 -20, boundingRect().height() - 15);
    if (scene)
        scene->addItem(this);
    createContextMenu();
	color_ = QColor(233,218,176);
}

RptPropertySetItem::RptPropertySetItem(std::vector<RptGuiItem*> processors, QGraphicsScene* scene, QGraphicsItem* parent)
    : RptGuiItem("PropertySet", parent)
{
    propertySet_ = new PropertySet();
    port_ = new RptPropertyPort(this);
    port_->moveBy(boundingRect().width()/2 -20, boundingRect().height() - 15);
    if (scene)
        scene->addItem(this);

    QPointF pos;
    for (size_t i=0; i<processors.size(); i++) {
        connectGuiItem(processors[i]);
        pos += processors[i]->scenePos();
    }
    pos /= processors.size();
    pos.setX(pos.x() + 200);
    
    setPos(pos);

    adjustArrows();
    createContextMenu();
	color_ = QColor(233,218,176);
}

RptPropertySetItem::RptPropertySetItem(PropertySet* propertySet, const std::map<Processor*,RptProcessorItem*> processorMap, QGraphicsScene* scene, QGraphicsItem* parent)
    : RptGuiItem("PropertySet", parent)
{
    propertySet_ = propertySet;
    //setName(propertySet->getName());
    port_ = new RptPropertyPort(this);
    port_->moveBy(boundingRect().width()/2 -20, boundingRect().height() - 15);
    if (scene)
        scene->addItem(this);

    // Connect to Processors of the PropertySet
    std::vector<Processor*> processors = propertySet_->getProcessors();
    for (size_t i=0; i<processors.size(); i++) {
        // removing Processor because it is added again by connectGuiItem FIXME: This is a hack - connectGuiItem should get an overhaul
        propertySet_->removeProcessor(processors[i]);
        connectGuiItem(processorMap.find(processors[i])->second);
    }
    loadMeta();
    adjustArrows();
    createContextMenu();
    color_ = QColor(233,218,176);
}


RptPropertySetItem::~RptPropertySetItem() {
    disconnectAll();
    delete port_;
    port_ = 0;
    delete propertySet_;
    propertySet_ = 0;
    QVector<int> dummy;
    emit sendProcessor(0, dummy);
}

void RptPropertySetItem::setName(std::string name) {
    //propertySet_->setName(name);
    RptGuiItem::setName(name);
}

RptPropertySetItem& RptPropertySetItem::saveMeta() {
    //propertySet_->clearMeta();
    TiXmlElement* meta = new TiXmlElement("RptPropertySetItem");
    meta->SetAttribute("x", static_cast<int>(x()));
    meta->SetAttribute("y", static_cast<int>(y()));
    meta->SetAttribute("name", getName());
    propertySet_->addToMeta(meta);
    return *this;
}

RptPropertySetItem& RptPropertySetItem::loadMeta() {
    TiXmlElement* meta = propertySet_->getFromMeta("RptPropertySetItem");
    float x,y;
    if (meta->QueryFloatAttribute("x",&x) != TIXML_SUCCESS || meta->QueryFloatAttribute("y",&y) != TIXML_SUCCESS)
        throw XmlAttributeException("The Position of a PropertySetItem remains unknown!");
    if (meta->Attribute("name"))
        setName(meta->Attribute("name"));
    setPos(x,y);
    return *this;
}

void RptPropertySetItem::createContextMenu() {
    RptGuiItem::createContextMenu();

    // createActions
    QAction* equalize = new QAction(tr("Equalize"), this);

    // add actions to context menu
    contextMenu_.addAction(equalize);

    // connect actions
    QObject::connect(equalize, SIGNAL(triggered()), this, SLOT(equalizeSlot()));
}

void RptPropertySetItem::equalizeSlot() {
    propertySet_->setProperties("all");
}

void RptPropertySetItem::updateToolTip() {
    std::string s;
    for (size_t i=0; i<propertySet_->getProcessors().size(); i++) {
        if (i!=0) s.append("\n");
        s.append(propertySet_->getProcessors()[i]->getClassName().getName());
    }
    setToolTip(QString(s.c_str()));
}

bool RptPropertySetItem::connectGuiItem(RptGuiItem* item) {   
    if (item->type() == RptProcessorItem::Type) {
        propertySet_->addProcessor(static_cast<RptProcessorItem*>(item)->getProcessor());
        guiItems_.push_back(item);
        arrows_.push_back(new RptArrow(port_, item));
        item->addPropertySet(this);
        if (scene())
            scene()->addItem(arrows_.back());
        //arrows_.back()->adjust();
        adjustArrows();

        updateToolTip();
        return true;
    }
    else if (item->type() == RptAggregationItem::Type) {
        std::vector<RptProcessorItem*> items = static_cast<RptAggregationItem*>(item)->getProcessorItems();
        for (size_t i=0; i<items.size(); i++) {
            propertySet_->addProcessor(items[i]->getProcessor());
        }
        guiItems_.push_back(item);
        arrows_.push_back(new RptArrow(port_, item));
        item->addPropertySet(this);
        if (scene())
            scene()->addItem(arrows_.back());
        //arrows_.back()->adjust();
        adjustArrows();

        updateToolTip();
        return true;
    }
    return false;
}

void RptPropertySetItem::aggregate(RptAggregationItem* aggregation) {
    // find items that belong to this property set as well as to the aggregation
    // then remove these items from guiItems_ and add the aggregation instead
    std::vector<RptProcessorItem*> items = aggregation->getProcessorItems();
    for (size_t i=0; i<guiItems_.size(); i++) {
        for (size_t j=0; j<items.size(); j++) {
            if (guiItems_[i] == items[j]) {
                guiItems_.erase(guiItems_.begin() + i);
                items[j]->removePropertySet(this);
                delete arrows_[i];
                arrows_.erase(arrows_.begin() + i--);

                break;
            }
        }
    }

    guiItems_.push_back(aggregation);
    //aggregation_->addPropertySet(this);
    arrows_.push_back(new RptArrow(this, aggregation));
    scene()->addItem(arrows_.back());
    //arrows_.back()->adjust();
    adjustArrows();
}

void RptPropertySetItem::deaggregate(RptAggregationItem* aggregation) {
    // find aggregation in guiItems_ and erase it
    for (size_t i=0; i<guiItems_.size(); i++) {
        if (guiItems_[i] == aggregation) {
            guiItems_.erase(guiItems_.begin() + i);
            delete arrows_[i];
            arrows_.erase(arrows_.begin() + i);
            break;
        }
    }

    std::vector<RptProcessorItem*> items = aggregation->getProcessorItems();
    for (size_t i=0; i<items.size(); i++) {
        for (size_t j=0; j<propertySet_->getProcessors().size(); j++) {
            if (items[i]->getProcessor() == propertySet_->getProcessors()[j]) {
                guiItems_.push_back(items[i]);
                items[i]->addPropertySet(this);
                //aggregation->removePropertySet(this);
                arrows_.push_back(new RptArrow(this, items[i]));
                scene()->addItem(arrows_.back());
                //arrows_.back()->adjust();
            }
        }
    }
    adjustArrows();

}

bool RptPropertySetItem::disconnectGuiItem(QGraphicsItem* item) {
    if (!item)
        return false;
    if (item->type() == RptProcessorItem::Type) {
        propertySet_->removeProcessor(static_cast<RptProcessorItem*>(item)->getProcessor());
        bool found = false;
        for (size_t i=0; i<guiItems_.size(); i++) {
            if (guiItems_[i] == item) {
                guiItems_.erase(guiItems_.begin() + i);
                found = true;
                break;
            }
        }
        // remove all arrows with item as DestNode
        if (found) {
            for (size_t i=0; i<arrows_.size(); i++) {
                if (arrows_[i]->getDestNode() == item) {
                    delete arrows_[i];
                    arrows_.erase(arrows_.begin() + i);
                    break;
                }
            }
        }

        static_cast<RptProcessorItem*>(item)->removePropertySet(this);

        // check if this processor was the last one of an aggregation connected to this propset
        if (item->parentItem() && item->parentItem()->type() == RptAggregationItem::Type) {
            RptAggregationItem* aggro = static_cast<RptAggregationItem*>(item->parentItem());
            bool found = false;
            for (size_t j=0; j<aggro->getProcessorItems().size(); j++) {
                RptProcessorItem* item = aggro->getProcessorItems()[j];
                for (size_t k=0; k<item->getPropertySets().size(); k++) {
                    if (item->getPropertySets()[k] == this) {
                        found = true;
                    }
                }
            }
            if (!found) {
                aggro->removePropertySet(this);
            }
        }

        updateToolTip();
        return true;
    }
    else if (item->type() == RptAggregationItem::Type) {
        std::vector<RptProcessorItem*> items = static_cast<RptAggregationItem*>(item)->getProcessorItems();
        for (size_t i=0; i<items.size(); i++) {
            propertySet_->removeProcessor(items[i]->getProcessor());
        }
        bool found = false;
        for (size_t i=0; i<guiItems_.size(); i++) {
            if (guiItems_[i] == item) {
                guiItems_.erase(guiItems_.begin() + i);
                found = true;
                break;
            }
        }
        if (found) {
            for (size_t i=0; i<arrows_.size(); i++) {
                if (arrows_[i]->getDestNode() == item) {
                    delete arrows_[i];
                    arrows_.erase(arrows_.begin() + i);
                    break;
                }
            }
        }

        static_cast<RptAggregationItem*>(item)->removePropertySet(this);

        updateToolTip();
        return true;
    }
    return false;
}

void RptPropertySetItem::disconnectAll() {
    while (guiItems_.size() > 0) {
        if (guiItems_.back()) {
            disconnectGuiItem(guiItems_.back());
        }
        else {
            // nullpointer? how did it get there? Get rid of it!
            guiItems_.pop_back();
        }
    }
}

void RptPropertySetItem::showAllArrows() {
    if (scene()) {
        for (size_t i=0; i<arrows_.size(); i++) {
            scene()->addItem(arrows_.at(i));
        }
    }
    adjustArrows();
}

void RptPropertySetItem::adjustArrows() {
    for (size_t i=0; i<arrows_.size(); i++) {
        arrows_[i]->adjust();
    }
}

QVector<int> RptPropertySetItem::getUnequalEntries(RptProcessorItem* item) {
    QVector<int> unequalEntries;

    for (size_t i=0; i<propertySet_->getProperties().size(); i++) {
        for (size_t j=0; j<item->getProcessor()->getProperties().size(); j++) {
            if (propertySet_->getProperties()[i]->getIdent() == item->getProcessor()->getProperties()[j]->getIdent()) {
                // TODO: compare properties WITHOUT huge and ugly switch-case block
                /* if (*(propertySet_->getProperties()[i]) != *(item->getProcessor()->getProperties()[j]) ) {
                    unequalEntries.push_back(j);
                }*/
            }
        }
    }

    return unequalEntries;
}

QVariant RptPropertySetItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange) {
        adjustArrows();
    }
    return RptGuiItem::itemChange(change, value);
}


QRectF RptPropertySetItem::boundingRect() const {
    QRectF rect = textItem_->boundingRect();
    QRectF newRect(
        rect.left()-15,
        rect.top(),
        (rect.width()>59) ? rect.width()+30 : 90,
        rect.height()+40
    );
    return newRect;
}

QPainterPath RptPropertySetItem::propertySetItemPath(QRectF rect) const {
    int offset = static_cast<int>(qMin(rect.width(), rect.height()) / 4.f);

    qreal x1, y1, x2, y2;
    rect.getCoords(&x1, &y1, &x2, &y2);

    // add space for shadows and framing
    x1 += 1;
    y1 += 1;
    x2 -= 3;
    y2 -= 3;

    QPainterPath path;
    path.moveTo(x1 + offset, y1);
    path.lineTo(x1, (y1 + y2)/2);
    path.lineTo(x1 + offset, y2);
    path.lineTo(x2 - offset, y2);
    path.lineTo(x2, (y1 + y2)/2);
    path.lineTo(x2 - offset, y1);
    path.lineTo(x1 + offset, y1);
    path.closeSubpath();
    return path;
}

void RptPropertySetItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::darkGray);
    QRectF shadowRect(boundingRect().left()+3,boundingRect().top()+3,boundingRect().width(), boundingRect().height());
    painter->drawPath(propertySetItemPath(shadowRect));
    
	// draw linear gradient for top highlight and bottom darkening
    QLinearGradient linGradient(0, 0, 0, boundingRect().height());
    linGradient.setColorAt(0.0, color_.light(220));
    linGradient.setColorAt(0.1, color_);
    linGradient.setColorAt(0.9, color_.dark(210));
    linGradient.setColorAt(1, Qt::black);
    painter->setBrush(linGradient);

	painter->drawPath(propertySetItemPath(boundingRect()));

    /*if (option->state & QStyle::State_Sunken) {
    newCursor.setShape(Qt::CursorShape::ClosedHandCursor);
    setCursor(newCursor);
    }*/

    if (option->state & QStyle::State_MouseOver)
        painter->setBrush(QColor(255, 255, 255, 70));
    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(Qt::green, 3));
    }

    painter->drawPath(propertySetItemPath(boundingRect()));
    //DEBUG
    /*painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());*/
}

//---------------------------------------------------------------------------

RptPropertyPort::RptPropertyPort(QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);
}

QRectF RptPropertyPort::boundingRect() const {
    QRectF rect(0,0,10,10);
    return rect;
}

void RptPropertyPort::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {    
    painter->setBrush(Qt::gray);
    painter->setPen(QPen(Qt::black, 0));

    if (option->state & QStyle::State_Sunken) {
        painter->setBrush(Qt::darkGray);
    }

    if (option->state & QStyle::State_MouseOver) {
        painter->setPen(QPen(Qt::red, 0));
    }
    painter->drawRect(boundingRect());

}

void RptPropertyPort::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    line_ = new QGraphicsLineItem();
    scene()->addItem(line_);
    line_->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QGraphicsItem::mousePressEvent(event);
}

void RptPropertyPort::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    line_->setLine(QLineF(scenePos(),event->scenePos()));
    QGraphicsItem::mouseMoveEvent(event);
}

void RptPropertyPort::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (scene()->itemAt(event->scenePos())) {

        if (scene()->itemAt(event->scenePos())->type() == RptProcessorItem::Type
                || scene()->itemAt(event->scenePos())->type() == RptAggregationItem::Type)
        {
            RptGuiItem* item = static_cast<RptGuiItem*>(scene()->itemAt(event->scenePos()));
            static_cast<RptPropertySetItem*>(parentItem())->connectGuiItem(item);
        }

        else if ( scene()->itemAt(event->scenePos())->type() == RptTextItem::Type
                && (scene()->itemAt(event->scenePos())->parentItem()->type() == RptProcessorItem::Type
                    || scene()->itemAt(event->scenePos())->parentItem()->type() == RptAggregationItem::Type) )
        {
            RptGuiItem* item = static_cast<RptGuiItem*>(scene()->itemAt(event->scenePos())->parentItem());
            static_cast<RptPropertySetItem*>(parentItem())->connectGuiItem(item);
        }
        
    }

    delete line_;
    QGraphicsItem::mouseReleaseEvent(event);
}


} //namespace voreen
