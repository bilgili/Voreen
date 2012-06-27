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

#include "voreen/core/io/datvolumereader.h"

#include <fstream>
#include <iostream>

#include "tgt/exception.h"
#include "tgt/vector.h"

#include "voreen/core/io/textfilereader.h"
#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

using tgt::vec3;
using tgt::ivec3;
using tgt::hor;
using tgt::lessThanEqual;

namespace voreen {

const std::string DatVolumeReader::loggerCat_ = "voreen.io.VolumeReader.dat";

DatVolumeReader::DatVolumeReader(IOProgress* progress)
    : VolumeReader(progress)
{
    extensions_.push_back("dat");
}

std::string DatVolumeReader::getRelatedRawFileName(const std::string& fileName) {
    TextFileReader reader(fileName);
    if (! reader)
        return "";

    std::string type;
    std::string objectFilename = "";
    std::istringstream args;
    while (reader.getNextLine(type, args, false) == true) {
        if (type == "ObjectFileName:") {
            args >> objectFilename;
            break;
        }
    }
    return objectFilename;
}

VolumeCollection* DatVolumeReader::read(const std::string &url)
    throw (tgt::FileException, std::bad_alloc)
{
    return readSlices(url, 0, 0);
}

VolumeCollection* DatVolumeReader::readMetaFile(const std::string &fileName, size_t firstSlice,size_t lastSlice)
    throw (tgt::FileException, std::bad_alloc)
{
    RawVolumeReader::ReadHints h;
    std::string objectFilename;
    vec3 sliceThickness = vec3(1.f, 1.f, 1.f);
    std::string taggedFilename;
    int nbrTags;
    std::string objectType;
    std::string gridType;
    bool error = false;

    LINFO("Loading dat file " << fileName);
    TextFileReader reader(fileName);

    if (!reader)
        throw tgt::FileNotFoundException("reading dat file", fileName);

    std::string type;
    std::istringstream args;

    while (reader.getNextLine(type, args, false)) {
        if (type == "ObjectFileName:") {
            args >> objectFilename;
            LDEBUG(type << " " << objectFilename);
        } else if (type == "TaggedFileName:") {
            args >> taggedFilename;
            LDEBUG(type << " " << taggedFilename);
        } else if (type == "Resolution:") {
            args >> h.dimensions_.x;
            args >> h.dimensions_.y;
            args >> h.dimensions_.z;
            LDEBUG(type << " " << h.dimensions_);
        } else if (type == "SliceThickness:") {
            args >> sliceThickness.x >> sliceThickness.y >> sliceThickness.z;
            LDEBUG(type << " " << sliceThickness);
        } else if (type == "Format:") {
            args >> h.format_;
            LDEBUG(type << " " << h.format_);
        } else if (type == "NbrTags:") {
            args >> nbrTags;
            LDEBUG(type << " " << nbrTags);
        } else if (type == "ObjectType:") {
            args >> objectType;
            LDEBUG(type << " " << objectType);
        } else if (type == "ObjectModel:") {
            args >> h.objectModel_;
            LDEBUG(type << " " << h.objectModel_);
        } else if (type == "GridType:") {
            args >> gridType;
            LDEBUG(type << " " << gridType);
        } else if (type == "BitsStored:") {
            args >> h.bitsStored_;
            LDEBUG(type << " " << h.bitsStored_);
        } else if (type == "ByteOrder:") {
            std::string s;
            args >> s;
            if (s == "big-endian" || s == "bigendian" || s == "bigEndian")
                h.bigEndianByteOrder_ = true;
            else if (s == "little-endian" || s == "littleendian" || s == "littleEndian")
                h.bigEndianByteOrder_ = false;
            else
                LERROR("Unknown byte order: " << s);
        } else if (type == "Unit:") {
            args >> h.unit_;
            LDEBUG(type << " " << h.unit_);
        } else if (type == "TransformMatrix:") {
            // first argument contains number of row
            std::string row;
            args >> row;
            // last element is the number
            int index = atoi((row.substr(row.length()-1, row.length()-1)).c_str());
            args >> h.transformation_[index].x;
            args >> h.transformation_[index].y;
            args >> h.transformation_[index].z;
            args >> h.transformation_[index][3];
            if (index == 3) {
                LDEBUG("TransformMatrix:");
                LDEBUG("  " << h.transformation_[0]);
                LDEBUG("  " << h.transformation_[1]);
                LDEBUG("  " << h.transformation_[2]);
                LDEBUG("  " << h.transformation_[3]);
            }
        }
        else if (type == "Modality:") {
            std::string modalityStr;
            args >> modalityStr;
            LDEBUG(type << " " << modalityStr);
            h.modality_ = Modality(modalityStr);
        }
        else if (type == "TimeStep:") {
            args >> h.timeStep_;
            LDEBUG(type << " " << h.timeStep_);
        }
        else if (type == "MetaString:") {
            args >> h.metaString_;
            LDEBUG(type << " " << h.metaString_);
        } else if (type == "SliceOrder:") {
            args >> h.sliceOrder_;
            LDEBUG(type << " " << h.sliceOrder_);
        } else if (type == "Spread:") {
            args >> h.spreadMin_ >> h.spreadMax_;
            LDEBUG(type << " " << h.spreadMin_ << " " << h.spreadMax_);
        } else {
            LERROR("Unknown type: " << type);
        }

        if (args.fail()) {
            LERROR("Format error");
            error = true;
        }
    }

    // check whether necessary meta-data could be read
    if (objectFilename.empty()) {
        LERROR("No raw file specified");
        error = true;
    }

    if (hor(lessThanEqual(h.dimensions_, ivec3(0)))) {
        LERROR("Invalid resolution or resolution not specified: " << h.dimensions_);
        error = true;
    }

    h.spacing_ = sliceThickness;

    if (!error) {
        RawVolumeReader rawReader(getProgress());
        rawReader.setReadHints(h);

        // do we have a relative path?
        if ((objectFilename.substr(0, 1) != "/")  && (objectFilename.substr(0, 1) != "\\") &&
            (objectFilename.substr(1, 2) != ":/") && (objectFilename.substr(1, 2) != ":\\"))
        {
            size_t p = fileName.find_last_of("\\/");
            // construct path relative to dat file
            objectFilename = fileName.substr(0, p + 1) + objectFilename;
        }

        VolumeCollection* volumeCollection = rawReader.readSlices(objectFilename, firstSlice, lastSlice);
        if (!volumeCollection->empty())
            volumeCollection->first()->setOrigin(VolumeOrigin(fileName));
        return volumeCollection;
    }
    else {
        throw tgt::CorruptedFileException("error while reading data", fileName);
    }
}

VolumeCollection* DatVolumeReader::readSlices(const std::string &url, size_t firstSlice, size_t lastSlice)
    throw (tgt::FileException, std::bad_alloc)
{
    VolumeOrigin origin(url);
    return readMetaFile(origin.getPath(), firstSlice, lastSlice);
}

VolumeCollection* DatVolumeReader::readBrick(const std::string& url, tgt::ivec3 brickStartPos, int brickSize)
    throw (tgt::FileException, std::bad_alloc)
{
    VolumeOrigin origin(url);
    return readMetaFileBrick(origin.getPath(), brickStartPos, brickSize);
}

VolumeCollection* DatVolumeReader::readVolumeFileBrick(const std::string &fileName, const tgt::ivec3& dims,
    tgt::ivec3 brickStartPos, int brickSize) throw (tgt::FileException, std::bad_alloc)
{
    RawVolumeReader rawReader(getProgress());

    rawReader.setReadHints(dims,               // dimensions of the volume
                           ivec3(1, 1, 1),     // thickness of one slice
                           16,                 // bits per voxel
                           "I",                // intensity image
                           "USHORT",           // one unsigned short per voxel
                           6);                 // header skip

    VolumeCollection* volumeCollection = rawReader.readBrick(fileName,brickStartPos, brickSize);

    return volumeCollection;
}

VolumeCollection* DatVolumeReader::readMetaFileBrick(const std::string &fileName,tgt::ivec3 brickStartPos,
    int brickSize) throw (tgt::FileException, std::bad_alloc)
{
    RawVolumeReader::ReadHints h;
    std::string objectFilename;
    std::string taggedFilename;
    vec3 sliceThickness = vec3(1.f, 1.f, 1.f);

    int nbrTags;
    std::string objectType;
    std::string gridType;
    bool error = false;

    //LINFO("Loading file " << fileName);
    TextFileReader reader(fileName);

    if (!reader)
        throw tgt::FileNotFoundException("reading dat file", fileName);

    std::string type;
    std::istringstream args;
    while (reader.getNextLine(type, args, false)) {

        if (type == "ObjectFileName:") {
            args >> objectFilename;
        } else if (type == "TaggedFileName:") {
            args >> taggedFilename;
        } else if (type == "Resolution:") {
            args >> h.dimensions_.x >> h.dimensions_.y >> h.dimensions_.z;
        } else if (type == "SliceThickness:") {
            args >> sliceThickness.x >> sliceThickness.y >> sliceThickness.z;
        } else if (type == "Format:") {
            args >> h.format_;
        } else if (type == "NbrTags:") {
            args >> nbrTags;
        } else if (type == "ObjectType:") {
            args >> objectType;
        } else if (type == "ObjectModel:") {
            args >> h.objectModel_;
        } else if (type == "GridType:") {
            args >> gridType;
        } else if (type == "BitsStored:") {
            args >> h.bitsStored_;
        } else if (type == "Unit:") {
            args >> h.unit_;
        } else if (type == "TransformMatrix:") {
            // first argument contains number of row
            std::string row;
            args >> row;
            // last element is the number
            int index = atoi((row.substr(row.length()-1, row.length()-1)).c_str());
            args >> h.transformation_[index].x;
            args >> h.transformation_[index].y;
            args >> h.transformation_[index].z;
            args >> h.transformation_[index][3];
        }
        else if (type == "Modality:") {
            std::string modalityStr;
            args >> modalityStr;
            h.modality_ = Modality(modalityStr);
        }
        else if (type == "TimeStep:") {
            args >> h.timeStep_;
        }
        else if (type == "MetaString:") {
            args >> h.metaString_;
        } else if (type == "Spread:") {
            args >> h.spreadMin_ >> h.spreadMax_;
            LDEBUG(type << " " << h.spreadMin_ << " " << h.spreadMax_);
        } else if (type == "ByteOrder:") {
            std::string s;
            args >> s;
            if (s == "big-endian" || s == "bigendian" || s == "bigEndian")
                h.bigEndianByteOrder_ = true;
            else if (s == "little-endian" || s == "littleendian" || s == "littleEndian")
                h.bigEndianByteOrder_ = false;
            else
                LERROR("Unknown byte order: " << s);
        } else {
            LERROR("Unknown type: " << type);
        }

        if (args.fail()) {
            LERROR("Format error");
            error = true;
        }
    }

    // check whether necessary meta-data could be read
    if (objectFilename.empty()) {
        LERROR("No raw file specified");
        error = true;
    }

    h.spacing_ = sliceThickness;

    if (hor(lessThanEqual(h.dimensions_, ivec3(0)))) {
        LERROR("Invalid resolution or resolution not specified: " << h.dimensions_);
        error = true;
    }

    if (!error) {
        RawVolumeReader rawReader(getProgress());
        rawReader.setReadHints(h);

        // do we have a relative path?
        if ((objectFilename.substr(0, 1) != "/")  && (objectFilename.substr(0, 1) != "\\") &&
            (objectFilename.substr(1, 2) != ":/") && (objectFilename.substr(1, 2) != ":\\"))
        {
            size_t p = fileName.find_last_of("\\/");
            // construct path relative to dat file
            objectFilename = fileName.substr(0, p + 1) + objectFilename;
        }

        VolumeCollection* volumeCollection = rawReader.readBrick(objectFilename, brickStartPos, brickSize);
        return volumeCollection;
    }
    else {
        throw tgt::CorruptedFileException("error while reading data", fileName);
    }
}

VolumeReader* DatVolumeReader::create(IOProgress* progress) const {
    return new DatVolumeReader(progress);
}

} // namespace voreen
