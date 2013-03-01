/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

std::string VolumeDisk::getBaseType() const {
    VolumeFactory vf;
    return vf.getBaseType(getFormat());
}

int VolumeDisk::getNumChannels() const {
    VolumeFactory vf;
    return vf.getNumChannels(getFormat());
}

int VolumeDisk::getBytesPerVoxel() const {
    VolumeFactory vf;
    return vf.getBytesPerVoxel(getFormat());
}

VolumeRAM* VolumeDisk::loadSlices(const size_t firstSlice, const size_t lastSlice) const {
    //check for wrong parameter
    tgtAssert(getDimensions().z > lastSlice, "lastSlice is out of volume dimension!!!");
    tgtAssert(firstSlice <= lastSlice, "firstSlice has to be less or equal lastSlice!!!");

    //create new VolumeRam
    VolumeFactory vf;
    VolumeRAM* vr = vf.create(getFormat(), tgt::svec3(getDimensions().x,getDimensions().y,lastSlice-firstSlice+1));
    //return if something went wrong
    if(!vr) return 0;

    //open file
    FILE* fin;
    fin = fopen(getFileName().c_str(),"rb");

    if (fin == 0) return 0;

    size_t bytesPerVoxel = static_cast<size_t>(vr->getBitsAllocated() / 8);
    size_t numVoxels = getDimensions().x*getDimensions().y*(lastSlice-firstSlice+1);
    size_t numBytes = numVoxels * bytesPerVoxel;

    int64_t offset = getOffset();
    if(offset < 0) {
        //Assume data is aligned to end of file.

        // get file size:
        fseek(fin, 0, SEEK_END);
        int64_t fileSize = ftell(fin);
        rewind(fin);

        //calculate real offset:
        offset = fileSize - hmul(getDimensions())*bytesPerVoxel;
    }

    //modify offset to start at first slice
    offset += getDimensions().x*getDimensions().y*firstSlice*bytesPerVoxel;

    fseek(fin, static_cast<long>(offset), SEEK_SET);

    //read into ram
    if(fread(reinterpret_cast<char*>(vr->getData()), numBytes, 1, fin) != 1) {
        LERRORC("voreen.RepresentationConverterLoadFromDisk", "fread() failed");
        fclose(fin);
        delete vr;
        return 0;
    }

    fclose(fin);

    //swap endian
    if(getSwapEndian()) {
        Volume* tempHandle = new Volume(vr, vec3(1.0f), vec3(0.0f));
        VolumeOperatorSwapEndianness::APPLY_OP(tempHandle);
        tempHandle->releaseAllRepresentations();
        delete tempHandle;
    }

    return vr;
}

VolumeRAM* VolumeDisk::loadBrick(const tgt::svec3 pOffset, const tgt::svec3 pDimensions) const {
    //check for wrong parameter
    tgtAssert(tgt::min(tgt::lessThanEqual(pOffset+pDimensions,getDimensions())) == 1, "Requested brick is outside of the volume!!!");

    //create new VolumeRam
    VolumeFactory vf;
    VolumeRAM* vr = vf.create(getFormat(), pDimensions);
    //return if something went wrong
    if(!vr) return 0;

    //open file
    FILE* fin;
    fin = fopen(getFileName().c_str(),"rb");

    if (fin == 0) return 0;

    size_t bytesPerVoxel = static_cast<size_t>(vr->getBitsAllocated() / 8);
    size_t numVoxels = pDimensions.x;
    size_t numBytes = numVoxels * bytesPerVoxel;

    int64_t offset = getOffset();
    if(offset < 0) {
        //Assume data is aligned to end of file.

        // get file size:
        fseek(fin, 0, SEEK_END);
        int64_t fileSize = ftell(fin);
        rewind(fin);

        //calculate real offset:
        offset = fileSize - hmul(getDimensions())*bytesPerVoxel;
    }

    //modify offset to start at first slice
    offset += getDimensions().x,getDimensions().y*pOffset.z*bytesPerVoxel;

    fseek(fin, static_cast<long>(offset), SEEK_SET);

    //read into ram
    size_t pointerOffset = 0;
    for(size_t z = 0; z < pDimensions.z; z++){
        for(size_t y = 0; y < pDimensions.y; y++) {
            //read into ram
            if(fread(reinterpret_cast<char*>(vr->getData())+pointerOffset, numBytes, 1, fin) != 1) {
                LERRORC("voreen.RepresentationConverterLoadFromDisk", "fread() failed");
                fclose(fin);
                delete vr;
                return 0;
            }
            //move offset
            pointerOffset += numBytes;

            //move to next read
            if(y < pDimensions.y)
                fseek(fin, static_cast<long>(getDimensions().x-pDimensions.x),SEEK_SET);
        }
        //move to next read
        if(z < pDimensions.z)
            fseek(fin,static_cast<long>((getDimensions().x*getDimensions().y)-(pDimensions.x*pDimensions.y)),SEEK_SET);
    }

    fclose(fin);

    //swap endian
    if(getSwapEndian()) {
        Volume* tempHandle = new Volume(vr, vec3(1.0f), vec3(0.0f));
        VolumeOperatorSwapEndianness::APPLY_OP(tempHandle);
        tempHandle->releaseAllRepresentations();
        delete tempHandle;
    }

    return vr;
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

        if (fin == 0) {
            //throw tgt::IOException("Unable to open raw file for reading", dr->getFileName());
            LERRORC("voreen.RepresentationConverterLoadFromDisk", "Unable to open raw file for reading");
            return 0;
        }

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
            delete tempHandle;
        }

        return volume;
    }
    else {
        //should have checked before...
        //LERROR("Failed to convert!");
        return 0;
    }
}

bool RepresentationConverterLoadFromDiskToGL::canConvert(const VolumeRepresentation* source) const {
    if(dynamic_cast<const VolumeDisk*>(source))
        return true;
    else
        return false;
}

VolumeRepresentation* RepresentationConverterLoadFromDiskToGL::convert(const VolumeRepresentation* source) const {
    //convert from disk to ram
    RepresentationConverterLoadFromDisk diskToRam;
    VolumeRepresentation* volumeRam = 0;
    if(diskToRam.canConvert(source))
        volumeRam = diskToRam.convert(source);
    else {
        LERRORC("voreen.RepresentationConverterLoadFromDiskToGL","Source volume is no disk volume!!!");
        return 0;
    }
    //convert from ram to disk
    RepresentationConverterUploadGL ramToGL;
    VolumeRepresentation* volumeGL = 0;
    if(ramToGL.canConvert(volumeRam))
        volumeGL = ramToGL.convert(volumeRam);
    else {
        LERRORC("voreen.RepresentationConverterLoadFromDiskToGL","volumeRam is a null pointer!!!");
        return 0;
    }
    //clean volumeRam and return
    delete volumeRam;
    return volumeGL;
}

} // namespace voreen
