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

#include <QImage>
#include <QPainter>
#include "rpttcttooltip.h"

namespace voreen {

RptTCTTooltip::RptTCTTooltip(QGraphicsItem* parent /* =0 */)
    : QGraphicsRectItem(parent), image_(0), tc_(0), id_(0)
{}

RptTCTTooltip::RptTCTTooltip(const QRectF & rect, QGraphicsItem* parent /* = 0*/)
    : QGraphicsRectItem(rect, parent), image_(0), tc_(0), id_(0)
{}

RptTCTTooltip::RptTCTTooltip(qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent /* = 0*/) 
    : QGraphicsRectItem(x, y, width, height, parent), image_(0), tc_(0), id_(0)
{}

RptTCTTooltip::~RptTCTTooltip()
{
    if (image_)
        delete image_;
}


void RptTCTTooltip::initialize(int id, TextureContainer* tc) {
    id_ = id;
    tc_ = tc;
    initImage();
}

void RptTCTTooltip::initImage()
{
    float* temp = tc_->getTargetAsFloats(id_);
    tgt::ivec2 size = tc_->getSize();
    image_ = new QImage(size.x, size.y, QImage::Format_RGB32);
    int x,y;
    float r,g,b;
    QColor color(255,255,255,255);
    int posi = 0;
    // The pixels are stored row by row from bottom to top an in each row from left to right
    for (y=0; y < size.y; ++y) {
        for (x=0; x < size.x; ++x) {
            //posi = (x+size.x*y)*4;
            r = temp[posi++];
            g = temp[posi++];
            b = temp[posi++];
            posi++;
            color.setRgbF(r,g,b);
            // In QImage (0,0) is top left
            image_->setPixel(x,size.y-1-y,color.rgb());
        }
    }
    delete[] temp;
    // fit rect
    float image_aspect = static_cast<float>(size.x)/static_cast<float>(size.y);
    float tip_aspect = rect().width()/rect().height();
    if (image_aspect > tip_aspect) // image is wider - lower tooltip's height
    {
        float newheight = rect().width()/image_aspect;
        float dh = rect().height() - newheight;
        setRect(rect().adjusted(.0,dh,.0,.0));
    }
    else if (image_aspect < tip_aspect) // image is higher - lower tooltip's width
    {
        float newwidth = rect().height()*image_aspect;
        float dw = rect().width() - newwidth;
        setRect(rect().adjusted(dw,.0,.0,.0));
    }
    
    //setRect();
}

void RptTCTTooltip::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);
    if (image_)
        painter->drawImage(this->rect(), *image_);
}

} //namespace voreen
