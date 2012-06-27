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

#include "voreen/core/io/datvolumereader.h"

#include <fstream>
#include <iostream>

#include "tgt/exception.h"
#include "tgt/vector.h"

#include "voreen/core/io/textfilereader.h"
#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/volume/volumeatomic.h"

using tgt::vec3;
using tgt::ivec3;
using tgt::hor;
using tgt::lessThanEqual;

namespace voreen {

const std::string DatVolumeReader::loggerCat_ = "voreen.io.VolumeReader.dat";

VolumeSet* DatVolumeReader::read(const std::string &fileName)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    std::string objectFilename;
    std::string taggedFilename;
    ivec3 resolution = ivec3(0, 0, 0);
    vec3 sliceThickness = vec3(1.f, 1.f, 1.f);
    std::string format;
    int nbrTags;
    std::string objectType;
    std::string objectModel;
    std::string gridType;
    tgt::mat4 transformation = tgt::mat4::identity;
    std::string unit;
    Modality modality = Modality::MODALITY_UNKNOWN;
    int zeroPoint = 0;
    float timeStep = -1.0f;
    std::string metaString = "";
    bool error = false;

    LINFO("Loading file " << fileName);
    TextFileReader reader(fileName);

    if (!reader)
        throw tgt::IOException();

    std::string type;
    std::istringstream args;
    int bits = 0;

    while (reader.getNextLine(type, args, false)) {

        if (type == "ObjectFileName:") {
            args >> objectFilename;
            LINFO(type << " " << objectFilename);
        } else if (type == "TaggedFileName:") {
            args >> taggedFilename;
            LINFO(type << " " << taggedFilename);
        } else if (type == "Resolution:") {
            args >> resolution[0];
            args >> resolution[1];
            args >> resolution[2];
            LINFO(type << " " << resolution[0] << " x " <<
                  resolution[1] << " x " << resolution[2]);
        } else if (type == "SliceThickness:") {
            args >> sliceThickness[0] >> sliceThickness[1] >> sliceThickness[2];
            LINFO(type << " " << sliceThickness[0] << " " <<
                  sliceThickness[1] << " " << sliceThickness[2]);
        } else if (type == "Format:") {
            args >> format;
            LINFO(type << " " << format);
        } else if (type == "NbrTags:") {
            args >> nbrTags;
            LINFO(type << " " << nbrTags);
        } else if (type == "ObjectType:") {
            args >> objectType;
            LINFO(type << " " << objectType);
        } else if (type == "ObjectModel:") {
            args >> objectModel;
            LINFO(type << " " << objectModel);
        } else if (type == "GridType:") {
            args >> gridType;
            LINFO(type << " " << gridType);
        } else if (type == "BitsStored:") {
            args >> bits;
            LINFO(type << " " << bits);
        } else if (type == "Unit:") {
            args >> unit;
            LINFO(type << " " << unit);
        } else if (type == "ZeroPoint:") {
            args >> zeroPoint;
            LINFO(type << " " << zeroPoint);
        } else if (type == "TransformMatrix:") {
            // first argument contains number of row
            std::string row;
            args >> row;
            // last element is the number
            int index = atoi((row.substr(row.length()-1, row.length()-1)).c_str());
            args >> transformation[index][0];
            args >> transformation[index][1];
            args >> transformation[index][2];
            args >> transformation[index][3];
            if (index == 3) {
                LINFO("TransformMatrix:");
                LINFO("  " << transformation[0]);
                LINFO("  " << transformation[1]);
                LINFO("  " << transformation[2]);
                LINFO("  " << transformation[3]);
            }
        }
        else if (type == "Modality:") {
            std::string modalityStr;
            args >> modalityStr;
            LINFO(type << " " << modalityStr);
            modality = Modality(modalityStr);
        }
        else if (type == "TimeStep:") {
            args >> timeStep;
            LINFO(type << " " << timeStep);
        }
        else if (type == "MetaString:") {
            args >> metaString;
            LINFO(type << " " << metaString);
        }
        else {
            LERROR("Unknown type: " << type);
        }

        if (args.fail()) {
            LERROR("Format error");
            error = true;
        }
    }

    // check wether necessary meta-data could be read
    if (objectFilename == "") {
        LERROR("No raw file specified");
        error = true;
    }
    if ( hor(lessThanEqual(resolution,ivec3(0,0,0))) ) {
        LERROR("Invalid resolution or resolution not specified: " << resolution[0] << " x " <<
               resolution[1] << " x " << resolution[2])
        error = true;
    }

    if (!error) {
        RawVolumeReader rawReader(progress_);
        rawReader.readHints(resolution, sliceThickness, bits, objectModel, format, zeroPoint,
            transformation, modality, timeStep, metaString);

        // do we have a relative path?
        if ((objectFilename.substr(0, 1) != "/")  && (objectFilename.substr(0, 1) != "\\") &&
            (objectFilename.substr(1, 2) != ":/") && (objectFilename.substr(1, 2) != ":\\"))
        {
            size_t p = fileName.find_last_of("\\");
            if (p == std::string::npos)
                p = fileName.find_last_of("/");

            // construct path relative to dat file
            objectFilename = fileName.substr(0, p + 1) + objectFilename;
        }

        VolumeSet* volumeSet = 0;
        try {
            volumeSet = rawReader.read(objectFilename);
        }
        catch (...) {
            throw; // throw it to the caller
        }
        fixOrigins(volumeSet, fileName);
        return volumeSet;
    } else {
        throw tgt::CorruptedFileException();
    }
}

} // namespace voreen
