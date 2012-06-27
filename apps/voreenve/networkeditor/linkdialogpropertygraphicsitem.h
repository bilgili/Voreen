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

#ifndef VRN_LINKDIALOGGRAPHICSITEM_H
#define VRN_LINKDIALOGGRAPHICSITEM_H

#include "propertygraphicsitem.h"
#include "networkeditor_common.h"

namespace voreen {

class LinkDialogArrowGraphicsItem;
class PropertyLinkDialog;

enum ColumnPosition {
    ColumnPositionLeft,
    ColumnPositionCenter,
    ColumnPositionRight
};

/**
 * This class serves as a visual representation of a \sa Property in a
 * \sa PropertyLinkDialog. Its purpose is the source and destination
 * of \sa LinkDialogArrowGraphicsItem which it will create in the
 * <code>mousePressEvent(QGraphicsSceneMouseEvent*)</code> method. If
 * this arrow is pulled over another LinkDialogPropertyGraphicsItem,
 * it will determine if the associated \sa Propertys are linkable.
 * If so, the arrow will be drawn in green color, if not the red color
 * is used.
 * If a valid arrow is created, this arrow will be published via the
 * <code>createdArrow(LinkDialogArrowGraphicsItem*)</code> signal.
 */
class LinkDialogPropertyGraphicsItem : public PropertyGraphicsItem {
Q_OBJECT
    friend class LinkDialogArrowGraphicsItem;

public:
    /**
     * Simple constructor
     * \param prop The Property to which this item belongs to
     * \param position Where should the new item be positioned
     */
    LinkDialogPropertyGraphicsItem(const Property* prop, ColumnPosition position);

    /// The type of this QGraphicsItem subclass
    int type() const;

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    enum {Type = UserType + UserTypesLinkDialogPropertyGraphicsItem};

    /// Returns the position of the LinkDialogPropertyGraphicsItem
    ColumnPosition getPosition() const;

    void setDelegate(PropertyLinkDialog* dialog);

    void setCurrentArrow(LinkDialogArrowGraphicsItem* arrow);

signals:
    /**
     * This signal is emitted as soon as a valid LinkDialogArrowGraphicsItem
     * is created. The ownership of this arrow is transferred as well.
     * \param arrow The newly created LinkDialogArrowGraphicsItem
     */
    void createdArrow(LinkDialogArrowGraphicsItem* arrow);

    /**
     * This signal is emitted each time an arrow is moved within the scene it belongs
     * to.
     * \param arrow The moved LinkDialogArrowGraphicsItem
     */
    void movedArrow(LinkDialogArrowGraphicsItem* arrow);

    /**
     * This signal is emitted as soon as the creation of a new arrow is aborted,
     * whether it was successful or not
     */
    void endedArrow();

    /**
     * This signal is emitted if a \sa LinkDialogArrowGraphicsItem is dragged
     * away from its existing connection and dropped someplace without a new
     * connection.
     * \param arrow The arrow which is to be deleted
     */
    void deleteArrow(QGraphicsItem* arrow);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

private:
    PropertyLinkDialog* delegate_;

    LinkDialogArrowGraphicsItem* currentArrow_;
    ColumnPosition position_;
    QRectF oldBounds_;
};

} // namespace

#endif // VRN_LINKDIALOGGRAPHICSITEM_H
