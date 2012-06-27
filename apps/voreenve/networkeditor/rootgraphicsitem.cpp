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

#include "rootgraphicsitem.h"

#include "voreen/modules/base/processors/utility/scale.h"
#include "voreen/core/ports/coprocessorport.h"
#include "voreen/core/ports/renderport.h"
#include "aggregationgraphicsitem.h"
#include "linkarrowgraphicsitemstub.h"
#include "portarrowgraphicsitem.h"
#include "portgraphicsitem.h"
#include "processorgraphicsitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

namespace {
    const qreal buttonsOffsetX = 6.0;
    const qreal buttonsOffsetY = 6.0;

    const qreal drawingRectMinimumWidth = 80.0;
    const qreal drawingRectMinimumHeight = 60.0;

    const QColor baseColor = QColor(50, 50, 50, 255);
    const QColor selectedColor = Qt::red;
    const QColor highlightColor = Qt::blue;
    const QColor shadowColor = Qt::black;
}

namespace voreen {

RootGraphicsItem::RootGraphicsItem(voreen::NetworkEditor* networkEditor)
#if (QT_VERSION >= 0x040600)
    : QGraphicsObject(0)
#else
    : QObject()
    , QGraphicsItem(0)
#endif
    , networkEditor_(networkEditor)
    , textItem_("", this)
    , propertyListItem_(this)
    , openPropertyListButton_(this)
    , widgetIndicatorButton_(this)
    , progressBar_(0)
    , currentLinkArrow_(0)
    , opacity_(1.0)
{
    tgtAssert(networkEditor, "passed null pointer");

    QObject::connect(&textItem_, SIGNAL(renameFinished()), this, SLOT(renameFinished()));
    QObject::connect(&textItem_, SIGNAL(textChanged()), this, SLOT(nameChanged()));
    QObject::connect(&openPropertyListButton_, SIGNAL(pressed()), this, SLOT(togglePropertyList()));
    QObject::connect(&widgetIndicatorButton_, SIGNAL(pressed()), this, SLOT(toggleProcessorWidget()));

    propertyListItem_.hide();
    widgetIndicatorButton_.hide();

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);
    setZValue(ZValueRootGraphicsItemNormal);
}

RootGraphicsItem::~RootGraphicsItem() {
    foreach (PortGraphicsItem* p, portGraphicsItems_)
        delete p;
}

void RootGraphicsItem::createChildItems() {
    QList<Port*> ports = getPorts();

    foreach (Port* p, ports) {
        PortGraphicsItem* portItem = new PortGraphicsItem(p, this);
        QObject::connect(portItem, SIGNAL(startedArrow()), this, SIGNAL(startedArrow()));
        QObject::connect(portItem, SIGNAL(endedArrow()), this, SIGNAL(endedArrow()));
        portGraphicsItems_.append(portItem);
    }

    propertyListItem_.hide();
    layoutChildItems();
}

void RootGraphicsItem::deleteChildItems() {
    foreach (PortGraphicsItem* p, portGraphicsItems_) {
        portGraphicsItems_.pop_front();
        delete p;
    }
}

