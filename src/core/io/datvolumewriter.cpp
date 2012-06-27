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

#include "voreen/core/io/datvolumewriter.h"
#include "voreen/core/volume/volumeatomic.h"

#include "tgt/filesystem.h"
#include "tgt/matrix.h"

namespace voreen {

const std::string DatVolumeWriter::loggerCat_("voreen.io.DatVolumeWriter");

DatVolumeWriter::DatVolumeWriter() {
    name_ = "Dat Writer";
    extensions_.push_back("dat");
}

void DatVolumeWriter::write(const std::string& filename, Volume* volume)
    throw (tgt::IOException)
{

    std::string datname = filename;
    std::string rawname = getFileNameWithoutExtension(filename) + ".raw";
    LINFO("saving " << datname << " and " << rawname);

    std::fstream datout(datname.c_str(), std::ios::out);
    std::fstream rawout(rawname.c_str(), std::ios::out | std::ios::binary);

    if (datout.bad() || rawout.bad())
        throw tgt::IOException();

    // write dat file
    std::string format;
    std::string model = "I";
    std::string zeroString = "";
    char* data = 0;
    size_t numbytes = 0;

    if (VolumeUInt8* vol = dynamic_cast<VolumeUInt8*>(volume)) {
        format = "UCHAR";
        data = reinterpret_cast<char*>(vol->voxel());
        numbytes = vol->getNumBytes();

        if (vol->isSigned() && vol->getZeroPoint() != VolumeElement<unsigned char>::getZero())
            zeroString = "ZeroPoint:\t" + vol->getZeroPoint();

    }
    else if (VolumeUInt16* vol = dynamic_cast<VolumeUInt16*>(volume)) {
        if (vol->getBitsStored() == 12)
            format = "USHORT_12";
        else
            format = "USHORT";

        data = reinterpret_cast<char*>(vol->voxel());
        numbytes = vol->getNumBytes();

        if (vol->isSigned() && vol->getZeroPoint() != VolumeElement<unsigned short>::getZero())
            zeroString = "ZeroPoint:\t" + vol->getZeroPoint();

    }
    else if (Volume4xUInt8* vol = dynamic_cast<Volume4xUInt8*>(volume)) {
        format = "UCHAR";
        model = "RGBA";

        data = reinterpret_cast<char*>(vol->voxel());
        numbytes = vol->getNumBytes();

        // I don't really know how to interpret the zero point if it's a vector... does that even make sense? FL
        if (vol->isSigned() && vol->getZeroPoint() != VolumeElement<tgt::col4>::getZero())
            zeroString = "ZeroPoint:\t" + vol->getZeroPoint().r;

    }
    else if (Volume3xUInt8* vol = dynamic_cast<Volume3xUInt8*>(volume)) {
        format = "UCHAR";
        model = "RGB";

        data = reinterpret_cast<char*>(vol->voxel());
        numbytes = vol->getNumBytes();

        // I don't really know how to interpret the zero point if it's a vector... does that even make sense? FL
        if (vol->isSigned() && vol->getZeroPoint() != VolumeElement<tgt::col3>::getZero())
            zeroString = "ZeroPoint:\t" + vol->getZeroPoint().r;

    }
    else if (Volume4xUInt16* vol = dynamic_cast<Volume4xUInt16*>(volume)) {
        format = "USHORT";
        model = "RGBA";

        data = reinterpret_cast<char*>(vol->voxel());
        numbytes = vol->getNumBytes();

        // I don't really know how to interpret the zero point if it's a vector... does that even make sense? FL
        if (vol->isSigned() && vol->getZeroPoint() != VolumeElement< tgt::Vector4<uint16_t> >::getZero())
            zeroString = "ZeroPoint:\t" + vol->getZeroPoint().r;

    }
    else if (Volume3xUInt16* vol = dynamic_cast<Volume3xUInt16*>(volume)) {
        format = "USHORT";
        model = "RGB";

        data = reinterpret_cast<char*>(vol->voxel());
        numbytes = vol->getNumBytes();

        // I don't really know how to interpret the zero point if it's a vector... does that even make sense? FL
        if (vol->isSigned() && vol->getZeroPoint() != VolumeElement< tgt::Vector3<uint16_t> >::getZero())
            zeroString = "ZeroPoint:\t" + vol->getZeroPoint().r;

    }
    else
        LERROR("Format currently not supported");

    datout << "ObjectFileName:\t" << tgt::FileSystem::fileName(rawname) << std::endl;

    tgt::ivec3 dimensions = volume->getDimensions();
    datout << "Resolution:\t" << dimensions.x << " " << dimensions.y << " " << dimensions.z << std::endl;

    tgt::vec3 spacing = volume->getSpacing();
    datout << "SliceThickness:\t" << spacing.x << " " << spacing.y << " " << spacing.z << std::endl;

    datout << "Format:\t\t" << format << std::endl;
    datout << "ObjectModel:\t" << model << std::endl;

    // write transformation matrix unless it is the identity matrix
    tgt::mat4 transformation = volume->meta().getTransformation();
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

    datout << zeroString << std::endl;
    datout.close();

    // write raw file
    rawout.write(data, numbytes);
    rawout.close();
}

} // namespace voreen
