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

#include "voreen/qt/widgets/doublesliderwidget.h"

namespace voreen {

DoubleSlider::DoubleSlider(QWidget *parent)
    :   QWidget(parent)
{
  minValue = 0.f;
  maxValue = 1.f;
  leftSliderActive = false;
  rightSliderActive = false;
  internalCall = false;
  sliderWidth = 5;
  setFixedHeight(20);  
}

void DoubleSlider::paintEvent(QPaintEvent* /*e*/) {  
  int leftMarker = tgt::iround(minValue * width());
  int rightMarker = tgt::iround(maxValue * width());
  QPoint leftSlider[5] = {
	  QPoint(leftMarker - sliderWidth, static_cast<int>(0.3f * height())),
	  QPoint(leftMarker - sliderWidth, height()),
	  QPoint(leftMarker + sliderWidth, height()),
	  QPoint(leftMarker + sliderWidth, static_cast<int>(0.3f * height())),
	  QPoint(leftMarker, 0)
  };
  QPoint rightSlider[5] = {
	  QPoint(rightMarker - sliderWidth, static_cast<int>(0.3f * height())),
	  QPoint(rightMarker - sliderWidth, height()),
	  QPoint(rightMarker + sliderWidth, height()),
	  QPoint(rightMarker + sliderWidth, static_cast<int>(0.3f * height())),
	  QPoint(rightMarker, 0)
  };
  QColor sliderColor(255, 255, 255);
  QColor sliderBrightColor(33, 184, 31);
  QColor sliderDarkColor(75, 130, 89);
  QColor lineColor(240, 237, 234);

  QPainter *paint = new QPainter();  
  paint->begin(this);
  paint->setRenderHint(QPainter::Antialiasing);
  
  //draw horizontal line
  paint->setPen(lineColor);
  paint->drawLine(0, height()/2, width(), height()/2);
  
  //draw left marker
  paint->setBrush(sliderColor);
  paint->setPen(sliderDarkColor);

  paint->save();
  paint->drawConvexPolygon(leftSlider, 5);
  paint->restore();

  //draw right marker
  paint->save();
  paint->drawConvexPolygon(rightSlider, 5);
  paint->restore();
  
  paint->end();
}

void DoubleSlider::mousePressEvent(QMouseEvent *e) {
	//calculate which marker is nearest to mouse position
	normalizedMousePos = static_cast<float>((e->pos()).x()) / static_cast<float>(width());
	mV1 = minValue;
	mV2 = maxValue;
	if (e->button() == Qt::LeftButton) {
		if (fabs(minValue - normalizedMousePos) < fabs(maxValue - normalizedMousePos)) {
			leftSliderActive = true;
			rightSliderActive = false;
		}
		else {
			leftSliderActive = false;
			rightSliderActive = true;
		}		
	}
	else if (e->button() == Qt::RightButton) {
		leftSliderActive = true;
		rightSliderActive = true;
	}
	moveSlider(normalizedMousePos);
	emit valuesChanged(minValue, maxValue);
}

void DoubleSlider::mouseMoveEvent(QMouseEvent *e){
	float normalizedMousePosTmp = static_cast<float>((e->pos()).x()) / static_cast<float>(width());
	if (normalizedMousePosTmp > 1.f)
		normalizedMousePosTmp = 1.f;
	else if (normalizedMousePosTmp < 0.f)
		normalizedMousePosTmp = 0.f;
	moveSlider(normalizedMousePosTmp);
	emit valuesChanged(minValue, maxValue);
}

void DoubleSlider::moveSlider(float mousePos) {
	internalCall = true;
	if (leftSliderActive && !rightSliderActive) 
		setMinValue( mousePos );
	if (rightSliderActive && !leftSliderActive) 
		setMaxValue( mousePos );
	if (rightSliderActive && leftSliderActive) {
		float mouseDiff = normalizedMousePos - mousePos;
		setMinValue( mV1 - mouseDiff );
		setMaxValue( mV2 - mouseDiff );
	}	
	internalCall = false;
}

void DoubleSlider::setMinValue(float val) {
	float normalizedSliderWidth = static_cast<float>(sliderWidth) / static_cast<float>(width());
	if ( val < 0 )
		val = 0.f;
	if ( val + normalizedSliderWidth < maxValue )
		minValue = val;		
	else {
		maxValue = val + normalizedSliderWidth;
		if (maxValue > 1.f) {
			maxValue = 1.f;
			minValue = 1.f - normalizedSliderWidth;
		} 
		else 
			minValue = val;		
	}
	repaint();
	if (!internalCall)
		emit valuesSet(minValue, maxValue);
}

void DoubleSlider::setMaxValue(float val) {
	float normalizedSliderWidth = static_cast<float>(sliderWidth) / static_cast<float>(width());
	if ( val > 1.f)
		val = 1.f;
	if (minValue + normalizedSliderWidth < val)
		maxValue = val;
	else {
		minValue = val - normalizedSliderWidth;
		if (minValue < 0.f) {
			minValue = 0.f;
			maxValue = normalizedSliderWidth;
		}
		else
			maxValue = val;
	}
	repaint();	
	if (!internalCall)
		emit valuesSet(minValue, maxValue);
}

void DoubleSlider::setSliderWidth(int sliderW) {
	sliderWidth = sliderW;
}

void DoubleSlider::setValues(float val1, float val2) {
	internalCall = true;
	if ( val1 < val2 ) {
		setMinValue(val1);
		setMaxValue(val2);
	}
	else {
		setMinValue(val2);
		setMaxValue(val1);
	}
	internalCall = false;
	emit valuesSet(minValue, maxValue);
}

float DoubleSlider::getMinValue() {
	return minValue;
}

float DoubleSlider::getMaxValue() {
	return maxValue;
}

} // namespace
