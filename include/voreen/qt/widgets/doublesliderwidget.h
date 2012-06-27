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

#ifndef VRN_DOUBLESLIDERWIDGET_H
#define VRN_DOUBLESLIDERWIDGET_H

#include "tgt/vector.h"
#include "tgt/math.h"

#include <QString>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QtGui>

namespace voreen {

/**
 * DoubleSlider is a QWidget providing two sliders on one ridge to define a range.
 * The sliders can be moved by mouse interaction or by \a setMaxValue() and \a setMinValue()
 * method. Each type of modification emits a different signal. The range of the values is from 0.0 to 1.0
 * and stored in float variables. To avoid an overlay of the two sliders the active one shunts the
 * inactive slider.
 *
 */
class DoubleSlider : public QWidget {
    Q_OBJECT
public:
    DoubleSlider(QWidget* parent = 0);
	 /**
	  * Returns the minimum of the range marked out by the two sliders
	  * (x-coordinate of the left slider in a normalized coordinate system).
      */
	float getMinValue();
	 /**
	  * Returns the maximum of the range marked out by the two sliders
	  * (x-coordinate of the right slider in a normalized coordinate system).
      */
	float getMaxValue();

public slots:
	 /**
	  * Sets the maximal value and emits valuesSet( float, float ).
      */
	 void setMaxValue( float );
	 /**
	  * Sets the minimal value and emits valuesSet( float, float ).
      */
	 void setMinValue( float );
	 /**
	  * Sets the minimal and maximal value. Emits valuesSet( float, float ).
      */
	 void setValues( float, float );
	 /**
	  * Sets the width of a slider. Default value is 5 pixels.
      */
	 void setSliderWidth( int );

signals:
	 /**
	  * This signal is emitted by mouse interaction
      */
	 void valuesChanged( float, float );
	 /**
	  * This signal is emitted by the set methods
      */
	 void valuesSet( float, float );


private:
	float minValue, maxValue;
	float normalizedMousePos, mV1, mV2;
	int sliderWidth;
	/*
	 * Only active slider(s) will be moved by moveSlider()
	 */
	bool leftSliderActive, rightSliderActive;
	bool internalCall;		// true -> set-method called by a DoubleSlider-method
	/*
	 * Internal Function to move the active slider(s). Called by \a mouseMoveEvent() and \a mousePressEvent().
	 */
	void moveSlider( float );

protected:
	virtual void paintEvent(QPaintEvent *);
	/*
	 * Calculates the slider next to mouse position, sets it active and calls \a moveSlider( float ). Emits
	 * valuesChanged( float, float )
	 */
	virtual void mousePressEvent(QMouseEvent* );
	/*
	 * Moves the active slider by calling moveSlider() and emits
	 * valuesChanged( float, float ).
	 */
	virtual void mouseMoveEvent(QMouseEvent* );

};

} // namespace

#endif // VRN_DOUBLESLIDERWIDGET_H
