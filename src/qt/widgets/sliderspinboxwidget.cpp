/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/qt/widgets/sliderspinboxwidget.h"

#include "tgt/math.h"

#include <qslider.h>
#include <qspinbox.h>

namespace voreen {

SliderSpinBoxWidget::SliderSpinBoxWidget(QWidget* parent ) : QWidget(parent) {
	setObjectName(QString::fromUtf8("SliderSpinBoxWidget"));
    resize(QSize(156, 86).expandedTo(minimumSizeHint()));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(sizePolicy.hasHeightForWidth());
    setSizePolicy(sizePolicy);
    vboxLayout = new QVBoxLayout(this);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    sliderSLD = new QSlider(this);
    sliderSLD->setObjectName(QString::fromUtf8("sliderSLD"));
    sliderSLD->setOrientation(Qt::Horizontal);
    sliderSLD->setTickPosition(QSlider::NoTicks);
    sliderSLD->setTickInterval(5);

    hboxLayout->addWidget(sliderSLD);

    spinBoxSPB = new QSpinBox(this);
    spinBoxSPB->setObjectName(QString::fromUtf8("spinBoxSPB"));

    hboxLayout->addWidget(spinBoxSPB);

    vboxLayout->addLayout(hboxLayout);

    spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout->addItem(spacerItem);

    setWindowTitle(QApplication::translate("SliderSpinBoxWidget", "SliderSpinBoxWidget", 0, QApplication::UnicodeUTF8));
    // signals and slots connections
    connect( sliderSLD, SIGNAL( valueChanged(int) ), this, SLOT( setValue(int) ) );
    connect( sliderSLD, SIGNAL( sliderPressed() ), this, SLOT( sliderPressed() ) );
    connect( sliderSLD, SIGNAL( sliderReleased() ), this, SLOT( sliderReleased() ) );
    connect( spinBoxSPB, SIGNAL( valueChanged(int) ), this, SLOT( setValue(int) ) );
    connect( spinBoxSPB, SIGNAL( editingFinished() ), this, SLOT( spinEditingFinished() ) );
	value_ = getValue();
}

QSize SliderSpinBoxWidget::sizeHint () const {
	return QSize(160,22);
}

void SliderSpinBoxWidget::setValue(int value) {
	if(value != value_) {
		value_ = value;
		sliderSLD->setValue(value_);
		spinBoxSPB->setValue(value_);
		emit valueChanged(value_);
	}
}

void SliderSpinBoxWidget::setMaxValue( int value ) {
    sliderSLD->setMaximum(value);
    spinBoxSPB->setMaximum(value);
    spinBoxSPB->updateGeometry();
}


void SliderSpinBoxWidget::setMinValue( int value ) {
    sliderSLD->setMinimum(value);
    spinBoxSPB->setMinimum(value);
}

int SliderSpinBoxWidget::getMinValue()const {
	return sliderSLD->minimum();
}

int SliderSpinBoxWidget::getMaxValue()const {
	return sliderSLD->maximum();
}

int SliderSpinBoxWidget::getValue()const {
	return sliderSLD->value();
}

void SliderSpinBoxWidget::sliderPressed() {
    emit sliderPressedChanged(true);
}

void SliderSpinBoxWidget::sliderReleased() {
    emit sliderPressedChanged(false);
    emit editingFinished();
}

void SliderSpinBoxWidget::spinEditingFinished() {
    emit editingFinished();
}

bool SliderSpinBoxWidget::isSliderDown()const {
	return sliderSLD->isSliderDown();
}

void SliderSpinBoxWidget::setFocusPolicy(Qt::FocusPolicy policy) {
    QWidget::setFocusPolicy(policy);
    sliderSLD->setFocusPolicy(policy);
    spinBoxSPB->setFocusPolicy(policy);
}


// ---------------------------------------------------------------------------


DoubleSliderSpinBoxWidget::DoubleSliderSpinBoxWidget(QWidget* parent ) : QWidget(parent) {
	setObjectName(QString::fromUtf8("DoubleSliderSpinBoxWidget"));
    resize(QSize(156, 86).expandedTo(minimumSizeHint()));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(sizePolicy.hasHeightForWidth());
    setSizePolicy(sizePolicy);
    vboxLayout = new QVBoxLayout(this);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    sliderSLD = new QSlider(this);
    sliderSLD->setObjectName(QString::fromUtf8("sliderSLD"));
    sliderSLD->setOrientation(Qt::Horizontal);
    sliderSLD->setTickPosition(QSlider::NoTicks);
    sliderSLD->setTickInterval(5);

    hboxLayout->addWidget(sliderSLD);

    spinBoxSPB = new QDoubleSpinBox(this);
    spinBoxSPB->setObjectName(QString::fromUtf8("spinBoxSPB"));

    hboxLayout->addWidget(spinBoxSPB);

    vboxLayout->addLayout(hboxLayout);

    spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout->addItem(spacerItem);

    setWindowTitle(QApplication::translate("DoubleSliderSpinBoxWidget", "DoubleSliderSpinBoxWidget", 0, QApplication::UnicodeUTF8));
    // signals and slots connections
    connect( sliderSLD, SIGNAL( valueChanged(int) ), this, SLOT( sliderValueChanged(int) ) );
    connect( sliderSLD, SIGNAL( sliderPressed() ), this, SLOT( sliderPressed() ) );
    connect( sliderSLD, SIGNAL( sliderReleased() ), this, SLOT( sliderReleased() ) );
    connect( spinBoxSPB, SIGNAL( valueChanged(double) ), this, SLOT( setValue(double) ) );
    connect( spinBoxSPB, SIGNAL( editingFinished() ), this, SLOT( spinEditingFinished() ) );

    value_ = getValue();
    adjustSliderScale();
}

QSize DoubleSliderSpinBoxWidget::sizeHint () const {
	return QSize(160,22);
}

void DoubleSliderSpinBoxWidget::setValue(double value) {
	if(value != value_) {
		value_ = value;
		spinBoxSPB->setValue(value_);
        sliderSLD->blockSignals(true);
        sliderSLD->setValue( static_cast<int>((spinBoxSPB->value() - spinBoxSPB->minimum()) /
                                   (spinBoxSPB->maximum() - spinBoxSPB->minimum()) * sliderSLD->maximum()) );
        sliderSLD->blockSignals(false);
        emit valueChanged(value_);
	}
}

void DoubleSliderSpinBoxWidget::setMaxValue( double value ) {
    spinBoxSPB->setMaximum(value);
    spinBoxSPB->updateGeometry();
    adjustSliderScale();
}


void DoubleSliderSpinBoxWidget::setMinValue( double value ) {
    spinBoxSPB->setMinimum(value);
    adjustSliderScale();
}

double DoubleSliderSpinBoxWidget::getMinValue() const {
    return spinBoxSPB->minimum();
}

void DoubleSliderSpinBoxWidget::setSingleStep( double step ) {
    spinBoxSPB->setSingleStep(step);
    adjustSliderScale();
}

void DoubleSliderSpinBoxWidget::setDecimals(int decimals) {

    spinBoxSPB->setDecimals(decimals);
}
    
double DoubleSliderSpinBoxWidget::getMaxValue() const {
	return spinBoxSPB->maximum();
}

double DoubleSliderSpinBoxWidget::getValue() const {
	return spinBoxSPB->value();
}

double DoubleSliderSpinBoxWidget::getSingleStep() const {
    return spinBoxSPB->singleStep();
}

int DoubleSliderSpinBoxWidget::getDecimals() const {
    return spinBoxSPB->decimals();
}
    
void DoubleSliderSpinBoxWidget::sliderPressed() {
    emit sliderPressedChanged(true);
}

void DoubleSliderSpinBoxWidget::sliderReleased() {
    emit sliderPressedChanged(false);
    emit editingFinished();
}

void DoubleSliderSpinBoxWidget::spinEditingFinished() {
    emit editingFinished();
}

bool DoubleSliderSpinBoxWidget::isSliderDown() const {
	return sliderSLD->isSliderDown();
}

void DoubleSliderSpinBoxWidget::setFocusPolicy(Qt::FocusPolicy policy) {
    QWidget::setFocusPolicy(policy);
    sliderSLD->setFocusPolicy(policy);
    spinBoxSPB->setFocusPolicy(policy);
}

void DoubleSliderSpinBoxWidget::adjustSliderScale() {

    double sliderScale = ( spinBoxSPB->maximum() - spinBoxSPB->minimum() ) / spinBoxSPB->singleStep();
    sliderSLD->setMinimum(0);
    sliderSLD->setMaximum(tgt::iround(sliderScale));

    sliderSLD->blockSignals(true);
    sliderSLD->setValue(static_cast<int>((spinBoxSPB->value() - spinBoxSPB->minimum()) /
                              (spinBoxSPB->maximum() - spinBoxSPB->minimum()) * sliderSLD->maximum()));
    sliderSLD->blockSignals(false);
}

void DoubleSliderSpinBoxWidget::sliderValueChanged( int value ) {
    
    setValue( spinBoxSPB->minimum() + ((double)value / sliderSLD->maximum()) * (spinBoxSPB->maximum() - spinBoxSPB->minimum()) );
}



} // namespace voreen
