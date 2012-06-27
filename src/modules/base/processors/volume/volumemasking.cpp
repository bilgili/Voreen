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

#include "voreen/modules/base/processors/volume/volumemasking.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"

namespace voreen {

VolumeMasking::VolumeMasking()
    : VolumeProcessor()
    , inport_(Port::INPORT, "volumehandle.input")
    , outport_(Port::OUTPORT, "volumehandle.output", 0)
    , enableProcessingProp_("enableProcessing", "Enable")
    , maskFunction_("maskFunction", "Masking function")
    , passedVoxelAction_("passedVoxelAction", "Passed voxels")
    , forceUpdate_(true)
    , volumeOwner_(false)
{
    addPort(inport_);
    addPort(outport_);

    passedVoxelAction_.addOption("maxIntensity", "Assign Max Intensity");
    passedVoxelAction_.addOption("passThrough", "Pass Through");

    enableProcessingProp_.onChange(CallMemberAction<VolumeMasking>(this, &VolumeMasking::forceUpdate));
    maskFunction_.onChange(CallMemberAction<VolumeMasking>(this, &VolumeMasking::forceUpdate));
    passedVoxelAction_.onChange(CallMemberAction<VolumeMasking>(this, &VolumeMasking::forceUpdate));

    addProperty(enableProcessingProp_);
    addProperty(maskFunction_);
    addProperty(passedVoxelAction_);
}

VolumeMasking::~VolumeMasking() {
}

Processor* VolumeMasking::create() const {
    return new VolumeMasking();
}

std::string VolumeMasking::getProcessorInfo() const {
    return "Masks the incoming volume based on the masking function, "
           "such that voxels are discarded, if the masking function "
           "assigns zero opacity to their intensities.";
}

void VolumeMasking::process() {

    maskFunction_.setVolumeHandle(inport_.getData());
    LGL_ERROR;

    if (!enableProcessingProp_.get()) {
        outport_.setData(inport_.getData(), volumeOwner_);
        volumeOwner_ = false;
    }
    else if (forceUpdate_ || inport_.hasChanged()) {
        maskVolume();
    }
}

void VolumeMasking::deinitialize() throw (VoreenException) {
    outport_.setData(0, volumeOwner_);

    VolumeProcessor::deinitialize();
}

// private methods
//

void VolumeMasking::forceUpdate() {
    forceUpdate_ = true;
}

void VolumeMasking::maskVolume() {
    tgtAssert(inport_.hasData(), "Inport has not data");

    forceUpdate_ = false;

    if (inport_.getData()->getVolume()) {
        Volume* v = inport_.getData()->getVolume()->clone();
        tgt::Texture* maskTexture = maskFunction_.get()->getTexture();
        maskTexture->downloadTexture();
        const int maskTexDim = maskTexture->getDimensions().x;

        // apply masking
        if (passedVoxelAction_.isSelected("maxIntensity")) {
            for (size_t i=0; i<v->getNumVoxels(); i++) {
                float intensity = v->getVoxelFloat(i);
                int alpha = maskTexture->texel< tgt::Vector4<uint8_t> >(static_cast<size_t>(intensity*(maskTexDim-1))).a;
                v->setVoxelFloat(alpha == 0 ? 0.f : 1.f, i);
                //progressBar_->setProgress(static_cast<float>(i) / static_cast<float>(v->getNumVoxels()));
            }
        }
        else if (passedVoxelAction_.isSelected("passThrough")) {
            for (size_t i=0; i<v->getNumVoxels(); i++) {
                float intensity = v->getVoxelFloat(i);
                int alpha = maskTexture->texel< tgt::Vector4<uint8_t> >(static_cast<size_t>(intensity*(maskTexDim-1))).a;
                if (alpha == 0)
                    v->setVoxelFloat(0.f, i);
            }
        }

        outport_.setData(new VolumeHandle(v), volumeOwner_);
        volumeOwner_ = true;
    }
    else {
        outport_.setData(0, volumeOwner_);
        volumeOwner_ = false;
    }
}

}   // namespace
