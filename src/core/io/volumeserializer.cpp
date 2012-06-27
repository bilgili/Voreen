/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/io/volumeserializer.h"

#include <cstring>

#include "voreen/core/io/volumereader.h"
#include "voreen/core/io/brickedvolumereader.h"
#include "voreen/core/io/volumewriter.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "tgt/filesystem.h"

namespace voreen {

//------------------------------------------------------------------------------

FormatClashException::FormatClashException(const Extensions& extensions)
    : extensions_(extensions)
{
    message_ = "FormatClashException: ";
    for (size_t i=0; i<extensions_.size(); i++) {
        message_ += extensions_[i] + " ";
    }
}

const FormatClashException::Extensions& FormatClashException::getExtensions() const throw() {
    return extensions_;
}

const char* FormatClashException::what() const throw() {
    return message_.c_str();
}


//------------------------------------------------------------------------------

VolumeSerializer::VolumeSerializer() {
}

VolumeSerializer::~VolumeSerializer() {
}

VolumeCollection* VolumeSerializer::load(const std::string& url) const
    throw (tgt::FileException, std::bad_alloc)
{
    VolumeReader* reader = getReader(url);
    return reader->read(url);
}

VolumeCollection* VolumeSerializer::loadSlices(const std::string& url, size_t firstSlice, size_t lastSlice) const
    throw (tgt::FileException, std::bad_alloc)
{
    VolumeReader* reader = getReader(url);
    return reader->readSlices(url, firstSlice, lastSlice);
}

VolumeCollection* VolumeSerializer::loadBrick(const std::string& url, tgt::ivec3 brickStartPos,
    int brickSize) const throw (tgt::FileException, std::bad_alloc)
{
    VolumeReader* reader = getReader(url);
    return reader->readBrick(url, brickStartPos, brickSize);
}

VolumeHandle* VolumeSerializer::load(const VolumeOrigin& origin) const
    throw (tgt::FileException, std::bad_alloc)
{
    VolumeReader* reader = getReader(origin.getURL());
    VolumeHandle* volumeHandle = reader->read(origin);
    return volumeHandle;
}

void VolumeSerializer::save(const std::string& filename, VolumeHandle* volumeHandle) const throw (tgt::FileException)
{
    VolumeWriter* writer = getWriter(filename);
    writer->write(filename, volumeHandle);
}

void VolumeSerializer::save(const std::string& filename, Volume* volume) const throw (tgt::FileException) {
    save(filename, new VolumeHandle(volume));
}

VolumeOrigin VolumeSerializer::convertOriginToRelativePath(const VolumeOrigin& origin, std::string& basePath) const
    throw (tgt::UnsupportedFormatException)
{
    VolumeReader* reader = getReader(origin.getURL());
    return reader->convertOriginToRelativePath(origin, basePath);
}

VolumeOrigin VolumeSerializer::convertOriginToAbsolutePath(const VolumeOrigin& origin, std::string& basePath) const
    throw (tgt::UnsupportedFormatException)
{
    VolumeReader* reader = getReader(origin.getURL());
    return reader->convertOriginToAbsolutePath(origin, basePath);
}

void VolumeSerializer::registerReader(VolumeReader* vr)
    throw (FormatClashException)
{
    std::vector<std::string> clashes; // already inserted extensions are tracked here

    // update extensions map
    const std::vector<std::string>& extensions = vr->getExtensions();
    for (size_t i = 0; i < extensions.size(); ++i) {
        std::pair<std::map<std::string, VolumeReader*>::iterator, bool> p =
            readersExtensionMap_.insert( std::make_pair(extensions[i], vr) );

        if (p.second == false) // if insertion was not successful
            clashes.push_back(extensions[i]);
    }

    // update formats map
    const std::vector<std::string>& formats = vr->getProtocols();
    for (size_t i = 0; i < formats.size(); ++i) {
        std::pair<std::map<std::string, VolumeReader*>::iterator, bool> p =
            readersProtocolMap_.insert( std::make_pair(formats[i], vr) );

        if (p.second == false) // if insertion was not successful
            clashes.push_back(formats[i]);
    }

    // was there a format clash?
    if (!clashes.empty())
        throw FormatClashException(clashes);
}

void VolumeSerializer::registerWriter(VolumeWriter* vw)
    throw (FormatClashException)
{
    std::vector<std::string> clashes; // already inserted extensions are tracked here

    // update extensions map
    const std::vector<std::string>& extensions = vw->getExtensions();
    for (size_t i = 0; i < extensions.size(); ++i) {
        std::pair<std::map<std::string, VolumeWriter*>::iterator, bool> p =
            writersMap_.insert( std::make_pair(extensions[i], vw) );

        if (p.second == false) // if insertion was not successful
            clashes.push_back(extensions[i]);
    }

    // was there a format clash?
    if (!clashes.empty())
        throw FormatClashException(clashes);
}

VolumeReader* VolumeSerializer::getReader(const std::string& url) const
    throw (tgt::UnsupportedFormatException)
{
    VolumeOrigin origin(url);

    // check if a reader for the URL's resource type is available
    std::string protocol = origin.getProtocol();
    if (!protocol.empty()) {
        if (readersProtocolMap_.find(protocol) != readersProtocolMap_.end())
            return readersProtocolMap_.find(protocol)->second;
        else
            throw tgt::UnsupportedFormatException(protocol, url);
    }

    // check if a reader for the filename extension is available
    std::string extension = tgt::FileSystem::fileExtension(origin.getPath(), true);

    if (readersExtensionMap_.find(extension) != readersExtensionMap_.end())
        return readersExtensionMap_.find(extension)->second;
    else
        throw tgt::UnsupportedFormatException(extension, url);
}

VolumeWriter* VolumeSerializer::getWriter(const std::string& filename) const
    throw (tgt::UnsupportedFormatException)
{
    std::string extension = tgt::FileSystem::fileExtension(filename, true);

    if (writersMap_.find(extension) == writersMap_.end())
        throw tgt::UnsupportedFormatException(extension, filename);

    return writersMap_.find(extension)->second;
}


} // namespace voreen