void RootGraphicsItem::layoutChildItems() {
    // ports should be distributed evenly across the side of the item

    // regular inports
    int numberOfPorts = getNumberOfVisiblePorts(getInports());
    for (int i = 0, j = 0; i < getInports().size(); ++i) {
        Port* inport = getInports()[i];
        PortGraphicsItem* inportItem = getPortGraphicsItem(inport);

        if (inportItem->isVisible()) {
            int x = (j + 1) * boundingRect().width() / (numberOfPorts + 1) - inportItem->boundingRect().width() / 2.0;
            int y = 0;

            inportItem->setPos(x, y);

            ++j;
        }
    }

    // coprocessor inports
    numberOfPorts = getNumberOfVisiblePorts(getCoProcessorInports());
    for (int i = 0, j = 0; i < getCoProcessorInports().size(); ++i) {
        CoProcessorPort* inport = getCoProcessorInports()[i];
        PortGraphicsItem* inportItem = getPortGraphicsItem(inport);

        if (inportItem->isVisible()) {
            ++j;

            int x = 0;
            int y = j * boundingRect().height() / (numberOfPorts + 1) - inportItem->boundingRect().height() / 2.0;

            inportItem->setPos(x, y);
        }
    }

    // regular outports
    numberOfPorts = getNumberOfVisiblePorts(getOutports());
    for (int i = 0, j = 0; i < getOutports().size(); ++i) {
        Port* outport = getOutports()[i];
        PortGraphicsItem* outportItem = getPortGraphicsItem(outport);

        if (outportItem->isVisible()) {
            ++j;

            int x = j * boundingRect().width() / (numberOfPorts + 1) - outportItem->boundingRect().width() / 2.0;
            int y = boundingRect().height() - (outportItem->boundingRect().height() / 2.f);

            outportItem->setPos(x, y);
        }
    }

    // coprocessor outports
    numberOfPorts = getNumberOfVisiblePorts(getCoProcessorOutports());
    for (int i = 0, j = 0; i < getCoProcessorOutports().size(); ++i) {
        Port* outport = getCoProcessorOutports()[i];
        PortGraphicsItem* outportItem = getPortGraphicsItem(outport);

        if (outportItem->isVisible()) {
            ++j;

            int x = boundingRect().width() - (outportItem->boundingRect().width() / 2.0);
            int y = j * boundingRect().height() / (numberOfPorts + 1) - outportItem->boundingRect().height() / 2.0;;

            outportItem->setPos(x, y);
        }
    }

    // the text label should be in the middle of the processoritem
    qreal x = (boundingRect().width() / 2.0) - (textItem_.boundingRect().width() / 2.0);
    qreal y = (boundingRect().height() / 2.0) - (textItem_.boundingRect().height() / 2.0);
    textItem_.setPos(x, y);

    // the propertylist item should be centered on the bottom of the proecssoritem
    x = (boundingRect().width() / 2.0) - (propertyListItem_.boundingRect().width() / 2.0);
    y = boundingRect().height();
    propertyListItem_.setPos(x, y);
    propertyListItem_.update();

    openPropertyListButton_.setPos(buttonsOffsetX, buttonsOffsetY);
    widgetIndicatorButton_.setPos(boundingRect().width() - widgetIndicatorButton_.boundingRect().width() - buttonsOffsetX, buttonsOffsetY);
}

// ------------------------------------------------------------------------------------------------
// getter/setter
// ------------------------------------------------------------------------------------------------

void RootGraphicsItem::setName(const QString& name) {
    textItem_.setPlainText(name);
}

QString RootGraphicsItem::getName() const {
    return textItem_.toPlainText();
}

void RootGraphicsItem::setOpacity(const qreal opacity) {
    opacity_ = opacity;
}

qreal RootGraphicsItem::getTextItemSpacing() const {
    return 20.0;
}

ProgressBarGraphicsItem* RootGraphicsItem::getProgressBar() const {
    return progressBar_;
}

// ------------------------------------------------------------------------------------------------
// Property
// ------------------------------------------------------------------------------------------------

void RootGraphicsItem::addPropertyGraphicsItems(const QList<PropertyGraphicsItem*>& items) {
    propertyListItem_.addPropertyGraphicsItems(items);
}

PropertyGraphicsItem* RootGraphicsItem::getPropertyGraphicsItem(const Property* prop) {
    return propertyListItem_.getPropertyGraphicsItem(prop);
}

void RootGraphicsItem::removePropertyGraphicsItem(PropertyGraphicsItem* item) {
    propertyListItem_.removePropertyGraphicsItem(item);
}

void RootGraphicsItem::togglePropertyList() {
    prepareGeometryChange();
    if (propertyListItem_.isVisible())
        propertyListItem_.setVisible(false);
    else
        propertyListItem_.setVisible(true);

    layoutChildItems();
    networkEditor_->adjustLinkArrowGraphicsItems();

    networkEditor_->resetCachedContent();
}

// ------------------------------------------------------------------------------------------------
// Aggregation
// ------------------------------------------------------------------------------------------------

void RootGraphicsItem::addAggregationPrefix(const QString& prefix) {
    const QString& name = getName();
    setName("[" + prefix + "] - " + name);
    emit renameFinished(false);
}

void RootGraphicsItem::removeAggregationPrefix() {
    QString name = getName();
    int endIndex = name.indexOf("] - ");
    if (endIndex == -1)
        return;
    else {
        int startIndex = name.indexOf("[");
        name.remove(startIndex, endIndex - startIndex + 4);   // 4 is the magic number for length("] - ")
        setName(name);
        emit renameFinished(false);
    }
}

