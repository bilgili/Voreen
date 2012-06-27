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

#ifndef VRN_PROPERTYGRAPHICSITEM_H
#define VRN_PROPERTYGRAPHICSITEM_H

#include <QObject>
#include <QGraphicsItem>
#include "voreen/core/vis/properties/property.h"

#include "voreen/qt/voreenqtglobal.h"

namespace voreen {

class ProcessorGraphicsItem;
class Property;
class PropertyListGraphicsItem;

class PropertyGraphicsItem : public QObject, public QGraphicsItem {    
    Q_OBJECT
#if (QT_VERSION >= 0x040600)
    Q_INTERFACES(QGraphicsItem)    
#endif
public:
    PropertyGraphicsItem(Property* prop, ProcessorGraphicsItem* processorGraphicsItem,
                         QGraphicsItem* parent);

    QRectF boundingRect() const;

    ProcessorGraphicsItem* getProcessorGraphicsItem();

    Property* getProperty();

    void deselect();

    int type() const { return Type; }
    enum {Type = UserType + UserTypesPropertyGraphicsItem};

    QPointF getLeftDockingPoint() const;
    QPointF getRightDockingPoint() const;

signals:
    void pressed(PropertyGraphicsItem* propertyGraphicsItem);

protected:
    void createLabel();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    Property* property_;
    QGraphicsTextItem* propertyLabel_;
    QColor backgroundcolor_;
    QColor activeColor_;
    ProcessorGraphicsItem* processorGraphicsItem_;
    PropertyListGraphicsItem* propertyListGraphicsItem_;
};


} //namespace voreen

#endif // VRN_PROPERTYGRAPHICSITEM_H
