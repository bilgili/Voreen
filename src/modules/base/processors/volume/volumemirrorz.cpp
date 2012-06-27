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

#include "voreen/modules/base/processors/volume/volumemirrorz.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"
#include "voreen/core/datastructures/volume/gradient.h"

namespace voreen {

const std::string VolumeMirrorZ::loggerCat_("voreen.VolumeMirrorZ");

VolumeMirrorZ::VolumeMirrorZ()
    : VolumeProcessor(),
    inport_(Port::INPORT, "volumehandle.input"),
    outport_(Port::OUTPORT, "volumehandle.output", 0)
{
    addPort(inport_);
    addPort(outport_);
}

VolumeMirrorZ::~VolumeMirrorZ() {}

Processor* VolumeMirrorZ::create() const {
    return new VolumeMirrorZ();
}

std::string VolumeMirrorZ::getProcessorInfo() const {
    return std::string("Mirror volume on z axis.");
}

void VolumeMirrorZ::process() {
    Volume* inputVolume = inport_.getData()->getVolume();
    Volume* outputVolume = inputVolume->clone();

    VolumeOperatorMirrorZ mirrorZ;
    mirrorZ(outputVolume);

    // assign computed volume to outport
    if (outputVolume)
        outport_.setData(new VolumeHandle(outputVolume), true);
    else
        outport_.setData(0, true);
}

void VolumeMirrorZ::deinitialize() throw (VoreenException) {
    outport_.setData(0, true);

    VolumeProcessor::deinitialize();
}

}   // namespace
