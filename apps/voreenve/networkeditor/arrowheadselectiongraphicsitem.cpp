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

#include "arrowheadselectiongraphicsitem.h"

#include "arrowgraphicsitem.h"
#include "tgt/assert.h"

namespace voreen {

ArrowHeadSelectionGraphicsItem::ArrowHeadSelectionGraphicsItem(ArrowGraphicsItem* parent)
    : QGraphicsItem(parent)
    , parent_(parent)
    , width_(0.0)
    , height_(0.0)
{
    setAcceptsHoverEvents(true);
}

int ArrowHeadSelectionGraphicsItem::type() const {
    return Type;
}

void ArrowHeadSelectionGraphicsItem::setSize(qreal width, qreal height) {
    tgtAssert(width >= 0.0, "non negative width expected");
    tgtAssert(height >= 0.0, "non negative height expected");
    width_ = width;
    height_ = height;
}

QRectF ArrowHeadSelectionGraphicsItem::boundingRect() const {
    return QRectF(0.0, 0.0, width_, height_);
}

void ArrowHeadSelectionGraphicsItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}

void ArrowHeadSelectionGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    parent_->mousePressEvent(event);
}

void ArrowHeadSelectionGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    parent_->mouseMoveEvent(event);
}

void ArrowHeadSelectionGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    parent_->mouseReleaseEvent(event);
}

} // namespace
