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
     * Constructor. \sa ArrowGraphicsItem::ArrowGraphicsItem</br>
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

protected:
    /// Returns the path describing this arrow.
    QPainterPath shape() const;

    /// Returns the center of the source item
    QPointF getSourcePoint() const;

    /// Returns the center of the destination item
    QPointF getDestinationPoint() const;
};

} // namespace

#endif // VRN_PORTARROWGRAPHICSITEM_H
