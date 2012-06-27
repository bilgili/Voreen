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

#ifndef VRN_VOREENBUTTON_H
#define VRN_VOREENBUTTON_H

#include <QtCore>
#include <QtGui>

class VoreenButton : public QToolButton {
public:
    VoreenButton(QWidget * parent = 0);
    VoreenButton(const QString& text, QWidget* parent = 0);
    VoreenButton(const QIcon& icon, const QString& text = 0, QWidget* parent = 0);

	void setColor(QColor & color)         { color_ = color; }
	void setHighlight(QColor & highlight) { highlightColor_ = highlight; }
	void setShadow(QColor & shadow)       { shadowColor_ = shadow; }

	//Range: 0.0 [invisible] - 1.0 [opaque]
	void setOpacity(qreal opacity)        { opacity_ = opacity; }

	//Range: 0 [rectangle] - 99 [oval]
	void setRoundness(int roundness)      { roundness_ = roundness; }

protected:
    static const int BUTTON_SIZE;
    
	void paintEvent(QPaintEvent* pe);

	void enterEvent(QEvent* e);
	void leaveEvent(QEvent* e);

	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);

private:
	QRect calculateIconPosition(QRect button_rect, QSize icon_size);

	bool hovered_;
	bool pressed_;

	QColor color_;
	QColor highlightColor_;
	QColor shadowColor_;

	qreal opacity_;

	int roundness_;
};

#endif // VRN_VOREENBUTTON_H
