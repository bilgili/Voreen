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

#include "voreen/qt/widgets/transfunc/histogrampainter.h"

#include "voreen/core/volume/histogram.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPoint>

namespace voreen {

HistogramPainter::HistogramPainter(QWidget* parent, tgt::vec2 xRange, tgt::vec2 yRange,
                                   int padding, int arrowLength)
    : QWidget(parent)
    , xRange_(xRange)
    , yRange_(yRange)
    , padding_(padding)
    , arrowLength_(arrowLength)
    , histogram_(0)
    , cache_(0)
{
}

HistogramPainter::~HistogramPainter() {
    delete histogram_;
    delete cache_;
}

void HistogramPainter::setHistogram(HistogramIntensity* histogram) {
    delete histogram_;
    histogram_ = histogram;
    delete cache_;
    cache_ = 0;
}

void HistogramPainter::setxRange(tgt::vec2 xRange) {
    xRange_ = xRange;
    delete cache_;
    cache_ = 0;
}

void HistogramPainter::paintEvent(QPaintEvent* event) {
    event->accept();

    if (cache_ == 0 || cache_->rect() != rect()) {
        delete cache_;
        cache_ = new QPixmap(rect().size());
        cache_->fill(Qt::transparent);
            
        QPainter paint(cache_);
    
        // put origin in lower lefthand corner
        QMatrix m;
        m.translate(0.0, static_cast<float>(height())-1);
        m.scale(1.f, -1.f);
        paint.setMatrix(m);
        paint.setMatrixEnabled(true);

        if (histogram_) {
            // draw histogram
            paint.setPen(Qt::NoPen);
            paint.setBrush(QColor(200, 0, 0, 120));
            paint.setRenderHint(QPainter::Antialiasing, true);

            //Qt can't handle polygons that have more than 65536 points
            //so we have to split the polygon
            int histogramWidth = static_cast<int>(histogram_->getBucketCount());
            QPointF* points;
            tgt::vec2 p;
            if (histogramWidth == 65536) {//16 bit dataset
                points = new QPointF[histogramWidth-1];
                for (int x=0; x<histogramWidth-3; ++x) {
                    float value = histogram_->getLogNormalized(x);
                    p = wtos(tgt::vec2(static_cast<float>(x)/(histogramWidth-1),
                                       value * (yRange_[1] - yRange_[0]) + yRange_[0]));
                    points[x].rx() = p.x;
                    points[x].ry() = p.y;
                }
                //needed for a closed polygon
                p = wtos(tgt::vec2((histogramWidth-3.f)/(histogramWidth-1.f), yRange_[0]));
                points[histogramWidth-3].rx() = p.x;
                points[histogramWidth-3].ry() = p.y;
                p = wtos(tgt::vec2(0.f, yRange_[0]));
                points[histogramWidth-2].rx() = p.x;
                points[histogramWidth-2].ry() = p.y;

                paint.drawPolygon(points, histogramWidth-1);

                //last points
                delete[] points;
                points = new QPointF[5];
                for (int x=histogramWidth-3; x<histogramWidth; ++x) {
                    float value = histogram_->getLogNormalized(x);
                    p = wtos(tgt::vec2(static_cast<float>(x)/(histogramWidth-1), value * (yRange_[1] - yRange_[0]) + yRange_[0]));
                    points[x-histogramWidth+3].rx() = p.x;
                    points[x-histogramWidth+3].ry() = p.y;
                }
                //needed for a closed polygon
                p = wtos(tgt::vec2(1.f, yRange_[0]));
                points[3].rx() = p.x;
                points[3].ry() = p.y;
                p = wtos(tgt::vec2((histogramWidth-3.f)/(histogramWidth-1.f), yRange_[0]));
                points[4].rx() = p.x;
                points[4].ry() = p.y;

                paint.drawPolygon(points, 5);
            }
            else {
                points = new QPointF[histogramWidth + 2];
                for (int x=0; x<histogramWidth; ++x) {
                    float value = histogram_->getLogNormalized(x);
                    p = wtos(tgt::vec2(static_cast<float>(x)/(histogramWidth-1), value * (yRange_[1] - yRange_[0]) + yRange_[0]));
                    points[x].rx() = p.x;
                    points[x].ry() = p.y;
                }
                //needed for a closed polygon
                p = wtos(tgt::vec2(1.f, yRange_[0]));
                points[histogramWidth].rx() = p.x;
                points[histogramWidth].ry() = p.y;
                p = wtos(tgt::vec2(0.f, yRange_[0]));
                points[histogramWidth+1].rx() = p.x;
                points[histogramWidth+1].ry() = p.y;

                paint.drawPolygon(points, histogramWidth + 2);
            }
            delete[] points;
        }
    }

    QPainter paint(this);
    paint.drawPixmap(0, 0, *cache_);    
}

tgt::vec2 HistogramPainter::wtos(tgt::vec2 p) {
    float sx = (p.x - xRange_[0]) / (xRange_[1] - xRange_[0]) * (static_cast<float>(width())  - 2 * padding_ - 1.5 * arrowLength_) + padding_;
    float sy = (p.y - yRange_[0]) / (yRange_[1] - yRange_[0]) * (static_cast<float>(height()) - 2 * padding_ - 1.5 * arrowLength_) + padding_;
    return tgt::vec2(sx, sy);
}

} // namespace voreen
