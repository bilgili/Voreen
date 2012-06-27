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

#include "voreen/core/io/datvolumewriter.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

#include "tgt/filesystem.h"
#include "tgt/matrix.h"

namespace voreen {

const std::string DatVolumeWriter::loggerCat_("voreen.io.DatVolumeWriter");

DatVolumeWriter::DatVolumeWriter() {
    extensions_.push_back("dat");
}

void DatVolumeWriter::write(const std::string& filename, VolumeHandle* volumeHandle)
    throw (tgt::IOException)
{

    tgtAssert(volumeHandle, "No volume handle");
    Volume* volume = volumeHandle->getVolume();
    if (!volume) {
        LWARNING("No volume");
        return;
    }

    std::string datname = filename;
    std::string rawname = getFileNameWithoutExtension(filename) + ".raw";
    LINFO("saving " << datname << " and " << rawname);

    std::fstream datout(datname.c_str(), std::ios::out);
    std::fstream rawout(rawname.c_str(), std::ios::out | std::ios::binary);

    if (datout.bad() || rawout.bad())
        throw tgt::IOException();

    char* data = 0;
    size_t numbytes = 0;
    datout << getDatFileString(volumeHandle, rawname, &data, numbytes);
    if (datout.bad())
        throw tgt::IOException();
    datout.close();

    // write raw file
    rawout.write(data, numbytes);
    if (rawout.bad())
        throw tgt::IOException();
    rawout.close();
}

std::string DatVolumeWriter::getDatFileString(VolumeHandle* const volumeHandle, const std::string& rawFileName,
                                              char** volData, size_t& numBytes)
{
    std::ostringstream datout;
    tgtAssert(volumeHandle, "No volume handle");
    Volume* volume = volumeHandle->getVolume();
    if ((!volume) || (!volData)) {
        LWARNING("No volume or no storage for casted volume data!");
        return "";
    }

    // write dat file
    std::string format;
    std::string model = "I";

    if (VolumeUInt8* vol = dynamic_cast<VolumeUInt8*>(volume)) {
        format = "UCHAR";
        *volData = reinterpret_cast<char*>(vol->voxel());
        numBytes = vol->getNumBytes();
    }
    else if (VolumeInt8* vol = dynamic_cast<VolumeInt8*>(volume)) {
        format = "CHAR";
        *volData = reinterpret_cast<char*>(vol->voxel());
        numBytes = vol->getNumBytes();
    }
    else if (VolumeUInt16* vol = dynamic_cast<VolumeUInt16*>(volume)) {
        if (vol->getBitsStored() == 12)
            format = "USHORT_12";
        else
            format = "USHORT";

        *volData = reinterpret_cast<char*>(vol->voxel());
        numBytes = vol->getNumBytes();

    }
    else if (VolumeInt16* vol = dynamic_cast<VolumeInt16*>(volume)) {
        format = "SHORT";
        *volData = reinterpret_cast<char*>(vol->voxel());
        numBytes = vol->getNumBytes();
    }
    else if (VolumeFloat* vol = dynamic_cast<VolumeFloat*>(volume)) {
        format = "FLOAT";
        *volData = reinterpret_cast<char*>(vol->voxel());
        numBytes = vol->getNumBytes();

    }
    else if (Volume4xUInt8* vol = dynamic_cast<Volume4xUInt8*>(volume)) {
        format = "UCHAR";
        model = "RGBA";

        *volData = reinterpret_cast<char*>(vol->voxel());
        numBytes = vol->getNumBytes();

    }
    else if (Volume3xUInt8* vol = dynamic_cast<Volume3xUInt8*>(volume)) {
        format = "UCHAR";
        model = "RGB";

        *volData = reinterpret_cast<char*>(vol->voxel());
        numBytes = vol->getNumBytes();

    }
    else if (Volume4xUInt16* vol = dynamic_cast<Volume4xUInt16*>(volume)) {
        format = "USHORT";
        model = "RGBA";

        *volData = reinterpret_cast<char*>(vol->voxel());
        numBytes = vol->getNumBytes();

    }
    else if (Volume3xUInt16* vol = dynamic_cast<Volume3xUInt16*>(volume)) {
        format = "USHORT";
        model = "RGB";

        *volData = reinterpret_cast<char*>(vol->voxel());
        numBytes = vol->getNumBytes();

    } else if (Volume3xFloat* vol = dynamic_cast<Volume3xFloat*>(volume)) {
        format = "FLOAT";
        model = "RGB";

        *volData = reinterpret_cast<char*>(vol->voxel());
        numBytes = vol->getNumBytes();
    } else
        LERROR("Format currently not supported");

    datout << "ObjectFileName:\t" << tgt::FileSystem::fileName(rawFileName) << std::endl;

    tgt::ivec3 dimensions = volume->getDimensions();
    datout << "Resolution:\t" << dimensions.x << " " << dimensions.y << " " << dimensions.z << std::endl;

    tgt::vec3 spacing = volume->getSpacing();
    datout << "SliceThickness:\t" << spacing.x << " " << spacing.y << " " << spacing.z << std::endl;

    datout << "Format:\t\t" << format << std::endl;
    datout << "ObjectModel:\t" << model << std::endl;
    datout << "Modality:\t" << volumeHandle->getModality() << std::endl;

    // write transformation matrix unless it is the identity matrix
    tgt::mat4 transformation = volume->getTransformation();
    if (transformation != tgt::mat4::createIdentity())
        datout << "TransformMatrix: row0\t" << transformation[0][0] << " " << transformation[0][1] << " "
               << transformation[0][2] << " " << transformation[0][3] << std::endl
               << "TransformMatrix: row1\t" << transformation[1][0] << " " << transformation[1][1] << " "
               << transformation[1][2] << " " << transformation[1][3] << std::endl
               << "TransformMatrix: row2\t" << transformation[2][0] << " " << transformation[2][1] << " "
               << transformation[2][2] << " " << transformation[2][3] << std::endl
               << "TransformMatrix: row3\t" << transformation[3][0] << " " << transformation[3][1] << " "
               << transformation[3][2] << " " << transformation[3][3] << std::endl;

    std::string metaString = volume->meta().getString();
    if (!metaString.empty())
        datout << "MetaString:\t" << metaString << std::endl;

    return datout.str();
}

VolumeWriter* DatVolumeWriter::create(ProgressBar* /*progress*/) const {
    return new DatVolumeWriter(/*progress*/);
}

} // namespace voreen
