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
{}

void RawVolumeReader::readHints(ivec3 dimensions, vec3 spacing, int bitsStored,
                                std::string objectModel, std::string format,
                                int zeroPoint, tgt::mat4 transformation, Modality modality,
                                float timeStep, std::string metaString, std::string unit,
                                int offset)
{
    dimensions_ = dimensions;
    bitsStored_ = bitsStored;
    spacing_ = spacing;
    objectModel_ = objectModel;
    format_ = format;
    zeroPoint_ = zeroPoint;
    transformation_ = transformation;
    modality_ = modality;
    timeStep_ = timeStep;
    metaString_ = metaString;
    offset_ = offset;
    unit_ = unit;
}

VolumeSet* RawVolumeReader::read(const std::string &fileName)
    throw(tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    return readSlices(fileName,0,0);
}

VolumeSet* RawVolumeReader::readSlices(const std::string &fileName, size_t firstSlice, size_t lastSlice )
    throw(tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    //Remember the dimensions of the entire volume.
    tgt::ivec3 originalVolumeDimensions = dimensions_;

	//Check if we have to read only some slices instead of the whole volume.
	if ( ! (firstSlice==0 && lastSlice==0)) {
		if (lastSlice > firstSlice) {
			dimensions_.z = lastSlice - firstSlice;
		}
	}

	std::string info = "Loading raw file " + fileName + " ";
    LINFO("Loading file " << fileName);

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
            if (zeroPoint_ != 0)
                v->setZeroPoint(zeroPoint_);
            volume = v;

        }
        else if ((format_ == "USHORT" && bitsStored_ == 12) || format_ == "USHORT_12") {
            LINFO(info << "(12 bit dataset)");
            VolumeUInt16* v = new VolumeUInt16(dimensions_, spacing_, 12);
            if (zeroPoint_ != 0)
                v->setZeroPoint(zeroPoint_);
            volume = v;
        }
        else if (format_ == "USHORT") {
            LINFO(info << "(16 bit dataset)");
            VolumeUInt16* v = new VolumeUInt16(dimensions_, spacing_);
            if (zeroPoint_ != 0)
                v->setZeroPoint(zeroPoint_);
            volume = v;
        }
        else if (format_ == "FLOAT8" || format_ == "FLOAT16") {
            LINFO(info << "(32 bit float dataset, converting to 8 or 16 bit)");
            VolumeFloat* v = new VolumeFloat(dimensions_, spacing_);
            if (zeroPoint_ != 0)
                v->setZeroPoint(static_cast<float>(zeroPoint_));
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
            if (zeroPoint_ != 0)
                v->setZeroPoint(tgt::col4(zeroPoint_));
            volume = v;
        }
        else if (format_ == "USHORT") {
            LINFO(info << "(4x16 bit dataset)");
            Volume4xUInt16* v = new Volume4xUInt16(dimensions_, spacing_);
            if (zeroPoint_ != 0)
                v->setZeroPoint(tgt::Vector4<uint16_t>(zeroPoint_));
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
            if (zeroPoint_ != 0)
                v->setZeroPoint(tgt::col3(zeroPoint_));
            volume = v;
        }
        else if (format_ == "USHORT") {
            LINFO(info << "(3x16 bit dataset)");
            Volume3xUInt16* v = new Volume3xUInt16(dimensions_, spacing_);
            if (zeroPoint_ != 0)
                v->setZeroPoint(tgt::Vector3<uint16_t>(zeroPoint_));
            volume = v;
        }
        else {
            throw tgt::CorruptedFileException("Format '" + format_ + "' not supported for object model RGB", fileName);
        }
    }
    else if (objectModel_ == "LA") { // luminance alpha
        LINFO(info << "(luminance16 alpha16 dataset)");
        Volume4xUInt8* v = new Volume4xUInt8(dimensions_, spacing_);
        if (zeroPoint_ != 0)
            v->setZeroPoint(tgt::col4(zeroPoint_));
        volume = v;
    }
    else {
        throw tgt::CorruptedFileException("unsupported ObjectModel '" + objectModel_ + "'", fileName);
    }

	//If we only have to read slices, skip to the position we have to read the first
	//slice from 
	uint64_t skip = (uint64_t)dimensions_.x * (uint64_t)dimensions_.y * (uint64_t)firstSlice* 
        (uint64_t) (volume->getBitsAllocated() / 8);

    //Now add that to the offset we might have received
	offset_ = offset_ + skip;

    #ifdef UNIX 
        fseek(fin,offset_,SEEK_SET);
    #else
        _fseeki64(fin,offset_,SEEK_SET);
    #endif

	volume->clear();

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
    else if (format_ == "FLOAT16")
        conv = new VolumeUInt16(dimensions_, spacing_);

    if (conv) {
        conv->convert(volume);
        std::swap(conv, volume);
        delete conv;
    }

    volume->meta().setFileName(fileName);
    volume->meta().setTransformation(transformation_);
    volume->meta().setString(metaString_);
	volume->meta().setParentVolumeDimensions(originalVolumeDimensions);
    volume->meta().setUnit(unit_);

    VolumeSet* volumeSet = new VolumeSet(fileName);
    VolumeSeries* volumeSeries = new VolumeSeries(modality_.getName(), modality_);
    volumeSet->addSeries(volumeSeries);
    VolumeHandle* volumeHandle = new VolumeHandle(volume, 0.0f);
    volumeHandle->setOrigin(fileName, modality_.getName(), 0.0f);
    volumeSeries->addVolumeHandle(volumeHandle);

    return volumeSet;
}

