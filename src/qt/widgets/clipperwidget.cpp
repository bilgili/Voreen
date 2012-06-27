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

#include "voreen/qt/widgets/clipperwidget.h"

#include <QLabel>
#include <QToolButton>

namespace voreen {

ClipperWidget::ClipperWidget(const char *text, QWidget *parent) : QWidget(parent) 
{
    slider1Down_ = false;
    slider2Down_ = false;
	slidersLocked_ = false;

	sliderMax_ = 100;

	slider1_ = new SliderSpinBoxWidget();
	slider1_->setMinValue(0);
	slider1_->setMaxValue(sliderMax_);
	oldSlider1Val_ = 0;
    
	slider2_ = new SliderSpinBoxWidget();
	slider2_->setMinValue(0);
	slider2_->setMaxValue(sliderMax_);
	slider2_->setValue(sliderMax_);
	oldSlider1Val_ = sliderMax_;

    QVBoxLayout* vlayout = new QVBoxLayout();
    QHBoxLayout* horzX0 = new QHBoxLayout();
    horzX0->addWidget(new QLabel(QString(text)+QString("0")));
    horzX0->addWidget(slider1_);
	vlayout->addItem(horzX0);
    QHBoxLayout* horzX1 = new QHBoxLayout();
    horzX1->addWidget(new QLabel(QString(text)+QString("1")));
    horzX1->addWidget(slider2_);
	vlayout->addItem(horzX1);

    QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->addItem(vlayout);
    QToolButton* lockButton_ = new QToolButton();
    lockButton_->setCheckable(true);
    lockButton_->setChecked(false);
    lockButton_->setIcon(QIcon(":/icons/paper-clip.png"));
    lockButton_->setToolTip(tr("Stick together the two sliders for this clipping plane"));
	hlayout->addWidget(lockButton_);

	setLayout(hlayout);

	connect(slider1_, SIGNAL(valueChanged(int)), this, SLOT(setLCDDisplay()));
    connect(slider2_, SIGNAL(valueChanged(int)), this, SLOT(setLCDDisplay()));
    connect(slider1_, SIGNAL(sliderPressedChanged(bool)), this, SLOT(sliderPressedChanged(bool)));
    connect(slider2_, SIGNAL(sliderPressedChanged(bool)), this, SLOT(sliderPressedChanged(bool)));
    connect(lockButton_, SIGNAL(toggled(bool)), this, SLOT(lockSliders(bool)));
}

void ClipperWidget::lockSliders(bool lock) {
	slidersLocked_ = lock;
}

void ClipperWidget::sliderPressedChanged(bool pressed) {
	if (sender() == slider1_) slider1Down_ = pressed;
	else if (sender() == slider2_) slider2Down_ = pressed;
    emit sliderPressed(pressed);
}

void ClipperWidget::setLCDDisplay() {
	if (sender() == slider1_) {
		if (slidersLocked_ && !slider2Down_)
			slider2_->setValue(slider2_->getValue()+slider1_->getValue()-oldSlider1Val_);
		else if (slider1_->getValue() >= slider2_->getValue()-1)
			slider2_->setValue(slider1_->getValue()+1);
	} else if (sender() == slider2_) {
		if (slidersLocked_ && !slider1Down_)
			slider1_->setValue(slider1_->getValue()+slider2_->getValue()-oldSlider2Val_);
		else 
			if (slider2_->getValue() <= slider1_->getValue()+1)
			slider1_->setValue(slider2_->getValue()-1);
	}
	
	oldSlider1Val_ = slider1_->getValue();
	oldSlider2Val_ = slider2_->getValue();

    emit sliderValueChanged();	
}

int ClipperWidget::get1stSliderValue() { 
    return slider1_->getValue(); 
}

int ClipperWidget::get2ndSliderValue() { 
    return (sliderMax_-slider2_->getValue());
}

} // namespace voreen
