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

#include "connectionbundle.h"
#include "connectionbundlehandle.h"
#include "networkeditor.h"
#include "portarrowgraphicsitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QStyle>

namespace {

const QColor baseColor = QColor(50, 50, 0, 255);
const QColor selectedColor = Qt::red;
const QColor highlightColor = Qt::blue;
const QColor shadowColor = Qt::black;

} // namespace

namespace voreen {

ConnectionBundleHandle::ConnectionBundleHandle(ConnectionBundle* bundle) : QGraphicsRectItem(0), bundle_(bundle), detachedFromDefault_(false) {
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    //setAcceptsHoverEvents(true);
    setZValue(ZValueRootGraphicsItemNormal);
}

int ConnectionBundleHandle::type() const {
    return Type;
}

void ConnectionBundleHandle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) {
    QRectF button_rect = boundingRect();

    QColor button_color = baseColor;

    // frame indicates selected process
    if (option->state & QStyle::State_Selected) {
        button_color = selectedColor;
#ifndef __GNUC__
        setZValue(ZValueRootGraphicsItemMouseOver);
#endif
    }
    // hover effect
    else if (option->state & QStyle::State_MouseOver){
        button_color = highlightColor;
#ifndef __GNUC__
        setZValue(ZValueRootGraphicsItemMouseOver);
#endif
    }
    else {
#ifndef __GNUC__
         setZValue(ZValueRootGraphicsItemNormal);
#endif
    }

    float xrad = 5.f;
    float yrad = 5.f;
    painter->drawRoundedRect(button_rect, xrad, yrad);

    //gradient
    QLinearGradient gradient(0, 0, 0, button_rect.height());
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, button_color);
    gradient.setColorAt(0.4, shadowColor);
    gradient.setColorAt(0.6, shadowColor);
    gradient.setColorAt(1.0, button_color);

    QBrush brush(gradient);
    painter->setBrush(brush);
    painter->setPen(QPen(QBrush(button_color), 2.0));

    painter->setOpacity(0.8f);

    painter->drawRoundedRect(button_rect, xrad, yrad);

    //glass highlight
    painter->setBrush(QBrush(Qt::white));
    painter->setPen(QPen(QBrush(Qt::white), 0.01));
    painter->setOpacity(0.30);
    button_rect.setHeight(button_rect.height()/2.0);
    painter->drawRoundedRect(button_rect, xrad, yrad);
}

void ConnectionBundleHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsRectItem::mouseMoveEvent(event);
    detachedFromDefault_ = true;
    bundle_->updateScene();
}

} // namespace voreen

