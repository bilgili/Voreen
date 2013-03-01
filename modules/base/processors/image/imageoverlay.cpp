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

#include "imageoverlay.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

ImageOverlay::ImageOverlay()
    : ImageProcessor("image/compositor")
    , imageInport_(Port::INPORT, "image.in", "Image Input")
    , overlayInport_(Port::INPORT, "image.overlay", "Overlay Input")
    , outport_(Port::OUTPORT, "image.out", "Image Output")
    , renderOverlay_("renderOverlay", "Render Overlay", true)
    , usePixelCoordinates_("usePixelCoordinates", "Use Pixel Coordinates", true)
    , overlayBottomLeft_("overlayBottomLeft", "Overlay Bottom Left", tgt::ivec2(10), tgt::ivec2(-4096), tgt::ivec2(4096))
    , overlayDimensions_("overlayDimensions", "Overlay Dimensions", tgt::ivec2(100), tgt::ivec2(0), tgt::ivec2(4096))
    , overlayBottomLeftRelative_("overlayBottomLeftRelative", "Overlay Bottom Left (Relative)", tgt::vec2(0.05f), tgt::vec2(-1.f), tgt::vec2(1.f))
    , overlayDimensionsRelative_("overlayDimensionsRelative", "Overlay Dimensions (Relative)", tgt::vec2(0.25f), tgt::vec2(0.f), tgt::vec2(1.f))
    , overlayOpacity_("overlayOpacity", "Overlay Opacity", 1.f)
    , renderBorder_("renderBorder", "Render Border", false)
    , borderWidth_("borderWidth", "Border Width", 1.f, 0.1f, 10.f)
    , borderColor_("borderColor", "Border Color", tgt::Color(0.f, 0.f, 0.f, 1.f))
    , copyShader_(0)
{
    borderColor_.setViews(Property::COLOR);
    addPort(imageInport_);
    addPort(overlayInport_);
    addPort(outport_);

    overlayDimensions_.onChange(CallMemberAction<ImageOverlay>(this, &ImageOverlay::overlayDimensionsChanged));
    overlayDimensionsRelative_.onChange(CallMemberAction<ImageOverlay>(this, &ImageOverlay::overlayDimensionsChanged));
    usePixelCoordinates_.onChange(CallMemberAction<ImageOverlay>(this, &ImageOverlay::overlayDimensionsChanged));

    addProperty(renderOverlay_);
    addProperty(usePixelCoordinates_);
    addProperty(overlayBottomLeft_);
    addProperty(overlayDimensions_);
    addProperty(overlayBottomLeftRelative_);
    addProperty(overlayDimensionsRelative_);
    addProperty(overlayOpacity_);
    addProperty(renderBorder_);
    addProperty(borderWidth_);
    addProperty(borderColor_);
}

Processor* ImageOverlay::create() const {
    return new ImageOverlay();
}

void ImageOverlay::initialize() throw (tgt::Exception) {
    ImageProcessor::initialize();

    copyShader_ = ShdrMgr.loadSeparate("passthrough.vert", "copyimage.frag",
        ImageProcessor::generateHeader(), false);

    overlayDimensionsChanged();
}

void ImageOverlay::deinitialize() throw (tgt::Exception) {
    ShdrMgr.dispose(copyShader_);
    copyShader_ = 0;

    ImageProcessor::deinitialize();
}

bool ImageOverlay::isReady() const {
    return (imageInport_.isReady() && outport_.isReady());
}

std::string ImageOverlay::generateHeader(const tgt::GpuCapabilities::GlVersion* version) {
    std::string header = ImageProcessor::generateHeader(version);
//    header += "#define MODE_ALPHA_BLENDING\n";
    header += "#define MODE_WEIGHTED_AVERAGE\n";
    return header;
}

void ImageOverlay::beforeProcess() {
    ImageProcessor::beforeProcess();
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
}

