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

#include "widgetindicatorbutton.h"

#include "networkeditor_common.h"
#include "rootgraphicsitem.h"
#include "voreen/core/processors/processorwidget.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace {
    // the default color
    const QColor standardColor = Qt::darkGray;
    // the normal hovering color
    const QColor hoverColor = QColor(Qt::darkGray).darker()/* QColor(44,44,114,255)*/;
    // the color if the button is pressed
    const QColor sunkenColor = QColor(32,181,230,128);

    const QColor someWidgetsVisible = QColor(245, 220, 120, 175);
    const QColor allWidgetsVisible = QColor(46, 192, 217, 160);

    // size of the WidgetIndicator
    const QSize size = QSize(12, 8);
}

namespace voreen {

WidgetIndicatorButton::WidgetIndicatorButton(QGraphicsItem* parent)
#if (QT_VERSION >= 0x040600)
    : QGraphicsObject(parent)
#else
    : QObject()
    , QGraphicsItem(parent)
#endif
    , widgets_(QList<ProcessorWidget*>())
{
    setAcceptsHoverEvents(true);
    setToolTip(tr("show/hide processor widget"));
}

QRectF WidgetIndicatorButton::boundingRect() const {
    return QRect(0, 0, size.width(), size.height());
}

int WidgetIndicatorButton::type() const {
    return Type;
}

void WidgetIndicatorButton::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) {
    painter->setPen(QPen(Qt::black, 4));
    painter->drawRect(boundingRect());

    if (option->state & QStyle::State_MouseOver)
        painter->setPen(QPen(hoverColor, 2));
    else
        painter->setPen(QPen(standardColor, 2));

    int visibleWidgetNum = 0;
    foreach (ProcessorWidget* widget, widgets_) {
        if (widget->isVisible())
            visibleWidgetNum++;
    }

    if (visibleWidgetNum > 0) {
        if (visibleWidgetNum == widgets_.size())
            painter->setBrush(allWidgetsVisible);
        else
            painter->setBrush(someWidgetsVisible);
    }

    painter->drawRect(boundingRect());
}

void WidgetIndicatorButton::setProcessorWidget(ProcessorWidget* widget) {
    widgets_.clear();

    if (widget)
        widgets_.push_back(widget);
}

void WidgetIndicatorButton::setProcessorWidgets(const QList<ProcessorWidget*>& widgets) {
    widgets_ = widgets;
}

void WidgetIndicatorButton::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit pressed();
        update();
    } else
        QGraphicsItem::mousePressEvent(event);
}

} // namespace
