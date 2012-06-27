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

#include "voreen/core/vis/transfunc/transfunc.h"


#include <cmath>
#include <tinyxml/tinyxml.h>

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/transfunc/transfuncintensity.h"

using tgt::vec2;
using tgt::ivec2;
using tgt::ivec3;
using tgt::ivec4;
using tgt::col4;
using tgt::dvec4;


namespace voreen {

void TransFunc::saveXml(TiXmlElement* root, const vec2& v) {
    TiXmlElement* e = new TiXmlElement("vec2");
    e->SetDoubleAttribute("x", v.x);
    e->SetDoubleAttribute("y", v.y);
    root->LinkEndChild(e);
}

void TransFunc::saveXml(TiXmlElement* root, const col4& c) {
    TiXmlElement* e = new TiXmlElement("col4");
    e->SetAttribute("r", c.r);
    e->SetAttribute("g", c.g);
    e->SetAttribute("b", c.b);
    e->SetAttribute("a", c.a);
    root->LinkEndChild(e);
}

void TransFunc::loadXml(TiXmlElement* root, vec2& v) {
    double temp;
    if (root->Attribute("x", &temp))
        v.x = static_cast<float>(temp);
    if (root->Attribute("y", &temp))
        v.y = static_cast<float>(temp);
}

void TransFunc::loadXml(TiXmlElement* root, col4& c) {
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

//---------------------------------------------------------------------------

const std::string TransFunc::loggerCat_("voreen.voreen.TransFunc");

TransFunc::TransFunc(): tex_(0) {
}

TransFunc::~TransFunc() {
    if (tex_)
        delete tex_;
}

bool TransFunc::load(const std::string filename){
    tex_ = TexMgr.load(filename, tgt::Texture::NEAREST, false, true, true);
    tex_->uploadTexture();

    if (tex_)
        return true;
    else
        return false;
}

void TransFunc::bind() {
    tex_->bind();
}

tgt::Texture* TransFunc::getTexture() {
    return tex_;
}

const tgt::Texture* TransFunc::getTexture() const {
    return tex_;
}

//---------------------------------------------------------------------------

TransFunc2D::TransFunc2D() {
}

TransFunc2D::~TransFunc2D() {
}

bool TransFunc2D::load(const std::string& filename) {
    if (tex_)
        delete tex_;
    LINFO("TransFunc2D (Image): " << filename);

    tgt::Texture* tex_ = TexMgr.load(filename, tgt::Texture::NEAREST, false, true, true);

    if (!tex_) {
        LERROR("unable to open file.");
        return false;
    }
    return true;
}

TransFunc2D* TransFunc2D::createPreIntegration(TransFunc* source) {
    int size = source->getTexture()->getWidth();
    TransFunc2D* tf = new TransFunc2D();
    tf->tex_ = new tgt::Texture( ivec3(size, size, 1), GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE );

    dvec4* table = new dvec4[size];
    dvec4 dv4(0.0);

    // calculate integral stuff
    table[0] = dvec4(0.0);
    for (int i = 1 ; i < size ; ++i) {
        col4 col1 = source->getTexture()->texel<col4>(i-1);
        col4 col2 = source->getTexture()->texel<col4>(i);
        double tauc = (col1.a + col2.a) / 2.0;
        dv4.r += (col1.r + col2.r) / 2.0 * tauc/255.0;
        dv4.g += (col1.g + col2.g) / 2.0 * tauc/255.0;
        dv4.b += (col1.b + col2.b) / 2.0 * tauc/255.0;
        dv4.a += tauc;
        table[i] = dv4;
    }

    ivec4 iv4(0);

    // calculate the look-up table
    for (int sb = 0 ; sb < size ; ++sb) {
        for (int sf = 0 ; sf < size ; ++sf) {
            int smin = std::min(sb, sf);
            int smax = std::max(sb, sf);

            if (smin != smax) {
                double factor = 1.0 / double(smax - smin);
                iv4.r = static_cast<int>((table[smax].r - table[smin].r) * factor + 0.5);
                iv4.g = static_cast<int>((table[smax].g - table[smin].g) * factor + 0.5);
                iv4.b = static_cast<int>((table[smax].b - table[smin].b) * factor + 0.5);
                iv4.a = static_cast<int>(256.0 * (1.0 - exp( (-(table[smax].a - table[smin].a) * factor)/255.0 )) + 0.5);
            }
            else {
                // -> smin == smax
                static const double factor = 1.0 / 255.0;
                col4 col = source->getTexture()->texel<col4>(smin);
                iv4.r = static_cast<int>(col.r * col.a * factor + 0.5);
                iv4.g = static_cast<int>(col.g * col.a * factor + 0.5);
                iv4.b = static_cast<int>(col.b * col.a * factor + 0.5);
                iv4.a = static_cast<int>((1.0 - exp( -col.a * factor )) * 256.0 + 0.5);
            }

            tf->tex_->texel<col4>(sf, sb) = col4( clamp(iv4, 0, 255) );;
        }
    }

    tf->tex_->uploadTexture();
    tf->tex_->setWrapping(tgt::Texture::CLAMP);

    // clean up
    delete[] table;

    return tf;
}

uint TransFunc2D::getPixelInQtFormat(uint x, uint y) {
    return (( uint(tex_->texel<col4>(x,y).r) << 16)
          | ( uint(tex_->texel<col4>(x,y).g) <<  8)
          | ( uint(tex_->texel<col4>(x,y).b)));
}

void TransFunc2D::setPixelFromQtFormat(uint x, uint y, uint color) {
    tex_->texel<col4>(x,y).r = static_cast<uint8_t>(color >> 16);
    tex_->texel<col4>(x,y).g = static_cast<uint8_t>(color >> 8);
    tex_->texel<col4>(x,y).b = static_cast<uint8_t>(color);
}

std::string TransFunc2D::getShaderDefines() {
    return "";
}

} // namespace voreen
