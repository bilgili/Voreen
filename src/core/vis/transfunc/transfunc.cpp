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

#include "voreen/core/vis/transfunc/transfunc.h"

#include "tgt/gpucapabilities.h"

#include <tinyxml/tinyxml.h>

namespace voreen {

const std::string TransFunc::loggerCat_("voreen.core.transfunc");

TransFunc::TransFunc(int width, int height)
    : tex_(0)
    , textureUpdateNeeded_(true)
{
    fitDimension(width, height);
    dimension_ = tgt::ivec2(width, height);
}

TransFunc::~TransFunc() {
    delete tex_;
}

const std::vector<std::string>& TransFunc::getLoadFileFormats() {
    return loadFileFormats_;
}

const std::vector<std::string>& TransFunc::getSaveFileFormats() {
    return saveFileFormats_;
}

void TransFunc::textureUpdateNeeded() {
    textureUpdateNeeded_ = true;
}

void TransFunc::bind() {
    if (textureUpdateNeeded_)
        updateTexture();

    tex_->bind();
}

tgt::Texture* TransFunc::getTexture() {
    if (textureUpdateNeeded_)
        updateTexture();

    return tex_;
}

void TransFunc::fitDimension(int& width, int& height) {
    if (!GpuCaps.isNpotSupported()) {
        int k = 1;
        while (k < width)
            k <<= 1;
        if (k != width)
            width = k;

        k = 1;
        while (k < height)
            k <<= 1;
        if (k != height)
            height = k;
    }

    int maxTexSize;
    if (GpuCaps.areSharedPalettedTexturesSupported())
        /*
            color table for the paletted textures may only have 256 entries
            (At least for GeForce 4. I didn't find a proper OpenGL getter).
        */
        maxTexSize = 256;
    else
        maxTexSize = GpuCaps.getMaxTextureSize();

    if (maxTexSize < width)
        width = maxTexSize;

    if (maxTexSize < height)
        height = maxTexSize;
}

void TransFunc::setTextureDimension(int width, int height) {
    fitDimension(width, height);

    if (width != dimension_.x) {
        dimension_.x = width;
        textureUpdateNeeded_ = true;
    }
    if (height != dimension_.y) {
        dimension_.y = height;
        textureUpdateNeeded_ = true;
    }
}

void TransFunc::saveXml(TiXmlElement* root, const tgt::vec2& v) {
    TiXmlElement* e = new TiXmlElement("vec2");
    e->SetDoubleAttribute("x", v.x);
    e->SetDoubleAttribute("y", v.y);
    root->LinkEndChild(e);
}

void TransFunc::saveXml(TiXmlElement* root, const tgt::col4& c) {
    TiXmlElement* e = new TiXmlElement("col4");
    e->SetAttribute("r", c.r);
    e->SetAttribute("g", c.g);
    e->SetAttribute("b", c.b);
    e->SetAttribute("a", c.a);
    root->LinkEndChild(e);
}

void TransFunc::loadXml(TiXmlElement* root, tgt::vec2& v) {
    double temp;
    if (root->Attribute("x", &temp))
        v.x = static_cast<float>(temp);
    if (root->Attribute("y", &temp))
        v.y = static_cast<float>(temp);
}

void TransFunc::loadXml(TiXmlElement* root, tgt::col4& c) {
    int temp;
    if (root->Attribute("r", &temp))
        c.r = temp;
    if (root->Attribute("g", &temp))
        c.g = temp;
    if (root->Attribute("b", &temp))
        c.b = temp;
    if (root->Attribute("a", &temp))
        c.a = temp;
}

} // namespace voreen
