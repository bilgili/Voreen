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

#include "tgt/texturereaderdevil.h"

#ifdef TGT_HAS_DEVIL

#include <IL/il.h>

#include "tgt/logmanager.h"
#include "tgt/filesystem.h"
#include <cstring>

namespace tgt {

//------------------------------------------------------------------------------
// TextureReaderDevil
//------------------------------------------------------------------------------


const std::string TextureReaderDevil::loggerCat_("tgt.Texture.Reader.Devil");

TextureReaderDevil::TextureReaderDevil() {
    name_ = "DevIL Reader";
    extensions_.push_back("bmp");
    extensions_.push_back("cut");
    extensions_.push_back("dcx");
    extensions_.push_back("dds");
    extensions_.push_back("ico");
    extensions_.push_back("gif");
    extensions_.push_back("jpg");
    extensions_.push_back("jpeg");
    extensions_.push_back("lbm");
    extensions_.push_back("lif");
    extensions_.push_back("mdl");
    extensions_.push_back("pcd");
    extensions_.push_back("pcx");
    extensions_.push_back("pic");
    extensions_.push_back("png");
    extensions_.push_back("pnm");
    extensions_.push_back("psd");
    extensions_.push_back("psp");
    extensions_.push_back("raw");
    extensions_.push_back("sgi");
    extensions_.push_back("tga");
    extensions_.push_back("tif");
    extensions_.push_back("wal");
    extensions_.push_back("act");
    extensions_.push_back("pal");
    extensions_.push_back("hdr");

    // Initialize DevIL
    ilInit();
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    ilEnable(IL_ORIGIN_SET); // Flip images
}

TextureReaderDevil::~TextureReaderDevil() {
}

Texture* TextureReaderDevil::loadTexture(const std::string& filename, Texture::Filter filter,
                                         bool compress, bool keepPixels, bool createOGLTex,
                                         bool textureRectangle)
{

#ifndef GL_TEXTURE_RECTANGLE_ARB
    if (textureRectangle){
        LERROR("Texture Rectangles not supported!");
        textureRectangle = false;
    }
#endif

    File* file = FileSys.open(filename);

    // check if file is open
    if (!file || !file->isOpen()) {
        delete file;
        return 0;
    }

    size_t len = file->size();

    // check if file is empty
    if (len == 0) {
        delete file;
        return 0;
    }

    // allocate memory
    char* imdata = new char[len];

    if (imdata == 0) {
        delete file;
        return 0;   // allocation failed
    }

    file->read(imdata, len);

    file->close();
    delete file;

    /*
        FIXME: I think the keepPixels option does not work properly
        -> I don't see why...afaik keepPixels has been used in some project (stefan)
    */
    ILuint ImageName;
    ilGenImages(1, &ImageName);
    ilBindImage(ImageName);
    Texture* t = new Texture();
    t->setName(filename);

    if (!ilLoadL(IL_TYPE_UNKNOWN, imdata, static_cast<ILuint>(len))) {
        LERROR("Failed to open via ilLoadL " << filename);
        delete[] imdata;
        delete t;
        return 0;
    }
    delete[] imdata;
    imdata = 0;

    t->setBpp(ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL));

    // determine image format
    ILint devilFormat;
    switch (ilGetInteger(IL_IMAGE_FORMAT)) {
        case IL_LUMINANCE:         // intensity channel only
            devilFormat = IL_LUMINANCE;
            t->setFormat(GL_LUMINANCE);
            break;
        case IL_LUMINANCE_ALPHA:   // intensity-alpha channels
            devilFormat = IL_LUMINANCE_ALPHA;
            t->setFormat(GL_LUMINANCE_ALPHA);
            break;
        case IL_RGB:
            devilFormat = IL_RGB;  // three color channels
            t->setFormat(GL_RGB);
            break;
        case IL_RGBA:
            devilFormat = IL_RGBA; // color-alpha channels
            t->setFormat(GL_RGBA);
            break;
        case IL_BGR:
            devilFormat = IL_RGB;  // B-G-R ordered color channels, convert to RGB
            t->setFormat(GL_RGB);
            break;
        case IL_BGRA:
            devilFormat = IL_RGBA; // R-G-B-A ordered color channels, convert to RGBA
            t->setFormat(GL_RGBA);
            break;
        default:
            LERROR("unsupported format: " << ilGetInteger(IL_IMAGE_FORMAT) << " (" << filename << ")");
            delete t;
            return 0;
    }

    // determine data type
    ILint devilDataType;
    switch (ilGetInteger(IL_IMAGE_TYPE)) {
    case IL_UNSIGNED_BYTE:
        devilDataType = IL_UNSIGNED_BYTE;
        t->setDataType(GL_UNSIGNED_BYTE);
        break;
    case IL_BYTE:
        devilDataType = IL_BYTE;
        t->setDataType(GL_BYTE);
        break;
    case IL_UNSIGNED_SHORT:
        devilDataType = IL_UNSIGNED_SHORT;
        t->setDataType(GL_UNSIGNED_SHORT);
        break;
    case IL_SHORT:
        devilDataType = IL_SHORT;
        t->setDataType(GL_SHORT);
        break;
    case IL_UNSIGNED_INT:
        devilDataType = IL_UNSIGNED_INT;
        t->setDataType(GL_UNSIGNED_INT);
        break;
    case IL_INT:
        devilDataType = IL_INT;
        t->setDataType(GL_INT);
        break;
    case IL_FLOAT:
        devilDataType = IL_FLOAT;
        t->setDataType(GL_FLOAT);
        break;
    default:
        LERROR("unsupported data type: " << ilGetInteger(IL_IMAGE_TYPE) << " (" << filename << ")");
        delete t;
        return 0;
    }

    if (!ilConvertImage(devilFormat, devilDataType)) {
        LERROR("failed to convert loaded image: " << filename);
        delete t;
        return 0;
    }

    tgt::ivec3 dims;
    dims.x = ilGetInteger(IL_IMAGE_WIDTH);
    dims.y = ilGetInteger(IL_IMAGE_HEIGHT);
    dims.z = ilGetInteger(IL_IMAGE_DEPTH);
    t->setDimensions(dims);
    LDEBUG("Image dimensions: " << t->getDimensions());
    tgtAssert( dims.z == 1, "depth is not equal 1");

#ifdef GL_TEXTURE_RECTANGLE_ARB
    if (textureRectangle)
        t->setType( GL_TEXTURE_RECTANGLE_ARB );
    else
#endif
        t->setType( GL_TEXTURE_2D );

    t->alloc();
    memcpy(t->getPixelData(), ilGetData(), t->getArraySize());

    bool success;
    if (textureRectangle)
        success = createRectangleTexture(t, filter, compress, createOGLTex);
    else {
        if (dims.y == 1)
            success = create1DTexture(t, filter, compress, createOGLTex);
        else
            success = create2DTexture(t, filter, compress, createOGLTex);
    }
    if (!success) {
        ilDeleteImages(1, &ImageName);
        if (!keepPixels)
            t->setPixelData(0);
        delete t;
        return 0;
    }

    ilDeleteImages(1, &ImageName);

    if (!keepPixels) {
        delete[] t->getPixelData();
        t->setPixelData(0);
    }

    return t;
}

} // namespace tgt

#endif // TGT_HAS_DEVIL
