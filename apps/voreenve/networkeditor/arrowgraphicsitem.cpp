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

#include "arrowgraphicsitem.h"

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QPainterPath>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

#include "tgt/assert.h"
#include "tgt/math.h"

namespace {
    // the size of the possible head of the arrow
    const qreal arrowHeadSize = 10.0;
}

namespace voreen {

ArrowGraphicsItem::ArrowGraphicsItem(QGraphicsItem* sourceItem, QGraphicsItem* destinationItem)
    : QGraphicsItem()
    , sourceItem_(sourceItem)
    , destinationItem_(destinationItem)
    , destinationPoint_(QPointF())
    , normalColor_(Qt::black)
    , selectedColor_(Qt::red)
    , image_(0)
    , arrowHeadSize_(arrowHeadSize)
    , sourceHeadDirection_(ArrowHeadDirectionNone)
    , destinationHeadDirection_(ArrowHeadDirectionNone)
{
    tgtAssert(sourceItem, "null pointer passed");
    setFlag(ItemIsSelectable);
    setZValue(ZValueArrowGraphicsItem);
    setAcceptsHoverEvents(true);
}

ArrowGraphicsItem::~ArrowGraphicsItem() {
    delete image_;
    image_ = 0;
}

int ArrowGraphicsItem::type() const {
    return Type;
}

void ArrowGraphicsItem::setSourceItem(QGraphicsItem* item) {
    tgtAssert(item, "passed null pointer");

    prepareGeometryChange();
    sourceItem_ = item;
    update();
}

void ArrowGraphicsItem::setDestinationItem(QGraphicsItem* item) {
    prepareGeometryChange();
    destinationItem_ = item;
    update();
}

void ArrowGraphicsItem::setDestinationPoint(const QPointF& destinationPoint) {
    prepareGeometryChange();
    destinationItem_ = 0;
    destinationPoint_ = destinationPoint;
    update();
}

QPointF ArrowGraphicsItem::getSourcePoint() const {
    return mapFromItem(sourceItem_, 0, 0);
}

QPointF ArrowGraphicsItem::getDestinationPoint() const {
    if (destinationItem_)
        return mapFromItem(destinationItem_, 0, 0);
    else
        return mapFromScene(destinationPoint_);
}

void ArrowGraphicsItem::swapItems() {
    if (destinationItem_) {
        QGraphicsItem* tmp = sourceItem_;
        sourceItem_ = destinationItem_;
        setDestinationItem(tmp);
    }
}

QRectF ArrowGraphicsItem::boundingRect() const {
    QRectF rect = shape().boundingRect();

    // add a few extra pixels for the arrow and the pen
    qreal penWidth = 1;
    qreal extra = (penWidth + arrowHeadSize) / 2.0;
    rect.adjust(-extra, -extra, extra, extra);

    return rect;
}

void ArrowGraphicsItem::setNormalColor(const QColor& color) {
    normalColor_ = color;
    update();
}

void ArrowGraphicsItem::setSelectedColor(const QColor& color) {
    selectedColor_ = color;
    update();
}

void ArrowGraphicsItem::setImage(QPixmap* image) {
    image_ = image;
    update();
}

void ArrowGraphicsItem::setTooltipText(const QString& tooltip) {
    tooltipText_ = tooltip;
}

QGraphicsItem* ArrowGraphicsItem::tooltip() const {
    if (tooltipText_.isEmpty())
        return 0;
    else {
        QGraphicsSimpleTextItem* tooltipText = new QGraphicsSimpleTextItem(tooltipText_);
        QGraphicsRectItem* tooltip = new QGraphicsRectItem((tooltipText->boundingRect()).adjusted(-4, 0, 4, 0));
        tooltipText->setParentItem(tooltip);
        tooltip->translate(-tooltip->rect().width(), -tooltip->rect().height());
        tooltip->setBrush(QBrush(QColor(253, 237, 212), Qt::SolidPattern));
        return tooltip;
    }
}

void ArrowGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) {
    QColor tmpColor;

    if (isSelected())
        tmpColor = selectedColor_;
    else
        tmpColor = normalColor_;

    if (option->state & QStyle::State_MouseOver) {
        if (tmpColor == Qt::black)    // Qt is unable to brighten up Qt::black
            tmpColor = Qt::white;
        else
            tmpColor = tmpColor.light();
    }

    painter->setPen(QPen(tmpColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QPainterPath path = shape();
    painter->drawPath(path);

    if (sourceHeadDirection_ != ArrowHeadDirectionNone) {
        QPointF origin = getSourcePoint();
        QPolygonF arrow = createArrowHeadPolygon(sourceHeadDirection_, origin);
        painter->setBrush(tmpColor);
        painter->drawPolygon(arrow);
    }

    if (destinationHeadDirection_ != ArrowHeadDirectionNone) {
        QPointF origin = getDestinationPoint();
        QPolygonF arrow = createArrowHeadPolygon(destinationHeadDirection_, origin);
        painter->setBrush(tmpColor);
        painter->drawPolygon(arrow);
    }

    if (image_) {
        QPointF imageOrigin = path.pointAtPercent(0.5);
        imageOrigin.rx() -= image_->size().width() / 2;
        imageOrigin.ry() -= image_->size().height() / 2;
        painter->drawPixmap(imageOrigin, *image_);
    }
}

QPolygonF ArrowGraphicsItem::createArrowHeadPolygon(ArrowHeadDirection direction, const QPointF& basePoint) const {
    double angle = -tgt::PI / 2.0;

    QPointF arrowP1;
    QPointF arrowP2;
    switch (direction) {
    case ArrowHeadDirectionNone:
        return QPolygonF();
    case ArrowHeadDirectionNS:
        arrowP1 = basePoint + QPointF(sin(angle - tgt::PI / 3.0) * arrowHeadSize_          , cos(angle - tgt::PI / 3.f) * arrowHeadSize_);
        arrowP2 = basePoint + QPointF(sin(angle - tgt::PI + tgt::PI / 3.0) * arrowHeadSize_, cos(angle - tgt::PI + tgt::PI / 3.0) * arrowHeadSize_);
        break;
    case ArrowHeadDirectionSN:
        arrowP1 = basePoint - QPointF(sin(angle - tgt::PI / 3.0) * arrowHeadSize_          , cos(angle - tgt::PI / 3.0) * arrowHeadSize_);
        arrowP2 = basePoint - QPointF(sin(angle - tgt::PI + tgt::PI / 3.0) * arrowHeadSize_, cos(angle - tgt::PI + tgt::PI / 3.0) * arrowHeadSize_);
        break;
    case ArrowHeadDirectionWE:
        arrowP1 = basePoint + QPointF(cos(angle - tgt::PI / 3.0) * arrowHeadSize_          , sin(angle - tgt::PI / 3.0) * arrowHeadSize_);
        arrowP2 = basePoint + QPointF(cos(angle - tgt::PI + tgt::PI / 3.0) * arrowHeadSize_, sin(angle - tgt::PI + tgt::PI / 3.0) * arrowHeadSize_);
        break;
    case ArrowHeadDirectionEW:
        arrowP1 = basePoint - QPointF(cos(angle - tgt::PI / 3.0) * arrowHeadSize_          , sin(angle - tgt::PI / 3.0) * arrowHeadSize_);
        arrowP2 = basePoint - QPointF(cos(angle - tgt::PI + tgt::PI / 3.0) * arrowHeadSize_, sin(angle - tgt::PI + tgt::PI / 3.0) * arrowHeadSize_);
        break;
    default:
        tgtAssert(false, "shouldn't get here");
    }

    return QPolygonF() << basePoint << arrowP1 << arrowP2;
}

} // namespace voreen
