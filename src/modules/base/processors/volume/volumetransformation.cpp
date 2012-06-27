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

#include "voreen/modules/base/processors/volume/volumetransformation.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

namespace voreen {

const std::string VolumeTransformation::loggerCat_("voreen.VolumeTransformation");

VolumeTransformation::VolumeTransformation()
    : VolumeProcessor(),
    inport_(Port::INPORT, "volumehandle.input"),
    outport_(Port::OUTPORT, "volumehandle.output", 0),
    voxel2Cube_("voxel2Cube", "Voxel- to cube-matrix transformation", false),
    transformMatrix_("transformMatrix", "Transformation Matrix", tgt::mat4::identity, tgt::mat4(-200.0), tgt::mat4(200.0))
{
    addPort(inport_);
    addPort(outport_);

    addProperty(transformMatrix_);
    addProperty(voxel2Cube_);
}

VolumeTransformation::~VolumeTransformation() {
}

Processor* VolumeTransformation::create() const {
    return new VolumeTransformation();
}

std::string VolumeTransformation::getProcessorInfo() const {
    return "Transforms the input volume's position and orientation by "
           "modifying its transformation matrix. "
           "Note: The volume data is copied by this operation.";
}

void VolumeTransformation::process() {

    Volume* inputVolume = inport_.getData()->getVolume();
    Volume* outputVolume = inputVolume->clone();

    tgt::mat4 cubeToVoxel = tgt::mat4::identity;
    if (voxel2Cube_.get()) {
        // the incoming matrix is specified as voxel2world matrix
        // we generate a cube2world matrix, which transforms the
        // proxy geometry in world space. therefore, we generate a
        // cube2voxel matrix, which is multiplied with the voxel2world
        // matrix. (voxel -> cube -> world)

        tgt::vec3 cubeSize = inputVolume->getCubeSize();
        tgt::ivec3 dimensions = inputVolume->getDimensions();
        unsigned int maxDim = std::max(dimensions.x, std::max(dimensions.y, dimensions.z));

        // TODO: consider spacing

        tgt::mat4 moveToOrigin = tgt::mat4::createTranslation(cubeSize / 2.0f);
        tgt::mat4 scaleByMaxDim = tgt::mat4::createScale(tgt::vec3(0.5f * static_cast<float>(maxDim)));
        cubeToVoxel = scaleByMaxDim * moveToOrigin;
    }
    outputVolume->setTransformation(transformMatrix_.get() * inputVolume->getTransformation() * cubeToVoxel);

    // cleanup and set data
    outport_.setData(new VolumeHandle(outputVolume), true);
}

void VolumeTransformation::deinitialize() throw (VoreenException) {
    outport_.deleteVolume();

    VolumeProcessor::deinitialize();
}

}   // namespace
