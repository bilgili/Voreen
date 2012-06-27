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

#ifndef CLIPPERWIDGET_H
#define CLIPPERWIDGET_H

#include <QWidget>

#include "voreen/qt/widgets/sliderspinboxwidget.h"

class QToolButton;

namespace voreen {

class ClipperWidget : public QWidget {
	Q_OBJECT

public:
	ClipperWidget(QWidget *parent = 0) : QWidget(parent) {}
	ClipperWidget(const char *text, QWidget *parent = 0);
	int get1stSliderValue();
	int get2ndSliderValue();

public slots:
	void sliderPressedChanged(bool pressed);
	void setLCDDisplay();
	void lockSliders(bool lock);

signals:
	void sliderPressed(bool pressed);
	void sliderValueChanged();

private:
	SliderSpinBoxWidget* slider1_;
	SliderSpinBoxWidget* slider2_;
	QToolButton* lockButton_;

	int sliderMax_;
	int oldSlider1Val_;
	int oldSlider2Val_;
	bool slider1Down_;
	bool slider2Down_;
	bool slidersLocked_;
};

} // namespace voreen

#endif //CLIPPERWIDGET_H
