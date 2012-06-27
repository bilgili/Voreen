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

#include "voreen/qt/qcolorluminancepicker.h"

#include <QMouseEvent>
#include <QPainter>

namespace voreen {
int QColorLuminancePicker::y2val(int y) {
    int d = height() - 2*coff - 1;
    if (d != 0)
        return 255 - (y - coff)*255/d;
    else return 0;
}

int QColorLuminancePicker::val2y(int v) {
    int d = height() - 2*coff - 1;
    return coff + (255-v)*d/255;
}

QColorLuminancePicker::QColorLuminancePicker(QWidget* parent)
    : QWidget(parent)
{
    hue = 0; val = 0; sat = 0;
    pix = 0;
    //    setAttribute(WA_NoErase, true);
	setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    setMinimumSize(10, 50);

}

QColorLuminancePicker::~QColorLuminancePicker() {
    delete pix;
}

void QColorLuminancePicker::mouseMoveEvent(QMouseEvent *m) {
    setVal(y2val(m->y()));
}

void QColorLuminancePicker::mousePressEvent(QMouseEvent *m) {
    setVal(y2val(m->y()));
}

void QColorLuminancePicker::setVal(int v) {
    if (val == v)
        return;

    val = qMax(0, qMin(v,255));

    delete pix;
    pix = 0;
    repaint();

    emit newHsv(hue, sat, val);
}

// receives from a hue,sat chooser and relays.
void QColorLuminancePicker::setCol(int h, int s)
{
    setCol(h, s, val);
    emit newHsv(h, s, val);
}

void QColorLuminancePicker::paintEvent(QPaintEvent *) {
    int w = width() - 5;

    QRect r(0, foff, w, height() - 2*foff);
    int wi = r.width() - 2;
    int hi = r.height() - 2;
    if (wi <= 1 || hi <= 1) return;
    if ((pix == 0) || (pix->height() != hi) || (pix->width() != wi)) {
        delete pix;
        QImage img(wi, hi, QImage::Format_RGB32);
        int y;
        for (y = 0 ; y < hi ; ++y) {
            QColor c;
            c.setHsv(hue, sat, y2val(y+coff));
            QRgb r = c.rgb();
            for (int x = 0 ; x < wi ; ++x)
                img.setPixel(x, y, r);
        }
        pix = new QPixmap(QPixmap::fromImage(img));
    }
    QPainter p(this);
    p.drawPixmap(1, coff, *pix);
    const QPalette &g = palette();
    qDrawShadePanel(&p, r, g, true);
    p.setPen(g.foreground().color());
    p.setBrush(g.foreground());
    QPolygon a;
    int y = val2y(val);
    a.setPoints(3, w, y, w+5, y+5, w+5, y-5);
    p.eraseRect(w, 0, 5, height());
    p.drawPolygon(a);
}

void QColorLuminancePicker::setCol(int h, int s , int v) {
    val = v;
    hue = h;
    sat = s;
    delete pix;
    pix = 0;
    repaint();
}

void QColorLuminancePicker::setCol(const QColor& c) {
    setCol(c.hue(), c.saturation(), c.value());
}

} // namespace
