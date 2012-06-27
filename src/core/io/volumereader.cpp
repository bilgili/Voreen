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

#include "voreen/core/io/volumereader.h"
#include "voreen/core/volume/volume.h"

#include <fstream>

namespace voreen {

const std::string VolumeReader::loggerCat_("voreen.io.VolumeReader");

VolumeReader::VolumeReader(IOProgress* progress /*= 0*/)
  : progress_(progress)
{}

void VolumeReader::read(Volume* volume, FILE* fin) {
    if (progress_) {

        int max = tgt::max(volume->getDimensions());

        // validate dimensions
        if (max <= 0 || max > 1e5) {
            LERROR("Invalid dimensions: " << volume->getDimensions());
            std::ostringstream stream;
            stream << volume->getDimensions();
            throw VoreenException("Invalid dimensions: " + stream.str());
        }

        //TODO: check what influences this has on performance. Choose larger block size? joerg
        progress_->setTotalSteps(max);

        // no remainder possible because getNumBytes is a multiple of max
        size_t sizeStep = volume->getNumBytes() / static_cast<size_t>(max);

        for (size_t i = 0; i < size_t(max); ++i) {
            if (fread(reinterpret_cast<char*>(volume->getData()) + sizeStep * i, 1, sizeStep, fin) == 0)
                LWARNING("fread() failed");
            progress_->setProgress(i);
        }
    }
    else {
        if (fread(reinterpret_cast<char*>(volume->getData()), 1, volume->getNumBytes(), fin) == 0)
        LWARNING("fread() failed");
    }
}

bool VolumeReader::isPersistent() const {
    return false;
}

void VolumeReader::reverseXSliceOrder(Volume* const volume) const {
    if (volume == 0)
        return;

    const int bytesPerVoxel = volume->getBytesPerVoxel();
    const tgt::ivec3 dim = volume->getDimensions();

    typedef unsigned char BYTE;
    BYTE* const data = reinterpret_cast<BYTE* const>(volume->getData());
    BYTE* aux = new BYTE[bytesPerVoxel];

    for (int x = 0; x < (dim.x / 2); ++x) {
        int _x = (dim.x - (x + 1));
        for (int z = 0; z < dim.z; ++z) {
            int a = (z * (dim.x * dim.y));
            for (int y = 0; y < dim.y; ++y) {
                int b = (y * dim.x);
                int l = a + b + x;
                int r = a + b + _x;
                memcpy(aux, (data + (l * bytesPerVoxel)), bytesPerVoxel);
                memcpy((data + (l * bytesPerVoxel)), (data + (r * bytesPerVoxel)), bytesPerVoxel);
                memcpy((data + (r * bytesPerVoxel)), aux, bytesPerVoxel);
            }
        }
    }
    delete [] aux;
}

void VolumeReader::reverseYSliceOrder(Volume* const volume) const {
    if (volume == 0)
        return;

    const int bytesPerVoxel = volume->getBytesPerVoxel();
    const tgt::ivec3 dim = volume->getDimensions();

    typedef unsigned char BYTE;
    BYTE* const data = reinterpret_cast<BYTE* const>(volume->getData());
    BYTE* aux = new BYTE[bytesPerVoxel];

    for (int y = 0; y < (dim.y / 2); ++y) {
        int _y = (dim.y - (y + 1));
        int b = (y * dim.x);
        int _b = (_y * dim.x);
        for (int z = 0; z < dim.z; ++z) {
            int a = (z * (dim.x * dim.y));
            for (int x = 0; x < dim.x; ++x) {
                int l = a + b + x;
                int r = a + _b + x;
                memcpy(aux, (data + (l * bytesPerVoxel)), bytesPerVoxel);
                memcpy((data + (l * bytesPerVoxel)), (data + (r * bytesPerVoxel)), bytesPerVoxel);
                memcpy((data + (r * bytesPerVoxel)), aux, bytesPerVoxel);
            }
        }
    }
    delete [] aux;
}

void VolumeReader::reverseZSliceOrder(Volume* const volume) const {
    if (volume == 0)
        return;

    const int bytesPerVoxel = volume->getBytesPerVoxel();
    const tgt::ivec3 dim = volume->getDimensions();
    const int sliceSize = dim.x * dim.y * bytesPerVoxel;

    typedef unsigned char BYTE;
    BYTE* const data = reinterpret_cast<BYTE* const>(volume->getData());
    BYTE* aux = new BYTE[sliceSize];
    for (int z = 0; z < dim.z / 2; ++z) {
        // copy slice i to auxiliary buffer
        //
        memcpy(aux, (data + (z * sliceSize)), sliceSize);

        // copy slice r = (dim.z - 1 - i) to position i
        //
        int r = (dim.z - (z + 1));
        memcpy((data + (z * sliceSize)), (data + (r * sliceSize)), sliceSize);

        // copy auxiliary buffer back to position r
        //
        memcpy((data + (r * sliceSize)), aux, sliceSize);
    }
    delete [] aux;
}

VolumeCollection* VolumeReader::readSlices(const std::string&, size_t, size_t)
    throw(tgt::FileException, std::bad_alloc)
{
    throw tgt::FileException("This VolumeReader can not read slice wise.");
}

VolumeHandle* VolumeReader::read(const VolumeOrigin& origin)
    throw (tgt::FileException, std::bad_alloc)
{

    VolumeHandle* result = 0;

    VolumeCollection* collection = read(origin.getPath());

    if (collection && collection->size() == 1) {
        result = collection->first();
    }
    else if (collection && collection->size() > 1) {
        delete collection;
        throw tgt::FileException("Only one volume expected", origin.getPath());
    }

    delete collection;

    return result;
}

VolumeCollection* VolumeReader::readBrick(const std::string& url, tgt::ivec3, int) throw(tgt::FileException, std::bad_alloc) {
     throw(new tgt::FileException("This file format does not support brick-wise reading of volume data.", url));
}

VolumeOrigin VolumeReader::convertOriginToRelativePath(const VolumeOrigin& origin, std::string& basePath) const {

    return VolumeOrigin(origin.getProtocol(),
                        tgt::FileSystem::relativePath(origin.getPath(), basePath),
                        origin.getSearchString());

}

VolumeOrigin VolumeReader::convertOriginToAbsolutePath(const VolumeOrigin& origin, std::string& basePath) const {

    // build new path only if this is not an absolute path
    if (origin.getPath().find("/") != 0 && origin.getPath().find("\\") != 0 && origin.getPath().find(":") != 1) {

        return VolumeOrigin(origin.getProtocol(),
                            tgt::FileSystem::absolutePath(basePath + "/" + origin.getPath()),
                            origin.getSearchString());
    }
    else
        return origin;
}

const std::vector<std::string>& VolumeReader::getExtensions() const {
    return extensions_;
}

const std::vector<std::string>& VolumeReader::getProtocols() const {
    return protocols_;
}

} // namespace voreen