void RootGraphicsItem::removeAllAggregationPrefixes() {
    int index = getName().indexOf("] - ");
    while (index != -1) {
        removeAggregationPrefix();
        index = getName().indexOf("] - ");
    }
}

void RootGraphicsItem::changeAggregationPrefix(const QString& prefix) {
    removeAggregationPrefix();
    addAggregationPrefix(prefix);
}

// ------------------------------------------------------------------------------------------------
// layer methods
// ------------------------------------------------------------------------------------------------

void RootGraphicsItem::setLayer(NetworkEditorLayer layer) {
    switch (layer) {
    case NetworkEditorLayerDataflow:
            propertyListItem_.setVisible(false);
            openPropertyListButton_.setVisible(false);
            setFlag(ItemIsMovable, true);

            foreach (PortGraphicsItem* portItem, getPortGraphicsItems()) {
                portItem->setLayer(layer);
                foreach (PortArrowGraphicsItem* arrow, portItem->getArrowList())
                    arrow->setLayer(layer);
            }
            break;
    case NetworkEditorLayerLinking:
            openPropertyListButton_.setVisible(true);
            setFlag(ItemIsMovable, false);

            foreach (PortGraphicsItem* portItem, getPortGraphicsItems()) {
                portItem->setLayer(layer);
                foreach (PortArrowGraphicsItem* arrow, portItem->getArrowList())
                    arrow->setLayer(layer);
            }
            break;
    default:
        tgtAssert(false, "shouldn't get here");
    }
}

NetworkEditorLayer RootGraphicsItem::currentLayer() const {
    return networkEditor_->currentLayer();
}

// ------------------------------------------------------------------------------------------------
// other methods
// ------------------------------------------------------------------------------------------------

void RootGraphicsItem::enterRenameMode() {
    textItem_.setTextInteractionFlags(Qt::TextEditorInteraction);
    textItem_.setFlag(QGraphicsItem::ItemIsFocusable, true);
    textItem_.setFocus();
}

QPointF RootGraphicsItem::linkArrowDockingPoint() const {
    return openPropertyListButton_.dockingPoint();
}

bool RootGraphicsItem::connect(PortGraphicsItem* outport, RootGraphicsItem* dest, bool testing) {
    tgtAssert(outport, "passed null pointer");
    tgtAssert(dest, "passed null pointer");

    foreach (PortGraphicsItem* p, dest->getPortGraphicsItems()) {
        if (connect(outport, p, true)) {
            if (testing)
                return true;
            else
                return connect(outport, p);
        }
    }
    return false;
}

bool RootGraphicsItem::connect(PortGraphicsItem* outport, PortGraphicsItem* inport, bool testing) {
    tgtAssert(outport, "passed null pointer");
    tgtAssert(inport, "passed null pointer");
    tgtAssert(networkEditor_->getProcessorNetwork(), "there was no processor network available");

    if (testing) {
        if (!outport->getPort() || !inport->getPort())
            return false;
        return outport->getPort()->testConnectivity(inport->getPort()) ;
    }

    if (networkEditor_->getProcessorNetwork()->connectPorts(outport->getPort(), inport->getPort())) {
        networkEditor_->processorConnectionsChanged();
        return true;
    }

#ifdef VRN_MODULE_BASE
    RenderPort* pout = dynamic_cast<RenderPort*>(outport->getPort());
    RenderPort* pin = dynamic_cast<RenderPort*>(inport->getPort());

    if (pout && pin && pout->doesSizeOriginConnectFailWithPort(pin)) {
        Processor* scale = new SingleScale;
        networkEditor_->getProcessorNetwork()->addProcessorInConnection(outport->getPort(), inport->getPort(), scale);

        ProcessorGraphicsItem* scaleItem = networkEditor_->getProcessorGraphicsItem(scale);
        QPointF p_out = outport->getParent()->pos();
        QPointF p_in =inport->getParent()->pos();
        qreal px = (p_out.x() + p_in.x()) / 2.f;
        qreal py = (p_out.y() + p_in.y()) / 2.f;
        scaleItem->setPos(px, py);
    }
#endif
    return false;
}

