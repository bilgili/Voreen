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

#include "volumecrop.h"
#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorsubset.h"

namespace voreen {

using tgt::vec3;
using tgt::ivec3;
using tgt::mat4;

const std::string VolumeCrop::loggerCat_("voreen.base.VolumeCrop");

VolumeCrop::VolumeCrop()
    : CachingVolumeProcessor()
    , inport_(Port::INPORT, "volumehandle.input", "Volume Input")
    , outport_(Port::OUTPORT, "volumehandle.output", "Volume Output", false)
    , clipRight_("rightClippingPlane", "Right Clip Plane (x)", 0, 0, 100000)
    , clipLeft_("leftClippingPlane", "Left Clip Plane (x)", 1, 0, 100000)
    , clipFront_("frontClippingPlane", "Front Clip Plane (y)", 0, 0, 100000)
    , clipBack_("backClippingPlane", "Back Clip Plane (y)", 1, 0, 100000)
    , clipBottom_("bottomClippingPlane", "Bottom Clip Plane (z)", 0, 0, 100000)
    , clipTop_("topClippingPlane", "Top Clip Plane (z)", 1, 0, 100000)
    , continuousCropping_("continuousCropping", "Continuous Cropping", false)
    , button_("button", "Crop")
    , croppedDimensions_("croppedDimensions", "Cropped Dimensions",
        tgt::ivec3(0), tgt::ivec3(0), tgt::ivec3(100000))
    , croppedSize_("croppedSize", "Cropped Size (MB)", 0, 0, 100000)
{
    addPort(inport_);
    addPort(outport_);

    clipRight_.onChange(CallMemberAction<VolumeCrop>(this, &VolumeCrop::onClipRightChange));
    clipLeft_.onChange(CallMemberAction<VolumeCrop>(this, &VolumeCrop::onClipLeftChange));
    clipFront_.onChange(CallMemberAction<VolumeCrop>(this, &VolumeCrop::onClipFrontChange));
    clipBack_.onChange(CallMemberAction<VolumeCrop>(this, &VolumeCrop::onClipBackChange));
    clipBottom_.onChange(CallMemberAction<VolumeCrop>(this, &VolumeCrop::onClipBottomChange));
    clipTop_.onChange(CallMemberAction<VolumeCrop>(this, &VolumeCrop::onClipTopChange));
    button_.onChange(CallMemberAction<VolumeCrop>(this, &VolumeCrop::crop));

    addProperty(clipRight_);
    addProperty(clipLeft_);
    addProperty(clipFront_);
    addProperty(clipBack_);
    addProperty(clipBottom_);
    addProperty(clipTop_);
    addProperty(continuousCropping_);
    addProperty(button_);

    addProperty(croppedDimensions_);
    addProperty(croppedSize_);

    croppedDimensions_.setWidgetsEnabled(false);
    croppedSize_.setWidgetsEnabled(false);
}

Processor* VolumeCrop::create() const {
    return new VolumeCrop();
}

void VolumeCrop::process() {
    tgtAssert(inport_.getData(), "no input volume");

    // adapt clipping plane properties on volume change

    tgt::svec3 inputDimensions = inport_.getData()->getDimensions();

    // compute size of cropped volume in each rendering pass
    tgt::svec3 dimensions;
    dimensions.x = clipLeft_.get() - clipRight_.get() + 1;
    dimensions.y = clipBack_.get() - clipFront_.get() + 1;
    dimensions.z = clipTop_.get() - clipBottom_.get() + 1;
    croppedDimensions_.setMaxValue(inputDimensions);
    croppedDimensions_.set(dimensions);

    size_t bpp = static_cast<size_t>(inport_.getData()->getBytesPerVoxel());
    size_t inputRamSize = hmul(inputDimensions) * bpp / (1024 * 1024);
    size_t croppedRamSize = hmul(dimensions) * bpp / (1024 * 1024);
    croppedSize_.setMaxValue(static_cast<int>(inputRamSize));
    croppedSize_.set(static_cast<int>(croppedRamSize));

    // actually crop volume only, if continuous cropping is enabled (potentially slow)
    if (continuousCropping_.get())
        crop();
}

void VolumeCrop::crop() {
    if (!inport_.hasData())
        return;

    tgt::ivec3 start, dimensions;
    start.x = clipRight_.get();
    start.y = clipFront_.get();
    start.z = clipBottom_.get();
    dimensions.x = clipLeft_.get() - clipRight_.get() + 1;
    dimensions.y = clipBack_.get() - clipFront_.get() + 1;
    dimensions.z = clipTop_.get() - clipBottom_.get() + 1;

    Volume* outputVolume = VolumeOperatorSubset::APPLY_OP(inport_.getData(), start, dimensions, this);

    // assign computed volume to outport
    outport_.setData(outputVolume);
}

void VolumeCrop::onClipRightChange() {
    if (clipRight_.get() >= clipLeft_.get())
        clipLeft_.set(clipRight_.get());
}

void VolumeCrop::onClipLeftChange() {
    if (clipRight_.get() >= clipLeft_.get())
        clipRight_.set(clipLeft_.get());
}

void VolumeCrop::onClipFrontChange() {
    if (clipFront_.get() >= clipBack_.get())
        clipBack_.set(clipFront_.get());
}

void VolumeCrop::onClipBackChange() {
    if (clipFront_.get() >= clipBack_.get())
        clipFront_.set(clipBack_.get());
}

void VolumeCrop::onClipBottomChange() {
    if (clipBottom_.get() >= clipTop_.get())
        clipTop_.set(clipBottom_.get());
}

void VolumeCrop::onClipTopChange() {
    if (clipBottom_.get() >= clipTop_.get())
        clipBottom_.set(clipTop_.get());
}

void VolumeCrop::adjustPropertiesToInput() {
    if(inport_.hasData()) {
        // adapt clipping plane properties to volume dimensions
        tgt::ivec3 dim = tgt::ivec3(inport_.getData()->getDimensions());

        clipRight_.setMaxValue(dim.x-1);
        clipLeft_.setMaxValue(dim.x-1);

        clipFront_.setMaxValue(dim.y-1);
        clipBack_.setMaxValue(dim.y-1);

        clipBottom_.setMaxValue(dim.z-1);
        clipTop_.setMaxValue(dim.z-1);
    }
}

}   // namespace
