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

#ifndef TGT_FONT_H
#define TGT_FONT_H

#include "tgt/bounds.h"
#include <vector>

#ifdef TGT_HAS_FTGL
#include <FTGL/ftgl.h>
#endif

namespace tgt {

/**
 * This class acts as a wrapper for the ftgl fonts. A FTTextureFont will be created and it is
 * possible to render a text to a location or get the bounding box for the text beforehand.
 * By default a font size of 72 is set, but can be changed via the \sa setSize method.
 * If TGT_HAS_FTGL is not set, the methods do nothing but will be present anyway.
 */
class TGT_API Font {
public:
    enum FontType {
        NIL,
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

    enum VerticalTextAlignment {
        Top,
        Middle,
        Bottom
    };

    /**
     * Creates a Font object from the given font file. A font size of 72 will be used by default
     * but that can be changed later on.
     * \param fontName The path to the font file, which should be used for this font object.
     * \param size The font size.
     */
    Font(const std::string& fontName
        , int size = 72
        , FontType fontType = TextureFont
        , float lineWidth = 4096.0f // this makes sure there are no unexpected line breaks
                                    // where exactly one line is expected
        , TextAlignment textAlignment = Left
        , VerticalTextAlignment verticalTextAlignment = Top); // Top is compatible to previous one-line render

    /**
     * Destructor - deletes the font object.
     */
    virtual ~Font();

    /**
     * Get the height of one line.
     */
    virtual float getLineHeight();

    /**
     * Get the width of one line.
     */
    virtual float getLineWidth();

    /**
     * Sets the line width to use.
     */
    virtual void setLineWidth(float lineWidth);

    /**
     * Sets the font size which should be used from now on.
     * \param size The new font size
     */
    virtual void setSize(int size);

    /**
     * Gets the font size.
     */
    virtual int getSize();

    /**
     * Set font name.
     */
    virtual void setFontName(const std::string& fontName);

    /**
     * Get font name.
     */
    virtual std::string getFontName();

    /**
     * Set font type.
     */
    virtual void setFontType(FontType fontType);

    /**
     * Get font type.
     */
    virtual FontType getFontType();

    /**
     * Returns the FontType for the given type string.
     * It the typeName doesn't match any FontType, NIL is
     * returned.
     */
    static FontType getFontType(const std::string& typeName);

    /**
     * Returns the passed type as string, which
     * might be used in a user interface.
     */
    static std::string getFontTypeName(FontType type);

    /**
     * Get horizontal text alignment.
     */
    virtual TextAlignment getTextAlignment();

    /**
     * Get horizontal text alignment.
     */
    static TextAlignment getTextAlignment(const std::string& textAlignment);

    /**
     * Returns the passed enum as string, which
     * might be used for serialisation.
     */
    static std::string getTextAlignmentName(TextAlignment textAlignment);

    /**
     * Get vertical text alignment.
     */
    virtual VerticalTextAlignment getVerticalTextAlignment();

    /**
     * Get vertical text alignment.
     */
    static VerticalTextAlignment getVerticalTextAlignment(const std::string& verticalTextAlignment);

    /**
     * Returns the passed enum as string, which
     * might be used for serialisation.
     */
    static std::string getVerticalTextAlignmentName(VerticalTextAlignment verticalTextAlignment);

    /**
     * Sets the font text alignment of the font.
     * Don't forget to specify line width!
     */
    virtual void setTextAlignment(TextAlignment textAlignment);

    /**
     * Sets the font text alignment of the font.
     * Don't forget to specify line width!
     */
    virtual void setVerticalTextAlignment(VerticalTextAlignment verticalTextAlignment);

    /**
     * Renders the text 'text' to the position 'pos'.
     * \sa pos The pen position of the first character
     * \sa text The text to be rendered
     */
    virtual void render(const tgt::vec3& pos, const std::string& text) const;

    /**
     * Computes the bounding box for the the text 'text' beginning at position 'pos'.
     * \sa pos The pen position of the first character
     * \sa text The text for which the bounding box should be computed
     */
    virtual tgt::Bounds getBounds(const tgt::vec3& pos, const std::string& text) const;

protected:
#ifdef TGT_HAS_FTGL
    FTFont* font_;
    FTSimpleLayout* simpleLayout_;
#endif

private:
    /**
     * Updates the font.
     */
    void update(bool reloadFont);

    int fontSize_;
    std::string fontName_;
    FontType fontType_;
    float lineWidth_;
    TextAlignment hAlign_;
    VerticalTextAlignment vAlign_;
};

} // namespace tgt

#endif
