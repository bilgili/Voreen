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

#include "voreen/qt/widgets/network/editor/propertylistgraphicsitem.h"

#include "voreen/core/vis/processors/processor.h"
#include "voreen/qt/voreenqtglobal.h"
#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/propertygraphicsitem.h"

namespace {
    qreal processorWidthToPropertyListWidthFactor = 0.9f;
}

namespace voreen {

PropertyListGraphicsItem::PropertyListGraphicsItem(ProcessorGraphicsItem* parent)
    : QObject()
    , QGraphicsItem(parent)
    , parent_(parent)
{
    tgtAssert(parent != 0, "passed null pointer");
    setZValue(ZValuePropertyListGraphicsItem);
    hide();
    createPropertyGraphicsItems();
}

void PropertyListGraphicsItem::setVisible(bool visible) {
    foreach (PropertyGraphicsItem* propertyItem, propertyGraphicsItems_) {
        propertyItem->setVisible(visible);
    }

    QGraphicsItem::setVisible(visible);
}

QRectF PropertyListGraphicsItem::boundingRect() const {
    qreal width = parent_->drawingRect().width() * processorWidthToPropertyListWidthFactor;

    qreal height = 0.f;

    foreach (PropertyGraphicsItem* propertyItem, propertyGraphicsItems_) {
        height += propertyItem->boundingRect().height();
    }

    return QRect(0, 0, static_cast<int>(width), static_cast<int>(height));
}

PropertyGraphicsItem* PropertyListGraphicsItem::getProperty(Property* property) {
    foreach (PropertyGraphicsItem* propertyItem, propertyGraphicsItems_) {
        if (propertyItem->getProperty() == property)
            return propertyItem;
    }

    return 0;
}

QList<PropertyGraphicsItem*>& PropertyListGraphicsItem::getPropertyGraphicsItems() {
    return propertyGraphicsItems_;
}

void PropertyListGraphicsItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}

void PropertyListGraphicsItem::createPropertyGraphicsItems() {
    qreal currentHeight = 0.f;
    const std::vector<Property*> properties = parent_->getProcessor()->getProperties();

    for (size_t i = 0; i < properties.size(); ++i) {
        Property* prop = properties[i];
        //if (!prop->isVisible())
        //    continue;

        PropertyGraphicsItem* propertyItem = new PropertyGraphicsItem(prop, parent_, this);
        propertyGraphicsItems_.push_back(propertyItem);
        QObject::connect(propertyItem, SIGNAL(pressed(PropertyGraphicsItem*)), parent_, SIGNAL(pressedPropertyGraphicsItem(PropertyGraphicsItem*)));
        propertyItem->setPos(0, currentHeight);
        currentHeight += propertyItem->boundingRect().height();
    }
}

} // namespace
