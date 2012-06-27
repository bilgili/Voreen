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

#include "voreen/qt/widgets/network/editor/tctooltip.h"

#include "voreen/core/vis/rendertarget.h"

#include <QImage>
#include <QPainter>

#include "tgt/vector.h"
#include "tgt/logmanager.h"

namespace voreen {

TCTooltip::TCTooltip(QGraphicsItem* parent)
    : QGraphicsRectItem(parent), image_(0)
{}

TCTooltip::TCTooltip(const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsRectItem(rect, parent), image_(0)
{}

TCTooltip::TCTooltip(qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent)
    : QGraphicsRectItem(x, y, width, height, parent), image_(0)
{}

TCTooltip::~TCTooltip() {
    delete image_;
}

void TCTooltip::initialize(RenderTarget* rt) {
    tgt::ivec2 size = rt->getSize();
    image_ = new QImage(size.x, size.y, QImage::Format_ARGB32);

    tgt::Texture* tex = rt->getColorTexture();
    if(!tex)
        return;

    tex->downloadTexture();

    int posi = 0;
    QColor color;

    float* temp = reinterpret_cast<float*>(tex->getPixelData());
    // The pixels are stored row by row from bottom to top an in each row from left to right
    for (int y=0; y < size.y; ++y) {
        for (int x=0; x < size.x; ++x) {
            tgt::Color col;
            col.r = temp[posi++];
            col.g = temp[posi++];
            col.b = temp[posi++];
            col.a = temp[posi++];

            // for some unknown reason the float can get values slight above 1.0, so clamp them
            // here to prevent warning from setRgbF
            col = tgt::clamp(col, 0.f, 1.f);

            color.setRgbF(col.r, col.g, col.b, col.a);

            //(0,0) is top left
            image_->setPixel(x, size.y - 1 - y, color.rgba());
        }
    }

    // fit rect
    float image_aspect = static_cast<float>(size.x) / static_cast<float>(size.y);
    float tip_aspect = rect().width() / rect().height();
    if (image_aspect > tip_aspect) {
        // image is wider - lower tooltip's height
        float newheight = rect().width() / image_aspect;
        float dh = rect().height() - newheight;
        setRect(rect().adjusted(0.f, dh, 0.f, 0.f));
    }
    else if (image_aspect < tip_aspect) {
        // image is higher - lower tooltip's width
        float newwidth = rect().height() * image_aspect;
        float dw = rect().width() - newwidth;
        setRect(rect().adjusted(dw, 0.f, 0.f, 0.f));
    }

    // checkers background
    QPixmap pm(20, 20);
    QPainter pmp(&pm);
    pmp.fillRect(0, 0, 10, 10, Qt::lightGray);
    pmp.fillRect(10, 10, 10, 10, Qt::lightGray);
    pmp.fillRect(0, 10, 10, 10, Qt::darkGray);
    pmp.fillRect(10, 0, 10, 10, Qt::darkGray);
    pmp.end();
    setBrush(QBrush(pm));
}

void TCTooltip::paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QGraphicsRectItem::paint(painter, option, widget);
    painter->drawRect(rect());
    if (image_)
        painter->drawImage(rect(), *image_);
}

} // namespace voreen
