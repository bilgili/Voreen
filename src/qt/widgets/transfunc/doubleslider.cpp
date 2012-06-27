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

#include "voreen/qt/widgets/transfunc/doubleslider.h"

#include "tgt/tgt_math.h"

#include <math.h>

#include <QColor>
#include <QMouseEvent>
#include <QPainter>

namespace voreen {

DoubleSlider::DoubleSlider(QWidget* parent)
    : QWidget(parent)
{
    leftOffset_ = 0;
    rightOffset_ = 0;
    minValue_ = 0.f;
    maxValue_ = 1.f;
    leftSliderActive_ = false;
    rightSliderActive_ = false;
    minimalAllowedSliderDistance_ = 0.0001f;
    sliderWidth_ = 4;
    setFixedHeight(16);
}

void DoubleSlider::setOffsets(int left, int right) {
    leftOffset_ = left;
    rightOffset_ = right;
}

void DoubleSlider::paintEvent(QPaintEvent* event) {
    event->accept();
    int leftMarker =  tgt::iround(minValue_ * (width()-leftOffset_-rightOffset_) + leftOffset_);
    int rightMarker = tgt::iround(maxValue_ * (width()-leftOffset_-rightOffset_) + leftOffset_);
    QPoint leftSlider[5] = {
        QPoint(leftMarker - sliderWidth_, static_cast<int>(0.3f * height())),
        QPoint(leftMarker - sliderWidth_, height()),
        QPoint(leftMarker + sliderWidth_, height()),
        QPoint(leftMarker + sliderWidth_, static_cast<int>(0.3f * height())),
        QPoint(leftMarker, 0)
    };
    QPoint rightSlider[5] = {
        QPoint(rightMarker - sliderWidth_, static_cast<int>(0.3f * height())),
        QPoint(rightMarker - sliderWidth_, height()),
        QPoint(rightMarker + sliderWidth_, height()),
        QPoint(rightMarker + sliderWidth_, static_cast<int>(0.3f * height())),
        QPoint(rightMarker, 0)
    };
    QColor sliderColor(255, 255, 255);
    QColor sliderDarkColor(75, 130, 89);
    QColor lineColor(75, 130, 89);

    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing);

    //draw horizontal line
    paint.setPen(lineColor);
    paint.drawLine(leftOffset_, height()/2, width()-rightOffset_, height()/2);

    //draw left marker
    paint.setBrush(sliderColor);
    paint.setPen(sliderDarkColor);

    paint.save();
    paint.drawConvexPolygon(leftSlider, 5);
    paint.restore();

    //draw right marker
    paint.save();
    paint.drawConvexPolygon(rightSlider, 5);
    paint.restore();

}

void DoubleSlider::mousePressEvent(QMouseEvent* e) {
    e->accept();
    //calculate which marker is nearest to mouse position
    normalizedMousePos_ = static_cast<float>((e->pos()).x()-leftOffset_) / static_cast<float>(width()-leftOffset_-rightOffset_);
    mV1_ = minValue_;
    mV2_ = maxValue_;
    if (e->button() == Qt::LeftButton) {
        if (fabs(minValue_ - normalizedMousePos_) < fabs(maxValue_ - normalizedMousePos_)) {
            leftSliderActive_ = true;
            rightSliderActive_ = false;
        }
        else {
            leftSliderActive_ = false;
            rightSliderActive_ = true;
        }
    }
    else if (e->button() == Qt::RightButton) {
        leftSliderActive_ = true;
        rightSliderActive_ = true;
    }
    moveSlider(normalizedMousePos_);
    emit toggleInteractionMode(true);
}

void DoubleSlider::mouseMoveEvent(QMouseEvent* e){
    e->accept();
    float normalizedMousePosTmp = static_cast<float>((e->pos()).x()-leftOffset_) / static_cast<float>(width()-leftOffset_-rightOffset_);
    if (normalizedMousePosTmp > 1.f)
        normalizedMousePosTmp = 1.f;
    else if (normalizedMousePosTmp < 0.f)
        normalizedMousePosTmp = 0.f;
    moveSlider(normalizedMousePosTmp);
    emit valuesChanged(minValue_, maxValue_);
}

void DoubleSlider::mouseReleaseEvent(QMouseEvent* e) {
    e->accept();
    emit toggleInteractionMode(false);
}

void DoubleSlider::moveSlider(float mousePos) {
    if (leftSliderActive_ && !rightSliderActive_)
        setMinValue(mousePos);
    if (rightSliderActive_ && !leftSliderActive_)
        setMaxValue(mousePos);
    if (rightSliderActive_ && leftSliderActive_) {
        float mouseDiff = normalizedMousePos_ - mousePos;
        setMinValue(mV1_ - mouseDiff);
        setMaxValue(mV2_ - mouseDiff);
    }
}

void DoubleSlider::setMinValue(float val) {
    if (val == minValue_)
        return;

    if (val < 0.f)
        val = 0.f;
    if (val + minimalAllowedSliderDistance_ < maxValue_)
        minValue_ = val;
    else {
        maxValue_ = val + minimalAllowedSliderDistance_;
        if (maxValue_ > 1.f) {
            maxValue_ = 1.f;
            minValue_ = 1.f - minimalAllowedSliderDistance_;
        }
        else
            minValue_ = val;
    }
    update();
    emit valuesChanged(minValue_, maxValue_);
}

void DoubleSlider::setMaxValue(float val) {
    if (val == maxValue_)
        return;

    if (val > 1.f)
        val = 1.f;
    if (minValue_ + minimalAllowedSliderDistance_ < val)
        maxValue_ = val;
    else {
        minValue_ = val - minimalAllowedSliderDistance_;
        if (minValue_ < 0.f) {
            minValue_ = 0.f;
            maxValue_ = minimalAllowedSliderDistance_;
        }
        else
            maxValue_ = val;
    }
    update();
    emit valuesChanged(minValue_, maxValue_);
}

void DoubleSlider::setSliderWidth(int sliderW) {
    sliderWidth_ = sliderW;
}

void DoubleSlider::setValues(float val1, float val2) {
    if (val1 < val2) {
        setMinValue(val1);
        setMaxValue(val2);
    }
    else {
        setMinValue(val2);
        setMaxValue(val1);
    }
}

float DoubleSlider::getMinValue() {
    return minValue_;
}

float DoubleSlider::getMaxValue() {
    return maxValue_;
}

void DoubleSlider::setMinimalAllowedSliderDistance(float dist) {
    minimalAllowedSliderDistance_ = dist;
}

} // namespace voreen
