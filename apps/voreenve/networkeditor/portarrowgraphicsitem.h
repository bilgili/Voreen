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

#ifndef VRN_PORTARROWGRAPHICSITEM_H
#define VRN_PORTARROWGRAPHICSITEM_H

#include "arrowgraphicsitem.h"

#include "networkeditor.h"

namespace voreen {

class PortGraphicsItem;

/**
 * A PortArrowGraphicsItem is used as an \sa ArrowGraphicsItem derivative to be used
 * between either a \sa PortGraphicsItem and a point, or between two
 * \sa PortGraphicsItems. As an anchor point, the center of the the \sa PortGraphicsItems
 * is used.
 */
class PortArrowGraphicsItem : public ArrowGraphicsItem {
public:
    /**
     * Constructor. \sa ArrowGraphicsItem::ArrowGraphicsItem
     * Will use all default parameters from the ArrowGraphicsItem constructor.
     * \param sourceItem The source item. Must be != 0
     */
    PortArrowGraphicsItem(PortGraphicsItem* sourceItem);

    /// The type of this QGraphicsItem subclass
    enum { Type = UserType + UserTypesPortArrowGraphicsItem };

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    int type() const;

    /**
     * Returns the source item as a PortGraphicsItem. If the stored item
     * is of a different type, an assertion will be raised (and 0 returned).
     * \return The source item as a PortGraphicsItem, or 0.
     */
    PortGraphicsItem* getSourceItem() const;

    /**
     * Returns the destination item as a PortGraphicsItem. If the stored item
     * is of a different type, an assertion will be raised (and 0 returned).
     * \return The source item as a PortGraphicsItem, or 0.
     */
    PortGraphicsItem* getDestinationItem() const;

    /**
     * Informs this instance of a change in the layer. Necessary changes are done here
     * (e.g. changing the color based on the layer).
     * \param layer The \sa NetworkEditorLayer layer
     */
    void setLayer(NetworkEditorLayer layer);

    /**
     * Returns the tooltip containing information about the contained port. \sa HasToolTip::tooltip()
     * \return The tooltip which is ready to be added to a QGraphicsScene
     */
    QGraphicsItem* tooltip() const;

    /**
     * This method returns the old \sa PortGraphicsItem if this arrow is dragged away from a connection
     * the return value is 0 otherwise
     */
    PortGraphicsItem* getOldPortGraphicsItem() const;

    /**
     * This is used by the network editor to signal to the arrow that it is part of a bundle.
     */
    void setBundleInfo(bool enable, ConnectionBundle* bundle = 0);

    /**
     * Returns wether or not this arrow is part of a bundle in the network editor.
     */
    bool isBundled() const;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

    /// Returns the path describing this arrow.
    QPainterPath shape() const;

    /// Returns the center of the source item
    QPointF getSourcePoint() const;

    /// Returns the center of the destination item
    QPointF getDestinationPoint() const;

private:
    PortGraphicsItem* oldDestinationItem_; ///<< this item stores the old PortItem if this arrow is dragged away from a connection

    QPointF clickedPoint_;
    bool movedAwayInEvent_;

    ConnectionBundle* bundle_;
    bool bundled_;
};

} // namespace

#endif // VRN_PORTARROWGRAPHICSITEM_H
