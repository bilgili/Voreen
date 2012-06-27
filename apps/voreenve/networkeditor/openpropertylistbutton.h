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

#ifndef VRN_OPENPROPERTYLISTBUTTON_H
#define VRN_OPENPROPERTYLISTBUTTON_H

#include <QObject>
#include <QGraphicsItem>

#include "networkeditor_common.h"

namespace voreen {

/**
 * This QGraphicsObject derivative is used in a \sa RootGraphicsItem to serve as both a docking point
 * for \sa LinkArrowGraphicsItems and to toggle the visibility of the \sa PropertyListGraphicsItem
 */
#if (QT_VERSION >= 0x040600)
class OpenPropertyListButton : public QGraphicsObject {
#else
class OpenPropertyListButton : public QObject, public QGraphicsItem {
#endif
Q_OBJECT
public:
    /**
     * Constructor of an instance. Sets the item to accept hover events and sets a tooltip text
     * \param parent The parent item. Might be 0
     */
    OpenPropertyListButton(QGraphicsItem* parent);

    /// The type of this QGraphicsItem subclass
    enum {Type = UserType + UserTypesOpenPropertyListButton};

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    int type() const;

    /**
     * The bounding rect of this PortGraphicsItem. \sa QGraphicsItem::boundingRect()
     * \return The bounding rect
     */
    QRectF boundingRect() const;

    /**
     * Returns the docking point used by the \sa LinkArrowGraphicsItems
     * \return The docking point
     */
    QPointF dockingPoint() const;

signals:
     /// This signal will be emitted if this item was been clicked upon
     void pressed();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
};

} //namespace voreen

#endif // VRN_OPENPROPERTYLISTBUTTON_H

