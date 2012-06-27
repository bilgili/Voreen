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

#include "voreen/core/io/brickedvolumereader.h"
#include "voreen/core/volume/bricking/brickingmanager.h"

#include <fstream>
#include <iostream>
#include <stdio.h>

#include "tgt/exception.h"
#include "tgt/vector.h"

#include "voreen/core/io/textfilereader.h"
#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/io/ioprogress.h"

using tgt::vec3;
using tgt::ivec3;
using tgt::hor;
using tgt::lessThanEqual;

namespace voreen {

const std::string BrickedVolumeReader::loggerCat_ = "voreen.io.BrickedReader.bvi";

BrickedVolumeReader::BrickedVolumeReader(voreen::IOProgress* progress) 
	:   VolumeReader(progress) {

        if (progress != 0){
            BrickedVolumeReader::ioProgress_ = progress;   
        }
		name_ = "Bricked Reader";
        extensions_.push_back("bvi");
		bpiStream_=0;
		bvFile_=0;

        currentBrick_ = 0;
        errorArrayPosition_ = 0;
}

BrickingInformation BrickedVolumeReader::getBrickingInformation() {
	return brickingInformation_;
}

bool BrickedVolumeReader::openFile(std::string filename) {

	std::string objectFilename;
    ivec3 dimensions = ivec3(0, 0, 0);
    vec3 spacing = vec3(1.f, 1.f, 1.f);
	vec3 llf = vec3(0.0f);
	vec3 urb = vec3(0.0f);
    std::string format;
    std::string objectModel;
	int bitsStored = 0;
	int brickSize = 0;
	
	bool error = false;

	TextFileReader reader(filename);

    if (!reader)
        throw tgt::IOException();

    std::string type;
    std::istringstream args;
    

    while (reader.getNextLine(type, args, false)) {

        if (type == "ObjectFileName:") {
            args >> objectFilename;

        } else if (type == "Resolution:") {
            args >> dimensions[0];
            args >> dimensions[1];
            args >> dimensions[2];

        } else if (type == "SliceThickness:") {
            args >> spacing[0] >> spacing[1] >> spacing[2];
        } else if (type == "Format:") {
            args >> format;
        } else if (type == "ObjectModel:") {
            args >> objectModel;
        } else if (type == "BitsStored:") {
            args >> bitsStored;
		} else if (type == "BrickSize:") {
			args >> brickSize;
		} else if (type == "LLF:") {
			args >> llf[0] >> llf[1] >> llf[2];
		} else if (type == "URB:") {
			args >> urb[0] >> urb[1] >> urb[2];
		} else if (type == "EmptyBricks:") {
			args >> brickingInformation_.numberOfBricksWithEmptyVolumes;
		} else if (type == "BytesAllocated:") {
			args >> brickingInformation_.originalVolumeBytesAllocated;
		}

        if (args.fail()) {
            LERROR("Format error");
			error = true;
        }
	}

	if (objectFilename == "") {
        LERROR("No bv file specified");
        error = true;
    }

	if (brickSize == 0) {
        LERROR("No brick size specified");
        error = true;
    }

    if ( hor(lessThanEqual(dimensions,ivec3(0,0,0))) ) {
        LERROR("Invalid resolution or resolution not specified: " << dimensions[0] << " x " <<
                  dimensions[1] << " x " << dimensions[2]);
        error = true;
    }

	if (!error) {

		//update the BrickingInformation
		brickingInformation_.originalVolumeName = objectFilename;
		brickingInformation_.originalVolumeDimensions = dimensions;
		brickingInformation_.originalVolumeNumVoxels = (uint64_t)dimensions.x * (uint64_t)dimensions.y * 
														(uint64_t)dimensions.z;
		brickingInformation_.originalVolumeSpacing = spacing;
		brickingInformation_.originalVolumeFormat = format;
		brickingInformation_.originalVolumeModel = objectModel;
		brickingInformation_.originalVolumeBitsStored = bitsStored;
		brickingInformation_.brickSize = brickSize;
		brickingInformation_.originalVolumeLLF = llf;
		brickingInformation_.originalVolumeURB = urb;
		brickingInformation_.numBricks.x = static_cast<int>( ceil((float)dimensions.x / 
											(float)brickingInformation_.brickSize));
		brickingInformation_.numBricks.y = static_cast<int>( ceil((float)dimensions.y / 
											(float)brickingInformation_.brickSize));
		brickingInformation_.numBricks.z = static_cast<int>( ceil((float)dimensions.z / 
											(float)brickingInformation_.brickSize));

        brickingInformation_.totalNumberOfBricksNeeded = brickingInformation_.numBricks.x *
            brickingInformation_.numBricks.y * brickingInformation_.numBricks.z;

        brickingInformation_.totalNumberOfResolutions = static_cast<int> ( ( log( 
            (float)brickSize) / log (2.0) ) + 1);

		// do we have a relative path?
        if ((objectFilename.substr(0,1) != "/")  && (objectFilename.substr(0,1) != "\\") &&
			(objectFilename.substr(1,2) != ":/") && (objectFilename.substr(1,2) != ":\\")) {
            size_t p = filename.find_last_of("\\");
            if (p == std::string::npos)
                p = filename.find_last_of("/");

            // construct path relative to bvi file
            objectFilename = filename.substr(0, p + 1) + objectFilename;
        }

		bvFile_ = fopen (objectFilename.c_str(),"rb");

		std::string bpiFileName = objectFilename.substr(0, objectFilename.rfind(".")) + ".bpi";
		bpiStream_ = new std::fstream(bpiFileName.c_str(), std::ios::in | std::ios::binary);

        int numberOfChars = brickingInformation_.totalNumberOfBricksNeeded * sizeof(uint64_t);
        char* temp = new char[numberOfChars];
        bpiStream_->read(temp,numberOfChars);
        positionArray_ = reinterpret_cast<uint64_t*>(temp);
        

        allVoxelsEqualArray_ = new char[brickingInformation_.totalNumberOfBricksNeeded];
        bpiStream_->read(allVoxelsEqualArray_,brickingInformation_.totalNumberOfBricksNeeded);

        numberOfChars = (brickingInformation_.totalNumberOfBricksNeeded - 
            brickingInformation_.numberOfBricksWithEmptyVolumes) * 
            brickingInformation_.totalNumberOfResolutions * sizeof(float);
        char* temp2 = new char[numberOfChars];

        bpiStream_->read(temp2,numberOfChars);
        errorArray_ = reinterpret_cast<float*>(temp2);

        return true;
	}
    return false;

	
}

void BrickedVolumeReader::resetBpiFilePosition() {
	bpiStream_->seekg(0);
}


void BrickedVolumeReader::closeFile() {

    if (bpiStream_ != 0) {
        bpiStream_->close();
        delete bpiStream_;
        bpiStream_ =0;
    }

    if (bvFile_ != 0) {
        fclose(bvFile_);
        delete bvFile_;
        bvFile_ = 0;
    }

    delete errorArray_;
    delete positionArray_;
    delete allVoxelsEqualArray_;
}

void BrickedVolumeReader::readBrickPosition(Brick* brick) {

    uint64_t pos = positionArray_[currentBrick_];
    brick->setBvFilePosition(pos);
    char allVoxelsEqual = allVoxelsEqualArray_[currentBrick_];
    if (allVoxelsEqual == '1') {
        brick->setAllVoxelsEqual(true);
    } else if (allVoxelsEqual == '0') {
        brick->setAllVoxelsEqual(false);
    } 
    currentBrick_++;
    if (!brick->getAllVoxelsEqual()) {
        for (int i=0; i<brickingInformation_.totalNumberOfResolutions; i++) {
            brick->addError(errorArray_[errorArrayPosition_]);
            errorArrayPosition_++;
        }
    }
	
}


void BrickedVolumeReader::readBrick(Brick *brick, char* volumeData, int numBytes, size_t lod) {

	uint64_t positionInFile = brick->getBvFilePosition();

	if (!brick->getAllVoxelsEqual() ) {
		for (size_t i=0; i<lod; i++) {
			size_t increase = brickingInformation_.numVoxelsInBrick / static_cast<int>(pow(8.0f,(float)i) ) * 
				brickingInformation_.originalVolumeBytesAllocated;
			positionInFile += increase;
		}
	}

    #ifdef UNIX 
        fseek(bvFile_,positionInFile,SEEK_SET);
    #else
        _fseeki64(bvFile_,positionInFile,SEEK_SET);
    #endif

    if (fread(volumeData, 1, numBytes, bvFile_) <= 0)
        return;
}



VolumeSet* BrickedVolumeReader::read(const std::string & filename)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{    
    return readSlices(filename,0,0);
}

VolumeSet* BrickedVolumeReader::readSlices(const std::string & filename, size_t , size_t)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc) {

	openFile(filename);
	LargeVolumeManager* largeVolumeManager = 0;

	std::string model = brickingInformation_.originalVolumeModel;
	std::string format = brickingInformation_.originalVolumeFormat;

    VolumeSet* volumeSet = new VolumeSet(tgt::FileSystem::fileName(filename));
	VolumeSeries* brickedSeries = new VolumeSeries("Bricked Volume",Modality::MODALITY_BRICKED_VOLUME);
	VolumeHandle* brickedHandle = new VolumeHandle(0,0);
	volumeSet->addSeries(brickedSeries);
    brickedSeries->addVolumeHandle(brickedHandle);

	if (model == "I") {
		if (format == "UCHAR") {
			largeVolumeManager = new BrickingManager<uint8_t>(brickedHandle, this, brickingInformation_,
                ioProgress_);
		} else if (format == "USHORT") {
			largeVolumeManager = new BrickingManager<uint16_t>(brickedHandle, this, brickingInformation_,
                ioProgress_);
		} else if (format == "FLOAT") {
			largeVolumeManager = new BrickingManager<float>(brickedHandle,this, brickingInformation_,
                ioProgress_);
		}
	} else if (model == "RGBA") {
		if (format == "UCHAR") {
			largeVolumeManager = new BrickingManager<tgt::col4>(brickedHandle, this, brickingInformation_,
                ioProgress_);
		} else if (format == "USHORT") {
			largeVolumeManager = new BrickingManager<tgt::Vector4<uint16_t> >(
				brickedHandle, this, brickingInformation_, ioProgress_);
		}
	} else if (model == "RGB") {
		if (format == "UCHAR") {
			largeVolumeManager = new BrickingManager<tgt::col3>(brickedHandle, this, brickingInformation_,
                ioProgress_);
		} else if (format == "USHORT") {
			largeVolumeManager = new BrickingManager<tgt::Vector3<uint16_t> >(
				brickedHandle, this, brickingInformation_, ioProgress_);
		}
	}
		
    brickedHandle->setLargeVolumeManager(largeVolumeManager);
	return volumeSet;
}

IOProgress* BrickedVolumeReader::ioProgress_ = 0;

} // namespace voreen
