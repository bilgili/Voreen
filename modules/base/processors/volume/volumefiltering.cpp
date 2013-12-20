/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#include "volumefiltering.h"

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatormedian.h"

#include "tgt/tgt_math.h"

namespace voreen {

const std::string VolumeFiltering::loggerCat_("voreen.VolumeFiltering");

VolumeFiltering::VolumeFiltering()
    : CachingVolumeProcessor()
    , inport_(Port::INPORT, "volumehandle.input", "Volume Input")
    , outport_(Port::OUTPORT, "volumehandle.output", "Volume Output", false)
    , enableProcessing_("enableProcessing", "Enable")
    , filteringOperator_("filteringOperator", "Operator")
    , kernelSize_("kernelSize", "Kernel Size")
    , forceUpdate_(true)
{
    addPort(inport_);
    addPort(outport_);

    filteringOperator_.addOption("median", "Median");
    //filteringOperator_.addOption("erosion", "Erosion");

    kernelSize_.addOption("3",  "3x3x3",    3);
    kernelSize_.addOption("5",  "5x5x5",    5);
    kernelSize_.addOption("7",  "7x7x7",    7);
    kernelSize_.addOption("9",  "9x9x9",    9);
    kernelSize_.addOption("15", "15x15x15", 15);

    enableProcessing_.onChange(CallMemberAction<VolumeFiltering>(this, &VolumeFiltering::forceUpdate));
    filteringOperator_.onChange(CallMemberAction<VolumeFiltering>(this, &VolumeFiltering::forceUpdate));
    kernelSize_.onChange(CallMemberAction<VolumeFiltering>(this, &VolumeFiltering::forceUpdate));

    addProperty(enableProcessing_);
    addProperty(filteringOperator_);
    addProperty(kernelSize_);
}

VolumeFiltering::~VolumeFiltering() {
}

Processor* VolumeFiltering::create() const {
    return new VolumeFiltering();
}

void VolumeFiltering::process() {
    if (!enableProcessing_.get()) {
        outport_.setData(const_cast<VolumeBase*>(inport_.getData()), false);
    }
    else if (forceUpdate_ || inport_.hasChanged()) {
        applyOperator();
    }
}

// private methods
//

void VolumeFiltering::forceUpdate() {
    forceUpdate_ = true;
}

void VolumeFiltering::applyOperator() {
    tgtAssert(inport_.hasData(), "Inport has no data");

    forceUpdate_ = false;

    if (inport_.getData()->getRepresentation<VolumeRAM>()) {
        const VolumeBase* input = inport_.getData();
        Volume* transformed = 0; // = input->clone();

        if (filteringOperator_.isSelected("median")) {
            //volOpMedian.setProgressBar(progressBar_);
            transformed = VolumeOperatorMedian::APPLY_OP(input, kernelSize_.getValue(), this);
        }
        //else if (filteringOperator_.isSelected("erosion")) {
            //VolumeOperatorErosion volOpErosion(kernelSize_.getValue());
            //volOpErosion(transformed);
        //}
        else {
            LERROR("Unknown operator: " << filteringOperator_.get());
        }

        outport_.setData(transformed);
    }
    else {
        outport_.setData(0);
    }
}


}   // namespace
