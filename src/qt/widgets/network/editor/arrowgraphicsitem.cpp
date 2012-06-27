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

#include "voreen/qt/widgets/network/editor/arrowgraphicsitem.h"

#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/tctooltip.h"
#include "voreen/qt/widgets/network/editor/portgraphicsitem.h"

#include "tgt/math.h"
#include "tgt/filesystem.h"

#include <QPainter>
#include <math.h>
#include <algorithm>

namespace voreen {

ArrowGraphicsItem::ArrowGraphicsItem(PortGraphicsItem* sourceNode, PortGraphicsItem* destNode)
    : QGraphicsItem()
    , source_(sourceNode)
    , dest_(destNode)
    , arrowSize_(10)
{
    setFlag(ItemIsSelectable);
    setZValue(ZValueArrowGraphicsItem);
    color_ = Qt::black;
    setAcceptsHoverEvents(true);
}

void ArrowGraphicsItem::setDestNode(PortGraphicsItem* node) {
    dest_ = node;
    color_ = Qt::black;
    adjust();
}

PortGraphicsItem* ArrowGraphicsItem::getSourceNode() {
    return source_;
}

PortGraphicsItem* ArrowGraphicsItem::getDestNode() {
    return dest_;
}

int ArrowGraphicsItem::type() const {
    return Type;
}

void ArrowGraphicsItem::adjust() {
    if (!source_ || !dest_)
        return;

    QLineF line(mapFromItem(source_, 0, 0), mapFromItem(dest_, 0, 0));

    prepareGeometryChange();
    sourcePoint_ = line.p1();

    // sourcePoint_ points to the top left corner of the source node, which is now corrected according to the nodes type
    if (!dynamic_cast<CoProcessorPort*>(static_cast<PortGraphicsItem*>(source_)->getPort())) {
        sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height());
        sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width()/2);
    }
    else {
        sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height()/2);
        sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width());
    }

    destPoint_ = line.p2();

    // same story as above
    if (!dynamic_cast<CoProcessorPort*>(static_cast<PortGraphicsItem*>(source_)->getPort()))
        destPoint_.setX(destPoint_.x() + dest_->boundingRect().width()/2);
    else
        destPoint_.setY(destPoint_.y() + dest_->boundingRect().height()/2);

    update();
}

void ArrowGraphicsItem::adjust(QPointF& dest) {
    QLineF line(mapFromItem(source_, 0, 0), mapFromScene(dest));

    QGraphicsItem* item = scene()->itemAt(dest);
    if (item) {
        PortGraphicsItem* srcPort = static_cast<PortGraphicsItem*>(source_);
        switch (item->type()) {
        case PortGraphicsItem::Type:
            if (srcPort->getParent()->testConnect(srcPort, static_cast<PortGraphicsItem*>(item)))
                color_ = Qt::green;
            else {
                if (srcPort->getParent()->sizeOriginConnectFailed(srcPort, static_cast<PortGraphicsItem*>(item)))
                    color_ = Qt::yellow;
                else
                    color_ = Qt::red;
            }
            break;
        case ProcessorGraphicsItem::Type:
            if (srcPort->getParent()->testConnect(srcPort, dynamic_cast<ProcessorGraphicsItem*>(item)))
                color_ = Qt::green;
            else
                color_ = Qt::red;
            break;
        default:
            color_ = Qt::black;
        }
    }
    else
        color_ = Qt::black;

    prepareGeometryChange();
    sourcePoint_ = line.p1();

    // sourcePoint_ points to the top left corner of the source node, which is now corrected according to the nodes type
    if (!dynamic_cast<CoProcessorPort*>(static_cast<PortGraphicsItem*>(source_)->getPort())) {
        sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height());
        sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width()/2);
    }
    else {
        sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height()/2);
        sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width());
    }

    destPoint_ = line.p2();
    update();
}

QRectF ArrowGraphicsItem::boundingRect() const {
    // the bounding box of the bezier path
    QRectF rect = shape().boundingRect();

    // add a few extra pixels for the arrow and the pen
    qreal penWidth = 1;
    qreal extra = (penWidth + arrowSize_) / 2.0;
    rect.adjust(-extra, -extra, extra, extra);

    return rect;
}

QPointF ArrowGraphicsItem::center() const {
    qreal centerX = (sourcePoint_.x() + destPoint_.x()) / 2.f;
    qreal centerY = (sourcePoint_.y() + destPoint_.y()) / 2.f;
    return QPointF(centerX, centerY);
}

QPainterPath ArrowGraphicsItem::shape() const {
    QPainterPath path(sourcePoint_);

    qreal defl = fabs(sourcePoint_.y() - destPoint_.y()) / 2.f + fabs(sourcePoint_.x() - destPoint_.x()) / 2.f;
    if (dynamic_cast<CoProcessorPort*>(source_->getPort())) {
        path.cubicTo(QPointF(sourcePoint_) + QPointF(defl, 0),
                     QPointF(destPoint_) - QPointF(defl, 0),
                     QPointF(destPoint_));
        path.cubicTo(QPointF(destPoint_) - QPointF(defl, 0),
				     QPointF(sourcePoint_) + QPointF(defl, 0),
                     QPointF(sourcePoint_));
    }
    else {
        path.cubicTo(QPointF(sourcePoint_) + QPointF(0, defl/2.0f),
                     QPointF(destPoint_) - QPointF(0, defl/2.0f),
                     QPointF(destPoint_) - QPointF(0, arrowSize_));
        //some space for the arrow's head
        path.lineTo(destPoint_);
        path.lineTo(destPoint_ - QPointF(0, arrowSize_));
        path.cubicTo(QPointF(destPoint_) - QPointF(0, defl/2.0f),
				     QPointF(sourcePoint_) + QPointF(0, defl/2.0f),
                     QPointF(sourcePoint_));
    }
    return path;
}

