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

#ifndef VRN_PROPERTYLISTGRAPHICSITEM_H
#define VRN_PROPERTYLISTGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QObject>

namespace voreen {

class LinkArrowGraphicsItem;
class Property;
class PropertyGraphicsItem;
class RootGraphicsItem;

/**
 * This object is responsible for organizing and layouting of the different \sa PropertyGraphicsItems
 * of a \sa RootGraphicsItem. Each PropertyGraphicsItem will be aligned and of the same size and will
 * appear in a regular list.
 */
#if (QT_VERSION >= 0x040600)
class PropertyListGraphicsItem : public QGraphicsObject {
#else
class PropertyListGraphicsItem : public QObject, public QGraphicsItem {
#endif
Q_OBJECT
public:
    /**
     * Constructor of an instance.
     * \param parent The parent RootGraphicsItem. Must be != 0
     */
    PropertyListGraphicsItem(RootGraphicsItem* parent);

    /// Destructor. Will delete all contained \sa PropertyGraphicsItems
    ~PropertyListGraphicsItem();

    /// Will resize all child items. Needs to be called if the size of the parent item has changed
    void resizeChildItems();

    /**
     * The bounding rect of this PortGraphicsItem. \sa QGraphicsItem::boundingRect()
     * \return The bounding rect
     */
    QRectF boundingRect() const;

    /**
     * Will propagate -in turn- the visibility to all \sa PropertyGraphicsItems
     * \param visible Should all PropertyGraphicsItems be visible, or not
     */
    void setVisible(bool visible);

    /**
     * Adds a list of already existing \sa PropertyGraphicsItems to this object. The ownership
     * is transferred as well, so this PropertyListGraphicsItem will delete these
     * PropertyGraphicsItems
     * \param items A list of PropertyGraphicsItems which should be added to this list
     */
    void addPropertyGraphicsItems(const QList<PropertyGraphicsItem*>& items);

    /**
     * Removes a \sa PropertyGraphicsItem from this list and reorders the other PropertyGraphicsItems
     * accordingly (i.e. if a piece in the middle was removed). Will also delete the PropertyGraphicsItem.
     * The PropertyGraphicsItem must be present in the list however.
     * \param item The PropertyGraphicsItem which should be removed
     */
    void removePropertyGraphicsItem(PropertyGraphicsItem* item);

    /**
     * If a PropertyGraphicsItem for the passed Property already exists, it is returned. Otherwise
     * a new PropertyGraphicsItem is created and it will be return in its stead. In the latter case, it
     * will be added to this list as well.
     * \param prop The property for which the PropertyGraphicsItem should be returned
     */
    PropertyGraphicsItem* getPropertyGraphicsItem(const Property* prop);

    /**
     * Returns the parent \sa RootGraphicsItem
     * \return The parent RootGraphicsItem
     */
    RootGraphicsItem* getParent() const;

protected:
     void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
    /// The parent item of this PropertyListGraphicsItem
    RootGraphicsItem* parent_;

    /// The list of the contained ProertyGraphicsItems
    QList<PropertyGraphicsItem*> propertyGraphicsItems_;

    /// The summed heights of each of the contained PropertyGraphicsItems
    qreal currentHeight_;
};

} // namespace

#endif // VRN_PROPERTYLISTGRAPHICSITEM_H
