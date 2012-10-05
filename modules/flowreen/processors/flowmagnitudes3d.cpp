/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "flowmagnitudes3d.h"
#include "modules/flowreen/datastructures/volumeflow3d.h"
#include "modules/flowreen/datastructures/volumeoperatorflowmagnitude.h"

#include "voreen/core/datastructures/volume/volume.h"

namespace voreen {

FlowMagnitudes3D::FlowMagnitudes3D()
    : Processor(),
    processedVolumeHandle_(0),
    volInport_(Port::INPORT, "volumehandle.input", "Volume Input"),
    volOutport_(Port::OUTPORT, "volumehandle.output", "Volume Output", true)
{
    addPort(volInport_);
    addPort(volOutport_);
}

FlowMagnitudes3D::~FlowMagnitudes3D() {
    if ((processedVolumeHandle_ != 0) && (processedVolumeHandle_ != currentVolumeHandle_))
        delete processedVolumeHandle_;
}

void FlowMagnitudes3D::process() {
    if (volInport_.isReady() && volInport_.hasChanged()) {
        currentVolumeHandle_ = volInport_.getData();
        if (dynamic_cast<const VolumeFlow3D*>(currentVolumeHandle_->getRepresentation<VolumeRAM>()) != 0)
            calculateMagnitudes();
        else
            LERROR("process(): supplied Volume seems to contain no flow data! Cannot proceed.");
    }
}

void FlowMagnitudes3D::initialize() throw (tgt::Exception) {
    FlowreenProcessor::init();
    Processor::initialize();
}

// private methods
//

void FlowMagnitudes3D::calculateMagnitudes() {
    VolumeOperatorFlowMagnitude magnitudeOp;
    VolumeRAM* output = magnitudeOp.apply(currentVolumeHandle_);

    if ((processedVolumeHandle_ != 0)
        && processedVolumeHandle_ != currentVolumeHandle_)
    {
        delete processedVolumeHandle_;
    }

    if (output != 0)
        processedVolumeHandle_ = new Volume(output, currentVolumeHandle_);
    else
        processedVolumeHandle_ = 0;

    volOutport_.setData(processedVolumeHandle_);
}

}   // namespace
