/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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
#include "voreen/core/utils/stringutils.h"
#include "tgt/logmanager.h"

#include <math.h>

#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QToolTip>

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
    sliderWidth_ = 6;
    minRWValue_ = 0.0f;
    maxRWValue_ = 1.0f;
    unit_ = "";
    showToolTip_ = false;
    setFixedHeight(16);

    setToolTip(QString::fromStdString(generateToolTipText(false, false)));
}

void DoubleSlider::setOffsets(int left, int right) {
    leftOffset_ = left;
    rightOffset_ = right;
}

void DoubleSlider::paintEvent(QPaintEvent* event) {
    event->accept();
    int leftMarker =  tgt::iround(minValue_ * (width()-leftOffset_-rightOffset_) + leftOffset_);
    int rightMarker = tgt::iround(maxValue_ * (width()-leftOffset_-rightOffset_) + leftOffset_);
    QPoint leftSlider[4] = {
        QPoint(leftMarker, height()),
        QPoint(leftMarker + sliderWidth_, height()),
        QPoint(leftMarker + sliderWidth_, static_cast<int>(0.4f * height())),
        QPoint(leftMarker, 0)
    };
    QPoint rightSlider[4] = {
        QPoint(rightMarker - sliderWidth_, static_cast<int>(0.4f * height())),
        QPoint(rightMarker - sliderWidth_, height()),
        QPoint(rightMarker, height()),
        QPoint(rightMarker, 0)
    };
    QPoint centerQuad[4] = {
        QPoint(rightMarker - sliderWidth_, static_cast<int>(0.4f * height())),
        QPoint(rightMarker - sliderWidth_, height()),
        QPoint(leftMarker + sliderWidth_, height()),
        QPoint(leftMarker + sliderWidth_, static_cast<int>(0.4f * height())),
    };

    QPalette pal = QApplication::palette();

    QColor sliderColor = pal.color(QPalette::Base);
    QColor sliderDarkColor = pal.color(QPalette::Mid);
    QColor lineColor = pal.color(QPalette::Dark);

    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing);

    //draw horizontal line
    paint.setPen(lineColor);
    paint.drawLine(leftOffset_, height()/2, width()-rightOffset_, height()/2);

    //draw center
    paint.setBrush(sliderDarkColor);
    paint.setPen(lineColor);
    paint.save();
    paint.drawConvexPolygon(centerQuad, 4);
    paint.restore();

    paint.setBrush(sliderColor);
    paint.setPen(sliderDarkColor);

    //draw left marker
    paint.save();
    paint.drawConvexPolygon(leftSlider, 4);
    paint.restore();

    //draw right marker
    paint.save();
    paint.drawConvexPolygon(rightSlider, 4);
    paint.restore();
}

void DoubleSlider::mousePressEvent(QMouseEvent* e) {
    e->accept();
    globalMousePos_ = e->globalPos();
    //calculate which marker is nearest to mouse position
    normalizedMousePos_ = static_cast<float>(e->pos().x()-leftOffset_) / static_cast<float>(width()-leftOffset_-rightOffset_);
    int leftMarker =  tgt::iround(minValue_ * (width()-leftOffset_-rightOffset_) + leftOffset_);
    int rightMarker = tgt::iround(maxValue_ * (width()-leftOffset_-rightOffset_) + leftOffset_);

    mV1_ = minValue_;
    mV2_ = maxValue_;
    if (e->button() == Qt::LeftButton) {
        if (e->pos().x() < (leftMarker + sliderWidth_)) {
            leftSliderActive_ = true;
            rightSliderActive_ = false;
        }
        else if(e->pos().x() > (rightMarker - sliderWidth_)) {
            leftSliderActive_ = false;
            rightSliderActive_ = true;
        }
        else if((leftMarker < e->pos().x()) && (rightMarker > e->pos().x())){
            leftSliderActive_ = true;
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
    globalMousePos_ = e->globalPos();
    float normalizedMousePosTmp = static_cast<float>((e->pos()).x()-leftOffset_) / static_cast<float>(width()-leftOffset_-rightOffset_);
    if (normalizedMousePosTmp > 1.f)
        normalizedMousePosTmp = 1.f;
    else if (normalizedMousePosTmp < 0.f)
        normalizedMousePosTmp = 0.f;
    moveSlider(normalizedMousePosTmp);

    emit valuesChanged(minValue_, maxValue_);
}

void DoubleSlider::showToolTip(std::string text) {
    if(!showToolTip_)
        return;

    //QToolTip::showText(globalMousePos_, "");
    QToolTip::showText(globalMousePos_, QString::fromStdString(text));
}

void DoubleSlider::mouseReleaseEvent(QMouseEvent* e) {
    leftSliderActive_ = false;
    rightSliderActive_ = false;
    e->accept();
    emit toggleInteractionMode(false);
}

std::string DoubleSlider::generateToolTipText(bool minBold, bool maxBold) {
    std::string text = "<p style='white-space:pre'>";

    if(minBold)
        text += "<b>";
    text += ftos(getMappedValue(getMinValue()));
    if(minBold)
        text += "</b>";

    text += " -> ";

    if(maxBold)
        text += "<b>";
    text += ftos(getMappedValue(getMaxValue()));
    if(maxBold)
        text += "</b>";

    if(unit_ != "")
        text += " [" + unit_ + "]";

    text += "<br>w: " + ftos(getMappedValue(getMaxValue()) - getMappedValue(getMinValue()));
    text += "<br>l: " + ftos( (getMappedValue(getMaxValue()) + getMappedValue(getMinValue())) * 0.5f);

    return text;
}

void DoubleSlider::moveSlider(float mousePos) {
    if (leftSliderActive_ && !rightSliderActive_) {
        setMinValue(mousePos);
        showToolTip(generateToolTipText(true, false));
    }
    if (rightSliderActive_ && !leftSliderActive_) {
        setMaxValue(mousePos);
        showToolTip(generateToolTipText(false, true));
    }
    if (rightSliderActive_ && leftSliderActive_) {
        float mouseDiff = normalizedMousePos_ - mousePos;
        setMinValue(mV1_ - mouseDiff);
        setMaxValue(mV2_ - mouseDiff);
        showToolTip(generateToolTipText(true, true));
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
    setToolTip(QString::fromStdString(generateToolTipText(false, false)));
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
    setToolTip(QString::fromStdString(generateToolTipText(false, false)));
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

void DoubleSlider::setMapping(float min, float max) {
    minRWValue_ = min;
    maxRWValue_ = max;
    setToolTip(QString::fromStdString(generateToolTipText(false, false)));
}

void DoubleSlider::setUnit(std::string unit) {
    unit_ = unit;
    setToolTip(QString::fromStdString(generateToolTipText(false, false)));
}

void DoubleSlider::showToolTip(bool stt) {
    showToolTip_ = stt;
}

float DoubleSlider::getMappedValue(float norm) {
    return minRWValue_ + (norm * (maxRWValue_ - minRWValue_));
}

void DoubleSlider::setMinimalAllowedSliderDistance(float dist) {
    minimalAllowedSliderDistance_ = dist;
}

} // namespace voreen
