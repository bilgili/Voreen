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

#include "voreen/qt/widgets/property/floatvec4propertywidget.h"
#include "voreen/qt/widgets/lightwidget.h"
#include "voreen/qt/widgets/sliderspinboxwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QWidget>
#include <QPushButton>
#include <QSlider>

#include "math.h"

namespace voreen {

LightWidget::LightWidget(QWidget* parent)
    : QWidget(parent)
    , radius_(50)
    , hemisphere_(false)
    , distance_(4.f)
{
    DoubleSliderSpinBoxWidget* distanceSlider = new DoubleSliderSpinBoxWidget(this);
    distanceSlider->setMaxValue(10.0f);
    distanceSlider->setMinValue(1.0f);
    distanceSlider->setValue(distance_);
    connect(distanceSlider, SIGNAL(valueChanged(double)), this, SLOT(updateDistance(double)));
    distanceSlider->move(QPoint(0,85));
    distanceSlider->setMaximumWidth(110);
    distanceSlider->setSingleStep(0.1f);
    distanceSlider->show();

    mousePosition_ = QPointF(50,50);
    QPushButton* hemisphereButton = new QPushButton(this);
    hemisphereButton->setCheckable(true);
    hemisphereButton->setChecked(hemisphere_);
    hemisphereButton->setText("Z");
    hemisphereButton->setGeometry(QRect(0, 0, 20, 20));
    connect(hemisphereButton, SIGNAL(toggled(bool)), this, SLOT(setHemisphere(bool)));
    hemisphereButton->move(QPoint(90, 95));
    hemisphereButton->show();

    lightPosition_ = QPointF(50,50);

    setMinimumSize(75, 140);
}

void LightWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRadialGradient radialGradient(QPointF(50, 50), radius_ * 2  , QPointF(lightPosition_.x(), lightPosition_.y()));

    QColor highlight(static_cast<int>(distance_*3)+100,
                     static_cast<int>(distance_*3)+100,
                     static_cast<int>(distance_*3)+100);
    if (!hemisphere_) {
        radialGradient.setColorAt(0.0, QColor(255,255,255));
        radialGradient.setColorAt((distance_/30)+ 0.5, highlight);
        radialGradient.setColorAt(1.0, QColor(0,0,0));
    }
    else {
        radialGradient.setColorAt(0.0, QColor(0,0,0));
        radialGradient.setColorAt((distance_/30) + 0.5, highlight);
        radialGradient.setColorAt(1.0, QColor(255,255,255));
    }

    painter.setBrush(QBrush(radialGradient));
    painter.setPen(highlight);
    painter.drawEllipse(QRectF(2, 2, radius_ * 2, radius_ * 2));
}

void LightWidget::setHemisphere(bool hemibool) {
    hemisphere_ = hemibool;
    int hem = 1;
    if(hemisphere_)
        hem = -1;
    emit(lightWidgetChanged(tgt::vec4(x_ * distance_, y_ * distance_, hem * sqrt(distance_ - (x_ * x_)*distance_ - (y_ * y_ )* distance_), 0)));
    update();
}

void LightWidget::mousePressEvent(QMouseEvent *event) {
    mousePosition_ = event->pos();
    mouseMoveEvent(event);
    update();
}

void LightWidget::mouseMoveEvent(QMouseEvent *event) {
    mousePosition_ = event->pos();
    x_ =  -60+(float)mousePosition_.x();
    y_ = -60+(float)mousePosition_.y();
    x_ = x_/60;
    y_ = y_/60;
    int hem = 1;
    if(hemisphere_) {
        hem = -1;
    }
    if((x_ * x_ + y_ * y_) <= 1) {
        lightPosition_ = mousePosition_;
        emit(lightWidgetChanged(tgt::vec4(x_ * distance_, y_ * distance_, hem * sqrt(distance_ - (x_ * x_)*distance_ - (y_ * y_ )* distance_), 0)));
    }
    update();
}

void LightWidget::updateDistance(double distance) {
    distance_ = distance;
    update();
    int hem = 1;
    if(hemisphere_) {
        hem = -1;
    }
    emit(lightWidgetChanged(tgt::vec4(x_ * distance_, y_ * distance_, hem * sqrt(distance_ - (x_ * x_)*distance_ - (y_ * y_ )* distance_), 0)));
}

void LightWidget::setLightPosition(tgt::vec4 position) {
    x_ = position.x;
    y_ = position.y;
    distance_ = position.z;
    update();
}

} //namespace voreen
