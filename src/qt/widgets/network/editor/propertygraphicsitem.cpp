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

#include "voreen/qt/widgets/network/editor/propertygraphicsitem.h"

#include "voreen/core/vis/properties/allproperties.h"
#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/propertylistgraphicsitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace {
    // qreal spacingBetweenBoundingBoxAndLabel = 5.f;
    // qreal spacingBetweenLabels = 5.f;

}

namespace voreen {

PropertyGraphicsItem::PropertyGraphicsItem(Property* prop, ProcessorGraphicsItem* processorGraphicsItem, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , property_(prop)
    , backgroundcolor_(Qt::white)
    , activeColor_(QColor(210, 248, 255))
    , processorGraphicsItem_(processorGraphicsItem)
    , propertyListGraphicsItem_(qgraphicsitem_cast<PropertyListGraphicsItem*>(parent))
{
    createLabel();
    setAcceptsHoverEvents(true);
    setFlag(ItemIsSelectable);
}

void PropertyGraphicsItem::createLabel() {
    QString labelText = "";
    labelText.append(QString::fromUtf8(property_->getGuiText().c_str()));
    labelText.append("<br><\br>");
     //get the datatyp of the property
    QString type;
    if (typeid(BoolProperty) == typeid(*property_)) {
        type = "bool";
    }
    else if (typeid(ColorProperty) == typeid(*property_)) {
        type = "Color";
    }
    else if (typeid(FileDialogProperty) == typeid(*property_)) {
        type = "FileDialog";
    }
    else if (typeid(FloatProperty) == typeid(*property_)) {
        type = "float";
    }
    else if (typeid(IntProperty) == typeid(*property_)) {
        type = "int";
    }
    else if (typeid(StringProperty) == typeid(*property_)) {
        type = "string";
    }
    else if (typeid(FloatVec2Property) == typeid(*property_)) {
        type = "FloatVec2";
    }
    else if (typeid(FloatVec3Property) == typeid(*property_)) {
        type = "FloatVec3";
    }
    else if (typeid(FloatVec4Property) == typeid(*property_)) {
        type = "FloatVec4";
    }
    else if (typeid(IntVec2Property) == typeid(*property_)) {
        type = "IntVec2";
    }
    else if (typeid(IntVec3Property) == typeid(*property_)) {
        type = "IntVec3";
    }
    else if (typeid(IntVec4Property) == typeid(*property_)) {
        type = "IntVec4";
    }
    else if (typeid(CameraProperty) == typeid(*property_)) {
        type = "Camera";
    }
    else if (typeid(TransFuncProperty) == typeid(*property_)) {
        type = "TransFunc";
    }
    else if (dynamic_cast<OptionPropertyBase*>(property_)) {
        type = "Option";
    }
    else {
        type = "Unsupported property type";
    }

    QString typeLine("<span style=\"font-size:7pt;color:#BDBDBD\"> %1 </span>");
    labelText.append(typeLine.arg(type));

    propertyLabel_ = new QGraphicsTextItem(this);
    propertyLabel_->setHtml(labelText);

    propertyLabel_->setTextWidth(propertyListGraphicsItem_->boundingRect().width());
}

void PropertyGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* ) {
    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(backgroundcolor_);
    if (option->state & QStyle::State_MouseOver) {
        painter->setBrush(activeColor_);
    }
    painter->drawRect(boundingRect());
}

QRectF PropertyGraphicsItem::boundingRect() const {
    QRectF rect = propertyLabel_->boundingRect();
    return rect;
}

void PropertyGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // change the backgroundcolor
    backgroundcolor_ = activeColor_;

    // signal to the networkeditor
    emit pressed(this);

    QGraphicsItem::mousePressEvent(event);
}

void PropertyGraphicsItem::deselect() {
    backgroundcolor_ = Qt::white;
}

// getter and setter fpr private variables

ProcessorGraphicsItem* PropertyGraphicsItem::getProcessorGraphicsItem(){
    return processorGraphicsItem_;
}

Property* PropertyGraphicsItem::getProperty(){
    return property_;
}

QPointF PropertyGraphicsItem::getLeftDockingPoint() const {
    return mapToScene(QPointF(boundingRect().x(), boundingRect().height() / 2.f));
}

QPointF PropertyGraphicsItem::getRightDockingPoint() const {
    return mapToScene(QPointF(boundingRect().width(), boundingRect().height() / 2.f));
}

} // namespace