VolumeSet* RawVolumeReader::readBrick(const std::string &fileName, tgt::ivec3 brickStartPos,int brickSize)
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
            if (zeroPoint_ != 0)
                v->setZeroPoint(zeroPoint_);
            volume = v;

        }
        else if ((format_ == "USHORT" && bitsStored_ == 12) || format_ == "USHORT_12") {
            //LINFO("Reading 12 bit dataset");
            VolumeUInt16* v = new VolumeUInt16(dimensions_, spacing_, 12);
            if (zeroPoint_ != 0)
                v->setZeroPoint(zeroPoint_);
            volume = v;
        }
        else if (format_ == "USHORT") {
            //LINFO("Reading 16 bit dataset");
            VolumeUInt16* v = new VolumeUInt16(dimensions_, spacing_);
            if (zeroPoint_ != 0)
                v->setZeroPoint(zeroPoint_);
            volume = v;
        }
        else if (format_ == "FLOAT8" || format_ == "FLOAT16") {
            //LINFO("Reading 32 bit float dataset, converting to 8 or 16 bit");
            VolumeFloat* v = new VolumeFloat(dimensions_, spacing_);
            if (zeroPoint_ != 0)
                v->setZeroPoint(static_cast<float>(zeroPoint_));
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
            if (zeroPoint_ != 0)
                v->setZeroPoint(tgt::col4(zeroPoint_));
            volume = v;
        }
        else if (format_ == "USHORT") {
            //LINFO("Reading 4x16 bit dataset");
            Volume4xUInt16* v = new Volume4xUInt16(dimensions_, spacing_);
            if (zeroPoint_ != 0)
                v->setZeroPoint(tgt::Vector4<uint16_t>(zeroPoint_));
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
            if (zeroPoint_ != 0)
                v->setZeroPoint(tgt::col3(zeroPoint_));
            volume = v;
        }
        else if (format_ == "USHORT") {
            //LINFO("Reading 3x16 bit dataset");
            Volume3xUInt16* v = new Volume3xUInt16(dimensions_, spacing_);
            if (zeroPoint_ != 0)
                v->setZeroPoint(tgt::Vector3<uint16_t>(zeroPoint_));
            volume = v;
        }
        else {
            throw tgt::CorruptedFileException("Format '" + format_ + "' not supported for object model RGB", fileName);
        }
    }
    else if (objectModel_ == "LA") { // luminance alpha
        //LINFO("Reading luminance16 alpha16 dataset");
        Volume4xUInt8* v = new Volume4xUInt8(dimensions_, spacing_);
        if (zeroPoint_ != 0)
            v->setZeroPoint(tgt::col4(zeroPoint_));
        volume = v;
    }
    else {
        throw tgt::CorruptedFileException("unsupported ObjectModel '" + objectModel_ + "'", fileName);
    }

    volume->clear();

    int voxelSize = (volume->getBitsAllocated() / 8);

    uint64_t initialSeekPos = (brickStartPos.z *(datasetDims.x*datasetDims.y)*brickSize*voxelSize )+
        (brickStartPos.y * datasetDims.x*brickSize*voxelSize) + (brickStartPos.x*brickSize*voxelSize);

    initialSeekPos = initialSeekPos + offset_;

    #ifdef UNIX 
        fseek(fin, initialSeekPos, SEEK_SET);
    #else
        _fseeki64(fin, initialSeekPos, SEEK_SET);
    #endif

        for (int i=0; i < brickSize; i++) {
            for (int j=0; j < brickSize; j++) {
                int volumePos = brickSize*j + (i*brickSize*brickSize);
                volumePos = volumePos*voxelSize;
                if (fread(reinterpret_cast<char*>(volume->getData()) + volumePos, 1, brickSize*voxelSize, fin) == 0)
                    LWARNING("fread() failed");                    
                #ifdef UNIX 
                    fseek(fin, (datasetDims.x - brickSize)*voxelSize, SEEK_CUR);
                #else
                    _fseeki64(fin, (datasetDims.x - brickSize)*voxelSize, SEEK_CUR);
                #endif
            }
            #ifdef UNIX 
                fseek(fin, (datasetDims.y - brickSize)*datasetDims.x*voxelSize, SEEK_CUR);
            #else
                _fseeki64(fin, (datasetDims.y - brickSize)*datasetDims.x*voxelSize, SEEK_CUR);
            #endif
        }

    fclose(fin);
   
    // convert if neccessary
    Volume* conv = 0;

    if (format_ == "FLOAT8")
        conv = new VolumeUInt8(dimensions_, spacing_);
    else if (format_ == "FLOAT16")
        conv = new VolumeUInt16(dimensions_, spacing_);

    if (conv) {
        conv->convert(volume);
        std::swap(conv, volume);
        delete conv;
    }

    volume->meta().setFileName(fileName);
    volume->meta().setTransformation(transformation_);
    volume->meta().setString(metaString_);
	volume->meta().setParentVolumeDimensions(datasetDims);
    volume->meta().setUnit(unit_);

    VolumeSet* volumeSet = new VolumeSet(tgt::FileSystem::fileName(fileName));
    VolumeSeries* volumeSeries = new VolumeSeries(modality_.getName(), modality_);
    volumeSet->addSeries(volumeSeries);
    VolumeHandle* volumeHandle = new VolumeHandle(volume, 0.0f);
    volumeHandle->setOrigin(fileName, modality_.getName(), 0.0f);
    volumeSeries->addVolumeHandle(volumeHandle);

    return volumeSet;
}

} // namespace voreen
