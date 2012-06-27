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

#include "portgraphicsitem.h"

#include "voreen/core/ports/allports.h"
#include "voreen/core/datastructures/rendertarget.h"
#include "aggregationgraphicsitem.h"
#include "portarrowgraphicsitem.h"
#include "processorgraphicsitem.h"
#include "rootgraphicsitem.h"
#include "textgraphicsitem.h"

#ifdef VRN_MODULES_PLOTTING
#include "modules/plotting/ports/plotport.h"
#endif

#include "voreen/qt/widgets/volumeviewhelper.h"

#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#if QT_VERSION >= 0x040600
#include <QGraphicsOpacityEffect>
#endif

namespace {
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
    tgt::col3 portCol = port_->getColorHint();
    QColor result = QColor(portCol.x, portCol.y, portCol.z);
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

void PortGraphicsItem::setCurrentArrow(PortArrowGraphicsItem* arrow) {
    currentArrow_ = arrow;
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

    if (getPort()->isLoopPort()) {
        if (getPort()->isOutport())
            painter->drawEllipse(boundingRect());
        else {
            QPolygonF triangle;
            const QPointF& topLeftPoint = boundingRect().topLeft();
            const QPointF bottomPoint = QPointF((boundingRect().left() + boundingRect().right()) / 2.f, boundingRect().bottom());
            const QPointF& topRightPoint = boundingRect().topRight();
            triangle << topLeftPoint << bottomPoint << topRightPoint;
            painter->drawConvexPolygon(triangle);
        }
    }
    else
        painter->drawRect(boundingRect());
}

void PortGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    tgtAssert(currentArrow_ == 0, "currentArrow was already set");
    if (parent_->currentLayer() == NetworkEditorLayerDataflow) {
        if (isOutport()) {
            currentArrow_ = new PortArrowGraphicsItem(this);
            QPointF scenePos = event->scenePos();
            currentArrow_->setDestinationPoint(scenePos);
            scene()->addItem(currentArrow_);
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

        if (currentArrow_->getOldPortGraphicsItem() == 0) {
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
                case OpenPropertyListButton::Type:
                    item = item->parentItem();
                case AggregationGraphicsItem::Type:
                case ProcessorGraphicsItem::Type:
                    {
                    RootGraphicsItem* p = static_cast<RootGraphicsItem*>(item);
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
                    if (currentArrow_ && (currentArrow_->getOldPortGraphicsItem() != portItem)) {
                        if ((item->parentItem() != parentItem()) && !(portItem->isOutport()))
                            getParent()->connect(this, portItem);
                    }
                    break;
                }
            case TextGraphicsItem::Type:
                item = item->parentItem();
                // intended fall-through
            case AggregationGraphicsItem::Type:
            case ProcessorGraphicsItem::Type:
                {
                    if (currentArrow_ && (currentArrow_->getOldPortGraphicsItem() == 0)) {
                        RootGraphicsItem* rootItem = static_cast<RootGraphicsItem*>(item);
                        getParent()->connect(this, rootItem);
                    }
                    break;
                }
            default:
                break;
            }
        }
    }

