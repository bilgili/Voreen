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

#include "voreen/core/io/tuvvolumereader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <limits>

#include "tgt/exception.h"
#include "tgt/logmanager.h"
#include "tgt/vector.h"

#include "voreen/core/volume/volumeatomic.h"

using tgt::ivec3;
using tgt::vec3;

namespace voreen {

const std::string TUVVolumeReader::loggerCat_ = "voreen.io.VolumeReader.tuv";

VolumeSet* TUVVolumeReader::read(const std::string &fileName, bool generateVolumeGL)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    LINFO(fileName);

    std::fstream fin(fileName.c_str(), std::ios::in | std::ios::binary);
    if (!fin.good())
        throw tgt::IOException();

    unsigned short dim[3];
    fin.read(reinterpret_cast<char*>(dim),6);
    ivec3 dimensions = ivec3(dim[0], dim[1], dim[2]);

    LINFO("Read 16 bit dataset");
    VolumeUInt16* dataset;
    try {
        dataset = new VolumeUInt16(dimensions, ivec3(1));
    }
    catch (std::bad_alloc) {
        throw; // throw it to the caller
    }

    fin.read(reinterpret_cast<char*>(dataset->voxel()), dataset->getNumBytes());

    if ( fin.eof() )
        throw tgt::CorruptedFileException();

    fin.close();

    VolumeSet* volumeSet = new VolumeSet(fileName);
    VolumeSeries* volumeSeries = new VolumeSeries(volumeSet, "unknown", Modality::MODALITY_UNKNOWN);
    volumeSet->addSeries(volumeSeries);
    VolumeHandle* volumeHandle = new VolumeHandle(volumeSeries, dataset, 0.0f);
    volumeSeries->addVolumeHandle(volumeHandle);
    if( generateVolumeGL == true )
        volumeHandle->generateHardwareVolumes(VolumeHandle::HARDWARE_VOLUME_GL);

    return volumeSet;
}

} // namespace voreen
