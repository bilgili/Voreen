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

#include "voreen/modules/base/processors/datasource/imagesource.h"

#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/voreenapplication.h"

#include "tgt/texturemanager.h"
#include "tgt/gpucapabilities.h"

namespace voreen {

const std::string ImageSource::loggerCat_("voreen.ImageSource");

ImageSource::ImageSource()
    : RenderProcessor(),
      outport_(Port::OUTPORT, "image.out"),
      imageFile_("imageFile", "Image File", "Image File", VoreenApplication::app()->getDataPath()),
      clearImage_("clearButton", "Clear Image"),
      imageSize_("imageSize", "Image Size", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(1 << 12), VALID),
      texture_(0),
      shader_(0),
      textureOwner_(false),
      forceReload_(false)
{
    addPort(outport_);

    clearImage_.onChange(CallMemberAction<ImageSource>(this, &ImageSource::clearImage));
    imageSize_.setWidgetsEnabled(false);
    addProperty(imageFile_);
    addProperty(clearImage_);
    addProperty(imageSize_);
}

ImageSource::~ImageSource() {
}

std::string ImageSource::getProcessorInfo() const {
    return "Loads an image file from disk and provides it as rendering. "
           "The outport dimensions are determined by the image dimensions.";
}

Processor* ImageSource::create() const {
    return new ImageSource();
}

void ImageSource::beforeProcess() {

    // perform enforced reload
    if (forceReload_) {
        loadImage(imageFile_.get());
        LGL_ERROR;
    }

    // resize port to image dimensions, if image is present
    if (texture_) {
        outport_.resize(texture_->getDimensions().xy());
        LGL_ERROR;
    }
}

void ImageSource::process() {

    // clear output
    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    // no image to render
    if (!texture_) {
        outport_.invalidateResult();
        outport_.deactivateTarget();
        LGL_ERROR;
        return;
    }

    // activate shader
    shader_->activate();

    // set common uniforms
    setGlobalShaderParameters(shader_);

    // bind image texture
    glActiveTexture(GL_TEXTURE0);
    texture_->bind();
    LGL_ERROR;

    // pass texture parameters to the shader
    shader_->setUniform("colorTex_", 0);
    shader_->setIgnoreUniformLocationError(true);
    shader_->setUniform("texParams_.dimensions_", tgt::vec2(texture_->getDimensions().xy()));
    shader_->setUniform("texParams_.dimensionsRCP_", tgt::vec2(1.f) / tgt::vec2(texture_->getDimensions().xy()));
    shader_->setIgnoreUniformLocationError(false);

    // execute the shader
    renderQuad();
    LGL_ERROR;

    // clean up
    shader_->deactivate();
    outport_.deactivateTarget();
    LGL_ERROR;
}

void ImageSource::initialize() throw (VoreenException) {
    RenderProcessor::initialize();

    shader_ = ShdrMgr.loadSeparate("passthrough.vert", "copyimage.frag",
        generateHeader() + "#define NO_DEPTH_TEX\n", false, false);

    if (!shader_) {
        LERROR("Failed to load shaders");
        initialized_ = false;
        throw VoreenException("Failed to load shader");
    }

    loadImage(imageFile_.get());
}

void ImageSource::deinitialize() throw (VoreenException) {
    ShdrMgr.dispose(shader_);
    shader_ = 0;
    clearImage();

    RenderProcessor::deinitialize();
}

void ImageSource::loadImage(const std::string& fname) {

    if (!isInitialized()) {
        LWARNING("loadImage(): not initialized");
        return;
    }

    // necessary since the passed string reference might be changed during clearImage/invalidate,
    std::string filename = fname;

    // clear image and check for empty filename
    if (texture_) {
        clearImage();
        LGL_ERROR;
    }
    if (filename.empty())
        return;

    // load image as texture
    texture_ = TexMgr.load(filename, tgt::Texture::LINEAR,
        false, false, true, false, !GpuCaps.isNpotSupported());
    LGL_ERROR;
    if (texture_) {
        imageFile_.set(filename);
        imageSize_.set(texture_->getDimensions().xy());
        textureOwner_ = true;
        LINFO("Loaded image with dimensions " << texture_->getDimensions().xy() << " from file " << filename);
    }
    else {
        LWARNING("Failed to load image: " << filename);
        imageFile_.set("");
        imageSize_.set(tgt::ivec2(0));
    }

    LGL_ERROR;
    invalidate();
}

void ImageSource::clearImage() {

    if (!isInitialized()) {
        LWARNING("clearImage(): not initialized");
        return;
    }

    LGL_ERROR;
    if (texture_) {
        if (textureOwner_)
            delete texture_;
        texture_ = 0;
        LGL_ERROR;
    }
    textureOwner_ = false;
    imageFile_.set("");
    imageSize_.set(tgt::ivec2(0));
}

void ImageSource::setImage(tgt::Texture* texture) {

    if (!isInitialized()) {
        LWARNING("loadImage(): not initialized");
        return;
    }

    clearImage();
    texture_ = texture;
    textureOwner_ = false;
    if (texture_) {
        imageFile_.set(texture_->getName());
        imageSize_.set(texture_->getDimensions().xy());
    }
    invalidate();
}

const tgt::Texture* ImageSource::getImage() const {
    return texture_;
}

void ImageSource::invalidate(int inv) {

    if (!isInitialized())
        return;

    if (!texture_ || (imageFile_.get() != texture_->getName())) {
        loadImage(imageFile_.get());
    }

    RenderProcessor::invalidate(inv);
}

} // namespace
