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

#include "voreen/core/io/brickedvolumewriter.h"
#include "voreen/core/volume/volumehandle.h"

namespace voreen {

const std::string BrickedVolumeWriter::loggerCat_("voreen.io.BrickedVolumeWriter");

BrickedVolumeWriter::BrickedVolumeWriter(BrickingInformation& brickingInformation)
    : brickingInformation_(brickingInformation)
{
    extensions_.push_back("bvi");
    bviout_ = 0;
    bvout_ = 0;
    bpiout_ = 0;

    bvPosition_ = 0;
    sizeOfUint64_ = sizeof(uint64_t);

    positionArray_ = new uint64_t[brickingInformation_.totalNumberOfBricksNeeded];
    allVoxelsEqualArray_ = new char[brickingInformation_.totalNumberOfBricksNeeded];
    errorArray_ = new float[brickingInformation_.totalNumberOfBricksNeeded *
        brickingInformation_.totalNumberOfResolutions];

    currentBrick_ = 0;
    errorArrayPosition_ = 0;

}

BrickedVolumeWriter::~BrickedVolumeWriter() {
    if (bvout_ != 0)
        delete bvout_;
    if (bviout_ != 0)
        delete bviout_;
    if (bpiout_ != 0)
        delete bpiout_;
}

void BrickedVolumeWriter::write(const std::string&, VolumeHandle* /*volumeHandle*/)
    throw (tgt::IOException)
{
}

bool BrickedVolumeWriter::openFile(std::string filename) {
    if (bviout_ != 0)
        delete bviout_;
    if (bvout_ != 0)
        delete bvout_;
    if (bpiout_ != 0)
        delete bpiout_;

    bviname_ = filename;
    bvname_ = getFileNameWithoutExtension(filename) + ".bv";
    bpiname_ = getFileNameWithoutExtension(filename) + ".bpi";

    bviout_ = new std::fstream(bviname_.c_str(), std::ios::out);
    bvout_ = new std::fstream(bvname_.c_str(), std::ios::out | std::ios::binary);
    bpiout_ = new std::fstream(bpiname_.c_str(), std::ios::out | std::ios::binary);

    if (bviout_->bad() || bvout_->bad() || bpiout_->bad() )
        throw tgt::IOException();

    return true;
}

void BrickedVolumeWriter::writeBviFile() {
    std::string format = brickingInformation_.originalVolumeFormat;
    std::string model = brickingInformation_.originalVolumeModel;

    tgt::ivec3 dimensions = brickingInformation_.originalVolumeDimensions;
    if (dimensions.x % brickingInformation_.brickSize != 0) {
       int factor = static_cast<int>(ceil( (float)dimensions.x / (float)brickingInformation_.brickSize));
       dimensions.x = factor * brickingInformation_.brickSize;
    }
    if (dimensions.y % brickingInformation_.brickSize != 0) {
       int factor = static_cast<int>(ceil( (float)dimensions.y / (float)brickingInformation_.brickSize));
       dimensions.y = factor * brickingInformation_.brickSize;
    }
    if (dimensions.z % brickingInformation_.brickSize != 0) {
       int factor = static_cast<int>(ceil( (float)dimensions.z / (float)brickingInformation_.brickSize));
       dimensions.z = factor * brickingInformation_.brickSize;
    }
    tgt::vec3 spacing = brickingInformation_.originalVolumeSpacing;
    tgt::vec3 llf = brickingInformation_.originalVolumeLLF;
    tgt::vec3 urb = brickingInformation_.originalVolumeURB;

    *bviout_ << "ObjectFileName:\t" << tgt::FileSystem::fileName(bvname_) << std::endl;
    *bviout_ << "Resolution:\t" << dimensions.x << " " << dimensions.y << " " << dimensions.z << std::endl;
    *bviout_ << "SliceThickness:\t" << spacing.x << " " << spacing.y << " " << spacing.z << std::endl;
    *bviout_ << "Format:\t\t" << format << std::endl;
    *bviout_ << "ObjectModel:\t" << model << std::endl;
    *bviout_ << "BitsStored:\t" << brickingInformation_.originalVolumeBitsStored << std::endl;
    *bviout_ << "BytesAllocated:\t" << brickingInformation_.originalVolumeBytesAllocated << std::endl;
    *bviout_ << "BrickSize:\t" << brickingInformation_.brickSize << std::endl;
    *bviout_ << "LLF:\t" << llf.x << " " << llf.y << " " << llf.z << std::endl;
    *bviout_ << "URB:\t" << urb.x << " " << urb.y << " " << urb.z << std::endl;
    *bviout_ << "EmptyBricks:\t" << brickingInformation_.numberOfBricksWithEmptyVolumes << std::endl;


    bpiout_->write(reinterpret_cast<char*>(positionArray_),
        brickingInformation_.totalNumberOfBricksNeeded*sizeof(uint64_t));

    bpiout_->write(allVoxelsEqualArray_,brickingInformation_.totalNumberOfBricksNeeded);

    bpiout_->write(reinterpret_cast<char*>(errorArray_),
        static_cast<size_t>(errorArrayPosition_)*sizeof(float));
}

void BrickedVolumeWriter::closeFile() {
    bviout_->close();
    bvout_->close();
    bpiout_->close();

    delete bviout_;
    delete bvout_;
    delete bpiout_;
    bvout_ = 0;
    bviout_ = 0;
    bpiout_ = 0;
}

void BrickedVolumeWriter::writeVolume(VolumeHandle* volumeHandle) {

    tgtAssert(volumeHandle, "No volume handle");
    Volume* volume = volumeHandle->getVolume();
    if (!volume) {
        LWARNING("No volume");
        return;
    }

    //char* position = reinterpret_cast<char*>(&bvPosition_);
    positionArray_[currentBrick_] = bvPosition_;
    //bpiout_->write(position,sizeOfUint64_);

    bool allVoxelsEqual = volume->getAllVoxelsEqual();

    char allEqual;
    if (allVoxelsEqual) {
        allEqual = '1';
        brickingInformation_.numberOfBricksWithEmptyVolumes++;
    }
    else
        allEqual = '0';

    allVoxelsEqualArray_[currentBrick_] = allEqual;

    currentBrick_++;
    //bpiout_->write(&allEqual,1);

    char* data = reinterpret_cast<char*>(volume->getData() );

    //If all voxels are equal just store the lowest level of detail by writing
    //the first voxel to the file.
    if (allVoxelsEqual) {
        int numbytes = volume->getBitsAllocated()/8;
        bvout_->write(data,numbytes );
        bvPosition_ += numbytes;
        return;
    }

    //If not all voxels are equal write the highest level of detail to file.
    size_t numBytes = volume->getNumBytes();
    size_t numVoxels = volume->getNumVoxels();
    bvout_->write(data,numBytes);
    bvPosition_ += numBytes;

    tgt::ivec3 newDims;
    Volume* scaledVolume;

    bool finished = false;
    std::vector<Volume*> volumeVector;
    Volume* temp = volume;

    //Create all levels of detail by downsampling the volume until only one
    //voxel remains. Put all downsampled volumes into the vector.
    while (!finished) {
        newDims = temp->getDimensions() / 2;
        //scaledVolume = temp->scale(newDims,Volume::LINEAR);
        scaledVolume = temp->downsample();
        volumeVector.push_back(scaledVolume);
        numVoxels = scaledVolume->getNumVoxels();
        temp = scaledVolume;
        if (numVoxels < 2) {
            finished=true;
        }
    }

    //The highest level of detail has obviously an error of 0, write that to file.
    //(This could be avoided, but to be consistent the error of 0 is written anyway).
    float error = 0.0f;
    /*char* errorOut = reinterpret_cast<char*>(&error);
    bpiout_->write(errorOut,4);*/
    errorArray_[errorArrayPosition_] = error;
    errorArrayPosition_++;

    //Now calculate the error for each level of detail and write it to file. Write the
    //actual volume data for every level of detail to file too.
    for (size_t i=0; i<volumeVector.size(); i++) {
        Volume* currentVolume = volumeVector.at(i);
        error = volume->calcError(currentVolume);
        /*errorOut = reinterpret_cast<char*>(&error);
        bpiout_->write(errorOut,4);*/
        errorArray_[errorArrayPosition_] = error;
        errorArrayPosition_++;

        data = reinterpret_cast<char*>(currentVolume->getData());
        numBytes = currentVolume->getNumBytes();
        bvout_->write(data,numBytes);
        bvPosition_ += numBytes;

        delete currentVolume;
    }


}


} // namespace voreen
