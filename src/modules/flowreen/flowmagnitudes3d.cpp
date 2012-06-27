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

#include "voreen/modules/flowreen/flowmagnitudes3d.h"
#include "voreen/modules/flowreen/volumeflow3d.h"
#include "voreen/modules/flowreen/volumeoperatorflowmagnitude.h"

#include "voreen/core/datastructures/volume/volumehandle.h"

namespace voreen {

FlowMagnitudes3D::FlowMagnitudes3D()
    : Processor(),
    processedVolumeHandle_(0),
    volInport_(Port::INPORT, "volumehandle.input"),
    volOutport_(Port::OUTPORT, "volumehandle.output", true)
{
    addPort(volInport_);
    addPort(volOutport_);
    volOutport_.setData(processedVolumeHandle_);
}

FlowMagnitudes3D::~FlowMagnitudes3D() {
    if ((processedVolumeHandle_ != 0) && (processedVolumeHandle_ != currentVolumeHandle_))
        delete processedVolumeHandle_;
}

std::string FlowMagnitudes3D::getProcessorInfo() const {
    return std::string("Calculates the magnitudes of the incomming Flow Volume and \
outputs them into a new Volume which can be used as a normal 3D texture.");
}

void FlowMagnitudes3D::process() {
    if (volInport_.isReady() && volInport_.hasChanged()) {
        currentVolumeHandle_ = volInport_.getData();
        if (dynamic_cast<VolumeFlow3D*>(currentVolumeHandle_->getVolume()) != 0)
            calculateMagnitudes();
        else
            LERROR("process(): supplied VolumeHandle seems to contain no flow data! Cannot proceed.");
    }
}

void FlowMagnitudes3D::initialize() throw (VoreenException) {
    FlowreenProcessor::init();
    Processor::initialize();
}

// private methods
//

void FlowMagnitudes3D::calculateMagnitudes() {
    VolumeOperatorFlowMagnitude magnitudeOp;
    Volume* input = currentVolumeHandle_->getVolume();
    Volume* output = magnitudeOp.apply<VolumeUInt8*>(input);

    if ((processedVolumeHandle_ != 0)
        && processedVolumeHandle_ != currentVolumeHandle_)
    {
            delete processedVolumeHandle_;
    }

    if (output != 0)
        processedVolumeHandle_ = new VolumeHandle(output, 0.0f);
    else
        processedVolumeHandle_ = 0;
    volOutport_.setData(processedVolumeHandle_);
}

}   // namespace
