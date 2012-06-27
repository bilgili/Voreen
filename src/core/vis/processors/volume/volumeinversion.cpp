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

#include "voreen/core/vis/processors/volume/volumeinversion.h"
#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/volume/volumeoperator.h"

namespace voreen {

VolumeInversion::VolumeInversion()
    : VolumeProcessor(),
    inputVolumeHandle_(0),
    processedVolumeHandle_(0),
    enableProcessingProp_("enableProcessing", "enable: "),
    forceUpdate_(true),
    inport_(Port::INPORT, "volumehandle.input"),
    outport_(Port::OUTPORT, "volumehandle.output", 0)

{
    addPort(inport_);
    addPort(outport_);
    enableProcessingProp_.onChange(
        CallMemberAction<VolumeInversion>(this, &VolumeInversion::forceUpdate));
    addProperty(enableProcessingProp_);
}

VolumeInversion::~VolumeInversion() {
    delete processedVolumeHandle_;
}

const std::string VolumeInversion::getProcessorInfo() const {
    return std::string("Inverts the incoming volume. This is a simple "
                       "VolumeProcessor for demonstrating the concept.");
}

void VolumeInversion::process() {
    if(!inport_.getData() || !outport_.isConnected())
        return;

    inputVolumeHandle_ = inport_.getData();
    if (enableProcessingProp_.get() == false) {
        outport_.setData(inputVolumeHandle_);
        return;
    }

    if ((inputVolumeHandle_) && ((forceUpdate_ == true) || (inport_.hasChanged())))
        invertVolume();
}

// private methods
//

void VolumeInversion::forceUpdate() {
    forceUpdate_ = true;
}

void VolumeInversion::invertVolume() {
    forceUpdate_ = false;

    if (inputVolumeHandle_ == 0)
        return;

    Volume* v = inputVolumeHandle_->getVolume()->clone();

    VolumeOperatorInvert invertOp;
    invertOp(v);

    delete processedVolumeHandle_;
    processedVolumeHandle_ = new VolumeHandle(v, 0.0f);
    outport_.setData(processedVolumeHandle_);
}

}   // namespace
