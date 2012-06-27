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

// include this before any windows headers
#include "ddsbase.h"

#include "voreen/core/io/pvmvolumereader.h"

#include <fstream>
#include <iostream>

#include "tgt/exception.h"
#include "tgt/texturemanager.h"

#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/volume/volumeatomic.h"

using std::string;
using tgt::Texture;

namespace voreen {

const std::string PVMVolumeReader::loggerCat_ = "voreen.io.VolumeReader.pvm";

PVMVolumeReader::PVMVolumeReader(IOProgress* progress)
  : VolumeReader(progress)
{
    name_ = "PVM Reader";
    extensions_.push_back("pvm");
}

VolumeSet* PVMVolumeReader::read(const std::string &fileName)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    uint8_t* data;
    uint8_t* tmpData;

    unsigned int width, height, depth, components;
    float scalex, scaley, scalez;
    unsigned char *description;
    unsigned char *courtesy;
    unsigned char *parameter;
    unsigned char *comment;

    LINFO(fileName);

    /*
        TODO This subroutine returns an array created with malloc but it should
        be created with 'new[]' because this chunk of data will be deleted with 'delete[]'.
        This can cause hard to find errors.

        As a temporary workaround the data are copied over into a new array
        and the c-array is deleted with 'free'.
        Because of some c-pointer vodoo done in ddsbase.cpp free must be invoked
        after the use of all other returned pointers. (roland)
    */
    try {
        tmpData = readPVMvolume(const_cast<char*>(fileName.c_str()), getProgress(), &width, &height, &depth, &components,
                                &scalex, &scaley, &scalez, &description, &courtesy,
                                &parameter, &comment);
        data = new uint8_t[width * height * depth * components];
    } catch (std::bad_alloc) {
        throw; // throw it to the caller
    }
    memcpy(data, tmpData, width * height * depth * components);

    Volume* dataset = 0;

    if (!data) {
        throw tgt::IOException();
    }
    else {
        LINFO("Size: " << width << " x " << height << " x " << depth);
        LINFO("Spacing: " << scalex << " x " << scaley << " x " << scalez);
        LINFO("Components: " << components);
        if (description)
            LINFO("Description: " << description);
        if (courtesy)
            LINFO("Courtesy: " << courtesy);
        if (parameter)
            LINFO("Parameter: " << parameter);
        if (comment)
            LINFO("Comment: " << comment);
        if (components == 1) {
            try {
				LINFO("Create 8 bit data set.");
                dataset = new VolumeUInt8(data, tgt::ivec3(width, height, depth),
                                          tgt::vec3(scalex, scaley, scalez));
            } catch (std::bad_alloc) {
                throw; // throw it to the caller
            }
        }
        else if (components == 2) {
            try {
				LINFO("Create 16 bit data set.");
                dataset = new VolumeUInt16((uint16_t*) data, tgt::ivec3(width, height, depth),
                                           tgt::vec3(scalex, scaley, scalez));
            } catch (std::bad_alloc) {
                throw; // throw it to the caller
            }
        }
        else LWARNING(" Bit depth not supported.");
    }

    // TODO now it is safe to free
    free(tmpData);

    VolumeSet* volumeSet = 0;
    try {
         volumeSet = new VolumeSet(0, fileName);
         VolumeSeries* volumeSeries = new VolumeSeries(volumeSet, "unknown", Modality::MODALITY_UNKNOWN);
         volumeSet->addSeries(volumeSeries);
         VolumeHandle* volumeHandle = new VolumeHandle(volumeSeries, dataset, 0.0f);
         volumeHandle->setOrigin(fileName, "unknown", 0.0f);
         volumeSeries->addVolumeHandle(volumeHandle);
    } catch (std::bad_alloc) {
        throw; // throw it to the caller
    }
    return volumeSet;
}

} // namespace voreen
