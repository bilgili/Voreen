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

#include "voreen/qt/qcolorpicker.h"

#include <QMouseEvent>
#include <QPainter>

namespace voreen {

QColorPicker::QColorPicker(QWidget* parent)
    : QFrame(parent)
{
    hue = 0; sat = 0;
    setCol(0, 0);
    pix = 0;

    setAttribute(Qt::WA_NoSystemBackground);
//    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );

	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    setMinimumSize(128, 50);
}

QColorPicker::~QColorPicker() {
    delete pix;
}

QPoint QColorPicker::colPt() { 
    return QPoint((360-hue)*(contentsRect().width()-1)/360, (255-sat)*(contentsRect().height()-1)/255); 
}

int QColorPicker::huePt(const QPoint &pt) {
    if (contentsRect().width()-1 != 0)
        return 360 - pt.x()*360/(contentsRect().width()-1); 
    else
        return 0;
}

int QColorPicker::satPt(const QPoint &pt) {
    if (contentsRect().height()-1 != 0)
        return 255 - pt.y()*255/(contentsRect().height()-1) ; 
    else
        return 0;
}

void QColorPicker::setCol(const QPoint &pt) {
    setCol(huePt(pt), satPt(pt)); 
}

// QSize QColorPicker::sizeHint() const
// {
//     return QSize(150 + 2*frameWidth(), 150 + 2*frameWidth());
// }

void QColorPicker::setCol(int h, int s) {
    int nhue = qMin(qMax(0,h), 359);
    int nsat = qMin(qMax(0,s), 255);
    if (nhue == hue && nsat == sat)
        return;
    QRect r(colPt(), QSize(20,20));
    hue = nhue;
    sat = nsat;
    r = r.unite(QRect(colPt(), QSize(20,20)));
    r.translate(contentsRect().x()-9, contentsRect().y()-9);
    repaint(r);
}

void QColorPicker::setCol(const QColor& c) {
    setCol(c.hue(), c.saturation());
}

void QColorPicker::mouseMoveEvent(QMouseEvent *m) {
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol(p);
    emit newCol(hue, sat);
}

void QColorPicker::mousePressEvent(QMouseEvent *m) {
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol(p);
    emit newCol(hue, sat);
}

void QColorPicker::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    drawFrame(&p);
    QRect r = contentsRect();
    if (r.width() <= 1 || r.height() <= 1)
        return;

    int w = r.width();
    int h = r.height();
    if (!pix || (pix->width() != w) || (pix->height() != h)) {
        QImage img(w, h, QImage::Format_RGB32);
        img.fill(Qt::blue);
        for (int y = 0 ; y < h ; ++y)
            for (int x = 0 ; x < w ; ++x) {
                QPoint p(x, y);
                QColor c;
                c.setHsv(huePt(p), satPt(p), 200);
                img.setPixel(x, y, c.rgb());
            }
        pix = new QPixmap(QPixmap::fromImage(img));
    }

    p.drawPixmap(r.topLeft(), *pix);
    QPoint pt = colPt() + r.topLeft();

    p.setPen(Qt::black);
    p.fillRect(pt.x()-9, pt.y(), 20, 2, Qt::black);
    p.fillRect(pt.x(), pt.y()-9, 2, 20, Qt::black);

}

} // namespace
