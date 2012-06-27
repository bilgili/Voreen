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

#include "voreen/modules/base/processors/volume/volumecubify.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/gradient.h"

namespace voreen {

const std::string VolumeCubify::loggerCat_("voreen.VolumeCubify");

VolumeCubify::VolumeCubify()
    : VolumeProcessor(),
    inport_(Port::INPORT, "volumehandle.input"),
    outport_(Port::OUTPORT, "volumehandle.output", 0)

{
    addPort(inport_);
    addPort(outport_);
}

std::string VolumeCubify::getProcessorInfo() const {
    return "Enlarges the volume to have the same size in all dimensions.";
}

void VolumeCubify::process() {
    Volume* inputVolume = inport_.getData()->getVolume();
    Volume* outputVolume = 0;

    tgt::ivec3 oldims = inputVolume->getDimensions();
    tgt::ivec3 newdims = tgt::ivec3(tgt::max(oldims));
    tgt::ivec3 llf = (newdims - oldims) / 2;
    tgt::ivec3 urb = (newdims + oldims) / 2;

    if(inputVolume->getBitsStored() == 8)
        outputVolume = new VolumeUInt8(newdims);
    else if(inputVolume->getBitsStored() == 16 || inputVolume->getBitsStored() == 12)
        outputVolume = new VolumeUInt16(newdims);
    else
        LERROR("Unsupported value for getBitsStored()");

    for (int voxel_z=0; voxel_z < newdims.z; voxel_z++) {
        for (int voxel_y=0; voxel_y < newdims.y; voxel_y++) {
            for (int voxel_x=0; voxel_x < newdims.x; voxel_x++) {

                tgt::ivec3 pos = tgt::ivec3(voxel_x, voxel_y, voxel_z);

                if(tgt::hor(tgt::lessThan(pos, llf)) || tgt::hor(tgt::greaterThanEqual(pos, urb)))
                    outputVolume->setVoxelFloat(0.f, pos);
                else {
                    tgt::ivec3 oldPos = pos - llf;
                    outputVolume->setVoxelFloat(inputVolume->getVoxelFloat(oldPos), pos);
                }
            }
        }
    }

    // assign computed volume to outport
    if (outputVolume)
        outport_.setData(new VolumeHandle(outputVolume));
    else
        outport_.setData(0);
}

}   // namespace
