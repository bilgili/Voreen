/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/qt/widgets/transfunc/colorluminancepicker.h"

#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>

namespace voreen {

ColorLuminancePicker::ColorLuminancePicker(QWidget* parent)
    : QWidget(parent)
    , hue_(0)
    , sat_(0)
    , val_(0)
    , pix_(0)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    setMinimumSize(10, 50);
}

ColorLuminancePicker::~ColorLuminancePicker() {
    delete pix_;
}

int ColorLuminancePicker::y2val(int y) {
    int d = height() - 2*coff - 1;
    if (d != 0)
        return 255 - (y - coff)*255/d;
    else
        return 0;
}

int ColorLuminancePicker::val2y(int v) {
    int d = height() - 2*coff - 1;
    return coff + (255-v)*d/255;
}

void ColorLuminancePicker::mouseMoveEvent(QMouseEvent* event) {
    setVal(y2val(event->y()));
}

void ColorLuminancePicker::mousePressEvent(QMouseEvent* event) {
    emit toggleInteractionMode(true);
    setVal(y2val(event->y()));
}

void ColorLuminancePicker::mouseReleaseEvent(QMouseEvent* event) {
    event->accept();
    emit toggleInteractionMode(false);
}

void ColorLuminancePicker::setVal(int v) {
    if (val_ == v)
        return;

    val_ = qMax(0, qMin(v,255));

    delete pix_;
    pix_ = 0;

    repaint();

    emit newHsv(hue_, sat_, val_);
}

void ColorLuminancePicker::setCol(int h, int s) {
    setCol(h, s, val_);
    emit newHsv(h, s, val_);
}

void ColorLuminancePicker::paintEvent(QPaintEvent* /*event*/) {
    int w = width() - 5;

    QRect r(0, foff, w, height() - 2*foff);
    int wi = r.width() - 2;
    int hi = r.height() - 2;
    if (wi <= 1 || hi <= 1)
        return;
    if ((pix_ == 0) || (pix_->height() != hi) || (pix_->width() != wi)) {
        if (!pix_)
            delete pix_;
        QImage img(wi, hi, QImage::Format_RGB32);
        for (int y = 0 ; y < hi ; ++y) {
            QColor c;
            c.setHsv(hue_, sat_, y2val(y+coff));
            QRgb r = c.rgb();
            for (int x = 0 ; x < wi ; ++x)
                img.setPixel(x, y, r);
        }
        pix_ = new QPixmap(QPixmap::fromImage(img));
    }
    // color bar
    QPainter p(this);
    p.drawPixmap(1, coff, *pix_);
    const QPalette &g = palette();
    qDrawShadePanel(&p, r, g, true);
    p.setPen(g.foreground().color());
    p.setBrush(g.foreground());

    // arrow
    QPolygon a;
    int y = val2y(val_);
    a.setPoints(3, w, y, w+5, y+5, w+5, y-5);
    p.eraseRect(w, 0, 5, height());
    p.drawPolygon(a);
}

void ColorLuminancePicker::setCol(int h, int s , int v) {
    val_ = v;
    hue_ = h;
    sat_ = s;
    delete pix_;
    pix_ = 0;
    repaint();
}

void ColorLuminancePicker::setCol(const QColor& c) {
    setCol(c.hue(), c.saturation(), c.value());
}

} // namespace voreen
