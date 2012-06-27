/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/core/datastructures/volume/volumedisk.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumefactory.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorswapendianness.h"

#include "voreen/core/io/volumereader.h"

#include <algorithm>
#include <typeinfo>

using tgt::vec3;
using tgt::bvec3;
using tgt::ivec3;
using tgt::svec3;
using tgt::col4;
using tgt::mat4;

namespace voreen {

const std::string VolumeDisk::loggerCat_("voreen.VolumeDisk");

VolumeDisk::VolumeDisk(const std::string& filename, const std::string& format, tgt::ivec3 dimensions, int64_t offset, bool swapEndian)
  : VolumeRepresentation(dimensions)
    , filename_(filename)
    , format_(format)
    , offset_(offset)
    , swapEndian_(swapEndian)
{
}

VolumeDisk::VolumeDisk(const VolumeDisk* diskrep)
    : VolumeRepresentation(diskrep->getDimensions())
      , filename_(diskrep->getFileName())
      , format_(diskrep->getFormat())
      , offset_(diskrep->getOffset())
{
}

VolumeDisk::~VolumeDisk() {
}

int VolumeDisk::getNumChannels() const {
    VolumeFactory vf;
    return vf.getNumChannels(getFormat());
}

int VolumeDisk::getBytesPerVoxel() const {
    VolumeFactory vf;
    return vf.getBytesPerVoxel(getFormat());
}

VolumeDisk* VolumeDisk::getSubVolume(tgt::svec3 dimensions, tgt::svec3 offset) const throw (std::bad_alloc){
    // create one voxel volume to get bits for current type
    VolumeFactory vf;
    VolumeRAM* volume = vf.create(getFormat(), tgt::svec3(1,1,1));

    int voxelSize = 1;
    if(volume){
        voxelSize = volume->getBitsAllocated() / 8;
        delete volume;
    }

    // calculate offset
    tgt::ivec3 dataDimsI = static_cast<tgt::ivec3>(getDimensions());
    tgt::ivec3 offsetI = static_cast<tgt::ivec3>(offset);
    int64_t initialStartPos = offset_ + (offsetI.z * (dataDimsI.x*dataDimsI.y)*voxelSize)+(offsetI.y * dataDimsI.x*voxelSize) + (offsetI.x*voxelSize);

    // create new disk representation
    VolumeDisk* newDiskRep = new VolumeDisk(filename_, format_, static_cast<tgt::ivec3>(dimensions), initialStartPos);

    return newDiskRep;
}

//--------------------------------------------------------

bool RepresentationConverterLoadFromDisk::canConvert(const VolumeRepresentation* source) const {
    if(dynamic_cast<const VolumeDisk*>(source))
        return true;
    else
        return false;
}

VolumeRepresentation* RepresentationConverterLoadFromDisk::convert(const VolumeRepresentation* source) const {
    const VolumeDisk* dr = dynamic_cast<const VolumeDisk*>(source);

    if(dr) {
        VolumeRAM* volume = 0;
        LDEBUGC("voreen.RepresentationConverterLoadFromDisk", "creating volume from diskrepr. " << dr->getFileName() << " format: " << dr->getFormat());
        VolumeFactory vf;
        volume = vf.create(dr->getFormat(), dr->getDimensions());

        if(!volume)
            return 0;

        FILE* fin;
        fin = fopen(dr->getFileName().c_str(),"rb");

        if (fin == 0)
            throw tgt::IOException("Unable to open raw file for reading", dr->getFileName());

        size_t bytesPerVoxel = static_cast<size_t>(volume->getBitsAllocated() / 8);
        size_t numVoxels = hmul(dr->getDimensions());
        size_t numBytes = numVoxels * bytesPerVoxel;

        int64_t offset = dr->getOffset();
        if(offset < 0) {
            //Assume data is aligned to end of file.

            // get file size:
            fseek(fin, 0, SEEK_END);
            int64_t fileSize = ftell(fin);
            rewind(fin);

            //calculate real offset:
            offset = fileSize - numBytes;
        }
#ifdef _MSC_VER
        _fseeki64(fin, offset, SEEK_SET);
#else
        fseek(fin, offset, SEEK_SET);
#endif

        if(fread(reinterpret_cast<char*>(volume->getData()), numBytes, 1, fin) != 1) {
            LERRORC("voreen.RepresentationConverterLoadFromDisk", "fread() failed");
            fclose(fin);
            delete volume;
            return 0;
        }

        fclose(fin);

        if(dr->getSwapEndian()) {
            Volume* tempHandle = new Volume(volume, vec3(1.0f), vec3(0.0f));
            VolumeOperatorSwapEndianness::APPLY_OP(tempHandle);
            tempHandle->releaseAllRepresentations();
        }

        return volume;
    }
    else {
        //should have checked before...
        //LERROR("Failed to convert!");
        return 0;
    }
}

} // namespace voreen
