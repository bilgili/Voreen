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

#include "voreen/qt/widgets/sliderspinboxwidget.h"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QSlider>
#include <QSpinBox>

namespace voreen {

SliderSpinBoxWidget::SliderSpinBoxWidget(QWidget* parent)
    : QWidget(parent)
    , isSliderTrackingEnabled_(true)
    , isSpinboxTrackingEnabled_(false)
{
    layout_ = new QHBoxLayout(this);
    layout_->setSpacing(6);
    layout_->setMargin(0);
    slider_ = new QSlider(this);
    slider_->setOrientation(Qt::Horizontal);
    slider_->setTickPosition(QSlider::NoTicks);
    slider_->setTickInterval(5);

    layout_->addWidget(slider_);
    spinbox_ = new QSpinBox(this);
    layout_->addWidget(spinbox_);


    // signals and slots connections
    connect(slider_, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
    connect(slider_, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
    connect(slider_, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
    connect(spinbox_, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
    connect(spinbox_, SIGNAL(editingFinished()), this, SLOT(spinEditingFinished()));

    value_ = getValue();

    setSliderTracking(isSliderTrackingEnabled_);
    setSpinboxTracking(isSpinboxTrackingEnabled_);
}

void SliderSpinBoxWidget::setView(Property::View view) {
    if (!(view & Property::SPINBOX))
        spinbox_->hide();
    if (!(view & Property::SLIDER))
        slider_->hide();
}

void SliderSpinBoxWidget::setSingleStep(int step) {
    spinbox_->setSingleStep(step);
    slider_->setSingleStep(step);
}

void SliderSpinBoxWidget::changeEvent(QEvent* e) {
    if (e->type() == QEvent::EnabledChange) {
        // this event will be fired _after_ the widget has been changed
        if (!isEnabled()) {
            spinbox_->setVisible(true);
            slider_->setVisible(false);
        }
        else {
            spinbox_->setVisible(true);
            slider_->setVisible(true);
        }
    }
}

void SliderSpinBoxWidget::setValue(int value) {
    if (value != value_ || slider_->value() != value || spinbox_->value() != value) {
        value_ = value;
        slider_->setValue(value_);
        spinbox_->setValue(value_);
        if (isSliderTrackingEnabled_ || !slider_->isSliderDown())
            emit valueChanged(value_);
    }
}

void SliderSpinBoxWidget::setMaxValue(int value) {
    slider_->setMaximum(value);
    spinbox_->setMaximum(value);
    spinbox_->updateGeometry();
}

void SliderSpinBoxWidget::setMinValue(int value) {
    slider_->setMinimum(value);
    spinbox_->setMinimum(value);
}

int SliderSpinBoxWidget::getMinValue() const {
    return slider_->minimum();
}

int SliderSpinBoxWidget::getMaxValue() const {
    return slider_->maximum();
}

int SliderSpinBoxWidget::getValue() const {
    return slider_->value();
}

void SliderSpinBoxWidget::sliderPressed() {
    emit sliderPressedChanged(true);
}

void SliderSpinBoxWidget::sliderReleased() {
    emit sliderPressedChanged(false);
    emit editingFinished();
    if (!isSliderTrackingEnabled_)
        emit valueChanged(value_);
}

void SliderSpinBoxWidget::spinEditingFinished() {
    emit editingFinished();
}

bool SliderSpinBoxWidget::isSliderDown() const {
    return slider_->isSliderDown();
}

void SliderSpinBoxWidget::setFocusPolicy(Qt::FocusPolicy policy) {
    QWidget::setFocusPolicy(policy);
    slider_->setFocusPolicy(policy);
    spinbox_->setFocusPolicy(policy);
}

void SliderSpinBoxWidget::setSliderTracking(bool tracking) {
    isSliderTrackingEnabled_ = tracking;
    // do not disable tracking of the slider, since we want
    // to keep the spinbox in sync anyway.
}

void SliderSpinBoxWidget::setSpinboxTracking(bool tracking) {
    isSpinboxTrackingEnabled_ = tracking;
    spinbox_->setKeyboardTracking(tracking);
}

bool SliderSpinBoxWidget::hasSliderTracking() const {
    return isSliderTrackingEnabled_;
}

bool SliderSpinBoxWidget::hasSpinboxTracking() const {
    return isSpinboxTrackingEnabled_;
}

// ---------------------------------------------------------------------------

DoubleSliderSpinBoxWidget::DoubleSliderSpinBoxWidget(QWidget* parent )
    : QWidget(parent)
    , isSliderTrackingEnabled_(true)
    , isSpinboxTrackingEnabled_(false)
{
    layout_ = new QHBoxLayout(this);
    layout_->setSpacing(6);
    layout_->setMargin(0);
    slider_ = new QSlider(this);
    slider_->setOrientation(Qt::Horizontal);
    slider_->setTickPosition(QSlider::NoTicks);
    slider_->setTickInterval(5);
    layout_->addWidget(slider_);

    spinbox_ = new QDoubleSpinBox(this);

    layout_->addWidget(spinbox_);

    // signals and slots connections
    connect(slider_, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
    connect(slider_, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
    connect(slider_, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
    connect(spinbox_, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    connect(spinbox_, SIGNAL(editingFinished()), this, SLOT(spinEditingFinished()));

    value_ = getValue();
    adjustSliderScale();

    setSliderTracking(isSliderTrackingEnabled_);
    setSpinboxTracking(isSpinboxTrackingEnabled_);
}

void DoubleSliderSpinBoxWidget::setView(Property::View view) {
    if (!(view & Property::SPINBOX))
        spinbox_->hide();
    if (!(view & Property::SLIDER))
        slider_->hide();
}

void DoubleSliderSpinBoxWidget::changeEvent(QEvent* e) {
    if (e->type() == QEvent::EnabledChange) {
        // this event will be fired _after_ the widget has been changed
        if (!isEnabled()) {
            spinbox_->setVisible(true);
            slider_->setVisible(false);
        }
        else {
            spinbox_->setVisible(true);
            slider_->setVisible(true);
        }
    }
}


void DoubleSliderSpinBoxWidget::setValue(double value) {
    if (value != value_) {
        value_ = value;
        spinbox_->setValue(value_);
        slider_->blockSignals(true);
        slider_->setValue( static_cast<int>((spinbox_->value() - spinbox_->minimum()) /
                                   (spinbox_->maximum() - spinbox_->minimum()) * slider_->maximum()) );
        slider_->blockSignals(false);
        if (isSliderTrackingEnabled_ || !slider_->isSliderDown())
            emit valueChanged(value_);
    }
}

void DoubleSliderSpinBoxWidget::setMaxValue( double value ) {
    spinbox_->setMaximum(value);
    spinbox_->updateGeometry();
    adjustSliderScale();
}

void DoubleSliderSpinBoxWidget::setMinValue( double value ) {
    spinbox_->setMinimum(value);
    adjustSliderScale();
}

double DoubleSliderSpinBoxWidget::getMinValue() const {
    return spinbox_->minimum();
}

void DoubleSliderSpinBoxWidget::setSingleStep( double step ) {
    spinbox_->setSingleStep(step);
    adjustSliderScale();
}

void DoubleSliderSpinBoxWidget::setDecimals(int decimals) {

    spinbox_->setDecimals(decimals);
}

double DoubleSliderSpinBoxWidget::getMaxValue() const {
    return spinbox_->maximum();
}

double DoubleSliderSpinBoxWidget::getValue() const {
    return spinbox_->value();
}

double DoubleSliderSpinBoxWidget::getSingleStep() const {
    return spinbox_->singleStep();
}

int DoubleSliderSpinBoxWidget::getDecimals() const {
    return spinbox_->decimals();
}

void DoubleSliderSpinBoxWidget::sliderPressed() {
    emit sliderPressedChanged(true);
}

void DoubleSliderSpinBoxWidget::sliderReleased() {
    emit sliderPressedChanged(false);
    emit editingFinished();
    if (!isSliderTrackingEnabled_)
        emit valueChanged(value_);
}

void DoubleSliderSpinBoxWidget::spinEditingFinished() {
    emit editingFinished();
}

bool DoubleSliderSpinBoxWidget::isSliderDown() const {
    return slider_->isSliderDown();
}

void DoubleSliderSpinBoxWidget::setFocusPolicy(Qt::FocusPolicy policy) {
    QWidget::setFocusPolicy(policy);
    slider_->setFocusPolicy(policy);
    spinbox_->setFocusPolicy(policy);
}

void DoubleSliderSpinBoxWidget::adjustSliderScale() {
    double sliderScale = ( spinbox_->maximum() - spinbox_->minimum() ) / spinbox_->singleStep();
    slider_->setMinimum(0);
    slider_->setMaximum(tgt::iround(sliderScale));

    slider_->blockSignals(true);
    slider_->setValue(static_cast<int>((spinbox_->value() - spinbox_->minimum()) /
                              (spinbox_->maximum() - spinbox_->minimum()) * slider_->maximum()));
    slider_->blockSignals(false);
}

void DoubleSliderSpinBoxWidget::sliderValueChanged(int value) {
    setValue(spinbox_->minimum() + ((double)value / slider_->maximum()) * (spinbox_->maximum() - spinbox_->minimum()));
}

void DoubleSliderSpinBoxWidget::setSliderTracking(bool tracking) {
    isSliderTrackingEnabled_ = tracking;
    // do not disable tracking of the slider, since we want
    // to keep the spinbox in sync anyway.
}

void DoubleSliderSpinBoxWidget::setSpinboxTracking(bool tracking) {
    isSpinboxTrackingEnabled_ = tracking;
    spinbox_->setKeyboardTracking(tracking);
}

bool DoubleSliderSpinBoxWidget::hasSliderTracking() const {
    return isSliderTrackingEnabled_;
}

bool DoubleSliderSpinBoxWidget::hasSpinboxTracking() const {
    return isSpinboxTrackingEnabled_;
}

} // namespace voreen