void RootGraphicsItem::disconnect(PortGraphicsItem* outport, PortGraphicsItem* inport) {
    tgtAssert(outport, "passed null pointer");
    tgtAssert(outport->getPort(), "no port present");
    tgtAssert(inport, "passed null pointer");
    tgtAssert(inport->getPort(), "no port present");

    networkEditor_->getProcessorNetwork()->disconnectPorts(outport->getPort(), inport->getPort());
    networkEditor_->processorConnectionsChanged();
}

QVariant RootGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemSelectedChange) {
        if (!value.toBool() && (textItem_.textInteractionFlags() & Qt::TextEditorInteraction))
            // item is deselected and in renaming mode -> finish renaming
            renameFinished();
    }

    return QGraphicsItem::itemChange(change, value);
}

void RootGraphicsItem::nameChanged() {
    if (!networkEditor_ || !networkEditor_->getProcessorNetwork())
        return;

    prepareGeometryChange();
    layoutChildItems();
    propertyListItem_.resizeChildItems();
    if (scene())
        scene()->invalidate();
}

void RootGraphicsItem::portsAndPropertiesChanged() {
    if (!networkEditor_ || !networkEditor_->getProcessorNetwork())
        return;

    prepareGeometryChange();
    layoutChildItems();
    propertyListItem_.resizeChildItems();
    if (scene())
        scene()->invalidate();
}

void RootGraphicsItem::renameFinished(bool) {}
void RootGraphicsItem::saveMeta() {}
void RootGraphicsItem::loadMeta() {}
void RootGraphicsItem::toggleProcessorWidget() {}
QList<QAction*> RootGraphicsItem::getProcessorWidgetContextMenuActions() {
    return QList<QAction*>();
}

// ------------------------------------------------------------------------------------------------
// Ports
// ------------------------------------------------------------------------------------------------

QList<Port*> RootGraphicsItem::getPorts() const {
    QList<Port*> result = getInports();
    result += getOutports();

    foreach (CoProcessorPort* p, getCoProcessorInports() + getCoProcessorOutports())
        result.append(p);

    return result;
}

QList<PortGraphicsItem*> RootGraphicsItem::getPortGraphicsItems() const {
    return portGraphicsItems_;
}

PortGraphicsItem* RootGraphicsItem::getPortGraphicsItem(const Port* port) const {
    tgtAssert(port, "passed null pointer");

    foreach (PortGraphicsItem* portItem, portGraphicsItems_) {
        if (portItem->getPort() == port)
            return portItem;
    }

    return 0;
}

int RootGraphicsItem::getNumberOfVisiblePorts(QList<Port*> ports) const {
    int result = 0;

    foreach (Port* p, ports) {
        PortGraphicsItem* portItem = getPortGraphicsItem(p);
        if (portItem->isVisible())
            result++;
    }

    return result;
}

int RootGraphicsItem::getNumberOfVisiblePorts(QList<CoProcessorPort*> ports) const {
    int result = 0;

    foreach (CoProcessorPort* p, ports) {
        PortGraphicsItem* portItem = getPortGraphicsItem(p);
        if (portItem->isVisible())
            result++;
    }

    return result;
}

qreal RootGraphicsItem::getMinimumWidthForPorts() const {
    QList<Port*> inports = getInports();
    QList<Port*> outports = getOutports();

    int numberOfPorts = qMax(inports.size(), outports.size());

    if (numberOfPorts == 0)
        return 0.0;
    else {
        PortGraphicsItem* portItem = 0;
        if (inports.size() == numberOfPorts)
            portItem = getPortGraphicsItem(inports[0]);
        else
            portItem = getPortGraphicsItem(outports[0]);

        return numberOfPorts * (portItem->boundingRect().width() + 5.0);
    }
}

qreal RootGraphicsItem::getMinimumHeightForPorts() const {
    QList<CoProcessorPort*> inports = getCoProcessorInports();
    QList<CoProcessorPort*> outports = getCoProcessorOutports();

    int numberOfPorts = qMax(inports.size(), outports.size());

    if (numberOfPorts == 0)
        return 0.0;
    else {
        PortGraphicsItem* portItem = 0;
        if (inports.size() == numberOfPorts)
            portItem = getPortGraphicsItem(inports[0]);
        else
            portItem = getPortGraphicsItem(outports[0]);

        return numberOfPorts * (portItem->boundingRect().height() + 5.0);
    }
}

// ------------------------------------------------------------------------------------------------
// event methods
// ------------------------------------------------------------------------------------------------

void RootGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (currentLayer() == NetworkEditorLayerLinking) {
        if (event->modifiers() == Qt::ControlModifier) {
            setFlag(ItemIsMovable, true);

            delete currentLinkArrow_;
            currentLinkArrow_ = 0;
            emit endedArrow();

        }
        else {
            setFlag(ItemIsMovable, false);
            if (currentLinkArrow_ == 0) {
                currentLinkArrow_ = new LinkArrowGraphicsItemStub(this);
                scene()->addItem(currentLinkArrow_);
                emit startedArrow();
            }

            QPointF pos = mapToScene(event->pos());
            currentLinkArrow_->adjust(pos);
        }
    }

    networkEditor_->adjustLinkArrowGraphicsItems();
    QGraphicsItem::mouseMoveEvent(event);
}

void RootGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (currentLayer() == NetworkEditorLayerLinking) {
        if (currentLinkArrow_) {
            QGraphicsItem* item = scene()->itemAt(event->scenePos());

            if (item) {
                if ((item->type() == OpenPropertyListButton::Type) && (item->parentItem() != this))
                    item = item->parentItem();

                switch (item->type()) {
                case PortGraphicsItem::Type:
                case TextGraphicsItem::Type:
                case WidgetIndicatorButton::Type:
                    item = item->parentItem();
                case AggregationGraphicsItem::Type:
                case ProcessorGraphicsItem::Type:
                    {
                    RootGraphicsItem* dest = dynamic_cast<RootGraphicsItem*>(item);
                    tgtAssert(dest, "link destination was no RootGraphicsItem");
                    emit createLink(this, dest);
                    }
                    break;
                }
            }

            delete currentLinkArrow_;
            currentLinkArrow_ = 0;
            emit endedArrow();
        }
    }

    networkEditor_->adjustLinkArrowGraphicsItems();

    // hack to prevent disappearing arrows
    scene()->setSceneRect(QRectF());
    //QRectF tmpRect = scene()->sceneRect();
    //scene()->setSceneRect(scene()->sceneRect().adjusted(-1,-1,2,2));
    //scene()->setSceneRect(tmpRect);

    QGraphicsItem::mouseReleaseEvent(event);
}

// ------------------------------------------------------------------------------------------------
// graphic methods
// ------------------------------------------------------------------------------------------------

QRectF RootGraphicsItem::boundingRect() const {
    QRectF rect = textItem_.boundingRect();

    if (rect.width() < drawingRectMinimumWidth)
        rect.setWidth(drawingRectMinimumWidth);

    if (rect.width() < getMinimumWidthForPorts())
        rect.setWidth(getMinimumWidthForPorts());

    if (rect.height() < drawingRectMinimumHeight)
        rect.setHeight(drawingRectMinimumHeight);

    if (rect.width() < getMinimumHeightForPorts())
        rect.setWidth(getMinimumHeightForPorts());

    rect.setWidth(rect.width() + getTextItemSpacing());

    return rect;
}

void RootGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* ) {
    QRectF button_rect = boundingRect();

    QColor button_color = baseColor;

    // frame indicates selected process
    if (option->state & QStyle::State_Selected) {
        button_color = selectedColor;
#ifndef __GNUC__
        setZValue(ZValueRootGraphicsItemMouseOver);
#endif
    }
    // hover effect
    else if (option->state & QStyle::State_MouseOver){
        button_color = highlightColor;
#ifndef __GNUC__
        setZValue(ZValueRootGraphicsItemMouseOver);
#endif
    }
    else {
#ifndef __GNUC__
         setZValue(ZValueRootGraphicsItemNormal);
#endif
    }

    painter->drawRect(button_rect);

    //gradient
    QLinearGradient gradient(0, 0, 0, button_rect.height());
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, button_color);
    gradient.setColorAt(0.4, shadowColor);
    gradient.setColorAt(0.6, shadowColor);
    gradient.setColorAt(1.0, button_color);

    QBrush brush(gradient);
    painter->setBrush(brush);
    painter->setPen(QPen(QBrush(button_color), 2.0));

    painter->setOpacity(opacity_);

    painter->drawRect(button_rect);

    //glass highlight
    painter->setBrush(QBrush(Qt::white));
    painter->setPen(QPen(QBrush(Qt::white), 0.01));
    painter->setOpacity(0.30);
    button_rect.setHeight(button_rect.height()/2.0);
    painter->drawRect(button_rect);
}

} // namespace
