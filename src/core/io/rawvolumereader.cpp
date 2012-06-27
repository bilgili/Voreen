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

#include "voreen/core/io/rawvolumereader.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>

#include "tgt/exception.h"
#include "tgt/filesystem.h"

#include "voreen/core/io/ioprogress.h"
#include "voreen/core/volume/volumeatomic.h"

using tgt::ivec3;
using tgt::vec3;

namespace voreen {

const std::string RawVolumeReader::loggerCat_ = "voreen.io.VolumeReader.raw";

RawVolumeReader::RawVolumeReader(IOProgress* progress)
    : VolumeReader(progress),
      dimensions_(),
      spacing_()
{
    protocols_.push_back("raw");
}

void RawVolumeReader::readHints(ivec3 dimensions, vec3 spacing, int bitsStored,
                                const std::string& objectModel, const std::string& format,
                                int headerskip, tgt::mat4 transformation, Modality modality,
                                float timeStep, const std::string& metaString, const std::string& unit,
                                const std::string& sliceOrder)
{
    dimensions_ = dimensions;
    bitsStored_ = bitsStored;
    spacing_ = spacing;
    objectModel_ = objectModel;
    format_ = format;
    headerskip_ = headerskip;
    transformation_ = transformation;
    modality_ = modality;
    timeStep_ = timeStep;
    metaString_ = metaString;
    unit_ = unit;
    sliceOrder_ = sliceOrder;
}

VolumeCollection* RawVolumeReader::read(const std::string &fileName)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    return readSlices(fileName,0,0);
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

    // pass read hints and load volume
    readHints(static_cast<ivec3>(dimensions), static_cast<vec3>(spacing), 0, objectModel, format, headerskip);
    VolumeHandle* handle = 0;
    VolumeCollection* collection = read(filename);
    if (!collection->empty())
        handle = collection->first();

    delete collection;

    return handle;
}

