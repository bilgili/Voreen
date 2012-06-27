/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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
#include "tgt/filesystem.h"

namespace voreen {

//------------------------------------------------------------------------------

FormatClashException::FormatClashException(const Extensions& extensions)
    : extensions_(extensions)
{}

const FormatClashException::Extensions& FormatClashException::getExtensions() const throw() {
    return extensions_;
}

const char* FormatClashException::what() const throw() {
    return "FormatClashException";
}


//------------------------------------------------------------------------------

VolumeSerializer::VolumeSerializer() {
}

VolumeSerializer::~VolumeSerializer() {
}

VolumeSet* VolumeSerializer::load(const std::string& filename)
    throw (tgt::FileException, std::bad_alloc)
{
    return loadSlices(filename,0, 0);
}

VolumeSet* VolumeSerializer::loadSlices(const std::string& filename, size_t firstSlice, size_t lastSlice)
    throw (tgt::FileException, std::bad_alloc)
{
    std::string realfile = filename;
    std::string extension = tgt::FileSystem::fileExtension(filename, true);

    // Special handling for DICOM "URLs", as DICOM file often have new extension. Also allows
    // specifiying entire directories by adding a trailing '/' or '\'.
    if (filename.find("dicom://") == 0) {
        extension = "dicom";
        realfile = filename.substr(8); // strip dicom://
    }

    if (readers_.find(extension) == readers_.end())
        throw tgt::UnsupportedFormatException(extension, filename);
    VolumeReader* reader = readers_.find(extension)->second;
    VolumeSet* volumeSet =0;
    if (filename.find("dicom://") == 0) {
        // dicom reader does not support readSlices() yet
        volumeSet = reader->read(realfile);
    } else if (dynamic_cast<BrickedVolumeReader*>(reader)) {
        BrickedVolumeReader* brickedReader = new BrickedVolumeReader();
        volumeSet = brickedReader->readSlices(realfile,firstSlice,lastSlice);
    } else {
        volumeSet = reader->readSlices(realfile,firstSlice,lastSlice);  
    }
    return volumeSet;
}

VolumeSet* VolumeSerializer::loadBrick(const std::string& filename, tgt::ivec3 brickStartPos,
    int brickSize) throw (tgt::FileException, std::bad_alloc)
{
    std::string realfile = filename;
    std::string extension = tgt::FileSystem::fileExtension(filename, true);

    // Special handling for DICOM "URLs", as DICOM file often have new extension. Also allows
    // specifiying entire directories by adding a trailing '/' or '\'.
    if (filename.find("dicom://") == 0) {
        extension = "dicom";
        realfile = filename.substr(8); // strip dicom://
    }

    if (readers_.find(extension) == readers_.end())
        throw tgt::UnsupportedFormatException(extension, filename);
    VolumeReader* reader = readers_.find(extension)->second;
    VolumeSet* volumeSet =0;
    if (dynamic_cast<BrickedVolumeReader*>(reader)) {
    } else {
        volumeSet = reader->readBrick(realfile,brickStartPos,brickSize);
    }
    return volumeSet;
}

VolumeHandle* VolumeSerializer::loadFromOrigin(VolumeHandle::Origin origin) {
    std::string extension;
        
    if (origin.filename.find("zip://") == 0) {
        // The filename starts with "zip://"
        extension = "zip";
        origin.filename = origin.filename.substr(6);
    }
    else
        extension = tgt::FileSystem::fileExtension(origin.filename, true);
    
    if (readers_.find(extension) == readers_.end())
        throw tgt::UnsupportedFormatException(extension, origin.filename);
    VolumeReader* reader = readers_.find(extension)->second;
    VolumeHandle* volumeHandle = reader->readFromOrigin(origin);
    return volumeHandle;    
}

void VolumeSerializer::save(const std::string& filename, Volume* volume)
    throw (tgt::FileException)
{
    std::string extension = tgt::FileSystem::fileExtension(filename, true);

    if (writers_.find(extension) != writers_.end())
        writers_.find(extension)->second->write(filename, volume);
    else
        throw tgt::UnsupportedFormatException(extension, filename);
}

void VolumeSerializer::registerReader(VolumeReader* vr)
    throw (FormatClashException)
{
    std::vector<std::string> clashes; // already inserted extensions are tracked here
    const std::vector<std::string>& extensions = vr->getExtensions();

    for (size_t i = 0; i < extensions.size(); ++i) {
        std::pair<Readers::iterator, bool> p =
            readers_.insert( std::make_pair(extensions[i], vr) );

        if (p.second == false) // if insertion was not successful
            clashes.push_back(extensions[i]);
    }

    // was there a format clash?
    if (!clashes.empty())
        throw FormatClashException(clashes);
}

void VolumeSerializer::registerWriter(VolumeWriter* vw)
    throw (FormatClashException)
{
    std::vector<std::string> clashes; // already inserted extensions are tracked here
    const std::vector<std::string>& extensions = vw->getExtensions();

    for (size_t i = 0; i < extensions.size(); ++i) {
        std::pair<Writers::iterator, bool> p =
            writers_.insert( std::make_pair(extensions[i], vw) );

        if (p.second == false) // if insertion was not successful
            clashes.push_back(extensions[i]);
    }

    // was there a format clash?
    if (!clashes.empty())
        throw FormatClashException(clashes);
}

} // namespace voreen
