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
#ifndef TRANSFERMARKERFIELD_H
#define TRANSFERMARKERFIELD_H

#include <QColorDialog>
#include <QVector>
#include <QFrame>

#include "transfermarker.h"
#include "transfergradient.h"

#include <voreen/core/vis/volumerendering.h>

namespace voreen {

class TransferMarkerField : public QWidget {
    Q_OBJECT

public:
	TransferMarkerField(QWidget* parent = 0, TransferGradient* gradient = 0, MessageReceiver* msgReceiver = 0);

	inline QVector<TransferMarker*>* getMarkers() {return markers_;}

    void saveToDisc(QString fileName);
    void readFromDisc(QString fileName);

public slots:
    void saveToDisc();
    void readFromDisc();

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    void clean();
    void addMarker(unsigned int intensity, unsigned int transparency, QColor color);

	int markerSize_;
	int curMarker_;
	QVector<TransferMarker*>* markers_;
	TransferGradient* gradient_;

	bool mouseDown_;
	MessageReceiver* msgReceiver_;
};

} // namespace voreen

#endif
