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

//
// NOTE: This file is currently not used.
//
#ifndef TRANSFERMARKER_H
#define TRANSFERMARKER_H

#include <QColor>
#include <QWidget>

#define MAX_NORM_VAL 255.0

namespace voreen {

class TransferMarker : public QWidget {
    Q_OBJECT

public:
    TransferMarker(QWidget* parent = 0);

	void setColor(QColor color);
	QColor getColor();

	void setIntensity(unsigned int intensity);
	unsigned int getIntensity();

	void setTransparency(unsigned int intensity);
	unsigned int getTransparency();
    void paintEvent(QPaintEvent *);

private:
	void updateToolTip();

	QColor color_;
	// intensity_ and transparency_ should lie in the interval [0...MAX_NORM_VAL]
	unsigned int intensity_;
	unsigned int transparency_;
};

} // namespace voreen

#endif
