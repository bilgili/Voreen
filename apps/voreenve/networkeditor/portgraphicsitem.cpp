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

#include "portgraphicsitem.h"

#include "voreen/core/ports/allports.h"
#include "voreen/core/datastructures/rendertarget.h"
#include "portarrowgraphicsitem.h"
#include "processorgraphicsitem.h"
#include "rootgraphicsitem.h"
#include "textgraphicsitem.h"

#include "voreen/qt/widgets/volumeviewhelper.h"

#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#if QT_VERSION >= 0x040600
#include <QGraphicsOpacityEffect>
#endif

namespace {
    // the colors for the different port types
    const QColor colorVolumePort = Qt::red;
    const QColor colorCoProcessorPort = Qt::green;
    const QColor colorRenderPort = Qt::blue;
    const QColor colorTextPort = Qt::cyan;
    const QColor colorVolumeCollectionPort = Qt::magenta;
    const QColor colorGeometryPort = Qt::yellow;
    const QColor colorUnknownPort = Qt::gray;

    // the inner color of the PortGraphicsItem
    const QColor colorBrush = Qt::lightGray;

    // the base size of the PortGraphicsItem
    const QSize sizeNormal = QSize(10, 10);

    // by this factor a PortGraphicsItem is expanded if it allows multiple connections
    const qreal sizeMultipleConnectionStrechFactor = 1.75f;
}

namespace voreen {

PortGraphicsItem::PortGraphicsItem(Port* port, RootGraphicsItem* parent)
#if (QT_VERSION >= 0x040600)
    : QGraphicsObject(parent)
#else
    : QObject()
    , QGraphicsItem(parent)
#endif
    , parent_(parent)
    , port_(port)
    , currentArrow_(0)
{
    tgtAssert(port, "passed null pointer");
    tgtAssert(parent, "passed null pointer");

    setFlag(ItemIsSelectable);
    setAcceptsHoverEvents(true);
}

void PortGraphicsItem::setLayer(NetworkEditorLayer layer) {
    switch (layer) {
    case NetworkEditorLayerDataflow:
        setFlag(ItemIsSelectable);
        setAcceptsHoverEvents(true);
#if QT_VERSION >= 0x040600
        setGraphicsEffect(0);
#else
        setVisible(true);
#endif
            break;
    case NetworkEditorLayerLinking:
        setFlag(ItemIsSelectable, false);
        setAcceptsHoverEvents(false);
#if QT_VERSION >= 0x040600
        {
        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect;
        effect->setOpacity(0.2);
        setGraphicsEffect(effect);
        }
#else
        setVisible(false);
#endif
        break;
    default:
        tgtAssert(false, "shouldn't get here");
    }
}

QColor PortGraphicsItem::getColor() const {
    QColor result;

    if (dynamic_cast<RenderPort*>(port_)) {
        result = colorRenderPort;
    }
    else if (dynamic_cast<VolumePort*>(port_)) {
        result = colorVolumePort;
    }
    else if (dynamic_cast<CoProcessorPort*>(port_)) {
        result = colorCoProcessorPort;
    }
    else if (dynamic_cast<VolumeCollectionPort*>(port_)) {
        result = colorVolumeCollectionPort;
    }
    else if (dynamic_cast<GeometryPort*>(port_)) {
        result = colorGeometryPort;
    }
    else if (dynamic_cast<TextPort*>(port_)) {
        result = colorTextPort;
    }
    else {
        result = colorUnknownPort;
    }
    result.setAlpha(100);

    return result;
}

bool PortGraphicsItem::isOutport() const {
    return port_->isOutport();
}

int PortGraphicsItem::type() const {
    return Type;
}

Port* PortGraphicsItem::getPort() const {
    return port_;
}

void PortGraphicsItem::addArrow(PortArrowGraphicsItem* arrow) {
    arrowList_.append(arrow);
}

bool PortGraphicsItem::removeArrow(PortArrowGraphicsItem* arrow) {
    return arrowList_.removeOne(arrow);
}

const QList<PortArrowGraphicsItem*>& PortGraphicsItem::getArrowList() {
    return arrowList_;
}

const QList<PortGraphicsItem*>& PortGraphicsItem::getConnectedPorts() {
   return connectedPorts_;
}

void PortGraphicsItem::addConnection(PortGraphicsItem* port) {
    connectedPorts_.append(port);
}

bool PortGraphicsItem::removeConnection(PortGraphicsItem* port) {
    return connectedPorts_.removeOne(port);
}

void PortGraphicsItem::update(const QRectF& rect) {
    if (isOutport()) {
        foreach (PortArrowGraphicsItem* arrow, arrowList_)
            arrow->update();
    }
    else {
        foreach (PortGraphicsItem* port, connectedPorts_)
            port->update(rect);
    }

    QGraphicsItem::update(rect);
}

QRectF PortGraphicsItem::boundingRect() const {
    QRectF rect(0,0,sizeNormal.width(),sizeNormal.height());
    if (port_->allowMultipleConnections() && !port_->isOutport()) {
        if (dynamic_cast<CoProcessorPort*>(port_))
            rect.setHeight(rect.height() * sizeMultipleConnectionStrechFactor);
        else
            rect.setWidth(rect.width() * sizeMultipleConnectionStrechFactor);
    }
    return rect;
}

void PortGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) {
    QColor color = getColor();
    painter->setPen(QPen(color, 2));
    painter->setBrush(colorBrush);

    if (option->state & QStyle::State_Sunken)
        painter->setBrush(color.darker(150));
    if (option->state & QStyle::State_MouseOver)
        painter->setBrush(color);

    if (getPort()->isLoopPort())
        painter->drawEllipse(boundingRect());
    else
        painter->drawRect(boundingRect());
}

void PortGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    tgtAssert(currentArrow_ == 0, "currentArrow was already set");
    if (parent_->currentLayer() == NetworkEditorLayerDataflow) {
        if (isOutport()) {
            currentArrow_ = new PortArrowGraphicsItem(this);
            scene()->addItem(currentArrow_);
            QPointF scenePos = event->scenePos();
            currentArrow_->setDestinationPoint(scenePos);
            emit startedArrow();
        }
    }

    QGraphicsItem::mousePressEvent(event);
}

void PortGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (isOutport()) {
        tgtAssert(currentArrow_ != 0, "currentArrow was not set");
        QPointF scenePos = event->scenePos();
        currentArrow_->setDestinationPoint(scenePos);

        QGraphicsItem* item = scene()->itemAt(scenePos);
        if (item && (item != this)) {
            switch (item->type()) {
            case PortGraphicsItem::Type:
                {
                    PortGraphicsItem* destPort = qgraphicsitem_cast<PortGraphicsItem*>(item);
                    if (getParent()->connect(this, destPort, true))
                        currentArrow_->setNormalColor(Qt::green);
                    else {
                        RenderPort* rSrc = dynamic_cast<RenderPort*>(getPort());
                        RenderPort* rDest = dynamic_cast<RenderPort*>(destPort->getPort());

                        if (rSrc && rDest && rSrc->doesSizeOriginConnectFailWithPort(rDest))
                            currentArrow_->setNormalColor(Qt::yellow);
                        else
                            currentArrow_->setNormalColor(Qt::red);
                    }
                break;
                }
            case TextGraphicsItem::Type:
                item = item->parentItem();
            case RootGraphicsItem::Type:
            case ProcessorGraphicsItem::Type:
                {
                RootGraphicsItem* p = static_cast<RootGraphicsItem*>(item);;
                if (getParent()->connect(this, p, true))
                    currentArrow_->setNormalColor(Qt::green);
                else
                    currentArrow_->setNormalColor(Qt::red);
                break;
                }
            default:
                currentArrow_->setNormalColor(Qt::black);
            }
        }
        else
            currentArrow_->setNormalColor(Qt::black);
        }
    QGraphicsItem::mouseMoveEvent(event);
}

void PortGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem* item = scene()->itemAt(event->scenePos());

    if (isOutport()) {
        if (item && item != this) {
            switch (item->type()) {
            case PortGraphicsItem::Type:
                {
                    PortGraphicsItem* portItem = static_cast<PortGraphicsItem*>(item);
                    if ((item->parentItem() != parentItem()) && !(portItem->isOutport()))
                        getParent()->connect(this, portItem);
                    break;
                }
            case TextGraphicsItem::Type:
                item = item->parentItem();
                // intended fall-through
            case RootGraphicsItem::Type:
            case ProcessorGraphicsItem::Type:
                {
                    RootGraphicsItem* rootItem = static_cast<RootGraphicsItem*>(item);
                    getParent()->connect(this, rootItem);
                    break;
                }
            default:
                break;
            }
        }
    }

    delete currentArrow_;
    currentArrow_ = 0;
    emit endedArrow();

    QGraphicsItem::mouseReleaseEvent(event);
}

RootGraphicsItem* PortGraphicsItem::getParent() const {
    return parent_;
}

QGraphicsItem* PortGraphicsItem::tooltip() const {
    if (!scene() || !scene()->parent())
        return 0;

    QString portInfo;

    // RenderPort information with preview
    if (RenderPort* rp = dynamic_cast<RenderPort*>(getPort())) {
        if (rp->isConnected()) {
            // max dimensions of the render tooltip
            tgt::ivec2 maxSize(100, 100);

            // fit rect
            float image_aspect = static_cast<float>(rp->getSize().x) / static_cast<float>(rp->getSize().y);
            float tip_aspect = maxSize.x / maxSize.y;
            if (image_aspect > tip_aspect) {
                // image is wider - lower tooltip's height
                float newheight = maxSize.x / image_aspect;
                float dh = maxSize.y - newheight;
                maxSize.y -= static_cast<int>(dh);
            }
            else if (image_aspect < tip_aspect) {
                // image is higher - lower tooltip's width
                float newwidth = maxSize.y * image_aspect;
                float dw = maxSize.x - newwidth;
                maxSize.x -= static_cast<int>(dw);
            }

            // create render tooltip
            TCTooltip* tooltip = new TCTooltip(-(maxSize.x+8), 0, maxSize.x, maxSize.y);
            RenderPort* rp = dynamic_cast<RenderPort*>(getPort());
            if (rp)
                tooltip->initialize(rp->getData());

            // name item
            QGraphicsSimpleTextItem* tooltipText = new QGraphicsSimpleTextItem(QString::fromStdString(getPort()->getName()));
            QGraphicsRectItem* tooltipTextRect = new QGraphicsRectItem((tooltipText->boundingRect()).adjusted(-3, -2, 3, 2));
            tooltipText->setParentItem(tooltipTextRect);
            tooltipTextRect->translate(-(maxSize.x+5), -tooltipTextRect->rect().height() + 1);
            tooltipTextRect->setBrush(QBrush(QColor(255, 255, 220), Qt::SolidPattern));

            // composite of render preview and port name
            QGraphicsItemGroup* itemGroup = new QGraphicsItemGroup();
            itemGroup->addToGroup(tooltip);
            itemGroup->addToGroup(tooltipTextRect);
            return itemGroup;
        }
        else {
            portInfo = QObject::tr("RenderPort: ") + "\n" + std::string(getPort()->getName()).c_str();
            //portInfo += "\n" + QObject::tr("no image");
        }
    }
    // volume information
    else if (VolumePort* volport = dynamic_cast<VolumePort*>(getPort())) {
        if (volport->getData() && volport->getData()->getVolume()) {
            portInfo = QObject::tr("VolumePort: ") + std::string(getPort()->getName()).c_str();
            portInfo +=  "\n";

            Volume* v = volport->getData()->getVolume();
            if (!volport->getData()->getOrigin().getURL().empty()) {
                std::string file = tgt::FileSystem::fileName(volport->getData()->getOrigin().getPath());
                portInfo += QObject::tr("Filename: %1\n").arg(file.c_str());
            }

            portInfo += QString::fromStdString("Data Type: " + VolumeViewHelper::getVolumeType(v) + "\n");
            portInfo += QObject::tr("Dimension: ") + QString::fromStdString(VolumeViewHelper::getVolumeDimension(v) + "\n");
            portInfo += QObject::tr("Spacing: ") + QString::fromStdString(VolumeViewHelper::getVolumeSpacing(v) + "\n");
            portInfo += QObject::tr("Bits Per Voxel: %1\n").arg(v->getBitsAllocated());
            QString numVoxelString;
            numVoxelString.setNum(static_cast<int>(v->getNumVoxels()));
            portInfo += QObject::tr("Num Voxels: ") + numVoxelString + "\n";
            portInfo += QObject::tr("Memory Size: ") + QString::fromStdString(VolumeViewHelper::getVolumeMemorySize(v));
        }
        else {
            portInfo = QObject::tr("VolumePort: ") + "\n" + std::string(getPort()->getName()).c_str();
            //portInfo +=  "\n" + QObject::tr("no volume");
        }
    }
    // just show port name
    else {
        portInfo = QObject::tr("Port: ") + "\n" + std::string(getPort()->getName()).c_str();
    }

    // create graphics item wrapping the portInfo string
    QGraphicsSimpleTextItem* tooltipText = new QGraphicsSimpleTextItem(portInfo);
    QGraphicsRectItem* tooltipRect = new QGraphicsRectItem((tooltipText->boundingRect()).adjusted(-4, -2, 4, 2));
    tooltipText->setParentItem(tooltipRect);
    tooltipRect->translate(-tooltipRect->rect().width() - 4, 10);
    tooltipRect->setBrush(QBrush(QColor(255, 255, 220), Qt::SolidPattern));
    return tooltipRect;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PortGraphicsItem::TCTooltip::TCTooltip(QGraphicsItem* parent)
    : QGraphicsRectItem(parent), image_(0)
{}

PortGraphicsItem::TCTooltip::TCTooltip(const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsRectItem(rect, parent), image_(0)
{}

PortGraphicsItem::TCTooltip::TCTooltip(qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent)
    : QGraphicsRectItem(x, y, width, height, parent), image_(0)
{}

PortGraphicsItem::TCTooltip::~TCTooltip() {
    delete image_;
}

void PortGraphicsItem::TCTooltip::initialize(RenderTarget* rt) {
    if (!rt)
        return;

    tgt::ivec2 size = rt->getSize();
    image_ = new QImage(size.x, size.y, QImage::Format_ARGB32);

    tgt::Texture* tex = rt->getColorTexture();
    if (!tex)
        return;

    tex->downloadTexture();

    QColor color;

    // The pixels are stored row by row from bottom to top an in each row from left to right
    for (int y=0; y < size.y; ++y) {
        for (int x=0; x < size.x; ++x) {
            tgt::Color col = tex->texelAsFloat(x,y);

            // fragment values are not necessarily clamped when written to the texture,
            // so clamp them here
            col = tgt::clamp(col, 0.f, 1.f);

            color.setRgbF(col.r, col.g, col.b, col.a);

            //(0,0) is top left
            image_->setPixel(x, size.y - 1 - y, color.rgba());
        }
    }

    // checkers background
    QPixmap pm(20, 20);
    QPainter pmp(&pm);
    pmp.fillRect(0, 0, 10, 10, Qt::lightGray);
    pmp.fillRect(10, 10, 10, 10, Qt::lightGray);
    pmp.fillRect(0, 10, 10, 10, Qt::darkGray);
    pmp.fillRect(10, 0, 10, 10, Qt::darkGray);
    pmp.end();
    setBrush(QBrush(pm));
}

void PortGraphicsItem::TCTooltip::paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QGraphicsRectItem::paint(painter, option, widget);
    painter->drawRect(rect());
    if (image_)
        painter->drawImage(rect(), *image_);
}

} // namespace