VolumeCollection* RawVolumeReader::readSlices(const std::string &fileName, size_t firstSlice, size_t lastSlice )
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    //Remember the dimensions of the entire volume.
    tgt::ivec3 originalVolumeDimensions = dimensions_;

    // check if we have to read only some slices instead of the whole volume.
    if ( ! (firstSlice==0 && lastSlice==0)) {
        if (lastSlice > firstSlice) {
            dimensions_.z = lastSlice - firstSlice;
        }
    }

    std::string info = "Loading raw file " + fileName + " ";

    if (dimensions_ == tgt::ivec3::zero) {
        throw tgt::CorruptedFileException("No readHints set.", fileName);
    }

    FILE* fin;
    fin = fopen(fileName.c_str(),"rb");

    if (fin==NULL) {
        throw tgt::IOException("Unable to open raw file for reading", fileName);
    }

    Volume* volume;

    if (objectModel_ == "I") {
        if (format_ == "UCHAR") {
            LINFO(info << "(8 bit dataset)");
            VolumeUInt8* v = new VolumeUInt8(dimensions_, spacing_);
            volume = v;

        }
        else if ((format_ == "USHORT" && bitsStored_ == 12) || format_ == "USHORT_12") {
            LINFO(info << "(12 bit dataset)");
            VolumeUInt16* v = new VolumeUInt16(dimensions_, spacing_, 12);
            volume = v;
        }
        else if (format_ == "USHORT") {
            LINFO(info << "(16 bit dataset)");
            VolumeUInt16* v = new VolumeUInt16(dimensions_, spacing_);
            volume = v;
        }
        else if (format_ == "FLOAT8" || format_ == "FLOAT16" || format_ == "FLOAT") {
            LINFO(info << "(32 bit float dataset, converting to 8 or 16 bit)");
            VolumeFloat* v = new VolumeFloat(dimensions_, spacing_);
            volume = v;
        }
        else {
            throw tgt::CorruptedFileException("Format '" + format_ + "' not supported", fileName);
        }
    }
    else if (objectModel_ == "RGBA") {
        if (format_ == "UCHAR") {
            LINFO(info << "(4x8 bit dataset)");
            Volume4xUInt8* v = new Volume4xUInt8(dimensions_, spacing_);
            volume = v;
        }
        else if (format_ == "USHORT") {
            LINFO(info << "(4x16 bit dataset)");
            Volume4xUInt16* v = new Volume4xUInt16(dimensions_, spacing_);
            volume = v;
        }
        else {
            throw tgt::CorruptedFileException("Format '" + format_ + "' not supported for object model RGBA", fileName);
        }
    }
    else if (objectModel_ == "RGB") {
        if (format_ == "UCHAR") {
            LINFO(info << "(3x8 bit dataset)");
            Volume3xUInt8* v = new Volume3xUInt8(dimensions_, spacing_);
            volume = v;
        }
        else if (format_ == "USHORT") {
            LINFO(info << "(3x16 bit dataset)");
            Volume3xUInt16* v = new Volume3xUInt16(dimensions_, spacing_);
            volume = v;
        } else if (format_ == "FLOAT") {
            LINFO(info << "(3x32 bit dataset)");
            Volume3xFloat* v = new Volume3xFloat(dimensions_, spacing_);
            volume = v;
        } else {
            throw tgt::CorruptedFileException("Format '" + format_ + "' not supported for object model RGB", fileName);
        }
    }
    else if (objectModel_ == "LA") { // luminance alpha
        LINFO(info << "(luminance16 alpha16 dataset)");
        Volume4xUInt8* v = new Volume4xUInt8(dimensions_, spacing_);
        volume = v;
    }
    else {
        throw tgt::CorruptedFileException("unsupported ObjectModel '" + objectModel_ + "'", fileName);
    }

    //If we only have to read slices, skip to the position we have to read the first
    //slice from
    uint64_t skip = (uint64_t)dimensions_.x * (uint64_t)dimensions_.y * (uint64_t)firstSlice*
        (uint64_t) (volume->getBitsAllocated() / 8);

    // now add that to the headerskip we might have received
    uint64_t offset = headerskip_ + skip;

    #ifdef _MSC_VER
        _fseeki64(fin,offset,SEEK_SET);
    #else
        fseek(fin,offset,SEEK_SET);
    #endif

    volume->clear();

    if (getProgress()) {
        getProgress()->setTitle("Loading volume");
        getProgress()->setMessage("Loading volume: " + tgt::FileSystem::fileName(fileName));
    }
    VolumeReader::read(volume, fin);

    if (lastSlice == 0) {
        if (feof(fin) ) {
               delete volume;
            // throw exception
            throw tgt::CorruptedFileException("unexpected EOF: raw file truncated or ObjectModel '" +
                objectModel_ + "' invalid", fileName);
        }
    }

    fclose(fin);

    // convert if neccessary
    Volume* conv = 0;

    if (format_ == "FLOAT8")
        conv = new VolumeUInt8(dimensions_, spacing_);
    else if ((format_ == "FLOAT16") || ((format_ == "FLOAT") && (objectModel_ != "RGB")))
        conv = new VolumeUInt16(dimensions_, spacing_);

    if (conv) {
        conv->convert(volume);
        std::swap(conv, volume);
        delete conv;
    }

    if (sliceOrder_ == "-x") {
        LINFO("slice order is -x, reversing order to +x...\n");
        reverseXSliceOrder(volume);
    } else if (sliceOrder_ == "-y") {
        LINFO("slice order is -y, reversing order to +y...\n");
        reverseYSliceOrder(volume);
    } else if (sliceOrder_ == "-z") {
        LINFO("slice order is -z, reversing order to +z...\n");
        reverseZSliceOrder(volume);
    }

    volume->setTransformation(transformation_);
    volume->meta().setString(metaString_);
    volume->meta().setUnit(unit_);
    volume->meta().setParentVolumeDimensions(originalVolumeDimensions);

    VolumeCollection* volumeCollection = new VolumeCollection();
    VolumeHandle* volumeHandle = new VolumeHandle(volume, 0.0f);
    volumeHandle->setModality(modality_);

    // encode raw parameters into search string
    std::ostringstream searchStream;
    searchStream << "objectModel=" << objectModel_ << "&";
    searchStream << "format=" << format_ << "&";
    searchStream << "dim_x=" << dimensions_.x << "&";
    searchStream << "dim_y=" << dimensions_.y << "&";
    searchStream << "dim_z=" << dimensions_.z << "&";
    searchStream << "spacing_x=" << spacing_.x << "&";
    searchStream << "spacing_y=" << spacing_.y << "&";
    searchStream << "spacing_z=" << spacing_.z << "&";
    searchStream << "headerskip=" << headerskip_;
    volumeHandle->setOrigin(VolumeOrigin("raw", fileName, searchStream.str()));

    volumeCollection->add(volumeHandle);

    return volumeCollection;
}