    if (currentArrow_) {
        PortGraphicsItem* oldPortItem = currentArrow_->getOldPortGraphicsItem();
        if (oldPortItem) {
            QList<QGraphicsItem*> items = scene()->items(event->scenePos());

            bool droppedOnOldPortItem = false;
            foreach (QGraphicsItem* item, items) {
                droppedOnOldPortItem = (item == oldPortItem);

                if (droppedOnOldPortItem)
                    break;
            }

            currentArrow_->setDestinationItem(oldPortItem);
            if (!droppedOnOldPortItem)
                getParent()->disconnect(this, oldPortItem);
            currentArrow_ = 0;
            emit endedArrow();
            QGraphicsItem::mouseReleaseEvent(event);
            return;
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
    RenderPort* rp = dynamic_cast<RenderPort*>(getPort());
    if (rp) {
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
            tooltip->initialize(rp->getRenderTarget());

            // name item
            QString textString = QString::fromStdString(getPort()->getName()) + "\n" + QString::number(rp->getSize().x) + " x " + QString::number(rp->getSize().y);
            QGraphicsSimpleTextItem* tooltipText = new QGraphicsSimpleTextItem(textString);
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
        if (volport->getData() && volport->getData()->getRepresentation<VolumeRAM>()) {
            portInfo = QObject::tr("VolumePort: ") + std::string(getPort()->getName()).c_str();
            portInfo +=  "\n";

            const VolumeBase* h = volport->getData();
            const VolumeRAM* v = volport->getData()->getRepresentation<VolumeRAM>();
//            if (!h)->getOrigin().getURL().empty()) {
//                std::string file = tgt::FileSystem::fileName(h->getOrigin().getPath());
//                portInfo += QObject::tr("Filename: %1\n").arg(file.c_str());
//            }

            portInfo += QString::fromStdString("Data Type: " + VolumeViewHelper::getVolumeType(v) + "\n");
            portInfo += QObject::tr("Dimension: ") + QString::fromStdString(VolumeViewHelper::getVolumeDimension(h) + "\n");
            portInfo += QObject::tr("Spacing: ") + QString::fromStdString(VolumeViewHelper::getVolumeSpacing(h) + "\n");
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
    } else if (TextPort* textport = dynamic_cast<TextPort*>(getPort())) {
        portInfo = QObject::tr("TextPort: ") + std::string(getPort()->getName()).c_str();
        if (!textport->getData().empty()) {
            portInfo +=  "\n";
            portInfo += QString::fromStdString("Content: " + textport->getData());
        }
    }
    // tgt Texture information
    else if (GenericPort<tgt::Texture>* texport = dynamic_cast<GenericPort<tgt::Texture>*>(getPort())) {
        portInfo = QObject::tr("TexturePort: ") + std::string(getPort()->getName()).c_str();
        portInfo +=  "\n";

        if (texport->getData()) {
            const tgt::Texture* tex = texport->getData();
            std::stringstream out;
            out << tex->getDimensions().x << " x " << tex->getDimensions().y << " x " << tex->getDimensions().z;

            portInfo += QObject::tr("Dimension: ") + QString::fromStdString(out.str() + "\n");
            portInfo += QObject::tr("Bits Per Pixel: %1").arg(tex->getBpp());
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

QImage* PortGraphicsItem::getRenderPortImage() const {
    RenderPort* rp = dynamic_cast<RenderPort*>(getPort());
    if (rp) {
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
            tooltip->initialize(rp->getRenderTarget());

            QImage* image = tooltip->getImage();

            return image;
        }
    }
    return 0;
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
    tgt::Texture* tex = rt->getColorTexture();
    if (!tex)
        return;

    // download pixel data to buffer
    tgt::col4* pixels = 0;
    try {
        GLubyte* pixels_b = tex->downloadTextureToBuffer(GL_RGBA, GL_UNSIGNED_BYTE);
        LGL_ERROR;
        if (pixels_b)
            pixels = reinterpret_cast<tgt::col4*>(pixels_b);
        else {
            LERRORC("TCTooltip::initialize", "failed to download texture");
            return;
        }
    }
    catch (std::bad_alloc&) {
        LERRORC("TCTooltip::initialize", "bad allocation");
        return;
    }

    image_ = new QImage(size.x, size.y, QImage::Format_ARGB32);

    // The pixels are stored row by row from bottom to top an in each row from left to right
    QColor color;
    for (int y=0; y < size.y; ++y) {
        for (int x=0; x < size.x; ++x) {
            tgt::col4 col = pixels[x + y*size.x];
            color.setRgb(col.r, col.g, col.b, col.a);
            //(0,0) is top left
            image_->setPixel(x, size.y - 1 - y, color.rgba());
        }
    }
    delete[] pixels;
    pixels = 0;

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

QImage* PortGraphicsItem::TCTooltip::getImage() const {
    return image_;
}

} // namespace
