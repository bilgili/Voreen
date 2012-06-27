/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#ifndef VRN_LINKARROWGRAPHICSITEM_H
#define VRN_LINKARROWGRAPHICSITEM_H

#include <QGraphicsItem>
#include "voreen/qt/widgets/network/editor/itooltip.h"
#include "voreen/qt/voreenqtglobal.h"

namespace voreen {

class PropertyLink;
class PropertyGraphicsItem;

class LinkArrowGraphicsItem : public QGraphicsItem, public ITooltip {
public:
    LinkArrowGraphicsItem(PropertyGraphicsItem* propertyItem1, PropertyGraphicsItem* propertyItem2, const PropertyLink* link);

    enum { Type = UserType + UserTypesLinkArrowGraphicsItem };

    int type() const;

    void adjust();

    QRectF boundingRect() const;

    QGraphicsItem* tooltip() const;

    PropertyGraphicsItem* firstPropertyItem() const;
    PropertyGraphicsItem* secondPropertyItem() const;

    const PropertyLink* getLink() const;
    const PropertyLink* getSecondLink() const;

    bool hasSecondLink() const;

    void setSecondLink(const PropertyLink* link);
    void switchLinks();

protected:
    QPainterPath shape() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*);

private:
    enum LinkArrowState {
        LinkArrowStatePropertyToProperty,
        LinkArrowStatePropertyToProcessor,
        LinkArrowStateProcessorToProperty,
        LinkArrowStateProcessorToProcessor
    };

    enum DockPositions {
        DockPositionsLeftLeft,
        DockPositionsLeftRight,
        DockPositionsRightLeft,
        DockPositionsRightRight
    };

    LinkArrowState getState() const;
    DockPositions getDockPositions() const;

    PropertyGraphicsItem* propertyItem1_;
    PropertyGraphicsItem* propertyItem2_;

    const PropertyLink* link_;
    const PropertyLink* secondLink_; ///< a single LinkArrowGraphicsItem is used for a transparant bidirectional link

    QColor color_;
};

} // namespace

#endif // VRN_LINKARROWGRAPHICSITEM_H
