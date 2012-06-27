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

#ifndef TGT_FONT_H
#define TGT_FONT_H

#include "tgt/bounds.h"

#ifdef TGT_HAS_FTGL
#include <FTGL/ftgl.h>
class FTFont;
#endif

namespace tgt {

enum FontType {
    BitmapFont,
    BufferFont,
    ExtrudeFont,
    OutlineFont,
    PixmapFont,
    PolygonFont,
    TextureFont    
};

enum TextAlignment {
    Left,
    Center,
    Right
};

/**
 * This class acts as a wrapper for the ftgl fonts. A FTTextureFont will be created and it is
 * possible to render a text to a location or get the bounding box for the text beforehand.
 * By default a font size of 72 is set, but can be changed via the \sa setSize method.
 * If TGT_HAS_FTGL is not set, the methods do nothing but will be present anyway.
 */
class Font {
public:
    /**
     * Crates a Font object from the given font file. A font size of 72 will be used by default
     * but that can be changed later on.
     * \param fontName The path to the font file, which should be used for this font object.
     * \param size The font size.
     */
    Font(const std::string& fontName, const int size = 72, FontType fontType = TextureFont);

    /**
     * Destructor - deletes the font object.
     */
    ~Font();

    /**
     * Get the height of one line.
     */
    float getLineHeight();

    /**
     * Sets the line width to use.
     */
    void setLineWidth(const float lineWidth);

    /**
     * Sets the font size which should be used from now on.
     * \param size The new font size
     */
    void setSize(const int size);

    /**
     * Sets the font text alignment of the font.
     * Don't forget to specify line width!
     */
    void setTextAlignment(TextAlignment textAlignment);

    /**
     * Renders the text 'text' to the position 'pos'. 
     * \sa pos The pen position of the first character
     * \sa text The text to be rendered
     */
    void render(const tgt::vec3& pos, const std::string& text);

    /**
     * Renders the text 'text' to the position 'pos' using layoutmanager. 
     * \sa pos The pen position of the first character
     * \sa text The text to be rendered
     */
    void renderWithLayout(const tgt::vec3& pos, const std::string& text);

    /**
     * Computes the bounding box for the the text 'text' beginning at position 'pos'.
     * \sa pos The pen position of the first character
     * \sa text The text for which the bounding box should be computed
     */
    tgt::Bounds getBounds(const tgt::vec3& pos, const std::string& text);

protected:
#ifdef TGT_HAS_FTGL
    FTFont* font_;
    FTSimpleLayout* simpleLayout_;
#endif
};

} //namespace tgt

#endif