void ImageOverlay::process() {
    tgtAssert(outport_.isReady(), "Outport not ready");
    tgtAssert(imageInport_.isReady(), "Inport not ready");
    tgtAssert(program_ && copyShader_, "Shader missing");

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind input image to tex unit
    TextureUnit imageUnit, imageUnitDepth;
    imageInport_.bindTextures(imageUnit.getEnum(), imageUnitDepth.getEnum());

    // 1. copy input image to outport
    copyShader_->activate();
    setGlobalShaderParameters(copyShader_);
    imageInport_.setTextureParameters(copyShader_, "texParams_");
    copyShader_->setUniform("colorTex_", imageUnit.getUnitNumber());
    copyShader_->setUniform("depthTex_", imageUnitDepth.getUnitNumber());
    renderQuad();
    copyShader_->deactivate();
    LGL_ERROR;

    // 2. render overlay over copied input image (using compositor shader)
    // check, if overlay dims are greater zero
    bool dimensionsValid = ( (usePixelCoordinates_.get()  && tgt::hand(tgt::greaterThan(overlayDimensions_.get(), tgt::ivec2(0)))) ||
                             (!usePixelCoordinates_.get() && tgt::hand(tgt::greaterThan(overlayDimensionsRelative_.get(), tgt::vec2(0.f)))) );
    if (renderOverlay_.get() && overlayInport_.isReady() && dimensionsValid) {
        // bind overlay to tex unit
        TextureUnit overlayUnit;
        tgt::Texture* overlayTex = overlayInport_.getColorTexture();
        tgtAssert(overlayTex, "No overlay texture");
        overlayUnit.activate();
        overlayTex->bind();

        program_->activate();
        setGlobalShaderParameters(program_);

        // image texture parameters
        imageInport_.setTextureParameters(program_, "textureParameters0_");
        program_->setUniform("colorTex0_", imageUnit.getUnitNumber());
        program_->setUniform("depthTex0_", imageUnitDepth.getUnitNumber());
        program_->setUniform("colorTex1_", overlayUnit.getUnitNumber());
        program_->setUniform("weightingFactor_", 1.f-overlayOpacity_.get());

        // determine overlay dimensions and bottom-left in float pixel coords
        tgt::vec2 outportDim = tgt::vec2(outport_.getSize());
        tgt::vec2 overlayDim, overlayBL;
        if (usePixelCoordinates_.get()) {
            overlayDim = tgt::vec2(overlayDimensions_.get());
            overlayBL = tgt::vec2(overlayBottomLeft_.get());
        }
        else {
            overlayDim = overlayDimensionsRelative_.get() * outportDim;
            overlayBL = overlayBottomLeftRelative_.get() * outportDim;
        }

        // overlay texture matrix mapping from normalized frag coords (outport) to overlay tex coords
        tgt::mat4 overlayTexCoordMatrix = tgt::mat4::identity;
        overlayTexCoordMatrix *= tgt::mat4::createScale(tgt::vec3(outportDim / overlayDim, 0.f));
        overlayTexCoordMatrix *= tgt::mat4::createTranslation(-tgt::vec3(overlayBL / outportDim, 1.f));

        // overlay texture parameters
        bool oldIgnoreError = program_->getIgnoreUniformLocationError();
        program_->setIgnoreUniformLocationError(true);
        program_->setUniform("textureParameters1_.dimensions_",    overlayDim);
        program_->setUniform("textureParameters1_.dimensionsRCP_", tgt::vec2(1.f) / overlayDim);
        program_->setUniform("textureParameters1_.matrix_", overlayTexCoordMatrix);
        program_->setIgnoreUniformLocationError(oldIgnoreError);
        LGL_ERROR;

        // render overlay at specified position and size
        tgt::vec2 bl = 2.f*overlayBL / outportDim - 1.f;
        tgt::vec2 dim = 2.f*overlayDim / outportDim;
        glDepthFunc(GL_ALWAYS);
        glBegin(GL_QUADS);
            glVertex2f(bl.x, bl.y);
            glVertex2f(bl.x + dim.x, bl.y);
            glVertex2f(bl.x + dim.x, bl.y + dim.y);
            glVertex2f(bl.x, bl.y + dim.y);
        glEnd();
        glDepthFunc(GL_LESS);
        program_->deactivate();
        LGL_ERROR;

        // render border around overlay
        if (renderBorder_.get()) {
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glColor4fv(borderColor_.get().elem);
            glLineWidth(borderWidth_.get());
            glDepthFunc(GL_ALWAYS);
            glBegin(GL_LINE_STRIP);
                glVertex2f(bl.x, bl.y);
                glVertex2f(bl.x + dim.x, bl.y);
                glVertex2f(bl.x + dim.x, bl.y + dim.y);
                glVertex2f(bl.x, bl.y + dim.y);
            glEnd();
            glPopAttrib();
            LGL_ERROR;
        }
    }

    outport_.deactivateTarget();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

void ImageOverlay::overlayDimensionsChanged() {

    if (!isInitialized() || !outport_.isReady())
        return;

    /*if (usePixelCoordinates_.get()) {
        if (tgt::hand(tgt::greaterThan(overlayDimensions_.get(), tgt::ivec2(0))))
            overlayInport_.resize(overlayDimensions_.get());
    }
    else {
        if (tgt::hand(tgt::greaterThan(overlayDimensionsRelative_.get(), tgt::vec2(0.f))))
            overlayInport_.resize(overlayDimensionsRelative_.get() * tgt::vec2(outport_.getSize()));
    } */
}

} // namespace voreen
