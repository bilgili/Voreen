/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef VRN_RPTARROW_H
#define VRN_RPTARROW_H

#include <QtGui>
#include "voreen/core/vis/processors/networkevaluator.h"
#include "rptgraphwidget.h"

namespace voreen {

class RptPortItem;

/**
 * This class creates an arrow from a QGraphicsItem (sourceNode) pointing to another QGraphicsItem (destNode)
 * or to a QPointF position.
 */
class RptArrow : public QGraphicsItem, public HasRptTooltip {
public:
    /**
     * Constructor. Every Arrow has to have a source node, where it starts.
     */
    RptArrow(QGraphicsItem* sourceNode, QGraphicsItem* destNode=0);
    ~RptArrow();
    
    /**
     * Returns the item where the arrow starts.
     */
	QGraphicsItem* getSourceNode() { return source_; }
    
    /**
     * Returns the item the arrow is pointing on (may be 0).
     */
	QGraphicsItem* getDestNode() { return dest_; }

    /**
     * Sets the dest item.
     */
    void setDestNode(QGraphicsItem* node);

    /**
     * Adjusts the arrow if source- or dest-item has moved.
     * This function has only an effect if the arrow has both, a source- and a dest-node.
     */
    void adjust();
    
    /**
     * Adjusts the arrow so that it points to the given QPointF.
     */
    void adjust(QPointF dest);

    /**
     * Used to identfy an item in the scene as RptArrow
     */     
    enum { Type = UserType + 4 };

    /**
     * Returns the type that identifies this item as a RptArrow.
     */
    int type() const { return Type; }

    QRectF boundingRect() const;
    QPointF center() const;
    QPainterPath shape() const;
    
    /**
     * Shows the Texture Container Target Tooltip if available - returns this-Pointer in case
     * of success
     * TODO: Oh, think twice... Use Exceptions!
     */
    QGraphicsItem* rptTooltip() const;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /**
    * Returns the mapped texture container target for the port of the source port.
    * If no texture container target is mapped, -1 is returned.
    */
    int getSourceTextureContainerTarget(NetworkEvaluator* neteval) const;

private:
    // helps drawing smooth curves
    qreal deflection() const;

    // arrow goes from source_ to dest_
    QGraphicsItem* source_;
    QGraphicsItem* dest_;

    // start and end point of arrow
    QPointF sourcePoint_;
    QPointF destPoint_;

    // size of arrowhead
    qreal arrowSize_;

    QColor color_;
};

} // namespace voreen

#endif // VRN_RPTARROW_H
