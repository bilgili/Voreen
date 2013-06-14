/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2005-2013 Visualization and Computer Graphics Group, *
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
#include <algorithm>
#include <sstream>

#ifdef TGT_HAS_FTGL
#include "tgt/tgt_gl.h"
#endif

namespace tgt {

#ifdef TGT_HAS_FTGL

Font::Font(const std::string& fontName, int size, FontType fontType, float lineWidth, TextAlignment textAlignment, VerticalTextAlignment verticalTextAlignment) {
    fontName_ = fontName;
    fontSize_ = size;
    fontType_ = fontType;
    lineWidth_ = lineWidth;
    hAlign_ = textAlignment;
    vAlign_ = verticalTextAlignment;
    font_ = 0;
    simpleLayout_ = new FTSimpleLayout();
    update(true);
}

Font::~Font() {
    delete font_;
    delete simpleLayout_;
}

Font::FontType Font::getFontType(const std::string& typeName) {
    if (typeName == "<empty>")
        return NIL;
    else if (typeName == "BitmapFont")
        return BitmapFont;
    else if (typeName == "BufferFont")
        return BufferFont;
    else if (typeName == "ExtrudeFont")
        return ExtrudeFont;
    else if (typeName == "OutlineFont")
        return OutlineFont;
    else if (typeName == "PixmapFont")
        return PixmapFont;
    else if (typeName == "PolygonFont")
        return PolygonFont;
    else if (typeName == "TextureFont")
        return TextureFont;
    else
        return NIL;
}

std::string Font::getFontTypeName(Font::FontType type) {
    switch (type) {
    case NIL:
        return "<empty>";
        break;
    case BitmapFont:
        return "BitmapFont";
        break;
    case BufferFont:
        return "BufferFont";
        break;
    case ExtrudeFont:
        return "ExtrudeFont";
        break;
    case OutlineFont:
        return "OutlineFont";
        break;
    case PixmapFont:
        return "PixmapFont";
        break;
    case PolygonFont:
        return "PolygonFont";
        break;
    case TextureFont:
        return "TextureFont";
        break;
    default:
        return "<unknown>";
    }
}

Font::TextAlignment Font::getTextAlignment(const std::string& textAlignment) {
    if (textAlignment == "<unknown>")
        return Left;
    else if (textAlignment == "Left")
        return Left;
    else if (textAlignment == "Center")
        return Center;
    else if (textAlignment == "Right")
        return Right;
    else
        return Left;
}

std::string Font::getTextAlignmentName(TextAlignment textAlignment) {
    switch (textAlignment) {
    case Left:
        return "Left";
        break;
    case Center:
        return "Center";
        break;
    case Right:
        return "Right";
        break;
    default:
        return "<unknown>";
    }
}

Font::VerticalTextAlignment Font::getVerticalTextAlignment(const std::string& verticalTextAlignment) {
    if (verticalTextAlignment == "<unknown>")
        return Top;
    else if (verticalTextAlignment == "Top")
        return Top;
    else if (verticalTextAlignment == "Middle")
        return Middle;
    else if (verticalTextAlignment == "Bottom")
        return Bottom;
    else
        return Top;
}

std::string Font::getVerticalTextAlignmentName(VerticalTextAlignment verticalTextAlignment) {
    switch (verticalTextAlignment) {
    case Top:
        return "Top";
        break;
    case Middle:
        return "Middle";
        break;
    case Bottom:
        return "Bottom";
        break;
    default:
        return "<unknown>";
    }
}

void Font::update(bool reloadFont) {

    if(reloadFont)
    {
        delete font_;
        font_ = 0;

        switch(fontType_) {
        case BitmapFont:
            font_ = new FTBitmapFont(fontName_.c_str()); break;
        case BufferFont:
            font_ = new FTBufferFont(fontName_.c_str()); break;
        case ExtrudeFont:
            font_ = new FTExtrudeFont(fontName_.c_str()); break;
        case OutlineFont:
            font_ = new FTOutlineFont(fontName_.c_str()); break;
        case PixmapFont:
            font_ = new FTPixmapFont(fontName_.c_str()); break;
        case PolygonFont:
            font_ = new FTPolygonFont(fontName_.c_str()); break;
        case TextureFont:
            font_ = new FTTextureFont(fontName_.c_str()); break;
        default:
            LWARNINGC("tgt.Font", "Unknown fontType. Defaulting to TextureFont.");
            font_ = new FTTextureFont(fontName_.c_str());
        }
    }

    if (font_ && !font_->Error()) {
        font_->FaceSize(fontSize_);

        //delete simpleLayout_;
        //simpleLayout_ = 0;
        //simpleLayout_ = new FTSimpleLayout();

        simpleLayout_->SetFont(font_);
        simpleLayout_->SetLineLength(lineWidth_);

        FTGL::TextAlignment hAlign = FTGL::ALIGN_LEFT;
        switch(hAlign_) {
        case Left:
            hAlign = FTGL::ALIGN_LEFT;
            break;
        case Center:
            hAlign = FTGL::ALIGN_CENTER;
            break;
        case Right:
            hAlign = FTGL::ALIGN_RIGHT;
            break;
        }
        simpleLayout_->SetAlignment(hAlign);
    } else {
        delete font_;
        font_ = 0;
        LERRORC("tgt.Font", "Font file could not be loaded: " << fontName_);
    }
}

float Font::getLineHeight() {
    return font_->LineHeight();
}

float Font::getLineWidth() {
    return lineWidth_;
}

void Font::setLineWidth(float lineWidth) {
    lineWidth_ = lineWidth;
    update(false);
}

void Font::setTextAlignment(TextAlignment textAlignment) {
    hAlign_ = textAlignment;
    update(false);
}

void Font::setVerticalTextAlignment(VerticalTextAlignment verticalTextAlignment) {
    vAlign_ = verticalTextAlignment;
    update(false);
}

void Font::setSize(int size) {
    fontSize_ = size;
    update(false);
}

int Font::getSize() {
    return fontSize_;
}

void Font::setFontName(const std::string& fontName) {
    fontName_ = fontName;
    update(true);
}

std::string Font::getFontName() {
    return fontName_;
}

void Font::setFontType(Font::FontType fontType) {
    fontType_ = fontType;
    update(true);
}

Font::FontType Font::getFontType() {
    return fontType_;
}

Font::TextAlignment Font::getTextAlignment() {
    return hAlign_;
}

Font::VerticalTextAlignment Font::getVerticalTextAlignment() {
    return vAlign_;
}

void Font::render(const vec3& pos, const std::string& text) const {
    if (simpleLayout_) {
        float delta = 0;

        std::string line;
        std::stringstream ss(text);
        std::getline(ss, line);
        FTPoint point(static_cast<double>(pos.x),
                      static_cast<double>(pos.y),
                      static_cast<double>(pos.z));
        FTBBox box = font_->BBox(line.c_str(), -1, point);
        delta -= box.Upper().Yf() - box.Lower().Yf(); // height of first line

        Bounds bounds = getBounds(pos, text);
        float height = bounds.getURB().y - bounds.getLLF().y;
        switch(vAlign_) {
            case Font::Top:
                delta += height;
                break;
            case Font::Middle:
                delta += height * 0.5f;
                break;
            case Font::Bottom:
                break;
        }
        vec3 dpos = vec3(pos.x, pos.y + delta, pos.z);
        glPushMatrix();
        glRasterPos3f(dpos.x, dpos.y, dpos.z);
        glTranslatef(dpos.x, dpos.y, dpos.z);
        simpleLayout_->Render(text.c_str(), -1, FTPoint(dpos.x, dpos.y, dpos.z));
        glPopMatrix();
    }
}

Bounds Font::getBounds(const vec3& pos, const std::string& text) const {
    if (!font_)
        return Bounds();

    FTPoint point(static_cast<double>(pos.x),
                  static_cast<double>(pos.y),
                  static_cast<double>(pos.z));

    float delta = 0;

    std::string line;
    std::stringstream ss(text);
    std::getline(ss, line);
    FTBBox box_tmp = font_->BBox(line.c_str(), -1, point);
    delta -= box_tmp.Upper().Yf() - box_tmp.Lower().Yf(); // height of first line

    FTBBox box = simpleLayout_->BBox(text.c_str(), -1, point);
    FTPoint upper = box.Upper();
    FTPoint lower = box.Lower();
    float height = upper.Yf() - lower.Yf();
    switch(vAlign_) {
        case Font::Top:
            delta += height;
            break;
        case Font::Middle:
            delta += height * 0.5f;
            break;
        case Font::Bottom:
            break;
    }

    vec3 upperTGT = vec3(upper.Xf(), upper.Yf() + delta, upper.Zf());
    vec3 lowerTGT = vec3(lower.Xf(), lower.Yf() + delta, lower.Zf());

    return Bounds(lowerTGT, upperTGT);
}

#else

Font::Font(const std::string& /*fontName*/, int /*size*/, FontType /*fontType*/, float /*lineWidth*/, TextAlignment /*textAlignment*/, VerticalTextAlignment /*verticalTextAlignment*/) {}

Font::~Font() {}

void Font::update(bool reloadFont) {}

float Font::getLineHeight() {
    return 1.f;
}

float Font::getLineWidth() {
    return 1.f;
}

void Font::setLineWidth(float /*lineWidth*/) {}

void Font::setSize(int /*size*/) {}

int Font::getSize() {
    return 1;
}

void Font::setFontName(const std::string& /*fontName*/) {}

std::string Font::getFontName() {
    return "";
}

void Font::setFontType(FontType /*fontType*/) {}

Font::FontType Font::getFontType() {
    return Font::TextureFont;
}

Font::TextAlignment Font::getTextAlignment() {
    return Font::Left;
}

Font::VerticalTextAlignment Font::getVerticalTextAlignment() {
    return Font::Middle;
}

Font::FontType Font::getFontType(const std::string& /*typeName*/) {
    return Font::NIL;
}

std::string Font::getFontTypeName(Font::FontType /*type*/) {
    return "<unknown>";
}

std::string Font::getTextAlignmentName(Font::TextAlignment /*textAlignment*/) {
    return "<unknown>";
}

Font::TextAlignment Font::getTextAlignment(const std::string& /*textAlignment*/) {
    return Font::Left;
}

Font::VerticalTextAlignment Font::getVerticalTextAlignment(const std::string& /*verticalTextAlignment*/) {
    return Font::Top;
}

std::string Font::getVerticalTextAlignmentName(Font::VerticalTextAlignment /*verticalTextAlignment*/) {
    return "<unknown>";
}

void Font::setTextAlignment(TextAlignment /*textAlignment*/) {}

void Font::setVerticalTextAlignment(VerticalTextAlignment /*verticalTextAlignment*/) {}

void Font::render(const vec3& /*pos*/, const std::string& /*text*/) const {}

Bounds Font::getBounds(const vec3& /*pos*/, const std::string& /*text*/) const {
    return Bounds();
}

#endif

} // namespace tgt
