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

#ifndef VRN_LINKARROWGRAPHICSITEM_H
#define VRN_LINKARROWGRAPHICSITEM_H

#include "arrowgraphicsitem.h"

namespace voreen {

class PropertyGraphicsItem;

/**
 * A LinkArrowGraphicsItem is used as an \sa ArrowGraphicsItem derivative to be used
 * between \sa PropertyGraphicsItems. Based on the state of the respective parents,
 * the arrow might not start/end directly at these PropertyGraphicsItems but at a
 * docking point of their parents.
 */
class LinkArrowGraphicsItem : public ArrowGraphicsItem {
public:
    /**
     * Constructor. Will use the default parameters from \sa ArrowGraphicsItem::ArrowGraphicsItem
     * with one exception. The default color is set to a light grey to differentiate this
     * type from other ArrowGraphicsItems.
     * \param sourceItem The source item. Must be != 0
     * \param destinationItem The destination item. May be 0, but use \sa setDestinationPoint(const QPoint&)
     */
    LinkArrowGraphicsItem(PropertyGraphicsItem* sourceItem, PropertyGraphicsItem* destinationItem);

    /// The type of this QGraphicsItem subclass
    enum { Type = UserType + UserTypesLinkArrowGraphicsItem };

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    int type() const;

    /**
     * Returns the source item as a PropertyGraphicsItem. If the stored item
     * is of a different type, an assertion will be raised (and 0 returned).
     * \return The source item as a PropertyGraphicsItem, or 0.
     */
    PropertyGraphicsItem* getSourceItem() const;

    /**
     * Returns the destination item as a PropertyGraphicsItem. If the stored item
     * is of a different type, an assertion will be raised (and 0 returned).
     * \return The source item as a PropertyGraphicsItem, or 0.
     */
    PropertyGraphicsItem* getDestinationItem() const;

    /**
     * Toggle the visibility of the arrow head on the source end of the arrow.
     * \param visible Is the source arrow head visible
     */
    void showSourceArrowHead(bool visible);

    /**
     * Toggle the visibility of the arrow head on the destination end of the arrow.
     * \param visible Is the destination arrow head visible
     */
    void showDestinationArrowHead(bool visible);

protected:
    /// Returns the path describing this arrow.
    QPainterPath shape() const;

    /// Returns the source anchor point based on the state of the source and destination items
    QPointF getSourcePoint() const;

    /// Returns the destination anchor point based on the state of the source and destination items
    QPointF getDestinationPoint() const;

private:
    /**
     * Internal enumeration used to differentiate the different states both properties and
     * their respective parents
     */
    enum LinkArrowState {
        LinkArrowStatePropertyToProperty,
        LinkArrowStatePropertyToProcessor,
        LinkArrowStateProcessorToProperty,
        LinkArrowStateProcessorToProcessor
    };

    /**
     * Internal enumeration used to save which sides of the properties should be used for \sa shape()
     * E.g. if the PropertyGraphicsItems are like this :    S  --->  D</br>
     * DockPositionsRightLeft will be chosen because the the right docking point of the source item
     * is closest to the destination item and the left docking point of the destination item is closest
     * to the source item.
     */
    enum DockPositions {
        DockPositionsLeftLeft,
        DockPositionsLeftRight,
        DockPositionsRightLeft,
        DockPositionsRightRight
    };

    /// Returns the current valid LinkArrowState
    LinkArrowState getState() const;
    /// Returns the currently valid dock positions
    DockPositions getDockPositions() const;

    /// should the arrow head on the source end of the arrow be shown
    bool showSourceArrowHead_;
    /// should the arrow head on the destination end of the arrow be shown
    bool showDestinationArrowHead_;
};

} // namespace

#endif // VRN_LINKARROWGRAPHICSITEM_H
