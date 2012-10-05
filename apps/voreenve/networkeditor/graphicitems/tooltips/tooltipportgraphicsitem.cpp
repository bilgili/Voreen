/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "tooltipportgraphicsitem.h"
#include "../../styles/nwestyle_base.h"

#include "../core/portgraphicsitem.h"
//ports
#include "voreen/core/ports/port.h"
#include "voreen/core/ports/renderport.h"
#include "voreen/core/ports/volumeport.h"
    //volume
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumepreview.h"

#include <QGraphicsTextItem>

namespace voreen {

ToolTipPortGraphicsItem::ToolTipPortGraphicsItem(PortGraphicsItem* item) 
    : ToolTipBaseGraphicsItem(item)
    , image_(0)
{
}

//---------------------------------------------------------------------------------------------------------------
//                  getter and setter                                                                            
//---------------------------------------------------------------------------------------------------------------
const QImage* ToolTipPortGraphicsItem::getImage() const{
    return image_;
}

const float ToolTipPortGraphicsItem::getImageAspect() const {
    return image_aspect_;
}

//---------------------------------------------------------------------------------------------------------------
//                  nwebasegraphicsitem functions                                                                
//---------------------------------------------------------------------------------------------------------------
QRectF ToolTipPortGraphicsItem::boundingRect() const {
    return currentStyle()->ToolTipPortGI_boundingRect(this);
}

QPainterPath ToolTipPortGraphicsItem::shape() const {
    return currentStyle()->ToolTipPortGI_shape(this);
}

void ToolTipPortGraphicsItem::initializePaintSettings() {
    currentStyle()->ToolTipPortGI_initializePaintSettings(this);
}

void ToolTipPortGraphicsItem::prePaint(QPainter* painter, const QStyleOptionGraphicsItem* option) {}

void ToolTipPortGraphicsItem::mainPaint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    currentStyle()->ToolTipPortGI_paint(this,painter,option,widget);
}

//---------------------------------------------------------------------------------------------------------------
//                  tooltipbasegraphicsitem functions                                                                
//---------------------------------------------------------------------------------------------------------------
void ToolTipPortGraphicsItem::updateToolTip(qreal x, qreal y) {
    prepareGeometryChange();
    toolTipText_->setHtml(QString::fromStdString(dynamic_cast<PortGraphicsItem*>(parent())->getPort()->getContentDescriptionHTML()));

    //test, if we have a image to show
    RenderPort* rp = 0;
    VolumePort* vp = 0;
    //try+catch to guarantee a call of TollTipBase::updateToolTip
    try {
        if (rp = dynamic_cast<RenderPort*>(dynamic_cast<PortGraphicsItem*>(parent())->getPort())) {
            tgt::ivec2 size = rp->getSize();
            tgt::Texture* tex = rp->getColorTexture();
            delete image_;
            image_ = 0;
            if (!tex)
                throw "exception";

            // download pixel data to buffer
            tgt::col4* pixels = 0;
            try {
                GLubyte* pixels_b = tex->downloadTextureToBuffer(GL_RGBA, GL_UNSIGNED_BYTE);
                LGL_ERROR;
                if (pixels_b)
                    pixels = reinterpret_cast<tgt::col4*>(pixels_b);
                else {
                    LERRORC("ToolTipPortGraphicsItem::updateToolTip", "failed to download texture");
                    throw "exception";
                }
            }
            catch (std::bad_alloc&) {
                LERRORC("ToolTipPortGraphicsItem::updateToolTip", "bad allocation");
                throw "exception";
            }

            image_ = new QImage(size.x, size.y, QImage::Format_ARGB32);

            // The pixels are stored row by row from bottom to top an in each row from left to right
            QColor color;
            for (int y=0; y < size.y; ++y) {
                for (int x=0; x < size.x; ++x) {
                    tgt::col4 col = pixels[x + y*size.x];
                    color.setRgb(col.r, col.g, col.b, col.a);
                    //(0,0) is top left
                    image_->setPixel(x, size.y - 1 - y, color.rgba());
                }
            }
            delete[] pixels;
            pixels = 0;

            image_aspect_ = static_cast<float>(size.x) / static_cast<float>(size.y);
        } 
        else if(vp = dynamic_cast<VolumePort*>(dynamic_cast<PortGraphicsItem*>(parent())->getPort())) {
            delete image_;
            image_ = 0;
            if (!vp->getData())
                throw "exception";

            VolumePreview* prev = vp->getData()->getDerivedData<VolumePreview>();
            int internHeight = prev->getHeight();

            QImage origImg = QImage(internHeight, internHeight, QImage::Format_ARGB32);
            for (int y=0; y<internHeight; y++) {
                for (int x=0; x<internHeight; x++) {
                    int previewIndex = y * internHeight + x;
                    int greyVal = prev->getData()[previewIndex];
                    origImg.setPixel(x, y, qRgb(greyVal, greyVal, greyVal));
                }
            }
            image_ = new QImage(origImg.scaled(63, 63, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

            image_aspect_ = 1;
        }
        ToolTipBaseGraphicsItem::updateToolTip(x,y);
    }
    catch (...){
        ToolTipBaseGraphicsItem::updateToolTip(x,y);
    }
}


} // namespace voreen
