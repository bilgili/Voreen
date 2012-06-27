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

#include "voreen/qt/widgets/histogrampainterwidget.h"

namespace voreen {

HistogramPainter::HistogramPainter(QWidget *parent)
    :   QWidget(parent)
{
	thresholdL_ = 0.f;
	thresholdU_ = 1.f;
	histogram_ = 0;
	setFixedHeight(60); 
}

void HistogramPainter::setHistogram(HistogramIntensity* histo_) {

	histogram_ = histo_;
}

void HistogramPainter::paintEvent(QPaintEvent *e)
{  
	QPainter *paint = new QPainter();  
	paint->begin(this);

	paint->setPen(Qt::NoPen);
    paint->setBrush(QColor(255, 255, 255, 128));
	paint->drawRect(QRect(QPoint(0,0), QPoint(width(),height())));

	if (histogram_){ 
	
	// draw histogram
    paint->setBrush(QColor(200, 0, 0, 120));
    paint->setRenderHint(QPainter::Antialiasing, true);

    int histogramWidth = histogram_->getBucketCount();

    QPointF *points = new QPointF[histogramWidth + 2];

    float max = 0.0;
    for (int i=0; i<histogramWidth; i++)
        if ((float)(histogram_->getValue(i)) > max) 
			max = (float)(histogram_->getValue(i));
	for (int x=0; x<histogramWidth; x++) {
		float value = (float)histogram_->getValue(x)/max;
		value = powf(value, 0.2f);
		tgt::vec2 p = tgt::vec2((float)x/histogramWidth * width(), (1.f - value) * (float)height());
		points[x].setX(p.x);
		points[x].setY(p.y);
	}
	tgt::vec2 p;

    points[histogramWidth].rx() = (float)width();
    points[histogramWidth].ry() = (float)height();
	
    points[histogramWidth + 1].rx() = 0.f;
    points[histogramWidth + 1].ry() = (float)height();

    paint->drawConvexPolygon(points, histogramWidth + 2);

    delete[] points;
	
	}

	// show threshold function
    paint->setPen(Qt::lightGray);
    paint->setBrush(Qt::Dense4Pattern);

    if (thresholdL_ > 0.0f) {
        paint->drawRect(0.f,0.f,thresholdL_* (float)width(), (float)height());
    }
    if (thresholdU_ < 1.0f) {
        paint->drawRect(thresholdU_ * (float)width(),0.f, (float)width(),(float)height());
    }

    paint->setRenderHint(QPainter::Antialiasing, false);

    paint->setPen(Qt::lightGray);
    paint->setBrush(Qt::NoBrush);
    paint->drawRect(0, 0, width() - 1, height() - 1);
	
	paint->end();
}

void HistogramPainter::setLowerThreshold( float value) {
	thresholdL_ = value;
	repaint();
}

void HistogramPainter::setUpperThreshold( float value ) {
	thresholdU_ = value;
	repaint();
}




} //namespace
