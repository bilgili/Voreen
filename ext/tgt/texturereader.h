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

#ifndef TGT_TEXTUREREADER_H
#define TGT_TEXTUREREADER_H

#include "tgt/texture.h"

#include <vector>

namespace tgt {

class TextureReader {
public:
    TextureReader();
    virtual ~TextureReader() {}

    virtual const std::vector<std::string>& getEndings() const { return extensions_; }

    virtual Texture* loadTexture(const std::string& filename, Texture::Filter filter,
                                 bool compress = false, bool keepPixels = false,
                                 bool createOGLTex = true, bool textureRectangle = false) = 0;

    virtual std::string getName() const { return name_; }

protected:
    static const std::string loggerCat_;

    std::vector<std::string> extensions_;
    std::string name_;

    /**
     *   Create OpenGL texture
     */
    bool create1DTexture(Texture* t, Texture::Filter filter, bool compress, bool createOGLTex = true);

    /**
     *   Create OpenGL texture
     */
    bool create2DTexture(Texture* t, Texture::Filter filter, bool compress, bool createOGLTex = true);

    /**
     *   Create OpenGL texture
     */
    bool createRectangleTexture(Texture* t, Texture::Filter filter, bool compress, bool createOGLTex = true);

    /**
     *   Create OpenGL texture
     */
    bool create3DTexture(Texture* t, Texture::Filter filter, bool compress, bool createOGLTex = true);

    /**
     *   Find nearest (larger) power-of-2 value
     */
    GLsizei checkSize(GLsizei s);
};

} // namespace tgt

#endif // TGT_TEXTUREREADER_H
