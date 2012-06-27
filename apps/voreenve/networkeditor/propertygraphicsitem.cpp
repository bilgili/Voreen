/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "propertygraphicsitem.h"

#include "voreen/core/properties/allproperties.h"
#include "propertylistgraphicsitem.h"
#include "rootgraphicsitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace voreen {

PropertyGraphicsItem::PropertyGraphicsItem(const Property* prop, PropertyListGraphicsItem* parent)
#if (QT_VERSION >= 0x040600)
    : QGraphicsObject(parent)
#else
    : QObject()
    , QGraphicsItem(parent)
#endif
    , property_(prop)
    , propertyListGraphicsItem_(parent)
{
    tgtAssert(prop != 0, "passed null pointer");
    createLabel();
}

void PropertyGraphicsItem::createLabel() {
    QString labelText = "";
    labelText.append(QString::fromStdString(property_->getGuiName()));
    labelText.append("<br><\br>");
    QString type = QString::fromStdString(Property::getPropertyTypeText(property_));

    QString typeLine("<span style=\"font-size:7pt;color:#BDBDBD\"> %1 </span>");
    labelText.append(typeLine.arg(type));

    propertyLabel_ = new QGraphicsTextItem(this);
    propertyLabel_->setHtml(labelText);

    resetSize();
}

int PropertyGraphicsItem::type() const {
    return Type;
}

QRectF PropertyGraphicsItem::boundingRect() const {
    QRectF rect = propertyLabel_->boundingRect();
    return rect;
}

void PropertyGraphicsItem::resetSize() {
    if (propertyListGraphicsItem_)
        propertyLabel_->setTextWidth(propertyListGraphicsItem_->boundingRect().width());
    else
        propertyLabel_->setTextWidth(200.f);
}


void PropertyGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(Qt::white);
    painter->drawRect(boundingRect());
}

RootGraphicsItem* PropertyGraphicsItem::getRootGraphicsItem() const {
    return propertyListGraphicsItem_->getParent();
}

const Property* PropertyGraphicsItem::getProperty() const {
    return property_;
}

QPointF PropertyGraphicsItem::getLeftDockingPoint() const {
    return mapToScene(QPointF(boundingRect().x(), boundingRect().height() / 2.f));
}

QPointF PropertyGraphicsItem::getRightDockingPoint() const {
    return mapToScene(QPointF(boundingRect().width(), boundingRect().height() / 2.f));
}

} // namespace
