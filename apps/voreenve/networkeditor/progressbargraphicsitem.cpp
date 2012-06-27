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

#include "progressbargraphicsitem.h"

#include <QApplication>
#include <QGraphicsScene>
#include <QLinearGradient>
#include <QPainter>

namespace voreen {

ProgressBarGraphicsItem::ProgressBarGraphicsItem(QGraphicsItem* parent, const QPointF& center, qreal width, qreal height)
    : QGraphicsItem(parent)
{
    resize(center, width, height);

    time_.start();
}

int ProgressBarGraphicsItem::type() const {
    return Type;
}

void ProgressBarGraphicsItem::resize(const QPointF& center, qreal width, qreal height) {
    boundingRect_ = QRectF(0, 0, width, height);
    qreal x = center.x() - width/2.0;
    qreal y = center.y() - height/2.0;

    setPos(x, y);
}

void ProgressBarGraphicsItem::show() {
    QGraphicsItem::show();
}

void ProgressBarGraphicsItem::hide() {
    QGraphicsItem::hide();
}

void ProgressBarGraphicsItem::forceUpdate() {
    update();
}

void ProgressBarGraphicsItem::update() {
    if (time_.elapsed() > 500 || progress_ == 1.f || progress_ == 0.f) {
        QGraphicsItem::update();
        scene()->invalidate();
        qApp->processEvents();
        time_.restart();
    }
}

QRectF ProgressBarGraphicsItem::boundingRect() const {
    return boundingRect_;
}

void ProgressBarGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    /*
        General Clip Path

   c1     1________________________________________________________4      c4
         /                                                          \
        |                                                            |
        |                                                            |
         \__________________________________________________________/
   c2     2                                                        3      c3

        c1-c4: control points for splines constructing the curved edges
    */

    QPainterPath clipPath;
    const qreal cutOffValue = 5.0;
    const qreal clipRatio = 5.0;
    const QPointF p1 = QPointF(boundingRect_.x() + cutOffValue, boundingRect_.y());
    const QPointF p2 = QPointF(boundingRect_.x() + cutOffValue, boundingRect_.bottom());
    const QPointF p3 = QPointF(boundingRect_.right() - cutOffValue, boundingRect_.bottom());
    const QPointF p4 = QPointF(boundingRect_.right() - cutOffValue, boundingRect_.y());

    const QPointF c1 = QPointF(p1.x() - clipRatio, p1.y());
    const QPointF c2 = QPointF(p2.x() - clipRatio, p2.y());
    const QPointF c3 = QPointF(p3.x() + clipRatio, p3.y());
    const QPointF c4 = QPointF(p4.x() + clipRatio, p4.y());

    clipPath.moveTo(p1);
    clipPath.cubicTo(c1, c2, p2);
    clipPath.lineTo(p3);
    clipPath.cubicTo(c3, c4, p4);
    clipPath.lineTo(p1);
    //clipPath.addRoundedRect(boundingRect_, 4.0, 4.0);
    painter->setClipPath(clipPath);

    // paint the background
    QLinearGradient upperBackgroundGradient(boundingRect_.x(), boundingRect_.y(), boundingRect_.x(), boundingRect_.y() + boundingRect_.height() / 2.0);
    //upperBackgroundGradient.setColorAt(0.0, QColor(163, 163, 163));
    //upperBackgroundGradient.setColorAt(1.0, QColor(208, 208, 208));
    upperBackgroundGradient.setColorAt(0.0, QColor(3, 3, 3));
    upperBackgroundGradient.setColorAt(0.0, QColor(21, 21, 21));

    QBrush upperBackgroundBrush(upperBackgroundGradient);
    painter->setBrush(upperBackgroundBrush);
    painter->setPen(Qt::NoPen);
    painter->drawRect(boundingRect_.x(), boundingRect_.y(), boundingRect_.width(), boundingRect_.height() / 2.0);

    //painter->setBrush(QBrush(Qt::white));
    painter->setBrush(QBrush(Qt::black));
    painter->drawRect(boundingRect_.x(), boundingRect_.y() + boundingRect_.height() / 2.0, boundingRect_.width(), boundingRect_.height() / 2.0);

    // now the progress overlay
    QRectF progressRect = boundingRect_;
    progressRect.setWidth(boundingRect_.width() * progress_);

    QLinearGradient upperForegroundGradient(progressRect.x(), progressRect.y(), progressRect.x(), progressRect.y() + progressRect.height() / 2.0);
    //upperForegroundGradient.setColorAt(0.0, QColor(173, 235, 199));
    //upperForegroundGradient.setColorAt(1.0, QColor(139, 227, 176));
    upperForegroundGradient.setColorAt(0.0, QColor(176, 191, 217));
    upperForegroundGradient.setColorAt(1.0, QColor(59, 106, 193));

    QBrush upperForegroundBrush(upperForegroundGradient);
    painter->setBrush(upperForegroundBrush);
    painter->drawRect(progressRect.x(), progressRect.y(), progressRect.width(), progressRect.height() / 2.0 + 1.0);

    //QBrush lowerForegroundBrush(QColor(38, 214, 118));
    QBrush lowerForegroundBrush(QColor(22, 85, 200));
    painter->setBrush(lowerForegroundBrush);
    painter->drawRect(progressRect.x(), progressRect.y() + progressRect.height() / 2.0, progressRect.width(), progressRect.height() / 2.0);
}

} // namespace
