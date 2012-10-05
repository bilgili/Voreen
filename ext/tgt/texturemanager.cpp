/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2005-2012 Visualization and Computer Graphics Group, *
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

#include "tgt/texturemanager.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "tgt/gpucapabilities.h"
#include "tgt/texturereader.h"


namespace {

// cannot use tolower directly, due to some template overloading issues
int lower_case(int c) {
    return tolower(c);
}

} // namespace


namespace tgt {

//------------------------------------------------------------------------------
// TextureManager
//------------------------------------------------------------------------------

const std::string TextureManager::loggerCat_("tgt.Texture.Manager");

TextureManager::TextureManager()
{}

TextureManager::~TextureManager() {
    // clean up
    for (std::set<TextureReader*>::iterator iter = readerSet_.begin(); iter != readerSet_.end(); ++iter)
        delete *iter;
}

Texture* TextureManager::loadIgnorePath(const std::string& completeFilename, Texture::Filter filter, bool compress,
                              bool keepPixels, bool createOGLTex, bool useCache)
{
    size_t found;
    std::string filename;

    found = completeFilename.find_last_of("/\\");

    if (found != std::string::npos){
        filename = completeFilename.substr(found+1);
    }
    else {
        filename = completeFilename;
    }

    if (compress && !GpuCaps.isTextureCompressionSupported())
        compress = false;

    if (useCache && isLoaded(filename)) {
        increaseUsage(filename);
        return get(filename);
    }

    Texture* t = 0;
    std::string ending = getEnding(filename);
    std::transform (ending.begin(), ending.end(), ending.begin(), lower_case);

    if (readers_.size() == 0)
        LWARNING("No TextureReaders are registered while calling TextureManager::loadIgnorePath()!");

    if (readers_.find(ending) != readers_.end()) {
        LDEBUG("Found matching reader: " << readers_[ending]->getName());

        t = readers_[ending]->loadTexture(completeFilename, filter, compress,
                                            keepPixels, createOGLTex);
        if (!t) {
            t = readers_[ending]->loadTexture(filename, filter, compress,
                                    keepPixels, createOGLTex);
        }
        if ((t) && (useCache)) {
            reg(t, filename);
            return t;
        }
    }
    return t;
}

Texture* TextureManager::load(const std::string& filename, Texture::Filter filter, bool compress,
                              bool keepPixels, bool createOGLTex, bool useCache)
{
    if (compress && !GpuCaps.isTextureCompressionSupported())
        compress = false;

    if (useCache && isLoaded(filename)) {
        increaseUsage(filename);
        return get(filename);
    }

    Texture* t = 0;
    std::string ending = getEnding(filename);
    std::transform (ending.begin(), ending.end(), ending.begin(), lower_case);

    if (readers_.size() == 0)
        LWARNING("No TextureReaders are registered while calling TextureManager::load()!");

    if (readers_.find(ending) != readers_.end()) {
        LDEBUG("Found matching reader: " << readers_[ending]->getName());

        std::string completeFilename = completePath(filename);
        t = readers_[ending]->loadTexture(completeFilename, filter, compress,
                                          keepPixels, createOGLTex);

        // else try just the filename without path
        if (!t && completeFilename != filename) {
            t = readers_[ending]->loadTexture(filename, filter, compress,
                                              keepPixels, createOGLTex);
        }

        if (t && useCache)
            reg(t, filename);

        if (!t)
            LERROR("Texture file not found: " + filename);
    }
    else {
        LERROR("No matching reader found for ending '" << ending << "' while loading " << filename);
    }

    return t;
}

/*
 * TODO: it only loads 3D and 16bit textures for now
 */
Texture* TextureManager::loadFromMemory(Texture* t, Texture::Filter filter, bool compress,
                                        bool createOGLTex)
{
    t->bpp_ = 2;

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
            LERROR(static_cast<int>(t->bpp_) << " bits per pixel...error!");
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

    return t;
}

std::string TextureManager::getEnding(const std::string& filename) const {
    std::string ending = "";
    size_t pos = filename.find_last_of('.');
    if (pos != std::string::npos)
        ending = filename.substr(pos + 1);

    return ending;
}

void TextureManager::registerReader(TextureReader* r) {
    readerSet_.insert(r);
    LDEBUG("TextureManager: Registering reader: " << r->getName());

    std::string formats = "";
    std::vector<std::string> knownEndings = r->getEndings();
    std::vector<std::string>::iterator i;
    for ( i = knownEndings.begin(); i != knownEndings.end(); ++i ) {
        readers_[*i] = r;
        formats += *i + " ";
    }
    LDEBUG("Known formats: " << formats);
}

} // namespace tgt
