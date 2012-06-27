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

#include "linkarrowgraphicsitemstub.h"

#include "rootgraphicsitem.h"

namespace voreen {

LinkArrowGraphicsItemStub::LinkArrowGraphicsItemStub(RootGraphicsItem* source)
    : QGraphicsItem()
    , source_(source)
{
    tgtAssert(source, "null pointer");
    setZValue(ZValueLinkArrowGraphicsItemStub);
}

int LinkArrowGraphicsItemStub::type() const {
    return Type;
}

void LinkArrowGraphicsItemStub::adjust(const QPointF& dest) {
    destPoint_ = dest;
    prepareGeometryChange();
}

QRectF LinkArrowGraphicsItemStub::boundingRect() const {
    // the bounding box of the bezier path
    QRectF rect = shape().boundingRect();

    // add a few extra pixels for the arrow and the pen
    qreal extra = 1;
    rect.adjust(-extra, -extra, extra, extra);

    return rect;
}

QPainterPath LinkArrowGraphicsItemStub::shape() const {
    const QPointF defl = QPointF(0.f, 100.f);
    const QPointF dock = source_->linkArrowDockingPoint();
    QPainterPath path(dock);
    path.cubicTo(dock - defl, destPoint_, destPoint_);
    path.cubicTo(destPoint_, dock -defl, dock);
    return path;
}

void LinkArrowGraphicsItemStub::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setBrush(Qt::black);
    painter->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawPath(shape());
}

} // namespace
