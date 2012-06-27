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

#ifndef VRN_LINKDIALOGARROWGRAPHICSITEM_H
#define VRN_LINKDIALOGARROWGRAPHICSITEM_H

#include "arrowgraphicsitem.h"
#include "networkeditor_common.h"

namespace voreen {

class LinkDialogPropertyGraphicsItem;

/**
 * This ArrowGraphicsItem subclass is used in the \sa PropertyLinkDialog
 * both as an existing connection between two \sa LinkDialogPropertyGraphicsItems
 * and as a probational connection between a source item and a QPoint in the
 * QGraphicsScene. This arrow can be set to be painted with a single arrow head
 * (i.e. <code>setBidirectional(false)</code>) or with both ends having an
 * arrowhead (i.e. <code>setBidirectional(true)</code>).
 */
class LinkDialogArrowGraphicsItem : public ArrowGraphicsItem {
public:
    /**
     * A simple constructor.
     * \param source The source LinkDialogPropertyGraphicsItem from which this
     * arrow originates.
     * \param destination Either the destination LinkDialogPropertyGraphicsItem
     * (if the connection already exists), or 0 if the arrow should point to a
     * single point in space.
     * \param bidirectional Sets the bidirectional quality (see above).
     */
    LinkDialogArrowGraphicsItem(LinkDialogPropertyGraphicsItem* source, LinkDialogPropertyGraphicsItem* destination = 0, bool bidirectional = false);

    /// The type of this QGraphicsItem subclass
    enum { Type = UserType + UserTypesLinkDialogArrowGraphicsItem };

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    int type() const;

    /**
     * Sets the destination once this item is determined.
     * \param dest The new destination item. This value must be != 0.
     */
    void setDestinationItem(LinkDialogPropertyGraphicsItem* dest);

    /**
     * Adjust the representation whether the link is bidirectional or not
     * /param bidirectional Is the represented link bidirectional or not
     */
    void setBidirectional(bool bidirectional);

    QPainterPath shape() const;

    /// Returns the source item of this arrow. Always exists
    LinkDialogPropertyGraphicsItem* getSourceItem() const;

    /// Returns the destination item of this arrow or 0 if there is no such item
    LinkDialogPropertyGraphicsItem* getDestinationItem() const;

    /// Returns the anchor point of the source item
    QPointF getSourcePoint() const;

    /// Returns the anchor point of the destination item (if it exists), or
    /// the point to which this arrow points.
    QPointF getDestinationPoint() const;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

private:
    /// is the represented link bidirectional or not
    bool bidirectional_;

    QPointF clickedPoint_;
    bool movedAwayInEvent_;
};

} // namespace

#endif // VRN_LINKDIALOGARROWGRAPHICSITEM_H
