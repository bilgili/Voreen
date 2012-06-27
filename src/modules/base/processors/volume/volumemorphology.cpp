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

#include "voreen/modules/base/processors/volume/volumemorphology.h"

#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"

#include "tgt/math.h"

namespace voreen {

const std::string VolumeMorphology::loggerCat_("voreen.VolumeMorphology");

VolumeMorphology::VolumeMorphology()
    : VolumeProcessor()
    , inport_(Port::INPORT, "volumehandle.input")
    , outport_(Port::OUTPORT, "volumehandle.output", 0)
    , enableProcessing_("enableProcessing", "Enable")
    , morphologicOperator_("morphologicOperator", "Operator")
    , kernelSize_("kernelSize", "Kernel Size")
    , forceUpdate_(true)
    , volumeOwner_(false)
{
    addPort(inport_);
    addPort(outport_);

    morphologicOperator_.addOption("dilation", "Dilation");
    morphologicOperator_.addOption("erosion", "Erosion");

    kernelSize_.addOption("3",  "3x3x3",    3);
    kernelSize_.addOption("5",  "5x5x5",    5);
    kernelSize_.addOption("7",  "7x7x7",    7);
    kernelSize_.addOption("9",  "9x9x9",    9);
    kernelSize_.addOption("15", "15x15x15", 15);
    kernelSize_.addOption("25", "25x25x25", 25);
    kernelSize_.addOption("35", "35x35x35", 35);
    kernelSize_.addOption("45", "45x45x45", 45);

    enableProcessing_.onChange(CallMemberAction<VolumeMorphology>(this, &VolumeMorphology::forceUpdate));
    morphologicOperator_.onChange(CallMemberAction<VolumeMorphology>(this, &VolumeMorphology::forceUpdate));
    kernelSize_.onChange(CallMemberAction<VolumeMorphology>(this, &VolumeMorphology::forceUpdate));

    addProperty(enableProcessing_);
    addProperty(morphologicOperator_);
    addProperty(kernelSize_);
}

VolumeMorphology::~VolumeMorphology() {
}

Processor* VolumeMorphology::create() const {
    return new VolumeMorphology();
}

std::string VolumeMorphology::getProcessorInfo() const {
    return "Provides the basic morphologic operators dilation and erosion for volumetric data. ";
}

void VolumeMorphology::process() {
    if (!enableProcessing_.get()) {
        outport_.setData(inport_.getData());
        volumeOwner_ = false;
    }
    else if (forceUpdate_ || inport_.hasChanged()) {
        applyOperator();
    }
}

void VolumeMorphology::deinitialize() throw (VoreenException) {
    if (volumeOwner_) {
        outport_.deleteVolume();
        volumeOwner_ = false;
    }
    VolumeProcessor::deinitialize();
}

// private methods
//

void VolumeMorphology::forceUpdate() {
    forceUpdate_ = true;
}

void VolumeMorphology::applyOperator() {
    tgtAssert(inport_.hasData(), "Inport has no data");

    forceUpdate_ = false;

    if (volumeOwner_)
        delete outport_.getData();

    if (inport_.getData()->getVolume()) {
        const Volume* input = inport_.getData()->getVolume();
        Volume* transformed = input->clone();

        if (morphologicOperator_.isSelected("dilation")) {
            VolumeOperatorDilation volOpDilation(kernelSize_.getValue());
            volOpDilation.apply<void>(transformed);
        }
        else if (morphologicOperator_.isSelected("erosion")) {
            VolumeOperatorErosion volOpErosion(kernelSize_.getValue());
            volOpErosion.apply<void>(transformed);
        }
        else {
            LERROR("Unknown operator: " << morphologicOperator_.get());
        }

        outport_.setData(new VolumeHandle(transformed));
        volumeOwner_ = true;
    }
    else {
        outport_.setData(0);
        volumeOwner_ = false;
    }
}
}   // namespace
