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

#ifndef VRN_ARROWHEADSELECTIONGRAPHICSITEM_H
#define VRN_ARROWHEADSELECTIONGRAPHICSITEM_H

#include <QGraphicsItem>

#include "networkeditor_common.h"

namespace voreen {

class ArrowGraphicsItem;

class ArrowHeadSelectionGraphicsItem : public QGraphicsItem {
public:
    ArrowHeadSelectionGraphicsItem(ArrowGraphicsItem* parent = 0);

    /// The type of this QGraphicsItem subclass
    enum { Type = UserType + UserTypesArrowHeadSelectionGraphicsItem };

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    int type() const;

    void setSize(qreal width, qreal height);
    QRectF boundingRect() const;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

private:
    ArrowGraphicsItem* parent_;
    qreal width_;
    qreal height_;
};

} // namespace

#endif // VRN_ARROWHEADSELECTIONGRAPHICSITEM_H
