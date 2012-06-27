/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
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

#include "tgt/texturereader.h"

#include "tgt/gpucapabilities.h"
#include "tgt/logmanager.h"

namespace tgt {

const std::string TextureReader::loggerCat_("tgt.Texture.Reader");

TextureReader::TextureReader()
{}

GLsizei TextureReader::checkSize(GLsizei s) {
    int k = 1;
    while (k < s)
        k <<= 1;
    return k;
}

bool TextureReader::rescaleTexture(Texture* t, Texture::Filter filter) {
    std::string name = (t->getName().empty() ? "" : " (" + t->getName() + ")");
    
    if ((t->dimensions_.x != checkSize(t->dimensions_.x)) ||
        (t->dimensions_.y != checkSize(t->dimensions_.y)))
    {
        if (GpuCaps.isNpotSupported()) {
            LDEBUG("Resizing using hardware support.");
            return true;
        } else {
            LWARNING("Texture size not power of 2. Resizing - SLOW!!!" + name);
            if (filter == Texture::MIPMAP || filter == Texture::ANISOTROPIC)
                LWARNING("Filter not supported for resizing -> setting to LINEAR." + name);
            filter = Texture::LINEAR;
        }

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        GLubyte* newPixels;
        GLuint newWidth, newHeight;
        
        // simply scale up to the next power of two
        newHeight = checkSize(t->dimensions_.y);
        newWidth = checkSize(t->dimensions_.x);

        size_t newArraySize = newWidth * newHeight *  t->bpp_;
        newPixels = new GLubyte[newArraySize];

        LWARNING("  Resizing " << t->dimensions_.x << "x" << t->dimensions_.y
                 << " to " << newWidth << "x" << newHeight << name);
        GLint scaleResult
            = gluScaleImage(t->format_, t->dimensions_.x, t->dimensions_.y, t->dataType_, t->pixels_,
                            newWidth, newHeight, t->dataType_, newPixels);

        if (scaleResult != 0) {
            LERROR("Error resizing texture to power of 2:" << gluErrorString(scaleResult) << name);
            return false;
        }
        
        t->dimensions_.y = newHeight;
        t->dimensions_.x = newWidth;
        delete[] t->pixels_;
        t->pixels_ = newPixels;
        t->arraySize_ = newArraySize;
    }
    return true;
}

bool TextureReader::create1DTexture(Texture* t, Texture::Filter filter, bool compress, bool createOGLTex) {
    t->type_ = GL_TEXTURE_1D;

    switch (t->bpp_) {
    case 3:
        t->format_ = GL_RGB;
        compress ? t->internalformat_ = GL_COMPRESSED_RGB_ARB : t->internalformat_ = GL_RGB;
        break;

    case 4:
        t->format_ = GL_RGBA;
        compress ? t->internalformat_ = GL_COMPRESSED_RGBA_ARB : t->internalformat_ = GL_RGBA;
        break;

    default:
        LERROR(static_cast<int>(t->bpp_)<< " bits per pixel...error!");
        return false;
    }

    if (createOGLTex) {
        //FIXME: needed? joerg
//         if (!rescaleTexture(t, filter))
//             return false;

        glGenTextures(1, &t->id_);
        glBindTexture(GL_TEXTURE_1D, t->id_);

        if (!GpuCaps.isAnisotropicFilteringSupported() && filter == Texture::ANISOTROPIC)
            filter = Texture::MIPMAP;

        t->setFilter(filter);
        t->uploadTexture();
    }
    return true;
}

bool TextureReader::create2DTexture(Texture* t, Texture::Filter filter, bool compress, bool createOGLTex) {
    t->type_ = GL_TEXTURE_2D;

    switch (t->bpp_) {
    case 3:
        t->format_ = GL_RGB;
        compress ? t->internalformat_ = GL_COMPRESSED_RGB_ARB : t->internalformat_ = GL_RGB;
        break;

    case 4:
        t->format_ = GL_RGBA;
        compress ? t->internalformat_ = GL_COMPRESSED_RGBA_ARB : t->internalformat_ = GL_RGBA;
        break;

    default:
        LERROR(static_cast<int>(t->bpp_)<< " bits per pixel...error!");
        return false;
    }

    if (createOGLTex) {
        if (!rescaleTexture(t, filter))
            return false;

        glGenTextures(1, &t->id_);
        glBindTexture(GL_TEXTURE_2D, t->id_);

        if (!GpuCaps.isAnisotropicFilteringSupported() && filter == Texture::ANISOTROPIC)
            filter = Texture::MIPMAP;

        t->setFilter(filter);
        t->uploadTexture();
    }
    return true;
}

bool TextureReader::createRectangleTexture(Texture* t, Texture::Filter filter, bool compress, bool createOGLTex) {
    switch (t->bpp_) {
    case 3:
        t->format_ = GL_RGB;
        compress ? t->internalformat_ = GL_COMPRESSED_RGB_ARB : t->internalformat_ = GL_RGB;
        break;

    case 4:
        t->format_ = GL_RGBA;
        compress ? t->internalformat_ = GL_COMPRESSED_RGBA_ARB : t->internalformat_ = GL_RGBA;
        break;

    default:
        LERROR(static_cast<int>(t->bpp_) << " bits per pixel...error!");
        return false;
    }

    if (createOGLTex) {
        //FIXME: needed? joerg
        //  if (!rescaleTexture(t, filter))
        //      return false;

        glGenTextures(1, &t->id_);
#ifdef GL_TEXTURE_RECTANGLE_ARB
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, t->id_);
#endif
        if (!GpuCaps.isAnisotropicFilteringSupported() && filter == Texture::ANISOTROPIC)
            filter = Texture::MIPMAP;

        t->setFilter(filter);
        t->uploadTexture();
    }
    return true;
}

bool TextureReader::create3DTexture(Texture* t, Texture::Filter filter, bool compress, bool createOGLTex) {
    t->type_ = GL_TEXTURE_3D;

    switch (t->bpp_) {
    case 1:
        t->internalformat_ = GL_INTENSITY;
        break;
    case 2:
        t->internalformat_ = GL_INTENSITY16;
        break;
    case 3:
        t->format_ = GL_RGB;
        compress ? t->internalformat_ = GL_COMPRESSED_RGB_ARB : t->internalformat_ = GL_RGB;
        break;
    case 4:
        t->format_ = GL_RGBA;
        compress ? t->internalformat_ = GL_COMPRESSED_RGBA_ARB : t->internalformat_ = GL_RGBA;
        break;
    default:
        LERROR(t->bpp_<< " bits per pixel...error!");
        return false;
    }

    //FIXME: needed? joerg
//     if (!rescaleTexture(t, filter))
//         return false;

    if (createOGLTex) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &t->id_);
        glBindTexture(GL_TEXTURE_3D, t->id_);

        if (!GpuCaps.isAnisotropicFilteringSupported() && filter == Texture::ANISOTROPIC)
            filter = Texture::MIPMAP;

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
        t->setFilter(filter);
        t->uploadTexture();
    }
    return true;
}

} // namespace tgt
