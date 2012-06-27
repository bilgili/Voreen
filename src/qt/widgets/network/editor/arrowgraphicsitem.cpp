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
#include "voreen/qt/widgets/network/editor/guiitem.h"
#include "voreen/qt/widgets/network/editor/tctooltip.h"

#include "tgt/math.h"

#include <QPainter>
#include <math.h>

namespace voreen {

ArrowGraphicsItem::ArrowGraphicsItem(QGraphicsItem* sourceNode, QGraphicsItem* destNode)
    : QGraphicsItem()
    , arrowSize_(10)
{
    source_ = sourceNode;
    dest_ = destNode;

    color_ = Qt::black;

    setFlag(ItemIsSelectable);
    if (source_->type() == PortGraphicsItem::Type)
        setZValue(0.2);
    else
        setZValue(0);
    // activate hover events for highlighting
    if (!acceptsHoverEvents())
        setAcceptsHoverEvents(true);

}

ArrowGraphicsItem::~ArrowGraphicsItem() {
    // check if arrow still in scene, because removeItem will crash then
//FIXME: unnecessary? joerg
//     if ((scene() != 0) && (scene()->views().count() > 0)) {
//         static_cast<NetworkEditor*>(scene()->views()[0])->hideTooltip(); // FIXME: Hack to fix a crash - make this prettier!
//         scene()->removeItem(this);
//     }
}

void ArrowGraphicsItem::setDestNode(QGraphicsItem* node) {
    dest_ = node;
    color_ = Qt::black;
    adjust();
}

void ArrowGraphicsItem::adjust() {
    if (!source_ || !dest_)
        return;

    QLineF line(mapFromItem(source_, 0, 0), mapFromItem(dest_, 0, 0));

    prepareGeometryChange();
    sourcePoint_ = line.p1();

    // sourcePoint_ points to the top left corner of the source node, which is now corrected according to the nodes type
    if (source_->type() == PortGraphicsItem::Type) {
        if (static_cast<PortGraphicsItem*>(source_)->getPortType().getSubString(0) != "coprocessor") {
            sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height());
            sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width()/2);
        }
        else {
            sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height()/2);
            sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width());
        }
    }
    else {
        sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height()/2);
        sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width()/2);
    }

    destPoint_ = line.p2();

    // same story as above
    if (source_->type() == PortGraphicsItem::Type) {
        if (static_cast<PortGraphicsItem*>(source_)->getPortType().getSubString(0) != "coprocessor")
            destPoint_.setX(destPoint_.x() + dest_->boundingRect().width()/2);
        else
            destPoint_.setY(destPoint_.y() + dest_->boundingRect().height()/2);
    }
    else {
        destPoint_.setX(destPoint_.x() + dest_->boundingRect().width()/2);
        destPoint_.setY(destPoint_.y() + dest_->boundingRect().height()/2);
    }

    update();
}

void ArrowGraphicsItem::adjust(QPointF& dest) {
    QLineF line(mapFromItem(source_, 0, 0), mapFromScene(dest));

    QGraphicsItem* item = scene()->itemAt(dest);
    if (source_->type() == PortGraphicsItem::Type && item) {
        PortGraphicsItem* srcPort = static_cast<PortGraphicsItem*>(source_);
        switch (item->type()) {
        case PortGraphicsItem::Type:
            if (srcPort->getParent()->testConnect(srcPort, static_cast<PortGraphicsItem*>(item)))
                color_ = Qt::green;
            else
                color_ = Qt::red;
            break;
        case ProcessorGraphicsItem::Type:
            if (srcPort->getParent()->testConnect(srcPort, static_cast<ProcessorGraphicsItem*>(item)))
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
    if (source_->type() == PortGraphicsItem::Type) {
        if (static_cast<PortGraphicsItem*>(source_)->getPortType().getSubString(0) != "coprocessor") {
            sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height());
            sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width()/2);
        }
        else {
            sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height()/2);
            sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width());
        }
    }
    else {
        sourcePoint_.setY(sourcePoint_.y() + source_->boundingRect().height()/2);
        sourcePoint_.setX(sourcePoint_.x() + source_->boundingRect().width()/2);
    }

    destPoint_ = line.p2();
    update();
}

qreal ArrowGraphicsItem::deflection() const {
    return fabs(sourcePoint_.y() - destPoint_.y()) / 2.f + fabs(sourcePoint_.x() - destPoint_.x()) / 2.f;
}

