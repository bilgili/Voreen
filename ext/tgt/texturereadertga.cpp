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

#include "tgt/texturereadertga.h"

#include "tgt/logmanager.h"
#include "tgt/filesystem.h"

#include <cstring>

namespace tgt {

//------------------------------------------------------------------------------
// TextureReaderTga
//------------------------------------------------------------------------------

const std::string TextureReaderTga::loggerCat_("tgt.Texture.Reader.Tga");

TextureReaderTga::TextureReaderTga() {
    name_ = "TGA Reader";
    extensions_.push_back("tga");
}

Texture* TextureReaderTga::loadTexture(const std::string& filename, Texture::Filter filter,
                                       bool compress, bool keepPixels, bool createOGLTex,
                                       bool textureRectangle)
{

#ifndef GL_TEXTURE_RECTANGLE_ARB
    if (textureRectangle) {
        LERROR("Texture Rectangles not supported!");
        textureRectangle = false;
    }
#endif

    GLubyte TGAheader[12];
    GLubyte header[6];
//    GLuint  bytesPerPixel;//FIXME: this is never initialized!

    File* file = FileSys.open(filename);

    // Check if file is open
    if (!file) {
        LERROR("Failed to open file " << filename);
        return 0;
    }

    if (!file->isOpen()) {
        LERROR("Failed to open file " << filename);
        delete file;
        return 0;
    }

    size_t len = file->size();

    // check if file is empty
    if (len == 0) {
        delete file;
        return 0;
    }

    if (file->read(reinterpret_cast<char*>(&TGAheader), sizeof(TGAheader)) != sizeof(TGAheader) ||
        file->read(reinterpret_cast<char*>(&header), sizeof(header)) != sizeof(header))
    {
        delete file;
        LERROR("Failed to read header! file: " << filename);
        return 0;
    }

    Texture* t = new Texture();
    t->setName(filename);

    ivec3 dimensions;
    dimensions.x = header[1] * 256 + header[0];           // determine the TGA width  (highbyte*256+lowbyte)
    dimensions.y = header[3] * 256 + header[2];           // determine the TGA height (highbyte*256+lowbyte)
    dimensions.z = 1;
    t->setDimensions(dimensions);
    LDEBUG("Image dimensions: " << t->getDimensions());

    if (dimensions.x <= 0 || dimensions.y <= 0) {
        delete file;
        LERROR("wrong dimensions: " << dimensions << " file: " << filename);
        return 0;
    }

    if (header[4] != 24 && header[4] != 32) {
        delete file;
        LERROR("Illegal bpp! file: " << filename);
        return 0;
    }

    int bpp = header[4];
    t->setBpp(bpp / 8);  // divide by 8 to get the bytes per pixel

    switch (t->getBpp()) {
//         case 1:
//             t->setFormat(GL_RGB);
        case 3:
            t->setFormat(GL_RGB);
            LDEBUG("RGB");
            break;

        case 4:
            t->setFormat(GL_RGBA);
            LDEBUG("RGBA");
            break;

        default:
            LERROR("unsupported bpp " << filename);
    }

#ifdef GL_TEXTURE_RECTANGLE_ARB
    if (textureRectangle)
        t->setType( GL_TEXTURE_RECTANGLE_ARB );
    else
#endif
        t->setType( GL_TEXTURE_2D );

    t->setDataType( GL_UNSIGNED_BYTE );
    t->alloc();

    if (TGAheader[2] == 2) {
        // file is not compressed
        LDEBUG("Reading uncompressed TGA file...");
        if (file->read(reinterpret_cast<char*>(t->getPixelData()), t->getArraySize()) != t->getArraySize()) {
            LERROR("Failed to read uncompressed image! file: " << filename);
            delete file;
            delete t;
            return 0;
        }
    } else {
        // file is compressed
        LDEBUG("Reading compressed TGA file " << filename << " ...");

        //TODO: error handling
        unsigned char chunk[4];
        unsigned char* at = t->getPixelData();
        int Bpp = bpp / 8;
        for (unsigned int bytesDone=0; bytesDone < t->getArraySize(); ) {
            unsigned char packetHead;
            file->read(reinterpret_cast<char*>(&packetHead), 1);
            if (packetHead > 128) {
                //RLE
                packetHead -= 127;
                file->read(reinterpret_cast<char*>(&chunk), Bpp);
                for (unsigned char b=0; b < packetHead; b++) {
                        std::memcpy(at, chunk, Bpp);
                        bytesDone += Bpp;
                        at += Bpp;
                }
            } else {
                //RAW
                packetHead++;
                file->read(reinterpret_cast<char*>(at), Bpp * packetHead);
                bytesDone += packetHead * Bpp;
                at += packetHead * Bpp;
            }
        }
    }

    file->close();
    delete file;

    // switch r & b
    GLubyte temp;
    for (int i=0; i < (dimensions.x*dimensions.y); ++i) {
        temp = t->texel<col3>(i).r;
        t->texel<col3>(i).r = t->texel<col3>(i).b;
        t->texel<col3>(i).b = temp;
    }

    // flip image horizontally
    //col3 tmp;
    //for (int y=0; y < dimensions.y/2; ++y) {
        //for (int x=0; x < dimensions.x; ++x) {
            //tmp = t->texel<col3>(x,y);
            //t->texel<col3>(x,y) = t->texel<col3>(x,dimensions.y-1-y);
            //t->texel<col3>(x,dimensions.y-1-y) = tmp;
        //}
    //}


    bool success;
    if (textureRectangle) {
        success = createRectangleTexture(t, filter, compress, createOGLTex);
    } else {
        if (dimensions.y == 1)
            success = create1DTexture(t, filter, compress, createOGLTex);
        else
            success = create2DTexture(t, filter, compress, createOGLTex);
    }

    if (!success) {
        LERROR("Failed to create texture for " << filename);
        delete t;
        return 0;
    }

    if (!keepPixels) {
        delete[] t->getPixelData();
        t->setPixelData(0);
    }
    return t;
}

} // namespace tgt
