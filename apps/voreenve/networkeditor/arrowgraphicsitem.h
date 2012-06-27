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

#ifndef VRN_ARROWGRAPHICSITEM_H
#define VRN_ARROWGRAPHICSITEM_H

#include <QGraphicsItem>

#include "hastooltip.h"
#include "networkeditor_common.h"

namespace voreen {

class ArrowHeadSelectionGraphicsItem;

/**
 * This is an abstract base class designed to be used for all arrow-like QGraphicsItems. These
 * arrows must start at a specific other QGraphicsItem and can end either in a QPointF or another
 * QGraphicsItem.
 * The arrow can have a specific normal color (\sa setNormalColor(const QColor&)), a selectedColor (
 * \sa setSelectedColor(const QColor&) ), a general tooltip (\sa setTooltipText(const QString&)) or
 * a custom designed tooltip (derive and overwrite \sa tooltip()).
 * Both ends of the arrow can have a head which can be modified by derived classes. The variables
 * are \sa sourceHeadDirection_ and \sa destinationHeadDirection_.
 */
class ArrowGraphicsItem : public QGraphicsItem, public HasToolTip {
    friend class ArrowHeadSelectionGraphicsItem;

public:
    /**
     * Constructor for a ArrowGraphicsItem. Will make the item selectable, allows hover
     * events to arrive, disables arrow heads on both ends, sets the colors to
     * black (normal) and red (selected) and the default arrowhead size to 10.
     * \param sourceItem The source item from which the arrow originates. Must be != 0.
     * \param destinationItem The possible destination item or 0 if no such item exits
     */
    ArrowGraphicsItem(QGraphicsItem* sourceItem, QGraphicsItem* destinationItem = 0);

    /**
     * Destructor. Will delete the image if it exists.
     */
    ~ArrowGraphicsItem();

    /// The type of this QGraphicsItem subclass
    enum { Type = UserType + UserTypesArrowGraphicsItem };

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
     * Sets the passed item as the new source item. Will trigger an update of the arrow.
     * \param item The new source item. May not be 0.
     */
    void setSourceItem(QGraphicsItem* item);

    /**
     * Sets the passed item as the new destination item. Will trigger an update of the arrow.
     * If 0 is passed, the outcome might be undefined.
     * \param item The new destination item. Should not be 0.
     */
    void setDestinationItem(QGraphicsItem* item);

    /**
     * Sets the destination point to the passed \sa QPointF. Will set the destination item to 0.
     * \param destinationPoint The new point to which this arrow should point
     */
    void setDestinationPoint(const QPointF& destinationPoint);

    /**
     * Exchanges the source and destination items if the destination item is != 0.
     * If the destination is 0, nothing happens. Will trigger a repaint.
     */
    void swapItems();

    /**
     * Returns the tooltip containing information about the contained port. \sa HasToolTip::tooltip()
     * If \sa setTooltipText(const QString&) has been called with a non-empty string, a tooltip based
     * on that string is shown instead of this method.
     * \return The tooltip which is ready to be added to a QGraphicsScene
     */
    virtual QGraphicsItem* tooltip() const;

    /**
     * Sets the color for the normal state (i.e. not selected) of the arrow
     * \param color The color
     */
    void setNormalColor(const QColor& color);

    /**
     * Sets the color for the selected state of the arrow
     * \param color The color
     */
    void setSelectedColor(const QColor& color);

    /**
     * Sets the image which will be drawn in the middle of the arrow. Triggers a repaint.
     * \param image The image which will be drawn
     */
    void setImage(QPixmap* image);

    /**
     * Will set a basic tooltip text. If a non-empty string is passed, \sa tooltip() will be called
     * and the result will be used instead.
     * \param tooltip The tooltip text which should be shown
     */
    void setTooltipText(const QString& tooltip);

protected:
    /**
     * This enum is used for determining from where to where a arrow head is drawn.
     * E.g. ArrowHeadDirectionNS means a downward facing arrow head with the broad side on the
     * top and the arrow head - head at the buttom.
     */
    enum ArrowHeadDirection {
        ArrowHeadDirectionNone,
        ArrowHeadDirectionNS,
        ArrowHeadDirectionSN,
        ArrowHeadDirectionWE,
        ArrowHeadDirectionEW
    };

    /**
     * Must return the shape of the arrow. Should be a closed path because otherwise rendering
     * or selection artifacts might appear.
     */
    virtual QPainterPath shape() const = 0;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    /**
     * Computes the anchor point of the arrow in the source item. Because this is heavily
     * dependent on the type of the source item, it should be reimplemented in derived
     * classes which have more information about the type of the item.
     * \return The point from which the arrow will originate
     */
    virtual QPointF getSourcePoint() const;

    /**
     * Computes the anchor point of the arrow in the destination item. Because this is
     * heavily dependent on the type of the item, it should be reimplemented in derived
     * classes which have more information about the type of the item.
     * \return The point to which the arrow will point
     */
    virtual QPointF getDestinationPoint() const;

    /**
     * Will create the arrow head based on the direction and the base point. Will create a special
     * item needed for better selectabiliy. Only used internally in the \sa paint method.
     * \param direction The direction in which the arrow head will point
     * \param basePoint The base point from which the arrow head will be drawn. Should always be
     * the last point of the arrow (i.e. the arrowhead-head)
     * \param arrowHeadItem The item which makes the arrow head selectable by passing the different
     * mouse events to the arrow
     */
    QPolygonF createArrowHead(ArrowHeadDirection direction, const QPointF& basePoint, ArrowHeadSelectionGraphicsItem* arrowHeadItem) const;

    /**
     * The source item from which this arrow will originate. Must be != 0.
     */
    QGraphicsItem* sourceItem_;

    /**
     * The destination item to which this arrow will point. If it is 0, \sa destinationPoint_ is used
     * instead.
     */
    QGraphicsItem* destinationItem_;

    /**
     * The destination point to which this arrow will point. This point will be used if
     * \sa destinationItem_ is 0. Might be in an undefined state if \sa setDestinationItem(QGraphicsItem*)
     * is called will a 0 pointer.
     */
    QPointF destinationPoint_;

    QColor normalColor_; ///< The color this arrow will use for the normal state (i.e. not selected)
    QColor selectedColor_; ///< The color this arrow will use for the selected state

    /**
     * This image will be drawn in the middle of the arrow if it is not 0. If this variable is 0,
     * nothing happens instead.
     */
    QPixmap* image_;

    /**
     * This variable will store the base text if a basic tooltip text is set via \sa setToolTipText(const QString&)
     * if this string is empty, \sa tooltip() will be called and the result be used instead.
     */
    QString tooltipText_;

    qreal arrowHeadSize_; ///< contains the size of the arrow head

    /**
     * Stores the direction of the head located on the source side of the arrow. This must be mutable because
     * it will be modified in some \sa paint methods.
     */
    mutable ArrowHeadDirection sourceHeadDirection_;

    /**
     * Stores the direction of the head located on the destination side of the arrow. This must be mutable
     * because it will be modified in some \sa paint methods.
     */
    mutable ArrowHeadDirection destinationHeadDirection_;

    /// This object contains the mouse focus "grabber" for the arrow head at the source point (if there is any)
    mutable ArrowHeadSelectionGraphicsItem* sourceSelectionItem_;

    /// This object contains the mouse focus "grabber" for the arrow head at the source point (if there is any)
    mutable ArrowHeadSelectionGraphicsItem* destinationSelectionItem_;
};

} // namespace voreen

#endif // VRN_ARROWGRAPHICSITEM_H
