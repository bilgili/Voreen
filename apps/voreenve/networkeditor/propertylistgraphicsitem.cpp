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

#include "propertylistgraphicsitem.h"

#include "voreen/core/processors/processor.h"
#include "networkeditor_common.h"
#include "processorgraphicsitem.h"
#include "propertygraphicsitem.h"
#include "rootgraphicsitem.h"

namespace {
    qreal processorWidthToPropertyListWidthFactor = 0.9f;
}

namespace voreen {

PropertyListGraphicsItem::PropertyListGraphicsItem(RootGraphicsItem* parent)
#if (QT_VERSION >= 0x040600)
    : QGraphicsObject(parent)
#else
    : QObject()
    , QGraphicsItem(parent)
#endif
    , parent_(parent)
    , currentHeight_(0.f)
{
    tgtAssert(parent != 0, "passed null pointer");
    setZValue(ZValuePropertyListGraphicsItem);
}

PropertyListGraphicsItem::~PropertyListGraphicsItem() {
    foreach (PropertyGraphicsItem* item, propertyGraphicsItems_)
        delete item;
}

void PropertyListGraphicsItem::resizeChildItems() {
    foreach (PropertyGraphicsItem* item, propertyGraphicsItems_)
        item->resetSize();
}

QRectF PropertyListGraphicsItem::boundingRect() const {
    qreal width = parent_->boundingRect().width() * processorWidthToPropertyListWidthFactor;

    qreal height = 0.f;

    foreach (PropertyGraphicsItem* propertyItem, propertyGraphicsItems_)
        height += propertyItem->boundingRect().height();

    return QRect(0, 0, static_cast<int>(width), static_cast<int>(height));
}

void PropertyListGraphicsItem::setVisible(bool visible) {
    foreach (PropertyGraphicsItem* propertyItem, propertyGraphicsItems_) {
        propertyItem->setVisible(visible);
    }

    QGraphicsItem::setVisible(visible);
}

void PropertyListGraphicsItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}

void PropertyListGraphicsItem::addPropertyGraphicsItems(const QList<PropertyGraphicsItem*>& items) {
#if (QT_VERSION >= 0x040500)
    propertyGraphicsItems_.append(items);
#else
    foreach (PropertyGraphicsItem* item, items)
        propertyGraphicsItems_.append(item);
#endif

    foreach (PropertyGraphicsItem* item, items) {
        item->setPos(0, currentHeight_);
        currentHeight_ += item->boundingRect().height();
    }
}

void PropertyListGraphicsItem::removePropertyGraphicsItem(PropertyGraphicsItem* item) {
    currentHeight_ -= item->boundingRect().height();

    if (item == propertyGraphicsItems_.last()) {
        propertyGraphicsItems_.removeLast();
    }
    else {
        int index = propertyGraphicsItems_.indexOf(item);
        tgtAssert(index != -1, "PropertyGraphicsItem did not exist in the list");
        for (int i = index; i < propertyGraphicsItems_.count(); ++i)
            propertyGraphicsItems_[i]->moveBy(0, -item->boundingRect().height());
        propertyGraphicsItems_.removeOne(item);
    }

     delete item;
}

PropertyGraphicsItem* PropertyListGraphicsItem::getPropertyGraphicsItem(const Property* prop) {
    // if a PropertyGraphicsItem for the prop already exists, return that
    foreach (PropertyGraphicsItem* propertyItem, propertyGraphicsItems_) {
        if (propertyItem->getProperty() == prop)
            return propertyItem;
    }

    // otherwise, create a new one
    PropertyGraphicsItem* propertyItem = new PropertyGraphicsItem(prop, this);
    propertyGraphicsItems_.append(propertyItem);

    propertyItem->setPos(0, currentHeight_);
    currentHeight_ += propertyItem->boundingRect().height();
    return propertyItem;
}

RootGraphicsItem* PropertyListGraphicsItem::getParent() const {
    return parent_;
}

} // namespace