QRectF ArrowGraphicsItem::boundingRect() const {
    qreal penWidth = 1;
    qreal extra = (penWidth + arrowSize_) / 2.0;
    qreal defl = deflection();
    QRectF brect = QRectF(sourcePoint_, QSizeF(destPoint_.x() - sourcePoint_.x(), destPoint_.y() - sourcePoint_.y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
    PortGraphicsItem* portItem = dynamic_cast<PortGraphicsItem*>(source_);
    if (portItem && portItem->getPortType().getSubString(0) != "coprocessor") {
        return brect.adjusted(0, -defl, 0, 2*defl); // bounding rect is bigger due to bezier stuff
    }
    else {
        return brect.adjusted(-defl, 0, 2*defl, 0); // bounding rect is bigger due to bezier stuff
    }
}

QPointF ArrowGraphicsItem::center() const {
    return QPointF((sourcePoint_.x() + destPoint_.x()) / 2, (sourcePoint_.y() + destPoint_.y()) / 2);
}

QPainterPath ArrowGraphicsItem::shape() const {
    QPainterPath path(sourcePoint_);

    qreal defl = deflection();
    if (source_->type() == PortGraphicsItem::Type) {
        if (static_cast<PortGraphicsItem*>(source_)->getPortType().getSubString(0) == "coprocessor") {
            path.cubicTo(QPointF(sourcePoint_) + QPointF(defl, 0),
                         QPointF(destPoint_) - QPointF(defl, 0),
                         QPointF(destPoint_) - QPointF(0, 0));
        }
        else {
            path.cubicTo(QPointF(sourcePoint_) + QPointF(0, defl),
                         QPointF(destPoint_) - QPointF(0, defl),
                         QPointF(destPoint_) - QPointF(0, arrowSize_));
            //some space for the arrow's head
            path.lineTo(destPoint_);
        }
    }
    else {
        path.lineTo(destPoint_);
    }
    return path;
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

    if ( (source_->type() == PortGraphicsItem::Type) && (static_cast<PortGraphicsItem*>(source_)->getPortType().getSubString(0) != "coprocessor") ) {
        painter->drawPolygon(QPolygonF() << destPoint_ << destArrowP1 << destArrowP2);
    }
}

int ArrowGraphicsItem::getSourceTextureContainerTarget(NetworkEvaluator* neteval) const {
    PortGraphicsItem* sourcePortItem = dynamic_cast<PortGraphicsItem*>(source_); // Only has TCTarget if it is a Port
    if (sourcePortItem) {
        try {
            return neteval->getTextureContainerTarget(sourcePortItem->getPort());
        }
        catch (std::exception&) {
            return -1;
        }
    }
    else {
        return -1;
    }
}

QGraphicsItem* ArrowGraphicsItem::tooltip() const {
    // If there is a tctarget and the network is valid I can show a Tooltip
    int tcTarget = -1;
    NetworkEvaluator* netEval = 0;
    QString portInfo = QObject::tr("no tooltip available");

    // Retrieve the NetworkEvaluator from the NetworkEditor that is the parent
    if (scene() && scene()->parent()) {
        if (dynamic_cast<NetworkEditor*>(scene()->parent()))
            netEval = dynamic_cast<NetworkEditor*>(scene()->parent())->getEvaluator();
        if (netEval)
            tcTarget = getSourceTextureContainerTarget(netEval);

        PortGraphicsItem* sourcePortItem = dynamic_cast<PortGraphicsItem*>(source_); // Only has TCTarget if it is a Port
        if (tcTarget == -1 && sourcePortItem) {
            portInfo = QObject::tr("Port type: ")
                + std::string(sourcePortItem->getPort()->getTypeIdentifier().getSubString(0)).c_str();
        }        
    }

    if (tcTarget != -1 && netEval->isValid()) {
        TCTooltip* tooltip = new TCTooltip(-100, -100, 100, 100);
        tooltip->initialize(tcTarget, netEval->getTextureContainer());
        return tooltip;
    }
    else {
        QGraphicsSimpleTextItem* noTooltipText = new QGraphicsSimpleTextItem(portInfo);
        QGraphicsRectItem* noTooltip = new QGraphicsRectItem((noTooltipText->boundingRect()).adjusted(-4, 0, 4, 0));
        noTooltipText->setParentItem(noTooltip);
        noTooltip->translate(-noTooltip->rect().width(), -noTooltip->rect().height());
        noTooltip->setBrush(QBrush(QColor(253, 237, 212), Qt::SolidPattern));
        return noTooltip;
    }
}

} // namespace voreen
