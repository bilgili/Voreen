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

#include "voreen/modules/base/processors/volume/volumehalfsample.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

namespace voreen {

const std::string VolumeHalfsample::loggerCat_("voreen.VolumeHalfsample");

VolumeHalfsample::VolumeHalfsample()
    : VolumeProcessor(),
    inport_(Port::INPORT, "input"),
    outport_(Port::OUTPORT, "output", 0)
{
    addPort(inport_);
    addPort(outport_);
}

VolumeHalfsample::~VolumeHalfsample() {}

Processor* VolumeHalfsample::create() const {
    return new VolumeHalfsample();
}

std::string VolumeHalfsample::getProcessorInfo() const {
    return "Reduces the Volumes resolution by half, by linearly downsampling 8 voxels to 1 voxel.";
}

void VolumeHalfsample::process() {
    Volume* inputVolume = inport_.getData()->getVolume();
    Volume* outputVolume = inputVolume->halfsample();

    // assign computed volume to outport
    if (outputVolume)
        outport_.setData(new VolumeHandle(outputVolume), true);
    else
        outport_.deleteVolume();
}

void VolumeHalfsample::deinitialize() throw (VoreenException) {
    outport_.deleteVolume();

    VolumeProcessor::deinitialize();
}

}   // namespace
