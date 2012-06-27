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

#include "voreen/qt/widgets/thresholdwidget.h"

namespace voreen {

ThresholdWidget::ThresholdWidget(QWidget* parent ) :
    QWidget(parent)
{
    setObjectName(QString::fromUtf8("ThresholdWidget"));
    resize(QSize(329, 149).expandedTo(minimumSizeHint()));
    vboxLayout = new QVBoxLayout(this);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(0);
    //vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(6);
    vboxLayout1->setMargin(0);

    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    upperValueLBL = new QLabel(this);
    upperValueLBL->setObjectName(QString::fromUtf8("upperValueLBL"));
    vboxLayout1->addWidget(upperValueLBL);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    //hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));

    upperValueSLB = new SliderSpinBoxWidget(this);
    upperValueSLB->setObjectName(QString::fromUtf8("upperValue"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(upperValueSLB->sizePolicy().hasHeightForWidth());
    upperValueSLB->setSizePolicy(sizePolicy);
    hboxLayout->addWidget(upperValueSLB);

    vboxLayout1->addLayout(hboxLayout);
    vboxLayout->addLayout(vboxLayout1);

    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setSpacing(6);
    vboxLayout2->setMargin(0);
    //vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    lowerValueLBL = new QLabel(this);
    lowerValueLBL->setObjectName(QString::fromUtf8("lowerValueLBL"));
    vboxLayout2->addWidget(lowerValueLBL);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    //hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));

    lowerValueSLB = new SliderSpinBoxWidget(this);
    lowerValueSLB->setObjectName(QString::fromUtf8("lowerValue"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(lowerValueSLB->sizePolicy().hasHeightForWidth());
    lowerValueSLB->setSizePolicy(sizePolicy1);

    hboxLayout1->addWidget(lowerValueSLB);

    vboxLayout2->addLayout(hboxLayout1);

    hounsfieldLBL = new QLabel(this);
    hounsfieldLBL->setObjectName(QString::fromUtf8("hounsfildLBL"));
    vboxLayout2->addWidget(hounsfieldLBL);

    hounsfield = new QComboBox(this);
    hounsfield->setObjectName(QString::fromUtf8("hounsfield"));

    vboxLayout2->addWidget(hounsfield);
    vboxLayout->addLayout(vboxLayout2);

    setWindowTitle(QApplication::translate("ThresholdWidget", "ThresholdWidget", 0, QApplication::UnicodeUTF8));
    upperValueLBL->setText(QApplication::translate("ThresholdWidget", "Upper threshold:", 0, QApplication::UnicodeUTF8));
    lowerValueLBL->setText(QApplication::translate("ThresholdWidget", "Lower threshold:", 0, QApplication::UnicodeUTF8));
    hounsfieldLBL->setText(QApplication::translate("ThresholdWidget", "Hounsfield value:", 0, QApplication::UnicodeUTF8));
    hounsfield->clear();
    hounsfield->addItem(QApplication::translate("ThresholdWidget", "All", 0, QApplication::UnicodeUTF8));
    hounsfield->addItem(QApplication::translate("ThresholdWidget", "Air (-1000)", 0, QApplication::UnicodeUTF8));
    hounsfield->addItem(QApplication::translate("ThresholdWidget", "Lung (-400 => -600)", 0, QApplication::UnicodeUTF8));
    hounsfield->addItem(QApplication::translate("ThresholdWidget", "Fat (-60 => -100)", 0, QApplication::UnicodeUTF8));
    hounsfield->addItem(QApplication::translate("ThresholdWidget", "Water (0)", 0, QApplication::UnicodeUTF8));
    hounsfield->addItem(QApplication::translate("ThresholdWidget", "Soft Tissue (40 => 80)", 0, QApplication::UnicodeUTF8));
    hounsfield->addItem(QApplication::translate("ThresholdWidget", "Bone (400 => 1000)", 0, QApplication::UnicodeUTF8));

    QMetaObject::connectSlotsByName(this);

    // signals and slots connections
    connect( lowerValueSLB, SIGNAL( valueChanged(int) ), this, SLOT( setLowerValue(int) ) );
    connect( upperValueSLB, SIGNAL( valueChanged(int) ), this, SLOT( setUpperValue(int) ) );

    connect( lowerValueSLB, SIGNAL( sliderPressedChanged(bool) ), this, SLOT( slidersPressedChange(bool) ) );
    connect( upperValueSLB, SIGNAL( sliderPressedChanged(bool) ), this, SLOT( slidersPressedChange(bool) ) );

    connect( hounsfield, SIGNAL( currentIndexChanged(int) ), this, SLOT( setHounsfieldRange(void) ) );

    lowerValue_ = lowerValueSLB->getValue();
    upperValue_ = upperValueSLB->getValue();
    minValue_ = lowerValueSLB->getMinValue();
    maxValue_ = lowerValueSLB->getMaxValue();
}

void ThresholdWidget::setValues(int lowerValue, int upperValue) {
    if (lowerValue<minValue_)
        lowerValue = minValue_;
    if (upperValue>maxValue_)
        upperValue = maxValue_;
    if (lowerValue == lowerValue_ && upperValue == upperValue_)
        return;
    if (lowerValue == lowerValue_) {
        upperValue_ = upperValue;
        upperValueSLB->setValue(upperValue_);
        emit upperValueChanged(upperValue_);
    }
    else if (upperValue == upperValue_) {
        lowerValue_ = lowerValue;
        lowerValueSLB->setValue(lowerValue_);
        emit lowerValueChanged(lowerValue_);
    }
    else {
        lowerValue_ = lowerValue;
        upperValue_ = upperValue;
        lowerValueSLB->setValue(lowerValue_);
        upperValueSLB->setValue(upperValue_);
        emit valuesChanged(lowerValue_, upperValue_);
    }
}

void ThresholdWidget::setLowerValue(int value) {
    if (value != lowerValue_) {
        lowerValue_ = value;
        emit lowerValueChanged(lowerValue_);
        if (value > upperValue_) {
            upperValue_ = value;
            upperValueSLB->setValue(upperValue_);
            emit upperValueChanged(upperValue_);
        }
        emit valuesChanged(lowerValue_, upperValue_);
    }
}

void ThresholdWidget::setUpperValue( int value ) {
    if (value != upperValue_) {
        upperValue_ = value;
        emit upperValueChanged(upperValue_);
        if (value < lowerValue_) {
            lowerValue_ = value;
            lowerValueSLB->setValue(lowerValue_);
            emit lowerValueChanged(lowerValue_);
        }
        emit valuesChanged(lowerValue_, upperValue_);
    }
}

void ThresholdWidget::setLowerStateChecked(bool state) {
    if (state != lowerState_) {
        lowerState_ = state;
        emit lowerStateToggled(lowerState_);
    }
}

void ThresholdWidget::setUpperStateChecked(bool state) {
    if (state != upperState_) {
        upperState_ = state;
        emit upperStateToggled(upperState_);
    }
}

void ThresholdWidget::setMaxValue(int value) {
    maxValue_ = value;
    lowerValueSLB->setMaxValue(value);
    upperValueSLB->setMaxValue(value);
}

void ThresholdWidget::setMinValue(int value) {
    minValue_ = value;
    lowerValueSLB->setMinValue(value);
    upperValueSLB->setMinValue(value);
}

int ThresholdWidget::getMinValue() const {
    return minValue_;
}

int ThresholdWidget::getMaxValue() const {
    return maxValue_;
}

int ThresholdWidget::getLowerValue() const {
    return lowerValue_;
}

int ThresholdWidget::getUpperValue()const {
    return upperValue_;
}

void ThresholdWidget::slidersPressedChange(bool pressed) {
    emit sliderPressedChanged(pressed);
}

void ThresholdWidget::resetThresholds() {
    setValues(minValue_, maxValue_);
    emit valuesChanged(minValue_, maxValue_);
}

void ThresholdWidget::setHounsfieldRange() {
    int lower = -1, upper = -1;
    switch (hounsfield->currentIndex()) {
    //All:
    case 0:
        lower = -1024;
        upper = 4096 - 1024;
        break;
    //Air:
    case 1:
        lower = -1010;
        upper = -990;
        break;
    //Lung:
    case 2:
        lower = -600;
        upper = -400;
        break;
    //Fat:
    case 3:
        lower = -100;
        upper = -60;
        break;
    //Water:
    case 4:
        lower = -5;
        upper = 5;
        break;
    //Soft Tissue:
    case 5:
        lower = 40;
        upper = 80;
        break;
    //Bone:
    case 6:
        lower = 400;
        upper = 1000;
        break;
    default:
        break;
    }
    lower += 1024;
    upper += 1024;
    if (getMaxValue() < 4095) {
        // 8 bit dataset loaded, values need to be transformed
        lower = static_cast<int>(lower / 1000.0 * getMaxValue());
        upper = static_cast<int>(upper / 1000.0 * getMaxValue());
    }
    upperValueSLB->setValue(upper);
    emit upperValueChanged(upper);
    lowerValueSLB->setValue(lower);
    emit lowerValueChanged(lower);
}

} // namespace voreen
