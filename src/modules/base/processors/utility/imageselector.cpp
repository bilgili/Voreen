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

#include "voreen/modules/base/processors/utility/imageselector.h"

#include "voreen/core/datastructures/imagesequence.h"
#include "voreen/core/interaction/mwheelnumpropinteractionhandler.h"

namespace voreen {

const std::string ImageSelector::loggerCat_("voreen.ImageSelector");

ImageSelector::ImageSelector()
    : RenderProcessor(),
      inport_(Port::INPORT, "imagesequence.in", 0),
      outport_(Port::OUTPORT, "image.out", 0),
      imageID_("imageID", "Selected Image", 0, 0, 100),
      wheelHandler_("wheelHandler.imageCycling", "Image Cycling", &imageID_),
      shader_(0)
{
    addPort(inport_);
    addPort(outport_);

    addProperty(imageID_);
    addInteractionHandler(&wheelHandler_);
}

std::string ImageSelector::getProcessorInfo() const {
    return "Selects a single image from the input image sequence and renders it to its outport.";
}

Processor* ImageSelector::create() const {
    return new ImageSelector();
}

void ImageSelector::process() {

    if (inport_.hasChanged())
        adjustToImageSequence();

    if ((imageID_.get() < 0) || (imageID_.get() >= static_cast<int>(inport_.getData()->size()))) {
        outport_.invalidateResult();
        return;
    }

    // get texture to render
    tgt::Texture* tex = inport_.getData()->at(imageID_.get());
    tgtAssert(tex, "Texture is null");

    // adjust outport size to texture dimensions
    outport_.resize(tex->getDimensions().xy());

    // clear outport
    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    // activate shader
    shader_->activate();

    // set common uniforms
    setGlobalShaderParameters(shader_);

    // bind image texture
    glActiveTexture(GL_TEXTURE0);
    tex->bind();

    // pass texture parameters to the shader
    shader_->setUniform("colorTex_", 0);
    shader_->setIgnoreUniformLocationError(true);
    shader_->setUniform("texParams_.dimensions_", tgt::vec2(tex->getDimensions().xy()));
    shader_->setUniform("texParams_.dimensionsRCP_", tgt::vec2(1.f) / tgt::vec2(tex->getDimensions().xy()));
    shader_->setIgnoreUniformLocationError(false);

    // execute the shader
    renderQuad();
    LGL_ERROR;

    // clean up
    shader_->deactivate();
    outport_.deactivateTarget();
    LGL_ERROR;
}

void ImageSelector::initialize() throw (VoreenException) {
    RenderProcessor::initialize();

    shader_ = ShdrMgr.loadSeparate("passthrough.vert", "copyimage.frag",
        generateHeader() + "#define NO_DEPTH_TEX\n", false, false);

    if (!shader_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shader!");
    }

    adjustToImageSequence();
}

void ImageSelector::deinitialize() throw (VoreenException) {
    ShdrMgr.dispose(shader_);
    shader_ = 0;

    RenderProcessor::deinitialize();
}

void ImageSelector::invalidate(int inv) {
    RenderProcessor::invalidate(inv);
}

void ImageSelector::adjustToImageSequence() {

    ImageSequence* sequence = inport_.getData();
    int max = sequence ? static_cast<int>(sequence->size()) : 0;

    if (sequence && !sequence->empty() /*&& (imageID_.get() < max)*/) {
        // adjust max id to size of collection
        if (imageID_.getMaxValue() != max - 1) {
            imageID_.setMaxValue(max - 1);
        }
        if (imageID_.get() > imageID_.getMaxValue())
            imageID_.set(imageID_.getMaxValue());

        tgtAssert((imageID_.get() >= 0) && (imageID_.get() < max), "Invalid image index");
    }
    else {
        if (max > 0)
            max -= 1;

        // If the imagesequence is smaller than the previous one, the maximum value
        // must be adjusted and the new value should be set.
        // The sequence is 0 when deserializing the workspace, so that we must
        // not set value in that case, because it is the just deserialized one!
        //
        imageID_.setMaxValue(max);
    }

}

} // namespace
