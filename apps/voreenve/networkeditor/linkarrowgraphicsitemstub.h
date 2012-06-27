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

#ifndef VRN_LINKARROWGRAPHICSITEMSTUB_H
#define VRN_LINKARROWGRAPHICSITEMSTUB_H

#include <QGraphicsItem>
#include "networkeditor_common.h"

namespace voreen {

class RootGraphicsItem;

/**
 * This class serves as a simple stub being drawn while a new link is
 * selected. This item only has a starting \sa RootGraphicsItem from which
 * it will fetch the anchor point and leads to a single point in the
 * QGraphicsScene. As soon as the two RootGraphicsItems are chosen, this
 * item should be deleted and be replaced by a \sa LinkArrowGraphicsItem
 */
class LinkArrowGraphicsItemStub : public QGraphicsItem {
public:
    /**
     * A simple constructor.
     * \param source The source item from which this item will originate
     */
    LinkArrowGraphicsItemStub(RootGraphicsItem* source);

    /// The type of this QGraphicsItem subclass
    enum { Type = UserType + UserTypesLinkArrowGraphicsItemStub };

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    int type() const;

    /**
     * Readjusts this arrow to point to the passed QPointF. Will prepare a
     * geometry change of this item
     * \param point The point to which this arrow should point
     */
    void adjust(const QPointF& point);

    /// Computes the bounding rect of this item
    QRectF boundingRect() const;

protected:
    QPainterPath shape() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*);

    /// The source item from which this arrow originates
    RootGraphicsItem* source_;

    /// The destination point of this arrow
    QPointF destPoint_;
};

} // namespace

#endif // VRN_LINKARROWGRAPHICSITEMSTUB_H
