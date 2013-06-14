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

#include "octreebrickpoolmanagerdisk.h"

#include "octreeutils.h"

#include "voreen/core/utils/stringutils.h"
#include "voreen/core/io/serialization/serialization.h"

#include <time.h>
#include <fstream>
#include <new>

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/tgt_math.h"
#include "tgt/filesystem.h"
#include "tgt/vector.h"

namespace voreen {

OctreeBrickPoolManagerDisk::OctreeBrickPoolManagerDisk(const size_t maxBufferSize, const bool brickPoolExists,
            const std::string& brickPoolPath, const std::string& bufferFilePrefix, const fileIOStrategy fileIO)
    : maxBufferSizeBytes_(maxBufferSize)
    , brickPoolPath_(brickPoolPath)
    , bufferFilePrefix_(bufferFilePrefix)
    , brickPoolExists_(brickPoolExists)
    , fileIOStrategy_(fileIO)
{
    brickPoolPath_ = tgt::FileSystem::absolutePath(brickPoolPath);
    bufferFilePrefix_ = (!bufferFilePrefix.empty() ? bufferFilePrefix : "brickbuffer");
}

OctreeBrickPoolManagerDisk::~OctreeBrickPoolManagerDisk() {
}

size_t OctreeBrickPoolManagerDisk::getNumBrickBuffers() const {
    return bufferFiles_.size();
}

std::vector<std::string> OctreeBrickPoolManagerDisk::getBufferFiles() const {
    return bufferFiles_;
}

void OctreeBrickPoolManagerDisk::initialize(size_t brickMemorySizeInByte) throw (VoreenException)
{
    OctreeBrickPoolManagerBase::initialize(brickMemorySizeInByte);

    // round max buffer size down to next multiple of brick memory size
    if (maxBufferSizeBytes_ < getBrickMemorySizeInByte())
        throw VoreenException("Max brick buffer size is smaller than the memory size of a single brick "
                              "[" + itos(maxBufferSizeBytes_) + " bytes < " + itos(getBrickMemorySizeInByte()) + " bytes]");
    singleBufferSizeBytes_ = maxBufferSizeBytes_;
    if (!isMultipleOf(singleBufferSizeBytes_, getBrickMemorySizeInByte()))
        singleBufferSizeBytes_ = tgt::ifloor((float)maxBufferSizeBytes_ / (float)getBrickMemorySizeInByte()) * getBrickMemorySizeInByte();

    // initialize/check brick pool
    if (brickPoolExists_) {
        if (!tgt::FileSystem::dirExists(brickPoolPath_))
            throw VoreenException("Brick pool path does not exist: " + brickPoolPath_);

        // collect brick buffer files from pool path
        std::vector<std::string> files = tgt::FileSystem::listFiles(brickPoolPath_, true);
        for (size_t i=0; i<files.size(); i++) {
            if (startsWith(files.at(i), bufferFilePrefix_))
                bufferFiles_.push_back(files.at(i));
        }
        if (bufferFiles_.empty()) {
            LWARNING("No files with prefix '" + bufferFilePrefix_ + "' found at brick pool path: " + brickPoolPath_);
        }
    }
}

void OctreeBrickPoolManagerDisk::deinitialize() throw (VoreenException) {
    bufferFiles_.clear();
    OctreeBrickPoolManagerBase::deinitialize();
}

char* OctreeBrickPoolManagerDisk::allocateNextDiskBuffer() throw (VoreenException) {
    char* buffer = 0;
    try{
        buffer = new char[getBrickBufferSizeInBytes()];
    }catch(std::bad_alloc& e) {
        throw VoreenException("Out of Memory!: " + std::string(e.what()));
    }

    std::stringstream path;
    path << brickPoolPath_ << "/" << bufferFilePrefix_ << "_" << itos(getNumBrickBuffers(),10);

    switch(fileIOStrategy_) {
    case USE_CPP_FUNCTIONS: {
        std::ofstream outfile(path.str().c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
        if(outfile.fail())
            throw VoreenException("Could not open: " + path.str());
        outfile.write(buffer,getBrickBufferSizeInBytes());
        outfile.close();
        } break;
    case USE_C_FUNCTIONS: {
        FILE* f = fopen(path.str().c_str(),"wb");
        setbuf(f,NULL);
        fwrite(buffer,1,getBrickBufferSizeInBytes(),f);
        fflush(f);
        fclose(f);
    } break;
    default:
        tgtAssert(false, "Unknown file IO strategy!!!");
        break;
    }

    bufferFiles_.push_back(path.str());
    return buffer;
}

void OctreeBrickPoolManagerDisk::saveBufferToDisk(size_t bufferIndex, const char* buffer) const throw (VoreenException) {
    tgtAssert(bufferIndex < getNumBrickBuffers(), "invalid buffer index");
    tgtAssert(buffer, "null pointer passed");

    switch(fileIOStrategy_) {
    case USE_CPP_FUNCTIONS: {
        std::ofstream outfile(bufferFiles_[bufferIndex].c_str(),std::ios::out | std::ios::binary | std::ios::trunc);
        if(outfile.fail())
            throw VoreenException("Could not open: " + bufferFiles_[bufferIndex]);
        outfile.write(buffer,getBrickBufferSizeInBytes());
        outfile.close();
        }break;
    case USE_C_FUNCTIONS: {
        FILE* f = fopen(bufferFiles_[bufferIndex].c_str(),"wb");
        setbuf(f,NULL);
        fwrite(buffer,1,getBrickBufferSizeInBytes(),f);
        fflush(f);
        fclose(f);
    } break;
    default:
        tgtAssert(false, "Unknown file IO strategy!!!");
        break;
    }

}

char* OctreeBrickPoolManagerDisk::loadDiskBuffer(size_t bufferIndex) const throw (VoreenException) {
    tgtAssert(bufferIndex < getNumBrickBuffers(), "invalid buffer index");
    tgtAssert(getBufferFiles().size() == getNumBrickBuffers(), "number of buffer files does not match number of numbers");

    const std::string bufferFile = getBufferFiles().at(bufferIndex);
    tgtAssert(!bufferFile.empty(), "buffer file path is empty");
    if (!tgt::FileSystem::fileExists(bufferFile))
        throw VoreenException("Buffer file does not exist: " + bufferFile);

    char* buffer = 0;
    try {
        buffer = new char[getBrickBufferSizeInBytes()];
    } catch(std::bad_alloc& e) {
        throw VoreenException("Out of memory!:" + std::string(e.what()));
    }

    switch(fileIOStrategy_) {
    case USE_CPP_FUNCTIONS: {
        std::ifstream infile(bufferFile.c_str(), std::ios::in | std::ios::binary);
        if(infile.fail())
            throw VoreenException("Could not open: " + bufferFile);
        infile.read(buffer,getBrickBufferSizeInBytes());
        infile.close();
    } break;
    case USE_C_FUNCTIONS: {
        FILE* f = fopen(bufferFile.c_str(),"rb");
        setbuf(f,NULL);
        fread(buffer,1,getBrickBufferSizeInBytes(),f);
        fclose(f);
    } break;
    default:
        tgtAssert(false, "Unknown file IO strategy!!!");
        break;
    }

    return buffer;
}

size_t OctreeBrickPoolManagerDisk::getBrickBufferSizeInBytes() const {
    return singleBufferSizeBytes_;
}

uint64_t OctreeBrickPoolManagerDisk::getBrickPoolMemoryAllocated() const {
    return 0; //< TODO
}

uint64_t OctreeBrickPoolManagerDisk::getBrickPoolMemoryUsed() const {
    return 0; //< TODO
}

void OctreeBrickPoolManagerDisk::serialize(XmlSerializer& s) const {
    OctreeBrickPoolManagerBase::serialize(s);

    s.serialize("maxSingleBufferSizeBytes", maxBufferSizeBytes_);
    s.serialize("singleBufferSizeBytes",    singleBufferSizeBytes_);

    s.serialize("bufferFiles", bufferFiles_);
    s.serialize("brickPoolExists", brickPoolExists_);
    s.serialize("brickPoolPath", brickPoolPath_);
    s.serialize("bufferFilePrefix", bufferFilePrefix_);
}

void OctreeBrickPoolManagerDisk::deserialize(XmlDeserializer& s) {
    OctreeBrickPoolManagerBase::deserialize(s);

    s.deserialize("maxSingleBufferSizeBytes", maxBufferSizeBytes_);
    s.deserialize("singleBufferSizeBytes",    singleBufferSizeBytes_);

    s.deserialize("bufferFiles", bufferFiles_);
    s.deserialize("brickPoolExists", brickPoolExists_);
    s.deserialize("brickPoolPath", brickPoolPath_);
    s.deserialize("bufferFilePrefix", bufferFilePrefix_);

    // initialize/check brick pool
    if (brickPoolExists_) {
        if (!tgt::FileSystem::dirExists(brickPoolPath_))
            throw VoreenException("Brick pool path does not exist: " + brickPoolPath_);

        // collect brick buffer files from pool path
        std::vector<std::string> files = tgt::FileSystem::listFiles(brickPoolPath_, true);
        for (size_t i=0; i<files.size(); i++) {
            if (startsWith(files.at(i), bufferFilePrefix_))
                bufferFiles_.push_back(files.at(i));
        }
        if (bufferFiles_.empty()) {
            LWARNING("No files with prefix '" + bufferFilePrefix_ + "' found at brick pool path: " + brickPoolPath_);
        }
    }
}


//-------------------------------------------------------------------------------------------------
// OctreeBrickPoolManagerDiskLimitedRam

OctreeBrickPoolManagerDiskLimitedRam::OctreeBrickPoolManagerDiskLimitedRam(const size_t maxSingleBufferSize, const size_t maxRamUsed, const bool brickPoolExists,
                                         const std::string& brickPoolPath, const std::string& bufferFilePrefix, const AllocateStrategy strategy, const fileIOStrategy fileIO)
    : OctreeBrickPoolManagerDisk(maxSingleBufferSize, brickPoolExists, brickPoolPath, bufferFilePrefix, fileIO)
    , maxRamUsed_(maxRamUsed)
    , nextVirtualMemoryAddress_(0)
    , nextTimestemp_(0)
    , allocateStrategy_(strategy)
{}

OctreeBrickPoolManagerDiskLimitedRam::~OctreeBrickPoolManagerDiskLimitedRam() {
}

OctreeBrickPoolManagerDiskLimitedRam* OctreeBrickPoolManagerDiskLimitedRam::create() const {
    return new OctreeBrickPoolManagerDiskLimitedRam(64, 512, false, "");
}

void OctreeBrickPoolManagerDiskLimitedRam::initialize(size_t brickMemorySizeInByte) throw (VoreenException)
{
    OctreeBrickPoolManagerDisk::initialize(brickMemorySizeInByte);

    // to work properly, at least 2 buffers have to fit into the ram
    if(2*getBrickBufferSizeInBytes() > maxRamUsed_)
        throw VoreenException("Max used memory in RAM is smaller than two times the size of a buffer. At least two buffer files have to fit in the RAM. "
                              "[" + itos(maxRamUsed_) + " bytes < 2*" + itos(getBrickMemorySizeInByte()) + " bytes]");

    //set brick vector of buffers in the ram
    brickBuffersInRAM_ = std::vector<brickBufferInRAM>(maxRamUsed_/getBrickBufferSizeInBytes(),brickBufferInRAM());
    bricksOfBuffersInUsage_ = std::vector<size_t>(maxRamUsed_/getBrickBufferSizeInBytes(),0);
}

void OctreeBrickPoolManagerDiskLimitedRam::deinitialize() throw (VoreenException) {
    for (size_t i=0; i<brickBuffersInRAM_.size(); i++) {
        if(brickBuffersInRAM_[i].writable_)
            saveBufferToDisk(brickBuffersInRAM_[i].index_,brickBuffersInRAM_[i].pointer_);
    }
    brickBuffersInRAM_.clear();
    deletedBricks_.clear();
    bricksInUsage_.clear();
    bricksOfBuffersInUsage_.clear();
    nextVirtualMemoryAddress_ = 0;
    OctreeBrickPoolManagerDisk::deinitialize();
}

bool OctreeBrickPoolManagerDiskLimitedRam::isBrickInRAM(uint64_t virtualMemoryAddress) const {
    size_t bufferID = static_cast<size_t>(virtualMemoryAddress / getBrickBufferSizeInBytes());
    for(size_t i = 0; i < brickBuffersInRAM_.size(); i++) {
        if(brickBuffersInRAM_[i].index_ == bufferID)
            return true;
    }
    return false;
}

size_t OctreeBrickPoolManagerDiskLimitedRam::isBufferInRAM(size_t bufferID) const {
    for(size_t i = 0; i < brickBuffersInRAM_.size(); i++) {
        if(brickBuffersInRAM_[i].index_ == bufferID)
            return i;
    }
    return (size_t)(-1);
}

uint64_t OctreeBrickPoolManagerDiskLimitedRam::allocateBrick() throw (VoreenException){
    switch(allocateStrategy_) {
    case IGNORE_DELETED_BRICKS:
        return ignoreFreeBricks();
        break;
    case USE_DELETED_BRICKS:
        return useFreeBricks();
        break;
    default:
        //shouldn't get here
        tgtAssert(false,"Shouldn't get here!!!");
        return 0;
        break;
    }
    //shouldn't get here
    tgtAssert(false,"Shouldn't get here!!!");
    return 0;
}

uint64_t OctreeBrickPoolManagerDiskLimitedRam::ignoreFreeBricks() {
    // TODO: checks
    size_t bufferID = static_cast<size_t>(nextVirtualMemoryAddress_ / getBrickBufferSizeInBytes());
    while (bufferID >= getBufferFiles().size()) {
    //load brick from disk
        brickBufferInRAM* leastUsedBuffer = 0;
        for(size_t i = 0; i < brickBuffersInRAM_.size(); i++){
            if(!brickBuffersInRAM_[i].inUsage_  && (leastUsedBuffer == 0 || leastUsedBuffer->lastUsed_ > brickBuffersInRAM_[i].lastUsed_)) {
                leastUsedBuffer = const_cast<brickBufferInRAM*>(&brickBuffersInRAM_[i]);
            }
        }
        //check, if all buffers are in use
        if(!leastUsedBuffer) {
            throw VoreenException("All buffers in RAM are in use!!!");
        }
        //save buffer if it is writable
        if(leastUsedBuffer->writable_)
            saveBufferToDisk(leastUsedBuffer->index_,leastUsedBuffer->pointer_);
        //clean struct and return
        delete[] leastUsedBuffer->pointer_;
        leastUsedBuffer->index_ = getBufferFiles().size();
        leastUsedBuffer->pointer_ = allocateNextDiskBuffer(); //no bad_alloc, since memory has been freed
        leastUsedBuffer->lastUsed_ = increaseTimestemp();
        //LERROR("Loaded Buffer ID: " << getBufferFiles().size()-1); //TODO: DEBUG
    }
    //return value
    uint64_t returnValue = nextVirtualMemoryAddress_;
    nextVirtualMemoryAddress_ += static_cast<uint64_t>(getBrickMemorySizeInByte());
    return returnValue;
}

uint64_t OctreeBrickPoolManagerDiskLimitedRam::useFreeBricks() {
    //case1: actual buffer is not full -> use it
    //case2: we have free bricks -> use them
    //case3: we have to allocate a new buffer

    //case 1
    if (nextVirtualMemoryAddress_%getBrickBufferSizeInBytes() != 0) {
        uint64_t returnValue = nextVirtualMemoryAddress_;
        nextVirtualMemoryAddress_ += static_cast<uint64_t>(getBrickMemorySizeInByte());
        return returnValue;
    } else //case2
    if (!deletedBricks_.empty()) {
        uint64_t returnValue = deletedBricks_.back();
        deletedBricks_.pop_back();
        return returnValue;
    } else { //case3
        return ignoreFreeBricks();
    }
}

void OctreeBrickPoolManagerDiskLimitedRam::deleteBrick(uint64_t virtualMemoryAddress) {
    deletedBricks_.push_back(virtualMemoryAddress);
    setBrickUnused(virtualMemoryAddress);
}

const uint16_t* OctreeBrickPoolManagerDiskLimitedRam::getBrick(uint64_t virtualMemoryAddress) const throw (VoreenException){
    //constraint SIZE_MAX equals no brick
    if(virtualMemoryAddress == (size_t)(-1)) return 0;
    //transform virtual memory address
    size_t bufferID = static_cast<size_t>(virtualMemoryAddress / getBrickBufferSizeInBytes());
    size_t bufferOffset = virtualMemoryAddress % getBrickBufferSizeInBytes();
    //load/get buffer
    size_t bufferIndex = loadBrickBufferIntoRAM(bufferID);
    //buffer exists in ram
    if(bufferIndex < (size_t)(-1)) {
        setBrickUsed(virtualMemoryAddress, bufferIndex);
        return reinterpret_cast<uint16_t*>(brickBuffersInRAM_[bufferIndex].pointer_ + bufferOffset);
    } else //buffer could not be laoded
        return 0;
}

uint16_t* OctreeBrickPoolManagerDiskLimitedRam::getWritableBrick(uint64_t virtualMemoryAddress) const throw (VoreenException){
    //constraint SIZE_MAX equals no brick
    if(virtualMemoryAddress == (size_t)(-1)) return 0;
    //transform virtual memory address
    size_t bufferID = static_cast<size_t>(virtualMemoryAddress / getBrickBufferSizeInBytes());
    size_t bufferOffset = virtualMemoryAddress % getBrickBufferSizeInBytes();
    //load/get buffer
    size_t bufferIndex = loadBrickBufferIntoRAM(bufferID);
    //buffer exists in ram
    if(bufferIndex < (size_t)(-1)) {
        brickBuffersInRAM_[bufferIndex].writable_ = true;
        setBrickUsed(virtualMemoryAddress, bufferIndex);
        return reinterpret_cast<uint16_t*>(brickBuffersInRAM_[bufferIndex].pointer_ + bufferOffset);
    } else //buffer could not be laoded
        return 0;
}

size_t OctreeBrickPoolManagerDiskLimitedRam::loadBrickBufferIntoRAM(size_t bufferID) const throw (VoreenException) {
    // TODO: add checks
    if (bufferID >= getBufferFiles().size()) {
        tgtAssert(false,"Buffer has not been created");
        LERROR("loadBrickBufferIntoRAM(): Buffer has not been created");
        return (size_t)(-1);
    }
    else {
        size_t bufferIndex = isBufferInRAM(bufferID);
        if(bufferIndex < (size_t)(-1)) {
            //increase timestemp and return index
            brickBuffersInRAM_[bufferIndex].lastUsed_ = increaseTimestemp();
            return bufferIndex;
        } else {
            //load brick from disk
            brickBufferInRAM* leastUsedBuffer = 0;
            for(size_t i = 0; i < brickBuffersInRAM_.size(); i++){
                if(!brickBuffersInRAM_[i].inUsage_  && (leastUsedBuffer == 0 || leastUsedBuffer->lastUsed_ > brickBuffersInRAM_[i].lastUsed_)) {
                    leastUsedBuffer = const_cast<brickBufferInRAM*>(&brickBuffersInRAM_[i]);
                    bufferIndex = i;
                }
            }
            //check, if all buffers are in use
            if(!leastUsedBuffer) {
                throw VoreenException("All buffers in RAM are in use!!!");
            }
            //save buffer if it is writable
            if(leastUsedBuffer->writable_)
                saveBufferToDisk(leastUsedBuffer->index_,leastUsedBuffer->pointer_);
            //clean struct and return
            delete[] leastUsedBuffer->pointer_;
            leastUsedBuffer->pointer_ = loadDiskBuffer(bufferID); //no bad_alloc, since memory has been freed
            leastUsedBuffer->index_ = bufferID;
            leastUsedBuffer->lastUsed_ = increaseTimestemp();
            //LERROR("Loaded Buffer ID: " << bufferID); //TODO: DEBUG
            return bufferIndex;
        }
    }
    //shouldn't get here
    tgtAssert(false,"Shouldn't get here!!!")
    return (size_t)(-1);
}

void OctreeBrickPoolManagerDiskLimitedRam::setBrickUnused(uint64_t virtualMemoryAddress) const {
    //remove address
    if(bricksInUsage_.erase(virtualMemoryAddress)){
        size_t bufferIndex = isBufferInRAM(static_cast<size_t>(virtualMemoryAddress / static_cast<uint64_t>(getBrickBufferSizeInBytes())));
        tgtAssert(bufferIndex < (size_t)(-1), "Used Buffer was not in RAM!!!");
        tgtAssert(bricksOfBuffersInUsage_[bufferIndex] > 0, "Buffer has less used bricks!!!")
        bricksOfBuffersInUsage_[bufferIndex] -= 1;
        if(bricksOfBuffersInUsage_[bufferIndex] == 0)
            brickBuffersInRAM_[bufferIndex].inUsage_ = false;
    }
}

void OctreeBrickPoolManagerDiskLimitedRam::setBrickUsed(uint64_t virtualMemoryAddress, size_t bufferIndex) const {
    //add address
    if(bricksInUsage_.insert(virtualMemoryAddress).second){
        bricksOfBuffersInUsage_[bufferIndex] += 1;
        if(bricksOfBuffersInUsage_[bufferIndex] == 1)
            brickBuffersInRAM_[bufferIndex].inUsage_ = true;
    }
}

size_t OctreeBrickPoolManagerDiskLimitedRam::increaseTimestemp() const {
    nextTimestemp_++;

    //handle overflow
    if(nextTimestemp_ == (size_t)(-1)){
        //find minimum
        size_t minV = -1;
        for(size_t i = 0; i < brickBuffersInRAM_.size(); ++i){
            if(brickBuffersInRAM_[i].lastUsed_ < minV)
                minV = brickBuffersInRAM_[i].lastUsed_;
        }
        //subtract min and find max
        size_t maxV = 0;
        for(size_t i = 0; i < brickBuffersInRAM_.size(); ++i){
            brickBuffersInRAM_[i].lastUsed_ -= minV;
            if(brickBuffersInRAM_[i].lastUsed_ > maxV)
                maxV = brickBuffersInRAM_[i].lastUsed_;
        }
        nextTimestemp_ = maxV+1;
    }
    return nextTimestemp_;
}

uint64_t OctreeBrickPoolManagerDiskLimitedRam::getBrickPoolMemoryUsed() const {
    // TODO
    return 0;
}

void OctreeBrickPoolManagerDiskLimitedRam::serialize(XmlSerializer& s) const {
    OctreeBrickPoolManagerDisk::serialize(s);

    s.serialize("maxRamUsed", maxRamUsed_);
    s.serialize("nextVirtualMemoryAddress", nextVirtualMemoryAddress_);
}

void OctreeBrickPoolManagerDiskLimitedRam::deserialize(XmlDeserializer& s) {
    OctreeBrickPoolManagerDisk::deserialize(s);

    s.deserialize("maxRamUsed", maxRamUsed_);
    s.deserialize("nextVirtualMemoryAddress", nextVirtualMemoryAddress_);

    //set brick vector of buffers in the ram
    brickBuffersInRAM_ = std::vector<brickBufferInRAM>(maxRamUsed_/getBrickBufferSizeInBytes(),brickBufferInRAM());
    bricksOfBuffersInUsage_ = std::vector<size_t>(maxRamUsed_/getBrickBufferSizeInBytes(),0);
}

} // namespace
