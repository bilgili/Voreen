/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2009 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "tgt/font.h"
#include "tgt/logmanager.h"

#ifdef TGT_HAS_FTGL
#include <FTGL/ftgl.h>
#include "tgt/tgt_gl.h"
#endif

namespace tgt {

#ifdef TGT_HAS_FTGL

Font::Font(const std::string& fontName, const int size, FontType fontType) {
    switch(fontType) {
        case BitmapFont:
            font_ = new FTBitmapFont(fontName.c_str()); break;
        case BufferFont:
            font_ = new FTBufferFont(fontName.c_str()); break;
        case ExtrudeFont:
            font_ = new FTExtrudeFont(fontName.c_str()); break;
        case OutlineFont:
            font_ = new FTOutlineFont(fontName.c_str()); break;
        case PixmapFont: 
            font_ = new FTPixmapFont(fontName.c_str()); break;
        case PolygonFont: 
            font_ = new FTPolygonFont(fontName.c_str()); break;
        case TextureFont: 
            font_ = new FTTextureFont(fontName.c_str()); break;
        default:
            LWARNINGC("tgt.Font", "Unknown fontType. Defaulting to TextureFont.");
            font_ = new FTTextureFont(fontName.c_str());
    }

    if (!font_->Error())
        font_->FaceSize(size);
    else {
        delete font_;
        font_ = 0;
        LERRORC("tgt.Font", "Font file could not be loaded: " << fontName);
    }

    simpleLayout_ = new FTSimpleLayout();
    simpleLayout_->SetFont(font_);
    simpleLayout_->SetLineLength(1000.0f);
}

Font::~Font() {
    delete font_;
    delete simpleLayout_;
}

float Font::getLineHeight() {
    return font_->LineHeight();
}

void Font::setLineWidth(const float lineWidth) {
    if(simpleLayout_)
        simpleLayout_->SetLineLength(lineWidth);
}

void Font::setTextAlignment(TextAlignment textAlignment) {
    FTGL::TextAlignment alignment = FTGL::ALIGN_LEFT;
    switch(textAlignment) {
        case Left:
            alignment = FTGL::ALIGN_LEFT;
            break;
        case Center:
            alignment = FTGL::ALIGN_CENTER;
            break;
        case Right:
            alignment = FTGL::ALIGN_RIGHT;
            break;
    }
    simpleLayout_->SetAlignment(alignment);
}

void Font::setSize(const int size) {
    if (font_)
        font_->FaceSize(size);
}

void Font::render(const tgt::vec3& pos, const std::string& text) {
    if (font_) {
        glRasterPos2f(pos.x, pos.y);
        font_->Render(text.c_str(), -1, FTPoint(pos.x, pos.y, pos.z));
    }
}

void Font::renderWithLayout(const tgt::vec3& pos, const std::string& text) {
    if (simpleLayout_) {
        glRasterPos2f(pos.x, pos.y);
        simpleLayout_->Render(text.c_str(), -1, FTPoint(pos.x, pos.y, pos.z));
    }
}

tgt::Bounds Font::getBounds(const tgt::vec3& pos, const std::string& text) {
    if (!font_)
        return tgt::Bounds();

    FTPoint point(static_cast<double>(pos.x),
                  static_cast<double>(pos.y),
                  static_cast<double>(pos.z));

    FTBBox box = font_->BBox(text.c_str(), -1, point);
    FTPoint upper = box.Upper();
    FTPoint lower = box.Lower();
    tgt::vec3 upperTGT = tgt::vec3(upper.Xf(), upper.Yf(), upper.Zf());
    tgt::vec3 lowerTGT = tgt::vec3(lower.Xf(), lower.Yf(), lower.Zf());

    return tgt::Bounds(lowerTGT, upperTGT);
}

#else

Font::Font(const std::string& /*fontName*/, const int /*size*/, FontType /*fontType*/) {}

Font::~Font() {}

float Font::getLineHeight() {
    return 0.f;
}

void Font::setLineWidth(const float /*lineWidth*/) {}

void Font::setSize(const int /*size*/) {}

void Font::setTextAlignment(TextAlignment /*textAlignment*/) {}

void Font::render(const tgt::vec3& /*pos*/, const std::string& /*text*/) {}

void Font::renderWithLayout(const tgt::vec3& /*pos*/, const std::string& /*text*/) {}

tgt::Bounds Font::getBounds(const tgt::vec3& /*pos*/, const std::string& /*text*/) {
    return tgt::Bounds();
}

#endif

} // namespace tgt
