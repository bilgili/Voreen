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

#include "volumedecomposer.h"

#include "voreen/core/datastructures/imagesequence.h"
#include "voreen/core/io/progressbar.h"
#include "tgt/texturemanager.h"
#include "tgt/gpucapabilities.h"

namespace voreen {

const std::string VolumeDecomposer::loggerCat_("voreen.base.VolumeDecomposer");

VolumeDecomposer::VolumeDecomposer()
    : RenderProcessor(),
      inport_(Port::INPORT, "volume.in", "Volume Input"),
      outport_(Port::OUTPORT, "imagesequence.out", "ImageSequence Output"),
      startSlice_("startSlice", "Start Slice", 0, 0, 1000),
      endSlice_("endSlice", "End Slice", 1000, 0, 1000),
      sliceSequence_(0)
{
    addPort(inport_);
    addPort(outport_);

    startSlice_.onChange(CallMemberAction<VolumeDecomposer>(this, &VolumeDecomposer::startSliceChanged));
    endSlice_.onChange(CallMemberAction<VolumeDecomposer>(this, &VolumeDecomposer::endSliceChanged));
    addProperty(startSlice_);
    addProperty(endSlice_);
}

VolumeDecomposer::~VolumeDecomposer() {}

Processor* VolumeDecomposer::create() const {
    return new VolumeDecomposer();
}

bool VolumeDecomposer::isReady() const {
    return (inport_.isConnected() && outport_.isConnected());
}

void VolumeDecomposer::initialize() throw (tgt::Exception) {
    RenderProcessor::initialize();
    sliceSequence_ = new ImageSequence();
}

void VolumeDecomposer::deinitialize() throw (tgt::Exception) {
    clearSliceSequence();
    delete sliceSequence_;

    RenderProcessor::deinitialize();
}

void VolumeDecomposer::process() {

    if (inport_.hasChanged())
        adjustToInputVolume();

    if (inport_.isReady()) {
        decomposeVolume();
        tgtAssert(sliceSequence_, "No slice sequence");
    }
    else {
        clearSliceSequence();
    }

    outport_.setData(sliceSequence_, false);
    outport_.invalidatePort();
}

void VolumeDecomposer::decomposeVolume() {

    tgtAssert(sliceSequence_, "No slice sequence");
    clearSliceSequence();

    if (startSlice_.get() > endSlice_.get()) {
        tgtAssert(false, "Start slice greater than end slice");
        LERROR("Start slice greater than end slice");
        return;
    }

    const VolumeBase* volumeHandle = inport_.getData();
    tgtAssert(volumeHandle, "No volume handle");
    const VolumeRAM* volume = volumeHandle->getRepresentation<VolumeRAM>();
    tgtAssert(volume, "No volume");
    //std::string volFilepath = volumeHandle->getOrigin().getPath();

    tgt::svec3 sliceDims(volume->getDimensions().x, volume->getDimensions().y, 1);

    size_t zStart = startSlice_.get();
    size_t zEnd = std::min(static_cast<size_t>(endSlice_.get()), volume->getDimensions().z-1);
    setProgress(0.f);
    for (size_t z=zStart; z<=zEnd; ++z) {
        setProgress(static_cast<float>(z-zStart) / static_cast<float>(zEnd-zStart+1));
        tgt::Texture* slice = new tgt::Texture(sliceDims, GL_LUMINANCE, GL_LUMINANCE, GL_FLOAT, tgt::Texture::LINEAR);
        for (int x=0; x<slice->getWidth(); ++x) {
            for (int y=0; y<slice->getHeight(); ++y) {
                float intensity = volume->getVoxelNormalized(x,y,z);
                slice->texel<float>(x,y) = intensity;
            }
        }
        slice->uploadTexture();
        //slice->setName(volFilepath);
        sliceSequence_->add(slice);
    }
    setProgress(1.f);

    LGL_ERROR;
}

void VolumeDecomposer::clearSliceSequence() {
    if (sliceSequence_) {
        for (size_t i=0;i<sliceSequence_->size(); ++i)
            delete (sliceSequence_->at(i));
        sliceSequence_->clear();
    }
}

void VolumeDecomposer::adjustToInputVolume() {

    if (startSlice_.get() > endSlice_.get()) {
        tgtAssert(false, "Start slice index greater than end slice index");
        LERROR("Start slice index greater than end slice index");
        return;
    }

    const VolumeBase* handle = inport_.getData();
    if (handle && handle->getRepresentation<VolumeRAM>()) {
        const VolumeRAM* volume = handle->getRepresentation<VolumeRAM>();
        int max = volume ? static_cast<int>(volume->getDimensions().z - 1) : 0;

        // adjust max id to size of collection
        if (endSlice_.getMaxValue() != max) {
            startSlice_.setMaxValue(max);
            endSlice_.setMaxValue(max);
            startSlice_.updateWidgets();
            endSlice_.updateWidgets();
        }

        // clamp slice range to new slice count
        if (startSlice_.get() > max)
            startSlice_.set(max);
        if (endSlice_.get() > max)
            endSlice_.set(max);

        tgtAssert( (startSlice_.get() >= 0) && (startSlice_.get() <= max) &&
                   (endSlice_.get() >= 0) && (endSlice_.get() <= max), "Invalid slice range");

    }
}

void VolumeDecomposer::startSliceChanged() {
    // start slice index must not be greater than end slice index
    if (startSlice_.get() > endSlice_.get())
        endSlice_.set(startSlice_.get());
}

void VolumeDecomposer::endSliceChanged() {
    // start slice index must not be greater than end slice index
    if (endSlice_.get() < startSlice_.get())
        startSlice_.set(endSlice_.get());
}


} // voreen namespace
