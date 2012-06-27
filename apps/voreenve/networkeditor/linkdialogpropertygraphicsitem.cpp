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

#include "linkdialogpropertygraphicsitem.h"

#include "linkdialogarrowgraphicsitem.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

namespace voreen {

LinkDialogPropertyGraphicsItem::LinkDialogPropertyGraphicsItem(const Property* prop, ColumnPosition position)
    : PropertyGraphicsItem(prop, 0)
    , currentArrow_(0)
    , position_(position)
{
    setFlag(ItemIsSelectable);
}

int LinkDialogPropertyGraphicsItem::type() const {
    return Type;
}

ColumnPosition LinkDialogPropertyGraphicsItem::getPosition() const {
    return position_;
}

void LinkDialogPropertyGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    tgtAssert(currentArrow_ == 0, "an arrow already existed");

    scene()->clearSelection();

    currentArrow_ = new LinkDialogArrowGraphicsItem(this);
    currentArrow_->setDestinationPoint(mapToScene(event->pos()));
    currentArrow_->setBidirectional(true);
    scene()->addItem(currentArrow_);
    oldBounds_ = scene()->sceneRect();
}

void LinkDialogPropertyGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    currentArrow_->setDestinationPoint(mapToScene(event->pos()));

    QList<QGraphicsItem*> items = scene()->items(mapToScene(event->pos()));
    QGraphicsItem* item = 0;

    foreach (QGraphicsItem* i, items) {
        if (i->type() == LinkDialogPropertyGraphicsItem::Type)
            item = i;
    }

    if (item) {
        LinkDialogPropertyGraphicsItem* gi = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(item);
        const Property* p1 = getProperty();
        const Property* p2 = gi->getProperty();

        // only if gi and this are in different columns or both are in the center, we want to procced further
        ColumnPosition thisPos = getPosition();
        ColumnPosition otherPos = gi->getPosition();
        if ((thisPos != otherPos) || ((thisPos == ColumnPositionCenter) && (otherPos == ColumnPositionCenter))) {
            if ((p1 != p2) && PropertyLink::arePropertiesLinkable(p1, p2))
                currentArrow_->setNormalColor(Qt::green);
            else {
                // those properties might be linkable if python is enabled
#ifdef VRN_WITH_PYTHON
                currentArrow_->setNormalColor(Qt::yellow);
#else
                currentArrow_->setNormalColor(Qt::red);
#endif
            }
        }
    }
    else
        currentArrow_->setNormalColor(Qt::black);

    scene()->setSceneRect(oldBounds_);  // this prevents the growing (and therefore the unwanted ability to scroll) of the sceneRect
    scene()->invalidate();
}

void LinkDialogPropertyGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QList<QGraphicsItem*> items = scene()->items(mapToScene(event->pos()));
    QGraphicsItem* item = 0;

    foreach (QGraphicsItem* i, items) {
        if (i->type() == LinkDialogPropertyGraphicsItem::Type)
            item = i;
    }

    if (item) {
        LinkDialogPropertyGraphicsItem* gi = qgraphicsitem_cast<LinkDialogPropertyGraphicsItem*>(item);

        // only if gi and this are in different columns or both are in the center, we want to procced further
        ColumnPosition thisPos = getPosition();
        ColumnPosition otherPos = gi->getPosition();
        if ((thisPos != otherPos) || ((thisPos == ColumnPositionCenter) && (otherPos == ColumnPositionCenter))) {
        //if (gi->isInLeftColumn() != isInLeftColumn()) {
            const Property* p1 = getProperty();
            const Property* p2 = gi->getProperty();
            if ((p1 != p2) && PropertyLink::arePropertiesLinkable(p1,p2)) {
                currentArrow_->setDestinationItem(gi);
                currentArrow_->setNormalColor(Qt::black);
                emit createdArrow(currentArrow_);
            }
            else {
#ifdef VRN_WITH_PYTHON
                currentArrow_->setDestinationItem(gi);
                currentArrow_->setNormalColor(Qt::yellow);
                emit createdArrow(currentArrow_);
#else
                delete currentArrow_;
#endif

            }
        }
        else
            delete currentArrow_;

        currentArrow_ = 0;
    }
    else {
        delete currentArrow_;
        currentArrow_ = 0;
    }
}

} // namespace
