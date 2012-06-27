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

#include "voreen/modules/base/processors/volume/volumesubset.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/gradient.h"

namespace voreen {

using tgt::vec3;
using tgt::ivec3;
using tgt::mat4;

const std::string VolumeSubSet::loggerCat_("voreen.VolumeSubSet");

VolumeSubSet::VolumeSubSet()
    : VolumeProcessor()
    , inport_(Port::INPORT, "volumehandle.input")
    , outport_(Port::OUTPORT, "volumehandle.output", 0)
    , clipRight_("rightClippingPlane", "Right clipping plane (x)", 0, 0, 100000, true)
    , clipLeft_("leftClippingPlane", "Left clipping plane (x)", 1, 1, 100000, true)
    , clipFront_("frontClippingPlane", "Front clipping plane (y)", 0, 0, 100000, true)
    , clipBack_("backClippingPlane", "Back clipping plane (y)", 1, 1, 100000, true)
    , clipBottom_("bottomClippingPlane", "Bottom clipping plane (z)", 0, 0, 100000, true)
    , clipTop_("topClippingPlane", "Top clipping plane (z)", 1, 1, 100000, true)
    , preserveLocation_("preserveLocation", "Preserve location", true)
    , continuousCropping_("continuousCropping", "Continuous cropping", false)
    , button_("button", "Crop")
    , croppedDimensions_("croppedDimensions", "Cropped dimensions",
        tgt::ivec3(0), tgt::ivec3(0), tgt::ivec3(100000))
    , croppedSize_("croppedSize", "Cropped size (MB)", 0, 0, 100000)
{
    addPort(inport_);
    addPort(outport_);

    clipRight_.onChange(CallMemberAction<VolumeSubSet>(this, &VolumeSubSet::onClipRightChange));
    clipLeft_.onChange(CallMemberAction<VolumeSubSet>(this, &VolumeSubSet::onClipLeftChange));
    clipFront_.onChange(CallMemberAction<VolumeSubSet>(this, &VolumeSubSet::onClipFrontChange));
    clipBack_.onChange(CallMemberAction<VolumeSubSet>(this, &VolumeSubSet::onClipBackChange));
    clipBottom_.onChange(CallMemberAction<VolumeSubSet>(this, &VolumeSubSet::onClipBottomChange));
    clipTop_.onChange(CallMemberAction<VolumeSubSet>(this, &VolumeSubSet::onClipTopChange));
    button_.onChange(CallMemberAction<VolumeSubSet>(this, &VolumeSubSet::crop));

    addProperty(clipRight_);
    addProperty(clipLeft_);
    addProperty(clipFront_);
    addProperty(clipBack_);
    addProperty(clipBottom_);
    addProperty(clipTop_);
    addProperty(preserveLocation_);
    addProperty(continuousCropping_);
    addProperty(button_);

    addProperty(croppedDimensions_);
    addProperty(croppedSize_);

    croppedDimensions_.setWidgetsEnabled(false);
    croppedSize_.setWidgetsEnabled(false);
}

VolumeSubSet::~VolumeSubSet() {}

Processor* VolumeSubSet::create() const {
    return new VolumeSubSet();
}

std::string VolumeSubSet::getProcessorInfo() const {
    return "Crops the input volume by axis-aligned clipping planes.";
}

void VolumeSubSet::process() {
    tgtAssert(inport_.getData()->getVolume(), "no input volume");

    // adapt clipping plane properties on volume change
    if (inport_.hasChanged()) {
        adjustClipPropertiesRanges();
    }

    tgt::ivec3 inputDimensions = inport_.getData()->getVolume()->getDimensions();

    // compute size of cropped volume in each rendering pass
    tgt::ivec3 dimensions;
    dimensions.x = clipLeft_.get() - clipRight_.get() + 1;
    dimensions.y = clipBack_.get() - clipFront_.get() + 1;
    dimensions.z = clipTop_.get() - clipBottom_.get() + 1;
    croppedDimensions_.setMaxValue(inputDimensions);
    croppedDimensions_.set(dimensions);

    croppedSize_.setMaxValue(hmul(inputDimensions) * inport_.getData()->getVolume()->getBytesPerVoxel() / (1024 * 1024));
    croppedSize_.set(hmul(dimensions) * inport_.getData()->getVolume()->getBytesPerVoxel() / (1024 * 1024));

    // actually crop volume only, if continuous cropping is enabled (potentially slow)
    if (continuousCropping_.get())
        crop();
}

void VolumeSubSet::crop() {
    if (!inport_.hasData())
        return;
    Volume* inputVolume = inport_.getData()->getVolume();
    Volume* outputVolume = 0;

    tgt::ivec3 start, dimensions;
    start.x = clipRight_.get();
    start.y = clipFront_.get();
    start.z = clipBottom_.get();
    dimensions.x = clipLeft_.get() - clipRight_.get() + 1;
    dimensions.y = clipBack_.get() - clipFront_.get() + 1;
    dimensions.z = clipTop_.get() - clipBottom_.get() + 1;

    outputVolume = inputVolume->createSubset(start, dimensions);
    outputVolume->setSpacing(inputVolume->getSpacing());

    // assign computed volume to outport
    if (outputVolume) {
        if(preserveLocation_.get()) {
            mat4 m = inputVolume->getTransformation();

            vec3 cubeSize = outputVolume->getURB() - outputVolume->getLLF();
            vec3 oldSize = inputVolume->getURB() - inputVolume->getLLF();
            vec3 scale = vec3(dimensions) / vec3(inputVolume->getDimensions());
            vec3 correctSize = oldSize * scale;
            LDEBUG(cubeSize << oldSize << scale << correctSize << dimensions << inputVolume->getDimensions() << (correctSize / cubeSize));

            vec3 center = (outputVolume->getURB() - outputVolume->getLLF()) / 2.0f;
            vec3 relCenter = (vec3(start) + (vec3(dimensions) / 2.0f)) / vec3(inputVolume->getDimensions());
            vec3 correctCenter = ((vec3(1.0f) - relCenter) * inputVolume->getLLF()) + (relCenter * inputVolume->getURB());
            LDEBUG(correctCenter << center);

            m = m * mat4::createTranslation(correctCenter);
            m = m * mat4::createScale(correctSize / cubeSize);

            outputVolume->setTransformation(m);
        }
        else {
            outputVolume->setTransformation(inputVolume->getTransformation());
        }
        VolumeHandle* vh = new VolumeHandle(outputVolume);
        outport_.setData(vh, true);
    }
    else
        outport_.setData(0, true);
}

void VolumeSubSet::deinitialize() throw (VoreenException) {
    outport_.setData(0, true);

    VolumeProcessor::deinitialize();
}

void VolumeSubSet::onClipRightChange() {
    if (clipRight_.get() >= clipLeft_.get())
        clipLeft_.set(clipRight_.get() + 1);
}

void VolumeSubSet::onClipLeftChange() {
    if (clipRight_.get() >= clipLeft_.get())
        clipRight_.set(clipLeft_.get() - 1);
}

void VolumeSubSet::onClipFrontChange() {
    if (clipFront_.get() >= clipBack_.get())
        clipBack_.set(clipFront_.get() + 1);
}

void VolumeSubSet::onClipBackChange() {
    if (clipFront_.get() >= clipBack_.get())
        clipFront_.set(clipBack_.get() - 1);
}

void VolumeSubSet::onClipBottomChange() {
    if (clipBottom_.get() >= clipTop_.get())
        clipTop_.set(clipBottom_.get() + 1);
}

void VolumeSubSet::onClipTopChange() {
    if (clipBottom_.get() >= clipTop_.get())
        clipBottom_.set(clipTop_.get() - 1);
}

void VolumeSubSet::adjustClipPropertiesRanges() {
    tgtAssert(inport_.getData() && inport_.getData()->getVolume(), "No input volume");
    tgt::ivec3 numSlices = inport_.getData()->getVolume()->getDimensions();

    // adapt clipping plane properties to volume dimensions
    clipRight_.setMaxValue(numSlices.x-2);
    clipLeft_.setMaxValue(numSlices.x-1);

    clipFront_.setMaxValue(numSlices.y-2);
    clipBack_.setMaxValue(numSlices.y-1);

    clipBottom_.setMaxValue(numSlices.z-2);
    clipTop_.setMaxValue(numSlices.z-1);

    if ((clipRight_.get() == 0)
        && (clipLeft_.get() == 0)
        && (clipFront_.get() == 0)
        && (clipBack_.get() == 0)
        && (clipBottom_.get() == 0)
        && (clipTop_.get() == 0))
    {
        clipLeft_.set(numSlices.x-1);
        clipBack_.set(numSlices.y-1);
        clipTop_.set(numSlices.z-1);
    }

    if (clipRight_.get() > clipRight_.getMaxValue())
        clipRight_.set(clipRight_.getMaxValue());

    if (clipLeft_.get() > clipLeft_.getMaxValue())
        clipLeft_.set(clipLeft_.getMaxValue());

    if (clipFront_.get() > clipFront_.getMaxValue())
        clipFront_.set(clipFront_.getMaxValue());

    if (clipBack_.get() > clipBack_.getMaxValue())
        clipBack_.set(clipBack_.getMaxValue());

    if (clipBottom_.get() > clipBottom_.getMaxValue())
        clipBottom_.set(clipBottom_.getMaxValue());

    if (clipTop_.get() > clipTop_.getMaxValue())
        clipTop_.set(clipTop_.getMaxValue());

}

}   // namespace
