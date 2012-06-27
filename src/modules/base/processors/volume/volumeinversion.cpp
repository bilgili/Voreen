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

#include "voreen/modules/base/processors/volume/volumeinversion.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"

namespace voreen {

VolumeInversion::VolumeInversion()
    : VolumeProcessor()
    , inport_(Port::INPORT, "volumehandle.input")
    , outport_(Port::OUTPORT, "volumehandle.output", 0)
    , enableProcessing_("enableProcessing", "Enable")
    , forceUpdate_(true)
    , volumeOwner_(false)
{
    addPort(inport_);
    addPort(outport_);

    enableProcessing_.onChange(CallMemberAction<VolumeInversion>(this, &VolumeInversion::forceUpdate));
    addProperty(enableProcessing_);
}

VolumeInversion::~VolumeInversion() {
}

Processor* VolumeInversion::create() const {
    return new VolumeInversion();
}

std::string VolumeInversion::getProcessorInfo() const {
    return "Inverts the incoming volume. "
           "This is a simple VolumeProcessor for demonstrating the concept.";
}

void VolumeInversion::process() {
    if (!enableProcessing_.get()) {
        outport_.setData(inport_.getData());
        volumeOwner_ = false;
    }
    else if (forceUpdate_ || inport_.hasChanged()) {
        invertVolume();
    }
}

void VolumeInversion::deinitialize() throw (VoreenException) {
    if (volumeOwner_) {
        outport_.deleteVolume();
        volumeOwner_ = false;
    }

    VolumeProcessor::deinitialize();
}

// private methods
//
void VolumeInversion::forceUpdate() {
    forceUpdate_ = true;
}

void VolumeInversion::invertVolume() {
    VolumeHandle* handle = inport_.getData();
    tgtAssert(handle, "Inport has no data");

    forceUpdate_ = false;

    if (volumeOwner_)
        outport_.deleteVolume();

    if (handle->getVolume()) {
        Volume* v = handle->getVolume()->clone();
        VolumeOperatorInvert invertOp;
        invertOp.apply<void>(v);
        outport_.setData(new VolumeHandle(v));
        volumeOwner_ = true;
    }
    else {
        outport_.setData(0);
        volumeOwner_ = false;
    }
}

}   // namespace
