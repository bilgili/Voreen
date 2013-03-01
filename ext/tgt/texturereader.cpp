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
        LERROR(static_cast<int>(t->bpp_)<< " bytes per pixel...error!");
        return false;
    }

    if (createOGLTex) {
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

    // derived internal format from format and bit depth
    switch (t->format_) {
    case GL_LUMINANCE:
        if (t->bpp_ == 1)
            t->internalformat_ = GL_LUMINANCE;
        else if (t->bpp_ == 2)
            t->internalformat_ = GL_LUMINANCE16;
        else {
            LERROR(t->bpp_ << " bytes per pixel not supported for texture format GL_LUMINANCE");
            return false;
        }
        break;

    case GL_LUMINANCE_ALPHA:
        if (t->bpp_ == 2)
            t->internalformat_ = GL_LUMINANCE_ALPHA;
        else if (t->bpp_ == 4)
            t->internalformat_ = GL_LUMINANCE16_ALPHA16;
        else {
            LERROR(t->bpp_ << " bytes per pixel not supported for texture format GL_LUMINANCE_ALPHA");
            return false;
        }
        break;

    case GL_RGB:
        if (t->bpp_ == 3)
            compress ? t->internalformat_ = GL_COMPRESSED_RGB_ARB : t->internalformat_ = GL_RGB;
        else if (t->bpp_ == 6)
            t->internalformat_ = GL_RGB16;
        else {
            LERROR(t->bpp_ << " bytes per pixel not supported for texture format GL_RGB");
            return false;
        }
        break;

    case GL_RGBA:
        if (t->bpp_ == 4)
            compress ? t->internalformat_ = GL_COMPRESSED_RGBA_ARB : t->internalformat_ = GL_RGBA;
        else if (t->bpp_ == 8)
            t->internalformat_ = GL_RGBA16;
        else {
            LERROR(t->bpp_ << " bytes per pixel not supported for texture format GL_RGBA");
            return false;
        }
        break;

    default:
        // unspecified or unknown format, try to derive from bpp
        LWARNING("Unspecified or unknown texture format, trying to derive format from bbp ...");
        switch (t->bpp_) {
        case 1:
            t->format_ = GL_LUMINANCE;
            t->internalformat_ = GL_LUMINANCE;
            break;

        case 2:
            t->format_ = GL_LUMINANCE;
            t->internalformat_ = GL_LUMINANCE16;
            break;

        case 3:
            t->format_ = GL_RGB;
            compress ? t->internalformat_ = GL_COMPRESSED_RGB_ARB : t->internalformat_ = GL_RGB;
            break;

        case 4:
            t->format_ = GL_RGBA;
            compress ? t->internalformat_ = GL_COMPRESSED_RGBA_ARB : t->internalformat_ = GL_RGBA;
            break;

        case 8: // 16-bit-per-channel RGBA
            t->format_ = GL_RGBA;
            t->internalformat_ = GL_RGBA16;
            break;

        case 12: //HDR-RGB, cut down to one byte per channel (until proper hdr-handling is implemented)
            t->format_ = GL_RGB;
            t->internalformat_ = GL_RGB;
            t->bpp_ = 3;
            break;

        default:
            LERROR(static_cast<int>(t->bpp_)<< " bytes per pixel not supported!");
            return false;
        }
    }

    if (createOGLTex) {
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
        LERROR(static_cast<int>(t->bpp_) << " bytes per pixel...error!");
        return false;
    }

    if (createOGLTex) {
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
        LERROR(t->bpp_<< " bytes per pixel...error!");
        return false;
    }

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
