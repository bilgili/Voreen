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

#include "voreen/modules/base/processors/volume/volumeresample.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"

namespace voreen {

const std::string VolumeResample::loggerCat_("voreen.VolumeResample");

VolumeResample::VolumeResample()
    : VolumeProcessor()
    , inport_(Port::INPORT, "volumehandle.input")
    , outport_(Port::OUTPORT, "volumehandle.output", 0)
    , enableProcessingProp_("enableProcessing", "Enable")
    , allowUpsampling_("allowUpsampling", "Allow Upsampling", false)
    , keepVoxelRatio_("keepVoxelSizeRatio", "Keep Voxel Ratio", true)
    , resampleDimensionX_("resampleDimensionX", "Dimension X", 1, 1, 4096)
    , resampleDimensionY_("resampleDimensionY", "Dimension Y", 1, 1, 4096)
    , resampleDimensionZ_("resampleDimensionZ", "Dimension Z", 1, 1, 4096)
    , filteringMode_("filteringMode", "Filtering")
    , forceUpdate_(true)
    , volumeOwner_(false)
    , blockSignals_(false)
{
    addPort(inport_);
    addPort(outport_);

    filteringMode_.addOption("nearest", "Nearest");
    filteringMode_.addOption("linear", "Linear");
    filteringMode_.set("linear");

    enableProcessingProp_.onChange(
        CallMemberAction<VolumeResample>(this, &VolumeResample::forceUpdate));
    resampleDimensionX_.onChange(
        CallMemberAction<VolumeResample>(this, &VolumeResample::forceUpdate));
    resampleDimensionY_.onChange(
        CallMemberAction<VolumeResample>(this, &VolumeResample::forceUpdate));
    resampleDimensionZ_.onChange(
        CallMemberAction<VolumeResample>(this, &VolumeResample::forceUpdate));
    keepVoxelRatio_.onChange(
        CallMemberAction<VolumeResample>(this, &VolumeResample::forceUpdate));
    filteringMode_.onChange(
        CallMemberAction<VolumeResample>(this, &VolumeResample::forceUpdate));

    resampleDimensionX_.onChange(
        Call1ParMemberAction<VolumeResample, int>(this, &VolumeResample::dimensionsChanged, 0));
    resampleDimensionY_.onChange(
        Call1ParMemberAction<VolumeResample, int>(this, &VolumeResample::dimensionsChanged, 1));
    resampleDimensionZ_.onChange(
        Call1ParMemberAction<VolumeResample, int>(this, &VolumeResample::dimensionsChanged, 2));

    allowUpsampling_.onChange(
        CallMemberAction<VolumeResample>(this, &VolumeResample::allowUpsamplingChanged));
    keepVoxelRatio_.onChange(
        CallMemberAction<VolumeResample>(this, &VolumeResample::keepVoxelRatioChanged));

    resampleDimensionX_.setTracking(false);
    resampleDimensionY_.setTracking(false);
    resampleDimensionZ_.setTracking(false);

    addProperty(enableProcessingProp_);
    addProperty(allowUpsampling_);
    addProperty(keepVoxelRatio_);
    addProperty(resampleDimensionX_);
    addProperty(resampleDimensionY_);
    addProperty(resampleDimensionZ_);
    addProperty(filteringMode_);
}

VolumeResample::~VolumeResample() {
}

std::string VolumeResample::getProcessorInfo() const {
    return "Resizes the input volume to the specified dimensions "
           "by using a selectable filtering mode.";
}

Processor* VolumeResample::create() const {
    return new VolumeResample();
}

void VolumeResample::process() {

    if (inport_.hasChanged())
        adjustDimensionProperties();

    if (!enableProcessingProp_.get()) {
        if (volumeOwner_)
            delete outport_.getData();
        outport_.setData(inport_.getData());
        volumeOwner_ = false;
    }
    else if (forceUpdate_ || inport_.hasChanged()) {
        resampleVolume();
    }
}

void VolumeResample::deinitialize() throw (VoreenException) {
    if (volumeOwner_) {
        outport_.deleteVolume();
        volumeOwner_ = false;
    }

    VolumeProcessor::deinitialize();
}

// private methods
//

void VolumeResample::resampleVolume() {
    tgtAssert(inport_.hasData(), "Inport has not data");
    forceUpdate_ = false;

    if (volumeOwner_)
        delete outport_.getData();

    if (inport_.getData()->getVolume()) {

        Volume::Filter filter = Volume::NEAREST;
        if (filteringMode_.get() == "linear")
            filter = Volume::LINEAR;

        tgt::ivec3 dimensions(resampleDimensionX_.get(), resampleDimensionY_.get(), resampleDimensionZ_.get());
        try {
            VolumeOperatorResample voResample(dimensions, filter);
            voResample.setProgressBar(progressBar_);
            Volume* v = voResample.apply<Volume*>(inport_.getData()->getVolume());
            outport_.setData(new VolumeHandle(v));
            volumeOwner_ = true;
        }
        catch (const std::bad_alloc&) {
            LERROR("resampleVolume(): bad allocation");
            outport_.setData(0);
            volumeOwner_ = false;
        }
    }
    else {
        outport_.setData(0);
        volumeOwner_ = false;
    }
}

void VolumeResample::adjustDimensionProperties() {
    if (!inport_.hasData() || !inport_.getData()->getVolume())
        return;

    tgt::ivec3 volDim = inport_.getData()->getVolume()->getDimensions();

    if (!allowUpsampling_.get()) {
        resampleDimensionX_.setMaxValue(volDim.x);
        resampleDimensionY_.setMaxValue(volDim.y);
        resampleDimensionZ_.setMaxValue(volDim.z);
    }
    else {
        resampleDimensionX_.setMaxValue(4096);
        resampleDimensionY_.setMaxValue(4096);
        resampleDimensionZ_.setMaxValue(4096);
    }
    resampleDimensionX_.updateWidgets();
    resampleDimensionY_.updateWidgets();
    resampleDimensionZ_.updateWidgets();

    if (resampleDimensionX_.get() == 1)
        resampleDimensionX_.set(volDim.x);
    if (resampleDimensionY_.get() == 1)
        resampleDimensionY_.set(volDim.y);
    if (resampleDimensionZ_.get() == 1)
        resampleDimensionZ_.set(volDim.z);

    resampleDimensionX_.set(tgt::clamp(resampleDimensionX_.get(), 1, resampleDimensionX_.getMaxValue()));
    resampleDimensionY_.set(tgt::clamp(resampleDimensionY_.get(), 1, resampleDimensionY_.getMaxValue()));
    resampleDimensionZ_.set(tgt::clamp(resampleDimensionZ_.get(), 1, resampleDimensionZ_.getMaxValue()));
}

void VolumeResample::forceUpdate() {
    if (blockSignals_)
        return;

    forceUpdate_ = true;
}

void VolumeResample::dimensionsChanged(int dim) {

    if (blockSignals_ || !keepVoxelRatio_.get() || !inport_.hasData() || !inport_.getData()->getVolume())
        return;

    tgt::ivec3 volDim = inport_.getData()->getVolume()->getDimensions();

    blockSignals_ = true;
    if (dim == 0) {
        int xDim = resampleDimensionX_.get();
        float yRatio = static_cast<float>(volDim.y) / volDim.x;
        float zRatio = static_cast<float>(volDim.z) / volDim.x;
        int yDim = tgt::iround(xDim * yRatio);
        int zDim = tgt::iround(xDim * zRatio);
        resampleDimensionY_.set(tgt::clamp(yDim, 1, resampleDimensionY_.getMaxValue()));
        resampleDimensionZ_.set(tgt::clamp(zDim, 1, resampleDimensionZ_.getMaxValue()));
    }
    else if (dim == 1) {
        int yDim = resampleDimensionY_.get();
        float xRatio = static_cast<float>(volDim.x) / volDim.y;
        float zRatio = static_cast<float>(volDim.z) / volDim.y;
        int xDim = tgt::iround(yDim * xRatio);
        int zDim = tgt::iround(yDim * zRatio);
        resampleDimensionX_.set(tgt::clamp(xDim, 1, resampleDimensionX_.getMaxValue()));
        resampleDimensionZ_.set(tgt::clamp(zDim, 1, resampleDimensionZ_.getMaxValue()));
    }
    else if (dim == 2) {
        int zDim = resampleDimensionZ_.get();
        float xRatio = static_cast<float>(volDim.x) / volDim.z;
        float yRatio = static_cast<float>(volDim.y) / volDim.z;
        int xDim = tgt::iround(zDim * xRatio);
        int yDim = tgt::iround(zDim * yRatio);
        resampleDimensionX_.set(tgt::clamp(xDim, 1, resampleDimensionX_.getMaxValue()));
        resampleDimensionY_.set(tgt::clamp(yDim, 1, resampleDimensionY_.getMaxValue()));
    }
    blockSignals_ = false;
}

void VolumeResample::allowUpsamplingChanged() {
    if (blockSignals_)
        return;

    adjustDimensionProperties();
}

void VolumeResample::keepVoxelRatioChanged() {
    if (blockSignals_)
        return;

    if (keepVoxelRatio_.get())
        dimensionsChanged(0);
}



}   // namespace
