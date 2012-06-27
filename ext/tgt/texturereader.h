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
