/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/image/imageoverlay.h"

namespace voreen {

const std::string ImageOverlay::shadeTexUnit0_ = "shadeTex0";
const std::string ImageOverlay::depthTexUnit0_ = "depthTex0";
const std::string ImageOverlay::shadeTexUnit1_ = "shadeTex1";
const std::string ImageOverlay::depthTexUnit1_ = "depthTex1";

ImageOverlay::ImageOverlay()
    : ImageProcessor("pp_combine")
    , tex_(0)
    , textureLoaded_(false)
    , filename_("filenameAsString", "Texture", "Select texture",
        "", "*.jpg;*.bmp;*.png", FileDialogProperty::OPEN_FILE, Processor::INVALID_PROGRAM)
    , left_("left", "Position left", 0.25f)
    , top_("top", "Position top", 0.25f)
    , width_("width", "Relative width", 0.5f)
    , height_("height", "Relative height", 0.5f)
    , opacity_("opacity", "Opacity", 1.0f)
    , inport_(Port::INPORT, "image.input")
    , outport_(Port::OUTPORT, "image.output")
    , privatePort_(Port::OUTPORT, "image.tmp", false)
{

    addProperty(top_);
    addProperty(left_);
    addProperty(width_);
    addProperty(height_);
    addProperty(opacity_);

    filename_.onChange(CallMemberAction<ImageOverlay>(this, &ImageOverlay::loadTexture));
    addProperty(filename_);

    addPort(inport_);
    addPort(outport_);
    addPrivateRenderPort(&privatePort_);
}

ImageOverlay::~ImageOverlay() {
    if (tex_) {
        if (textureLoaded_) TexMgr.dispose(tex_);
        else delete tex_;
        LGL_ERROR;
    }
}

const std::string ImageOverlay::getProcessorInfo() const {
    return "Creates an image-overlay on top of the rendered image. The relative position and the relative size of the image can be altered.";
}

Processor* ImageOverlay::create() const {
    return new ImageOverlay();
}

void ImageOverlay::initialize() throw (VoreenException) {
    loadTexture();

    std::vector<std::string> units;
    units.push_back(shadeTexUnit0_);
    units.push_back(depthTexUnit0_);
    units.push_back(shadeTexUnit1_);
    units.push_back(depthTexUnit1_);
    tm_.registerUnits(units);

    ImageProcessor::initialize();
}

std::string ImageOverlay::generateHeader() {
    std::string header = ImageProcessor::generateHeader();
    header += "#define COMBINE_ALPHA_COMPOSITING\n";
    return header;
}

void ImageOverlay::process() {

    if(!outport_.isReady())
        return;

    if (!inport_.isReady())
        outport_.activateTarget();
    else
        privatePort_.activateTarget();


    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearDepth(1.0);

    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderOverlayImage();

    if (!inport_.isReady())
        outport_.deactivateTarget();
    else
        privatePort_.deactivateTarget();

    glEnable(GL_DEPTH_TEST);

    // leave if there's nothing to render above
    if (inport_.isReady()) {
        outport_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        privatePort_.bindTextures(tm_.getGLTexUnit(shadeTexUnit1_), tm_.getGLTexUnit(depthTexUnit1_));
        inport_.bindTextures(tm_.getGLTexUnit(shadeTexUnit0_), tm_.getGLTexUnit(depthTexUnit0_));

        // initialize shader
        program_->activate();
        setGlobalShaderParameters(program_);
        program_->setUniform("shadeTex0_", static_cast<GLint>(tm_.getTexUnit(shadeTexUnit0_)));
        program_->setUniform("depthTex0_", static_cast<GLint>(tm_.getTexUnit(depthTexUnit0_)));
        program_->setUniform("shadeTex1_", static_cast<GLint>(tm_.getTexUnit(shadeTexUnit1_)));
        program_->setUniform("depthTex1_", static_cast<GLint>(tm_.getTexUnit(depthTexUnit1_)));
        privatePort_.setTextureParameters(program_, "textureParameters0_");
        inport_.setTextureParameters(program_, "textureParameters1_");

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        glDepthFunc(GL_LESS);
        outport_.deactivateTarget();

        program_->deactivate();
        glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    }
    LGL_ERROR;
}

void ImageOverlay::renderOverlayImage() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    if ( tex_ ) {
        glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
        tex_->bind();
        tex_->enable();
        LGL_ERROR;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

        glColor4f(1.0f, 1.0f, 1.0f, opacity_.get());

        glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(-1.0f+2*left_.get(), 1.0f-2*top_.get()-2*height_.get());
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(-1.0f+2*left_.get()+2*width_.get(), 1.0f-2*top_.get()-2*height_.get());
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(-1.0f+2*left_.get()+2*width_.get(), 1.0f-2*top_.get());
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(-1.0f+2*left_.get(), 1.0f-2*top_.get());

        glEnd();

        glDisable(GL_BLEND);

        tex_->disable();
    }
}

void ImageOverlay::setOverlayImageModeEvt() {

    loadTexture();
    invalidate();
}

void ImageOverlay::loadTexture() {
    // is a texture already loaded? -> then delete
    if (tex_) {
        if (textureLoaded_) {
            TexMgr.dispose(tex_);
            //textureloaded_ = false;
        }
        else
            delete tex_;

        LGL_ERROR;
        tex_ = 0;
    }

    // create Texture
    if (!filename_.get().empty()) {
        tex_ = TexMgr.load(filename_.get(), tgt::Texture::LINEAR, false, false, true, true,
                           !GpuCaps.isNpotSupported());
    }
//    if (tex_) {
        //textureloaded_ = true;
	if (!textureLoaded_) {
		textureLoaded_ = true;
		// after loading a texture, the standard width and height will be set to fit the texture.
        float widthTotal = (float) tex_->getWidth();
        float heightTotal = (float) tex_->getHeight();
        float texRatio = widthTotal / heightTotal;
        if (texRatio > 1) {
            left_.set(0.25f,true);
            width_.set(0.5f,true);
            height_.set(0.5f/texRatio,true);
            top_.set(0.5f-0.25f/texRatio,true);
        } else {
            top_.set(0.25f,true);
            height_.set(0.5f,true);
            width_.set(0.5f*texRatio,true);
            left_.set(0.5f-0.25f*texRatio,true);
        }
    }
}

} //namespace voreen
