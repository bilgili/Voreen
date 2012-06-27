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

#include "voreen/core/io/rawvolumereader.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>

#include "tgt/exception.h"
#include "tgt/filesystem.h"

#include "voreen/core/io/ioprogress.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

using tgt::ivec3;
using tgt::vec3;

namespace voreen {

const std::string RawVolumeReader::loggerCat_ = "voreen.io.VolumeReader.raw";

RawVolumeReader::ReadHints::ReadHints(tgt::ivec3 dimensions, tgt::vec3 spacing, int bitsStored,
                                      const std::string& objectModel, const std::string& format,
                                      int headerskip, bool bigEndian)
    : dimensions_(dimensions), spacing_(spacing), bitsStored_(bitsStored),
      objectModel_(objectModel), format_(format), headerskip_(headerskip),
      bigEndianByteOrder_(bigEndian),
      transformation_(tgt::mat4::identity),
      modality_(Modality::MODALITY_UNKNOWN),
      timeStep_(-1.f),
      spreadMin_(0.f),
      spreadMax_(0.f),
      metaString_(""),
      unit_(""),
      sliceOrder_("+z")
{}

RawVolumeReader::RawVolumeReader(IOProgress* progress)
    : VolumeReader(progress)
{
    protocols_.push_back("raw");
}

void RawVolumeReader::setReadHints(ivec3 dimensions, vec3 spacing, int bitsStored,
                                const std::string& objectModel, const std::string& format,
                                int headerskip, bool bigEndian)
{
    hints_ = ReadHints(dimensions, spacing, bitsStored, objectModel, format, headerskip, bigEndian);
}

void RawVolumeReader::setReadHints(const ReadHints& hints) {
    hints_ = hints;
}

VolumeCollection* RawVolumeReader::read(const std::string &url)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    return readSlices(url, 0, 0);
}

VolumeHandle* RawVolumeReader::read(const VolumeOrigin& origin)
    throw (tgt::FileException, std::bad_alloc)
{
    // read parameters from origin
    std::string filename = origin.getPath();
    std::string objectModel = origin.getSearchParameter("objectModel");
    std::string format = origin.getSearchParameter("format");

    tgt::ivec3 dimensions;
    std::istringstream s1(origin.getSearchParameter("dim_x"));
    s1 >> dimensions.x;
    s1.str(origin.getSearchParameter("dim_y"));
    s1.clear();
    s1 >> dimensions.y;
    s1.str(origin.getSearchParameter("dim_z"));
    s1.clear();
    s1 >> dimensions.z;

    tgt::vec3 spacing;
    std::istringstream s2(origin.getSearchParameter("spacing_x"));
    s2 >> spacing.x;
    s2.str(origin.getSearchParameter("spacing_y"));
    s2.clear();
    s2 >> spacing.y;
    s2.str(origin.getSearchParameter("spacing_z"));
    s2.clear();
    s2 >> spacing.z;

    int headerskip;
    std::istringstream s3(origin.getSearchParameter("headerskip"));
    s3 >> headerskip;

    int byteOrder;
    std::istringstream s4(origin.getSearchParameter("bigEndian"));
    s4 >> byteOrder;
    bool bigEndian = (byteOrder == 1);

    // pass read hints and load volume
    setReadHints(static_cast<ivec3>(dimensions), static_cast<vec3>(spacing), 0, objectModel, format, headerskip, bigEndian);
    VolumeHandle* handle = 0;
    VolumeCollection* collection = read(filename);
    if (!collection->empty())
        handle = collection->first();

    delete collection;

    return handle;
}

namespace {

inline void endian_swap(uint32_t& x) {
    x = (x>>24) |
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}

inline void endian_swap(uint16_t& x) {
    x = (x>>8) | (x<<8);
}

}