void ArrowGraphicsItem::setColor(QColor color) {
    color_ = color;
}

void ArrowGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) {
    QColor tmpColor;    // needed for hover effect

    if (isSelected())
        tmpColor = Qt::red;
    else
        tmpColor = color_;
    if (option->state & QStyle::State_MouseOver) {
        if (tmpColor == Qt::black)    // Qt is unable to brighten up Qt::black
            tmpColor = Qt::white;
        else
            tmpColor = tmpColor.light();
    }

    painter->setPen(QPen(tmpColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QPainterPath path(sourcePoint_);
    path.addPath(shape());
    painter->drawPath(path);

    // Right now I want a fixed angle for the bezier stuff
    double angle = -tgt::PIf / 2.0;

    QPointF sourceArrowP1 = sourcePoint_ + QPointF(sin(angle + tgt::PIf / 3.f) * arrowSize_,
        cos(angle + tgt::PIf / 3.f) * arrowSize_);
    QPointF sourceArrowP2 = sourcePoint_ + QPointF(sin(angle + tgt::PIf - tgt::PIf / 3.f) * arrowSize_,
        cos(angle + tgt::PIf - tgt::PIf / 3.f) * arrowSize_);
    QPointF destArrowP1 = destPoint_ + QPointF(sin(angle - tgt::PIf / 3.f) * arrowSize_,
        cos(angle - tgt::PIf / 3.f) * arrowSize_);
    QPointF destArrowP2 = destPoint_ + QPointF(sin(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize_,
        cos(angle - tgt::PIf + tgt::PIf / 3.f) * arrowSize_);

    painter->setBrush(tmpColor);

    if (!dynamic_cast<CoProcessorPort*>(static_cast<PortGraphicsItem*>(source_)->getPort())) {
        painter->drawPolygon(QPolygonF() << destPoint_ << destArrowP1 << destArrowP2);
    }
}

QGraphicsItem* ArrowGraphicsItem::tooltip() const {
    // If there is a render target and the network is valid I can show a tooltip
    QString portInfo = QObject::tr("no information available");

    // Retrieve the NetworkEvaluator from the NetworkEditor that is the parent
    if (scene() && scene()->parent()) {
        RenderPort* rp = dynamic_cast<RenderPort*>(source_->getPort());
        if (source_) {
            portInfo = QObject::tr("port name: ") + std::string(source_->getPort()->getName()).c_str();

            // volume information
            if (VolumePort* volport = dynamic_cast<VolumePort*>(source_->getPort())) {
                portInfo += "\n";
                if (volport->getData() && volport->getData()->getVolume()) {
                    Volume* v = volport->getData()->getVolume();

                    std::string file = tgt::FileSystem::fileName(volport->getData()->getOrigin().getPath());
                    portInfo += QObject::tr("file: %1\n").arg(file.c_str());

                    tgt::ivec3 d = v->getDimensions();
                    portInfo += QObject::tr("dimension: %1x%2x%3\n").arg(d.x).arg(d.y).arg(d.z);

                    tgt::vec3 s = v->getSpacing();
                    portInfo += QObject::tr("spacing: %1; %2; %3\n").arg(s.x).arg(s.y).arg(s.z);

                    portInfo += QObject::tr("bits: %1\n").arg(v->getBitsAllocated());

                    size_t size = v->getNumBytes();
                    QString str;
                    if (size >= 1024 * 1024) {
                        str = QObject::tr("%1 MB").arg(size / (1024 * 1024));
                    }
                    else if (size >= 1024) {
                        str = QObject::tr("%1 KB").arg(size / 1024);
                    }
                    else {
                        str =  QObject::tr("%1 bytes").arg(size);
                    }
                    portInfo += QObject::tr("size: %1").arg(str);
                } else {
                    portInfo += QObject::tr("empty volume");
                }
            }
            else if (rp) {
                //TODO: tc_
                if (rp->isConnected()) {
                    // show render target
                    TCTooltip* tooltip = new TCTooltip(-100, -100, 100, 100);
                    if (source_) {
                        RenderPort* rp = dynamic_cast<RenderPort*>(source_->getPort());
                        if (rp)
                            tooltip->initialize(rp->getData());
                    }
                    return tooltip;
                }
            }
        }
    }

    // show text information
    QGraphicsSimpleTextItem* tooltipText = new QGraphicsSimpleTextItem(portInfo);
    QGraphicsRectItem* tooltip = new QGraphicsRectItem((tooltipText->boundingRect()).adjusted(-4, 0, 4, 0));
    tooltipText->setParentItem(tooltip);
    tooltip->translate(-tooltip->rect().width(), -tooltip->rect().height());
    tooltip->setBrush(QBrush(QColor(253, 237, 212), Qt::SolidPattern));
    return tooltip;
}

void ArrowGraphicsItem::setLayer(NetworkEditorLayer layer) {
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

} // namespace voreen
