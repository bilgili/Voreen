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

#include "tgt/texture.h"
#include "tgt/types.h"

#include "tgt/gpucapabilities.h"
#include "tgt/filesystem.h"

namespace tgt {

//------------------------------------------------------------------------------
// Texture
//------------------------------------------------------------------------------

Texture::Texture(const tgt::ivec3& dimensions, GLint format, GLint internalformat,
                GLenum dataType, Filter filter)
    : dimensions_(dimensions)
    , format_(format)
    , internalformat_(internalformat)
    , dataType_(dataType)
    , filter_(filter)
    , wrapping_(REPEAT)
    , priority_(-1.f)
    , pixels_(0)
{
    init(true);
}

Texture::Texture(const tgt::ivec3& dimensions, GLint format,
                GLenum dataType, Filter filter)
    : dimensions_(dimensions)
    , format_(format)
    , internalformat_(format)
    , dataType_(dataType)
    , filter_(filter)
    , wrapping_(REPEAT)
    , priority_(-1.f)
    , pixels_(0)
{
    init(true);
}

Texture::Texture(GLubyte* data, const tgt::ivec3& dimensions, GLint format, GLint internalformat,
                GLenum dataType, Filter filter)
    : dimensions_(dimensions)
    , format_(format)
    , internalformat_(internalformat)
    , dataType_(dataType)
    , filter_(filter)
    , wrapping_(REPEAT)
    , priority_(-1.f)
    , pixels_(data)
{
    init(false);
    arraySize_ = hmul(dimensions_) * bpp_;
}

Texture::Texture(GLubyte* data, const tgt::ivec3& dimensions, GLint format,
                GLenum dataType, Filter filter)
    : dimensions_(dimensions)
    , format_(format)
    , internalformat_(format)
    , dataType_(dataType)
    , filter_(filter)
    , wrapping_(REPEAT)
    , priority_(-1.f)
    , pixels_(data)
{
    init(false);
    arraySize_ = hmul(dimensions_) * bpp_;
}

void Texture::init(bool allocData) {
#ifndef GL_TEXTURE_RECTANGLE_ARB
    textureRectangle = false;
#endif

    calcType();
    bpp_ = calcBpp(format_, dataType_);

    generateId();

    if (allocData)
        alloc();

    applyFilter();
}

int Texture::calcBpp(GLint format, GLenum dataType) {

    int numChannels = calcNumChannels(format);

    int typeSize = 0;
    switch (dataType) {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
            typeSize = 1;
            break;

        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
            typeSize = 2;
            break;

        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_FLOAT:
            typeSize = 4;
            break;

        case GL_DOUBLE:
            typeSize = 8;
            break;

        default:
            LWARNINGC("tgt.Texture", "unknown dataType");
    }

    return typeSize * numChannels;
}

int Texture::calcBpp(GLint internalformat) {

    int bpp = 0;
    switch (internalformat) {
        case 1:
        case GL_COLOR_INDEX:
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_ALPHA:
        case GL_INTENSITY:
        case GL_LUMINANCE:
        case GL_DEPTH_COMPONENT:
            bpp = 1;
            break;

        case 2:
        case GL_LUMINANCE_ALPHA:
        case GL_INTENSITY16:
        case GL_DEPTH_COMPONENT16:
            bpp = 2;
            break;

        case GL_RGB:
        case GL_BGR:
        case GL_DEPTH_COMPONENT24:
            bpp = 3;
            break;

        case GL_RGBA:
        case GL_RGBA8:
        case GL_BGRA:
        case GL_DEPTH_COMPONENT32:
        case GL_R32F:
        case GL_LUMINANCE32F_ARB:
            bpp = 4;
            break;

        case GL_RGB16:
        case GL_RGB16F_ARB:
            bpp = 6;
            break;

        case GL_RGBA16:
        case GL_RGBA16F_ARB:
            bpp = 8;
            break;

        case GL_RGB32F_ARB:
            bpp = 12;
            break;

        case GL_RGBA32F_ARB:
            bpp = 16;
            break;

        default:
            LWARNINGC("tgt.Texture", "unknown internal format");
            break;
    }

    return bpp;
}

int Texture::calcNumChannels(GLint format) {

    switch (format) {
    case 1:
    case GL_COLOR_INDEX:
    case GL_RED:
    case GL_R32F:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_INTENSITY:
    case GL_LUMINANCE:
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT24:
    case GL_ALPHA_INTEGER_EXT:
        return 1;
        break;

    case 2:
    case GL_LUMINANCE_ALPHA:
        return 2;
        break;

    case 3:
    case GL_RGB:
    case GL_BGR:
        return 3;
        break;

    case 4:
    case GL_RGBA:
    case GL_BGRA:
    case GL_RGBA16:
    case GL_RGBA16F_ARB:
    case GL_LUMINANCE32F_ARB:
        return 4;
        break;

    default:
        LWARNINGC("tgt.Texture", "unknown format");
        return 0;
    }
}

int Texture::getSizeOnGPU() const {
    int bpp = calcBpp(internalformat_);
    return bpp * hmul(dimensions_);
}

GLenum Texture::calcType(bool textureRectangle) {
    if (dimensions_.z == 1)    {
        if (dimensions_.y == 1)
            type_ = GL_TEXTURE_1D;
        else
            type_ = GL_TEXTURE_2D;
    }
    else {
        type_ = GL_TEXTURE_3D;
    }

#ifdef GL_TEXTURE_RECTANGLE_ARB
    if (type_ == GL_TEXTURE_2D && textureRectangle)
        type_ = GL_TEXTURE_RECTANGLE_ARB;
#endif

    return type_;
}

void Texture::setFilter(Filter filter) {
    filter_ = filter;
    applyFilter();
}

void Texture::applyFilter() {
    bind();

    switch(filter_) {
        case NEAREST:
            glTexParameteri(type_,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(type_,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
            break;

        case LINEAR:
            glTexParameteri(type_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(type_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            break;

        case ANISOTROPIC:
            glTexParameterf(type_, GL_TEXTURE_MAX_ANISOTROPY_EXT, GpuCaps.getMaxTextureAnisotropy());

        case MIPMAP:
            glTexParameteri(type_,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(type_,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(type_, GL_SGIS_generate_mipmap, GL_TRUE);
            break;
    }
}

void Texture::setWrapping(Wrapping w) {
    wrapping_ = w;
    applyWrapping();
}

void Texture::applyWrapping() {
    bind();
    GLint wrap = wrapping_;

    /*
        set wrapping for all diminesions
    */
    glTexParameteri(type_, GL_TEXTURE_WRAP_S, wrap);

    if (type_ == GL_TEXTURE_2D || type_ == GL_TEXTURE_3D)
        glTexParameteri(type_, GL_TEXTURE_WRAP_T, wrap);
    if (GL_TEXTURE_3D)
        glTexParameteri(type_, GL_TEXTURE_WRAP_R, wrap);
}

void Texture::uploadTexture() {
    bind();

    switch(type_) {
        case GL_TEXTURE_1D:
            glTexImage1D(GL_TEXTURE_1D, 0, internalformat_, dimensions_.x, 0,
                         format_, dataType_, pixels_);
            break;

        case GL_TEXTURE_2D:
            glTexImage2D(GL_TEXTURE_2D, 0, internalformat_, dimensions_.x, dimensions_.y, 0,
                         format_, dataType_, pixels_);
            break;

        case GL_TEXTURE_3D:
            glTexImage3D(GL_TEXTURE_3D, 0, internalformat_,
                         dimensions_.x, dimensions_.y, dimensions_.z, 0,
                         format_, dataType_, pixels_);
            break;

#ifdef GL_TEXTURE_RECTANGLE_ARB
        case GL_TEXTURE_RECTANGLE_ARB:
            glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, internalformat_, dimensions_.x, dimensions_.y, 0,
                            format_, dataType_, pixels_);
            break;
#endif

    }
}

tgt::Color Texture::texelAsFloat(size_t x, size_t y) const {
    tgt::Color ret = tgt::Color(0.0f);
    switch(format_) {
        case GL_RGBA:
            switch(dataType_) {
                case GL_UNSIGNED_BYTE: {
                    tgt::Vector4<uint8_t> t = texel< tgt::Vector4<uint8_t> >(x,y);
                    ret.x = (float )t.x / 0xFF;
                    ret.y = (float )t.y / 0xFF;
                    ret.z = (float )t.z / 0xFF;
                    ret.w = (float )t.w / 0xFF;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tgt::Vector4<uint16_t> t = texel< tgt::Vector4<uint16_t> >(x,y);
                    ret.x = (float )t.x / 0xFFFF;
                    ret.y = (float )t.y / 0xFFFF;
                    ret.z = (float )t.z / 0xFFFF;
                    ret.w = (float )t.w / 0xFFFF;
                    break;
                }
                case GL_FLOAT:
                    ret = texel<tgt::Color>(x,y);
                    break;
                default:
                    LWARNINGC("tgt.texture", "texelAsFloat: Unknown data type!");
            }
            break;
        case GL_RGB:
            switch(dataType_) {
                case GL_UNSIGNED_BYTE: {
                    tgt::Vector3<uint8_t> t = texel< tgt::Vector3<uint8_t> >(x,y);
                    ret.x = (float )t.x / 0xFF;
                    ret.y = (float )t.y / 0xFF;
                    ret.z = (float )t.z / 0xFF;
                    ret.w = 1.0f;
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    tgt::Vector3<uint16_t> t = texel< tgt::Vector3<uint16_t> >(x,y);
                    ret.x = (float )t.x / 0xFFFF;
                    ret.y = (float )t.y / 0xFFFF;
                    ret.z = (float )t.z / 0xFFFF;
                    ret.w = 1.0f;
                    break;
                }
                case GL_FLOAT: {
                    tgt::Vector3f t = texel<tgt::Vector3f>(x,y);
                    ret.x = t.x;
                    ret.y = t.y;
                    ret.z = t.z;
                    ret.w = 1.0f;
                    break;
                }
                default:
                    LWARNINGC("tgt.texture", "texelAsFloat: Unknown data type!");
            }
            break;
        case GL_LUMINANCE:
        case GL_ALPHA:
            switch(dataType_) {
                case GL_UNSIGNED_BYTE: {
                    tgt::Vector3<uint8_t> t = tgt::vec3(texel<uint8_t>(x,y));
                    ret.x = (float )t.x / 0xFF;
                    ret.y = (float )t.y / 0xFF;
                    ret.z = (float )t.z / 0xFF;
                    ret.w = 1.0f;
                    break;
                                       }
                case GL_UNSIGNED_SHORT: {
                    tgt::Vector3<uint16_t> t = tgt::vec3(texel<uint16_t>(x,y));
                    ret.x = (float )t.x / 0xFFFF;
                    ret.y = (float )t.y / 0xFFFF;
                    ret.z = (float )t.z / 0xFFFF;
                    ret.w = 1.0f;
                    break;
                                        }
                case GL_FLOAT: {
                    tgt::Vector3f t = tgt::vec3(texel<GLfloat>(x,y));
                    ret.x = t.x;
                    ret.y = t.y;
                    ret.z = t.z;
                    ret.w = 1.0f;
                    break;
                }
                default:
                    LWARNINGC("tgt.texture", "texelAsFloat: Unknown data type!");
        }
        break;

        default:
            LWARNINGC("tgt.texture", "texelAsFloat: Unknown format!");
    }
    return ret;
}

void Texture::downloadTexture() const {
    bind();

    if (pixels_ == 0)
        alloc();

    glGetTexImage(type_, 0, format_, dataType_, pixels_);
}

GLubyte* Texture::downloadTextureToBuffer() const {
    bind();

    int arraySize = hmul(dimensions_) * bpp_;
    GLubyte* pixels = new GLubyte[arraySize];

    glGetTexImage(type_, 0, format_, dataType_, pixels);
    return pixels;
}

void Texture::downloadTextureToBuffer(GLubyte* pixels, size_t numBytesAllocated) const {
    bind();

    size_t arraySize = hmul(dimensions_) * bpp_;
    if(numBytesAllocated < arraySize) {
        LWARNINGC("tgt.texture", "downloadTextureToBuffer: allocated buffer is too small");
    }
    else {
        glGetTexImage(type_, 0, format_, dataType_, pixels);
    }
}

GLubyte* Texture::downloadTextureToBuffer(GLint format, GLenum dataType) const {
    bind();

    int arraySize = hmul(dimensions_) * calcBpp(format, dataType);
    GLubyte* pixels = new GLubyte[arraySize];

    glGetTexImage(type_, 0, format, dataType, pixels);
    return pixels;
}

bool Texture::isTextureRectangle() const {
#ifdef GL_TEXTURE_RECTANGLE_ARB
    return (type_ == GL_TEXTURE_RECTANGLE_ARB);
#else
    return false;
#endif
}

} // namespace tgt