VolumeCollection* RawVolumeReader::readSlices(const std::string &url, size_t firstSlice, size_t lastSlice )
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    VolumeOrigin origin(url);
    std::string fileName = origin.getPath();

    ReadHints& h = hints_;

    // check dimensions
    if (tgt::hor(tgt::lessThan(h.dimensions_, ivec3(0))) || tgt::hor(tgt::greaterThan(h.dimensions_, ivec3(10000)))) {
        LERROR("Invalid volume dimensions: " << h.dimensions_);
        return 0;
    }

    //Remember the dimensions of the entire volume.
    tgt::ivec3 originalVolumeDimensions = h.dimensions_;

    // check if we have to read only some slices instead of the whole volume.
    if ( ! (firstSlice==0 && lastSlice==0)) {
        if (lastSlice > firstSlice) {
            h.dimensions_.z = lastSlice - firstSlice;
        }
    }

    std::string info = "Loading raw file " + fileName + " ";

    if (h.dimensions_ == tgt::ivec3::zero)
        throw tgt::CorruptedFileException("No readHints set.", fileName);

    FILE* fin;
    fin = fopen(fileName.c_str(),"rb");

    if (fin == 0)
        throw tgt::IOException("Unable to open raw file for reading", fileName);

    Volume* volume;

    if (h.objectModel_ == "I") {
        if (h.format_ == "UCHAR") {
            LINFO(info << "(8 bit dataset)");
            VolumeUInt8* v = new VolumeUInt8(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "CHAR") {
            LWARNING(info << "(8 bit signed dataset, converting to 8 bit unsigned)");
            VolumeInt8* v = new VolumeInt8(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if ((h.format_ == "USHORT" && h.bitsStored_ == 12) || h.format_ == "USHORT_12") {
            LINFO(info << "(12 bit dataset)");
            VolumeUInt16* v = new VolumeUInt16(h.dimensions_, h.spacing_, 12);
            volume = v;
        }
        else if (h.format_ == "USHORT") {
            LINFO(info << "(16 bit dataset)");
            VolumeUInt16* v = new VolumeUInt16(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "SHORT") {
            LWARNING(info << "(16 bit signed dataset, converting to 16 bit unsigned)");
            VolumeInt16* v = new VolumeInt16(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "UINT") {
            LWARNING(info << "(32 bit dataset, converting to 16 bit)");
            VolumeUInt32* v = new VolumeUInt32(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "INT") {
            LWARNING(info << "(32 bit signed dataset, converting to 16 bit unsigned)");
            VolumeInt32* v = new VolumeInt32(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "FLOAT") {
            LINFO(info << "(32 bit float dataset)");
            VolumeFloat* v = new VolumeFloat(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "FLOAT8" || h.format_ == "FLOAT16") {
            LWARNING(info << "(32 bit float dataset, converting to 8 or 16 bit)");
            VolumeFloat* v = new VolumeFloat(h.dimensions_, h.spacing_);
            volume = v;
        }
        else {
            fclose(fin);
            throw tgt::CorruptedFileException("Format '" + h.format_ + "' not supported", fileName);
        }
    }
    else if (h.objectModel_ == "RGBA") {
        if (h.format_ == "UCHAR") {
            LINFO(info << "(4x8 bit dataset)");
            Volume4xUInt8* v = new Volume4xUInt8(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "USHORT") {
            LINFO(info << "(4x16 bit dataset)");
            Volume4xUInt16* v = new Volume4xUInt16(h.dimensions_, h.spacing_);
            volume = v;
        }
        else {
            fclose(fin);
            throw tgt::CorruptedFileException("Format '" + h.format_ + "' not supported for object model RGBA", fileName);
        }
    }
    else if (h.objectModel_ == "RGB") {
        if (h.format_ == "UCHAR") {
            LINFO(info << "(3x8 bit dataset)");
            Volume3xUInt8* v = new Volume3xUInt8(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "USHORT") {
            LINFO(info << "(3x16 bit dataset)");
            Volume3xUInt16* v = new Volume3xUInt16(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "FLOAT") {
            LINFO(info << "(3x32 bit dataset)");
            Volume3xFloat* v = new Volume3xFloat(h.dimensions_, h.spacing_);
            volume = v;
        } else {
            fclose(fin);
            throw tgt::CorruptedFileException("Format '" + h.format_ + "' not supported for object model RGB", fileName);
        }
    }
    else if (h.objectModel_ == "LA") { // luminance alpha
        LINFO(info << "(luminance16 alpha16 dataset)");
        Volume4xUInt8* v = new Volume4xUInt8(h.dimensions_, h.spacing_);
        volume = v;
    }
    else {
        fclose(fin);
        throw tgt::CorruptedFileException("unsupported ObjectModel '" + h.objectModel_ + "'", fileName);
    }

    //If we only have to read slices, skip to the position we have to read the first
    //slice from
    uint64_t skip = (uint64_t)h.dimensions_.x * (uint64_t)h.dimensions_.y * (uint64_t)firstSlice*
        (uint64_t) (volume->getBitsAllocated() / 8);

    // now add that to the headerskip we might have received
    uint64_t offset = h.headerskip_ + skip;

    #ifdef _MSC_VER
        _fseeki64(fin, offset, SEEK_SET);
    #else
        fseek(fin, offset, SEEK_SET);
    #endif

    volume->clear();

    if (getProgress()) {
        getProgress()->setTitle("Loading Volume");
        // getProgress()->setMessage("Loading volume: " + tgt::FileSystem::fileName(fileName));
        getProgress()->setMessage("Loading volume: " + fileName);
    }
    VolumeReader::read(volume, fin);

    if (lastSlice == 0) {
        if (feof(fin) ) {
            delete volume;
            // throw exception
            throw tgt::CorruptedFileException("unexpected EOF: raw file truncated or ObjectModel '" +
                                              h.objectModel_ + "' invalid", fileName);
        }
    }

    fclose(fin);

    // need to swap endianess?
    if (h.bigEndianByteOrder_) {
        if (h.format_ == "FLOAT") {
            LWARNING("Swapping byte order of 32-bit float volume");
            uint32_t* data = reinterpret_cast<uint32_t*>(volume->getData());
            int numElements = volume->getNumVoxels()*volume->getNumChannels();
            for (int i=0; i<numElements; i++) {
                endian_swap(data[i]);
            }
            VolumeFloat* vf = dynamic_cast<VolumeFloat*>(volume);
            if (vf)
                vf->invalidate();
        }
        else if (h.format_ == "USHORT" || h.format_ == "SHORT" || h.format_ == "USHORT_12") {
            LWARNING("Swapping byte order of 16-bit volume");
            uint16_t* data = reinterpret_cast<uint16_t*>(volume->getData());
            int numElements = volume->getNumVoxels()*volume->getNumChannels();
            for (int i=0; i<numElements; i++) {
                endian_swap(data[i]);
            }
        }
        else if (h.format_ == "UINT" || h.format_ == "INT") {
            LWARNING("Swapping byte order of 32-bit volume");
            uint32_t* data = reinterpret_cast<uint32_t*>(volume->getData());
            int numElements = volume->getNumVoxels()*volume->getNumChannels();
            for (int i=0; i<numElements; i++) {
                endian_swap(data[i]);
            }
        }
    }

    // convert if neccessary
    Volume* conv = 0;

    if (h.format_ == "FLOAT8")
        conv = new VolumeUInt8(h.dimensions_, h.spacing_);
    else if (h.format_ == "FLOAT16")
        conv = new VolumeUInt16(h.dimensions_, h.spacing_);
    else if (h.format_ == "CHAR")
        conv = new VolumeUInt8(h.dimensions_, h.spacing_);
    else if (h.format_ == "SHORT" || h.format_ == "UINT" || h.format_ == "INT")
        conv = new VolumeUInt16(h.dimensions_, h.spacing_);

    if (conv) {
        conv->convert(volume);
        std::swap(conv, volume);
        delete conv;
    }

    // normalize float data to [0.0; 1.0]
    if (h.format_ == "FLOAT") {
        VolumeFloat* vf = dynamic_cast<VolumeFloat*>(volume);

        if (vf && h.spreadMin_ != h.spreadMax_) {
            LINFO("Normalizing float volume with min/max: " << tgt::vec2(vf->min(), vf->max()));
            const size_t n = vf->getNumVoxels();

            // use spread values  if available
            if (h.spreadMin_ != h.spreadMax_) {
                LINFO("Using spread " << tgt::vec2(h.spreadMin_, h.spreadMax_));

                const float d = h.spreadMax_ - h.spreadMin_;
                float* voxel = vf->voxel();
                for (size_t i = 0; i < n; ++i)
                    voxel[i] = (voxel[i] - h.spreadMin_) / d;
            } else {
                const float d = vf->max() - vf->min();
                const float p = vf->min();
                float* voxel = vf->voxel();
                for (size_t i = 0; i < n; ++i)
                    voxel[i] = (voxel[i] - p) / d;
            }
            vf->invalidate();
        }
    }

    if (h.sliceOrder_ == "-x") {
        LINFO("slice order is -x, reversing order to +x...\n");
        reverseXSliceOrder(volume);
    } else if (h.sliceOrder_ == "-y") {
        LINFO("slice order is -y, reversing order to +y...\n");
        reverseYSliceOrder(volume);
    } else if (h.sliceOrder_ == "-z") {
        LINFO("slice order is -z, reversing order to +z...\n");
        reverseZSliceOrder(volume);
    }

    volume->setTransformation(h.transformation_);
    volume->meta().setString(h.metaString_);
    volume->meta().setUnit(h.unit_);
    volume->meta().setParentVolumeDimensions(originalVolumeDimensions);

    VolumeHandle* volumeHandle = new VolumeHandle(volume, 0.0f);
    volumeHandle->setModality(h.modality_);

    // encode raw parameters into search string
    std::ostringstream searchStream;
    searchStream << "objectModel=" << h.objectModel_ << "&";
    searchStream << "format=" << h.format_ << "&";
    searchStream << "headerskip=" << h.headerskip_ << "&";
    if (h.bigEndianByteOrder_)
        searchStream << "bigEndian=" << h.bigEndianByteOrder_ << "&";
    searchStream << "dim_x=" << h.dimensions_.x << "&";
    searchStream << "dim_y=" << h.dimensions_.y << "&";
    searchStream << "dim_z=" << h.dimensions_.z << "&";
    searchStream << "spacing_x=" << h.spacing_.x << "&";
    searchStream << "spacing_y=" << h.spacing_.y << "&";
    searchStream << "spacing_z=" << h.spacing_.z << "&";

    volumeHandle->setOrigin(VolumeOrigin("raw", fileName, searchStream.str()));

    VolumeCollection* volumeCollection = new VolumeCollection();
    volumeCollection->add(volumeHandle);

    return volumeCollection;
}

VolumeCollection* RawVolumeReader::readBrick(const std::string &url, tgt::ivec3 brickStartPos,int brickSize)
    throw(tgt::FileException, std::bad_alloc)
{
    VolumeOrigin origin(url);
    std::string fileName = origin.getPath();

    ReadHints& h = hints_;
    tgt::ivec3 datasetDims = h.dimensions_;
    h.dimensions_ = tgt::ivec3(brickSize);

    if (h.dimensions_ == tgt::ivec3::zero) {
        throw tgt::CorruptedFileException("No readHints set.", fileName);
    }

    FILE* fin;
    fin = fopen(fileName.c_str(),"rb");

    if (fin == 0)
        throw tgt::IOException("Unable to open raw file for reading", fileName);

    Volume* volume;

    if (h.objectModel_ == "I") {
        if (h.format_ == "UCHAR") {
            VolumeUInt8* v = new VolumeUInt8(h.dimensions_, h.spacing_);
            volume = v;

        }
        else if ((h.format_ == "USHORT" && h.bitsStored_ == 12) || h.format_ == "USHORT_12") {
            VolumeUInt16* v = new VolumeUInt16(h.dimensions_, h.spacing_, 12);
            volume = v;
        }
        else if (h.format_ == "USHORT") {
            VolumeUInt16* v = new VolumeUInt16(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "FLOAT8" || h.format_ == "FLOAT16" || h.format_ == "FLOAT") {
            VolumeFloat* v = new VolumeFloat(h.dimensions_, h.spacing_);
            volume = v;
        }
        else {
            fclose(fin);
            throw tgt::CorruptedFileException("Format '" + h.format_ + "' not supported", fileName);
        }
    }
    else if (h.objectModel_ == "RGBA") {
        if (h.format_ == "UCHAR") {
            //LINFO("Reading 4x8 bit dataset");
            Volume4xUInt8* v = new Volume4xUInt8(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "USHORT") {
            //LINFO("Reading 4x16 bit dataset");
            Volume4xUInt16* v = new Volume4xUInt16(h.dimensions_, h.spacing_);
            volume = v;
        }
        else {
            fclose(fin);
            throw tgt::CorruptedFileException("Format '" + h.format_ + "' not supported for object model RGBA", fileName);
        }
    }
    else if (h.objectModel_ == "RGB") {
        if (h.format_ == "UCHAR") {
            //LINFO("Reading 3x8 bit dataset");
            Volume3xUInt8* v = new Volume3xUInt8(h.dimensions_, h.spacing_);
            volume = v;
        }
        else if (h.format_ == "USHORT") {
            //LINFO("Reading 3x16 bit dataset");
            Volume3xUInt16* v = new Volume3xUInt16(h.dimensions_, h.spacing_);
            volume = v;
        } else if (h.format_ == "FLOAT") {
            Volume3xFloat* v = new Volume3xFloat(h.dimensions_, h.spacing_);
            volume = v;
        } else {
            fclose(fin);
            throw tgt::CorruptedFileException("Format '" + h.format_ + "' not supported for object model RGB", fileName);
        }
    }
    else if (h.objectModel_ == "LA") { // luminance alpha
        //LINFO("Reading luminance16 alpha16 dataset");
        Volume4xUInt8* v = new Volume4xUInt8(h.dimensions_, h.spacing_);
        volume = v;
    }
    else {
        fclose(fin);
        throw tgt::CorruptedFileException("unsupported ObjectModel '" + h.objectModel_ + "'", fileName);
    }

    volume->clear();

    int voxelSize = (volume->getBitsAllocated() / 8);

    uint64_t initialSeekPos = (brickStartPos.z *(datasetDims.x*datasetDims.y)*brickSize*voxelSize )+
        (brickStartPos.y * datasetDims.x*brickSize*voxelSize) + (brickStartPos.x*brickSize*voxelSize);

    // add header skip
    initialSeekPos += h.headerskip_;

    #ifdef _MSC_VER
        _fseeki64(fin, initialSeekPos, SEEK_SET);
    #else
        fseek(fin, initialSeekPos, SEEK_SET);
    #endif

        for (int i=0; i < brickSize; i++) {
            for (int j=0; j < brickSize; j++) {
                int volumePos = brickSize*j + (i*brickSize*brickSize);
                volumePos = volumePos*voxelSize;
                if (fread(reinterpret_cast<char*>(volume->getData()) + volumePos, 1, brickSize*voxelSize, fin) == 0)
                    LWARNING("fread() failed");
                #ifdef _MSC_VER
                    _fseeki64(fin, (datasetDims.x - brickSize)*voxelSize, SEEK_CUR);
                #else
                    fseek(fin, (datasetDims.x - brickSize)*voxelSize, SEEK_CUR);
                #endif
            }
            #ifdef _MSC_VER
                _fseeki64(fin, (datasetDims.y - brickSize)*datasetDims.x*voxelSize, SEEK_CUR);
            #else
                fseek(fin, (datasetDims.y - brickSize)*datasetDims.x*voxelSize, SEEK_CUR);
            #endif
        }

    fclose(fin);

    // convert if neccessary
    Volume* conv = 0;

    if (h.format_ == "FLOAT8")
        conv = new VolumeUInt8(h.dimensions_, h.spacing_);
    else if (h.format_ == "FLOAT16")
        conv = new VolumeUInt16(h.dimensions_, h.spacing_);

    if (conv) {
        conv->convert(volume);
        std::swap(conv, volume);
        delete conv;
    }

    volume->setTransformation(h.transformation_);
    volume->meta().setString(h.metaString_);
    volume->meta().setUnit(h.unit_);
    volume->meta().setParentVolumeDimensions(datasetDims);

    VolumeCollection* volumeCollection = new VolumeCollection();
    VolumeHandle* volumeHandle = new VolumeHandle(volume, 0.0f);
    volumeHandle->setModality(h.modality_);

    // encode raw parameters into search string
    std::ostringstream searchStream;
    searchStream << "objectModel=" << h.objectModel_ << "&";
    searchStream << "format=" << h.format_ << "&";
    searchStream << "headerskip=" << h.headerskip_ << "&";
    if (h.bigEndianByteOrder_)
        searchStream << "bigEndian=" << h.bigEndianByteOrder_ << "&";
    searchStream << "dim_x=" << h.dimensions_.x << "&";
    searchStream << "dim_y=" << h.dimensions_.y << "&";
    searchStream << "dim_z=" << h.dimensions_.z << "&";
    searchStream << "spacing_x=" << h.spacing_.x << "&";
    searchStream << "spacing_y=" << h.spacing_.y << "&";
    searchStream << "spacing_z=" << h.spacing_.z << "&";
    volumeHandle->setOrigin(VolumeOrigin("raw", fileName, searchStream.str()));

    volumeCollection->add(volumeHandle);

    return volumeCollection;
}

VolumeHandle* RawVolumeReader::readSliceStack(const std::vector<std::string>& sliceFiles)
    throw(tgt::FileException, std::bad_alloc) {

    if (hints_.dimensions_.z > 1) {
        LWARNING("Setting z-dimension of slices to 1.");
        hints_.dimensions_.z = 1;
    }

    std::vector<Volume*> volumes;
    for (size_t i=0; i<sliceFiles.size(); i++) {
        try {
            VolumeCollection* collection = read(sliceFiles[i]);
            if (!collection->empty() && collection->first()->getVolume()) {
                volumes.push_back(collection->first()->getVolume());
            }
            delete collection;
        }
        catch (tgt::FileException& e) {
            LWARNING("Reading slice '" << sliceFiles[i] << "' failed: " << e.what());
        }
        catch (std::bad_alloc) {
            LWARNING("Reading slice '" << sliceFiles[i] << "' failed: bad allocation");
        }
    }

    if (volumes.empty()) {
        LWARNING("No slices");
        return 0;
    }

    LINFO("Constructing volume from " << volumes.size() << " slices of dimensions " << hints_.dimensions_.xy());

    Volume* result = 0;
    try {
        result = volumes.front()->clone();
        result->resize(ivec3(hints_.dimensions_.x, hints_.dimensions_.y, volumes.size()));
        result->clear();
        result->setTransformation(tgt::mat4::identity);
    }
    catch (std::bad_alloc) {
        LERROR("Reading slice stack failed: bad allocation");
    }

    // copy slices to output volume
    if (result) {
        size_t sliceDataSize = volumes.front()->getNumBytes();
        tgtAssert(result->getNumBytes() == sliceDataSize*volumes.size(), "Invalid slice sizes");

        for (size_t i=0; i<volumes.size(); i++) {

            if (volumes[i]->getNumBytes() != sliceDataSize) {
                LERROR("Slice '" << sliceFiles[i] << "' differs in size.");
                continue;
            }

            memcpy(((char*)result->getData())+i*sliceDataSize, (char*)volumes[i]->getData(), sliceDataSize);
        }
    }


    // free slices
    for (size_t i=0; i<volumes.size(); i++) {
        delete volumes[i];
    }

    if (result) {
        VolumeHandle* outputHandle = new VolumeHandle(result, hints_.timeStep_);

        // encode raw parameters into search string (currently only first slice)
        std::ostringstream searchStream;
        searchStream << "objectModel=" << hints_.objectModel_ << "&";
        searchStream << "format=" << hints_.format_ << "&";
        searchStream << "headerskip=" << hints_.headerskip_ << "&";
        if (hints_.bigEndianByteOrder_)
            searchStream << "bigEndian=" << hints_.bigEndianByteOrder_ << "&";
        searchStream << "dim_x=" << hints_.dimensions_.x << "&";
        searchStream << "dim_y=" << hints_.dimensions_.y << "&";
        searchStream << "dim_z=" << hints_.dimensions_.z << "&";
        searchStream << "spacing_x=" << hints_.spacing_.x << "&";
        searchStream << "spacing_y=" << hints_.spacing_.y << "&";
        searchStream << "spacing_z=" << hints_.spacing_.z << "&";

        outputHandle->setOrigin(VolumeOrigin("raw", sliceFiles.front(), searchStream.str()));

        return outputHandle;
    }
    else {
        return 0;
    }
}

VolumeReader* RawVolumeReader::create(IOProgress* progress) const {
    return new RawVolumeReader(progress);
}

}   // namespace voreen
