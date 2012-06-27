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

#ifndef VRN_PROPERTYGRAPHICSITEM_H
#define VRN_PROPERTYGRAPHICSITEM_H

#include <QObject>
#include <QGraphicsItem>
#include "voreen/core/properties/property.h"

#include "networkeditor_common.h"

namespace voreen {

class Property;
class PropertyListGraphicsItem;
class RootGraphicsItem;

/**
 * This class represents a single \sa Property belonging to a specific \sa RootGraphicsItem
 * It has a string with the GUI name of the property in it and provides to docking points
 * for incoming or outgoing \sa LinkArrowGraphicsItems
 */
#if (QT_VERSION >= 0x040600)
class PropertyGraphicsItem : public QGraphicsObject {
#else
class PropertyGraphicsItem : public QObject, public QGraphicsItem {
#endif
Q_OBJECT
public:
    /**
     * Constructor of an instance. Creates the child items.
     * \param prop The property which this PropertyGraphicsItem represents. Must be != 0
     * \param parent The parent \sa PropertyListGraphicsItem of this PropertyGraphicsItem. May be 0
     */
    PropertyGraphicsItem(const Property* prop, PropertyListGraphicsItem* parent);

    /// The type of this QGraphicsItem subclass
    enum {Type = UserType + UserTypesPropertyGraphicsItem};

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
     * Resets the size of this item. Needs to be called if the size of the parent RootGraphicsItem
     * changes after creation of this item.
     */
    void resetSize();

    /**
     * Returns the \sa RootgraphicsItem to which this PropertyGraphicsItem belongs
     * \return The RootGraphicsItem to which this PropertyGraphicsItem belongs
     */
    RootGraphicsItem* getRootGraphicsItem() const;

    /**
     * Returns the property which is represented by this PropertyGraphicsItem
     * \return The property which is represented by this PropertyGraphicsItem
     */
    const Property* getProperty() const;

    /**
     * Computes and returns the left docking point used for \sa LinkArrowGraphicsItems.
     * This point lies in the center of the left border of this object.
     * \return The left docking point position
     */
    QPointF getLeftDockingPoint() const;

    /**
     * Computes and returns the right docking point used for \sa LinkArrowGraphicsItems.
     * This point lies in the center of the right border of this object.
     * \return The right docking point position
     */
    QPointF getRightDockingPoint() const;

    /**
     * Adds a prefix in front the the Guiname of the \sa Property
     * \param prefix The prefix without any delimiters
     */
    void addPrefix(const QString& prefix);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
    /// Creates the label and sets it with an initial value
    void createLabel();
    void createLabel(const QString& prefix);

    /// The property which is represented by this PropertyGraphicsItem
    const Property* property_;
    /// The parent item in which this PropertyGraphicsItem is organized, or 0
    PropertyListGraphicsItem* propertyListGraphicsItem_;

    /// The text label with the name and the type of the property
    QGraphicsTextItem* propertyLabel_;
};

} //namespace voreen

#endif // VRN_PROPERTYGRAPHICSITEM_H
