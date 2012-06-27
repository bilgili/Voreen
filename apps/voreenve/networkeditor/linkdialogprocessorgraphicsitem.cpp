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

#include "linkdialogprocessorgraphicsitem.h"

#include <QBrush>
#include <QGradient>
#include <QPainter>

namespace {
    const qreal drawingRectMinimumWidth = 100.0;
    const qreal drawingRectMinimumHeight = 60.0;

    const qreal drawingRectWidthOffset = 30.0;
    const qreal drawingRectHeightOffset = 10.0;

    const QColor baseColor = QColor(50, 50, 50, 255);
    const QColor selectedColor = Qt::red;
    const QColor highlightColor = Qt::blue;
    const QColor shadowColor = Qt::black;
}

namespace voreen {

LinkDialogProcessorGraphicsItem::LinkDialogProcessorGraphicsItem(const QString& name)
    : QGraphicsItem()
    , textItem_(name, this)
{
    qreal x = (boundingRect().width() / 2.f) - (textItem_.boundingRect().width() / 2.f);
    qreal y = (boundingRect().height() / 2.f) - (textItem_.boundingRect().height() / 2.f);
    textItem_.setPos(x, y);
}

int LinkDialogProcessorGraphicsItem::type() const {
    return Type;
}

QRectF LinkDialogProcessorGraphicsItem::boundingRect() const {
    QRectF rect = textItem_.boundingRect();

    qreal width = rect.width();
    qreal height = rect.height();

    width += drawingRectWidthOffset;
    height += drawingRectHeightOffset;

    if (width < drawingRectMinimumWidth)
        width = drawingRectMinimumWidth;

    if (height < drawingRectMinimumHeight)
        height = drawingRectMinimumHeight;

    rect.setWidth(width);
    rect.setHeight(height);

    return rect;
}

void LinkDialogProcessorGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    QRectF button_rect = boundingRect();

    painter->drawRect(button_rect);

    //gradient
    QLinearGradient gradient(0, 0, 0, button_rect.height());
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, baseColor);
    gradient.setColorAt(0.4, shadowColor);
    gradient.setColorAt(0.6, shadowColor);
    gradient.setColorAt(1.0, baseColor);

    QBrush brush(gradient);
    painter->setBrush(brush);
    painter->setPen(QPen(QBrush(baseColor), 2.0));

    painter->drawRect(button_rect);

    //glass highlight
    painter->setBrush(QBrush(Qt::white));
    painter->setPen(QPen(QBrush(Qt::white), 0.01));
    painter->setOpacity(0.30);
    button_rect.setHeight(button_rect.height()/2.0);
    painter->drawRect(button_rect);
}

} // namespace
