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

#include "voreen/core/vis/processors/volume/vectormagnitude.h"
#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/volume/gradient.h"

namespace voreen {

VectorMagnitude::VectorMagnitude()
    : VolumeProcessor(),
    processedVolumeHandle_(0),
    inport_(Port::INPORT, "volumehandle.input"),
    outport_(Port::OUTPORT, "volumehandle.output", 0)

{
    addPort(inport_);
    addPort(outport_);
}

VectorMagnitude::~VectorMagnitude() {
    delete processedVolumeHandle_;
}

const std::string VectorMagnitude::getProcessorInfo() const {
    return std::string("");
}

void VectorMagnitude::process() {
    if(!inport_.getData() || !outport_.isConnected())
        return;

    Volume* v = inport_.getData()->getVolume();
    Volume* result = 0;
    result = calcGradientMagnitudes<uint8_t>(v);

    if(processedVolumeHandle_) {
        delete processedVolumeHandle_;
        processedVolumeHandle_ = 0;
    }

    if(result)
        processedVolumeHandle_ = new VolumeHandle(result, 0.0f);
    outport_.setData(processedVolumeHandle_);
}

}   // namespace