VolumeCollection* RawVolumeReader::readBrick(const std::string &fileName, tgt::ivec3 brickStartPos,int brickSize)
    throw(tgt::FileException, std::bad_alloc)
{
    tgt::ivec3 datasetDims = dimensions_;
    dimensions_ = tgt::ivec3(brickSize);

    if (dimensions_ == tgt::ivec3::zero) {
        throw tgt::CorruptedFileException("No readHints set.", fileName);
    }

    FILE* fin;
    fin = fopen(fileName.c_str(),"rb");

    if (fin==NULL) {
        throw tgt::IOException("Unable to open raw file for reading", fileName);
    }

    Volume* volume;

    if (objectModel_ == "I") {
        if (format_ == "UCHAR") {
            //LINFO("Reading 8 bit dataset");
            VolumeUInt8* v = new VolumeUInt8(dimensions_, spacing_);
            volume = v;

        }
        else if ((format_ == "USHORT" && bitsStored_ == 12) || format_ == "USHORT_12") {
            //LINFO("Reading 12 bit dataset");
            VolumeUInt16* v = new VolumeUInt16(dimensions_, spacing_, 12);
            volume = v;
        }
        else if (format_ == "USHORT") {
            //LINFO("Reading 16 bit dataset");
            VolumeUInt16* v = new VolumeUInt16(dimensions_, spacing_);
            volume = v;
        }
        else if (format_ == "FLOAT8" || format_ == "FLOAT16" || format_ == "FLOAT") {
            //LINFO("Reading 32 bit float dataset, converting to 8 or 16 bit");
            VolumeFloat* v = new VolumeFloat(dimensions_, spacing_);
            volume = v;
        }
        else {
            throw tgt::CorruptedFileException("Format '" + format_ + "' not supported", fileName);
        }
    }
    else if (objectModel_ == "RGBA") {
        if (format_ == "UCHAR") {
            //LINFO("Reading 4x8 bit dataset");
            Volume4xUInt8* v = new Volume4xUInt8(dimensions_, spacing_);
            volume = v;
        }
        else if (format_ == "USHORT") {
            //LINFO("Reading 4x16 bit dataset");
            Volume4xUInt16* v = new Volume4xUInt16(dimensions_, spacing_);
            volume = v;
        }
        else {
            throw tgt::CorruptedFileException("Format '" + format_ + "' not supported for object model RGBA", fileName);
        }
    }
    else if (objectModel_ == "RGB") {
        if (format_ == "UCHAR") {
            //LINFO("Reading 3x8 bit dataset");
            Volume3xUInt8* v = new Volume3xUInt8(dimensions_, spacing_);
            volume = v;
        }
        else if (format_ == "USHORT") {
            //LINFO("Reading 3x16 bit dataset");
            Volume3xUInt16* v = new Volume3xUInt16(dimensions_, spacing_);
            volume = v;
        } else if (format_ == "FLOAT") {
            Volume3xFloat* v = new Volume3xFloat(dimensions_, spacing_);
            volume = v;
        } else {
            throw tgt::CorruptedFileException("Format '" + format_ + "' not supported for object model RGB", fileName);
        }
    }
    else if (objectModel_ == "LA") { // luminance alpha
        //LINFO("Reading luminance16 alpha16 dataset");
        Volume4xUInt8* v = new Volume4xUInt8(dimensions_, spacing_);
        volume = v;
    }
    else {
        throw tgt::CorruptedFileException("unsupported ObjectModel '" + objectModel_ + "'", fileName);
    }

    volume->clear();

    int voxelSize = (volume->getBitsAllocated() / 8);

    uint64_t initialSeekPos = (brickStartPos.z *(datasetDims.x*datasetDims.y)*brickSize*voxelSize )+
        (brickStartPos.y * datasetDims.x*brickSize*voxelSize) + (brickStartPos.x*brickSize*voxelSize);

    // add header skip
    initialSeekPos += headerskip_;

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

    if (format_ == "FLOAT8")
        conv = new VolumeUInt8(dimensions_, spacing_);
    else if ((format_ == "FLOAT16") || ((format_ == "FLOAT") && (objectModel_ != "RGB")))
        conv = new VolumeUInt16(dimensions_, spacing_);

    if (conv) {
        conv->convert(volume);
        std::swap(conv, volume);
        delete conv;
    }

    volume->setTransformation(transformation_);
    volume->meta().setString(metaString_);
    volume->meta().setUnit(unit_);
    volume->meta().setParentVolumeDimensions(datasetDims);

    VolumeCollection* volumeCollection = new VolumeCollection();
    VolumeHandle* volumeHandle = new VolumeHandle(volume, 0.0f);
    volumeHandle->setModality(modality_);

    // encode raw parameters into search string
    std::ostringstream searchStream;
    searchStream << "objectModel=" << objectModel_ << "&";
    searchStream << "format=" << format_ << "&";
    searchStream << "dim_x=" << dimensions_.x << "&";
    searchStream << "dim_y=" << dimensions_.y << "&";
    searchStream << "dim_z=" << dimensions_.z << "&";
    searchStream << "spacing_x=" << spacing_.x << "&";
    searchStream << "spacing_y=" << spacing_.y << "&";
    searchStream << "spacing_z=" << spacing_.z << "&";
    searchStream << "headerskip=" << headerskip_;
    volumeHandle->setOrigin(VolumeOrigin("raw", fileName, searchStream.str()));

    volumeCollection->add(volumeHandle);

    return volumeCollection;
}

}   // namespace voreen
