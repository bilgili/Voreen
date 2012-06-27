/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include <algorithm>
#include <cstring>

#include "voreen/core/io/volumereader.h"
#include "voreen/core/io/volumewriter.h"

namespace {

// extracts the extension of a given filename and transforms it to lowercase
std::string extractExtensionLowerCase(const std::string& filename) {
    std::string extension = filename.substr(filename.rfind(".") + 1, filename.length());
    std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

    return extension;
}

} // namespace 

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
    std::string extension = extractExtensionLowerCase(filename);
    if (readers_.find(extension) == readers_.end())
        throw tgt::UnsupportedFormatException(extension, filename);
    VolumeReader* reader = readers_.find(extension)->second;
    VolumeSet* volumeSet = reader->read(filename);
    return volumeSet;
}

void VolumeSerializer::save(const std::string& filename, Volume* volume)
    throw (tgt::FileException)
{
    std::string extension = extractExtensionLowerCase(filename);

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
