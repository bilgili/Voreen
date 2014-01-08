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

#include "transfuncoverlay.h"

#include "tgt/textureunit.h"

#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/transfunc/preintegrationtable.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"

#include <sstream>

using tgt::TextureUnit;

namespace voreen {

TransFuncOverlay::TransFuncOverlay()
    : ImageProcessor("image/compositor")
    , imageInport_(Port::INPORT, "image.in", "Image Inpput")
    , privatePort_(Port::OUTPORT, "private","Privateport")
    , outport_(Port::OUTPORT, "image.out", "Image Output")
    , fontProp_("voreen.fontprop", "Font:")
    , transferFunc_("transferFunction", "Transfer Function:")
    , renderPreIntegrationTable_("renderPreIntegrationTable", "Render Pre-Integration Table", false)
    , renderOverlay_("renderOverlay", "Render Overlay", true)
    , usePixelCoordinates_("usePixelCoordinates", "Move/Resize Overlay by ")
    , overlayBottomLeft_("overlayBottomLeft", "Overlay Bottom Left", tgt::ivec2(10), tgt::ivec2(-4096), tgt::ivec2(4096))
    , overlayDimensions_("overlayDimensions", "Overlay Dimensions", tgt::ivec2(100), tgt::ivec2(0), tgt::ivec2(4096))
    , overlayBottomLeftRelative_("overlayBottomLeftRelative", "Overlay Bottom Left (Relative)", tgt::vec2(0.05f), tgt::vec2(-1.f), tgt::vec2(1.f))
    , overlayDimensionsRelative_("overlayDimensionsRelative", "Overlay Dimensions (Relative)", tgt::vec2(0.25f), tgt::vec2(0.f), tgt::vec2(1.f))
    , overlayOpacity_("overlayOpacity", "Overlay Opacity", 1.f)
    , fontColor_("fontColor","Font Color", tgt::Color(0.f, 0.f, 0.f, 1.f))
    , scalingProp_("scalingProp","Scaling",1.f,0.01,10000.f)
    , tfUnit_("tfUnit","Unit (max 7 Chars)","")
    , renderBorder_("renderBorder", "Render Border", true)
    , borderWidth_("borderWidth", "Border Width", 2.f, 0.1f, 10.f)
    , borderColor_("borderColor", "Border Color", tgt::Color(0.f, 0.f, 0.f, 1.f))
    , copyShader_(0)
{
    borderColor_.setViews(Property::COLOR);
    fontColor_.setViews(Property::COLOR);
    addPort(imageInport_);
    addPrivateRenderPort(&privatePort_);
    addPort(outport_);

    addProperty(fontProp_);
       fontProp_.setVisible(false);
    addProperty(transferFunc_);
    addProperty(renderPreIntegrationTable_);

    addProperty(renderOverlay_);
    renderOverlay_.setGroupID("general");
    addProperty(overlayOpacity_);
    overlayOpacity_.setGroupID("general");
    addProperty(usePixelCoordinates_);
    usePixelCoordinates_.setGroupID("general");
    usePixelCoordinates_.addOption("true","Pixel Coordinates",true);
    usePixelCoordinates_.addOption("false","Resative Position",false);
    usePixelCoordinates_.select("true");
    usePixelCoordinates_.onChange(CallMemberAction<TransFuncOverlay>(this, &TransFuncOverlay::onChangeUsePixelCoordinates));
    setPropertyGroupGuiName("general","General");

    addProperty(overlayBottomLeft_);
    overlayBottomLeft_.setGroupID("overlayAbs");
    addProperty(overlayDimensions_);
    overlayDimensions_.setGroupID("overlayAbs");

    addProperty(overlayBottomLeftRelative_);
    overlayBottomLeftRelative_.setGroupID("overlayPer");
    addProperty(overlayDimensionsRelative_);
    overlayDimensionsRelative_.setGroupID("overlayPer");

    setPropertyGroupGuiName("overlayAbs","Overlay Options (absolute)");
    setPropertyGroupGuiName("overlayPer","Overlay Options (relative)");

    addProperty(fontColor_);
    fontColor_.setGroupID("overlay settings");
    addProperty(tfUnit_);
    tfUnit_.setGroupID("overlay settings");
    addProperty(scalingProp_);
    scalingProp_.setGroupID("overlay settings");
    addProperty(renderBorder_);
    renderBorder_.setGroupID("overlay settings");
    addProperty(borderWidth_);
    borderWidth_.setGroupID("overlay settings");
    addProperty(borderColor_);
    borderColor_.setGroupID("overlay settings");
    setPropertyGroupGuiName("overlay settings","Overlay Settings");

    onChangeUsePixelCoordinates();
    //privatePort_.resize(ppSizeX_,ppSizeY_);
}

Processor* TransFuncOverlay::create() const {
    return new TransFuncOverlay();
}

void TransFuncOverlay::initialize() throw (tgt::Exception) {
    ImageProcessor::initialize();

    copyShader_ = ShdrMgr.loadSeparate("passthrough.vert", "copyimage.frag",
        ImageProcessor::generateHeader(), false);
}

void TransFuncOverlay::deinitialize() throw (tgt::Exception) {
    ShdrMgr.dispose(copyShader_);
    copyShader_ = 0;

    ImageProcessor::deinitialize();
}

bool TransFuncOverlay::isReady() const {
    return (imageInport_.isReady() && outport_.isReady());
}

std::string TransFuncOverlay::generateHeader(const tgt::GpuCapabilities::GlVersion* version) {
    std::string header = ImageProcessor::generateHeader(version);
    header += "#define MODE_ALPHA_BLENDING_B_OVER_A\n";
    //header += "#define MODE_WEIGHTED_AVERAGE\n";
    return header;
}

void TransFuncOverlay::beforeProcess() {
    ImageProcessor::beforeProcess();
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
}

void TransFuncOverlay::process() {
    #ifndef VRN_MODULE_FONTRENDERING
    LWARNING("Empty output, enable module 'fontrendering'.");
    return;
    #endif

    tgtAssert(outport_.isReady(), "Outport not ready");
    tgtAssert(imageInport_.isReady(), "Inport not ready");
    tgtAssert(program_ && copyShader_, "Shader missing");

    if(dynamic_cast<TransFunc1DKeys*>(transferFunc_.get()) == 0){
        LWARNING("No transfer function of class TransFuncIntensity is given!!!");
        return;
    }
    TransFunc1DKeys* tfi = dynamic_cast<TransFunc1DKeys*>(transferFunc_.get());

    //render overlay
    privatePort_.activateTarget();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
            glClearColor(fontColor_.get().r,fontColor_.get().g,fontColor_.get().b,0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //render the transfer function texture
            const tgt::Texture* tfTex = 0;
            if(renderPreIntegrationTable_.get())
                tfTex = tfi->getPreIntegrationTable(1.0f / (41.0f * 2.0f))->getTexture();
            else
                tfTex = tfi->getTexture();

            tgtAssert(tfTex, "No transfer function texture");
            tfTex->bind();
            glColor4f(1.f,1.f,1.f,1.f);
            glDisable(GL_DEPTH_TEST);
                glBegin(GL_QUADS);
                    glVertex2f(-0.8f,-0.9f);
                    glVertex2f(-0.5f,-0.9f);
                    glVertex2f(-0.5f,0.7f);
                    glVertex2f(-0.8f,0.7f);
                glEnd();
                glColor4f(0.8f,0.8f,0.8f,1.f);
                glBegin(GL_QUADS);
                    glVertex2f(-0.8f,-0.9f);
                    glVertex2f(-0.65f,-0.9f);
                    glVertex2f(-0.65f,-0.58f);
                    glVertex2f(-0.8f,-0.58f);

                    glVertex2f(-0.65f,-0.58f);
                    glVertex2f(-0.5f,-0.58f);
                    glVertex2f(-0.5f,-0.26f);
                    glVertex2f(-0.65f,-0.26f);

                    glVertex2f(-0.8f,-0.26f);
                    glVertex2f(-0.65f,-0.26f);
                    glVertex2f(-0.65f,0.06f);
                    glVertex2f(-0.8f,0.06f);

                    glVertex2f(-0.65f,0.06f);
                    glVertex2f(-0.5f,0.06f);
                    glVertex2f(-0.5f,0.38f);
                    glVertex2f(-0.65f,0.38f);

                    glVertex2f(-0.8f,0.38f);
                    glVertex2f(-0.65f,0.38f);
                    glVertex2f(-0.65f,0.7f);
                    glVertex2f(-0.8f,0.7f);
                glEnd();
                glColor4f(1.f,1.f,1.f,1.f);

                if(renderPreIntegrationTable_.get())
                    glEnable(GL_TEXTURE_2D);
                else
                    glEnable(GL_TEXTURE_1D);

                glEnable(GL_BLEND);
                    glBlendColor(0.0f,0.0f,0.0f,overlayOpacity_.get());
                    glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ZERO,GL_CONSTANT_ALPHA);
                    glBegin(GL_QUADS);
                    if(renderPreIntegrationTable_.get()) {
                        glTexCoord2f(0.f, 0.f); glVertex2f(-0.8f,-0.9f);
                        glTexCoord2f(1.f, 0.f); glVertex2f(-0.5f,-0.9f);
                        glTexCoord2f(1.f, 1.f); glVertex2f(-0.5f,0.7f);
                        glTexCoord2f(0.f, 1.f); glVertex2f(-0.8f,0.7f);
                    }
                    else {
                        glTexCoord1f(0.f); glVertex2f(-0.8f,-0.9f);
                        glTexCoord1f(0.f); glVertex2f(-0.5f,-0.9f);
                        glTexCoord1f(1.f); glVertex2f(-0.5f,0.7f);
                        glTexCoord1f(1.f); glVertex2f(-0.8f,0.7f);
                    }
                    glEnd();
                glDisable(GL_BLEND);
                glDisable(GL_TEXTURE_1D);
            glEnable(GL_DEPTH_TEST);
            //render fonts
            MatStack.pushMatrix();
                MatStack.translate(-1.f,-1.f,0.f);
                float scaleFactorX = 2.0f / (float)privatePort_.getSize().x;
                float scaleFactorY = 2.0f / (float)privatePort_.getSize().y;
                MatStack.scale(scaleFactorX, scaleFactorY, 1.f);
                glColor4f(fontColor_.get().r,fontColor_.get().g,fontColor_.get().b,fontColor_.get().a*overlayOpacity_.get());
                fontProp_.get()->setSize(privatePort_.getSize().y/12);
                fontProp_.get()->setVerticalTextAlignment(tgt::Font::Middle);
                fontProp_.get()->setFontType(tgt::Font::BitmapFont);
                fontProp_.get()->setLineWidth(privatePort_.getSize().x*0.35f);
                fontProp_.get()->setTextAlignment(tgt::Font::Center);
                fontProp_.get()->render(tgt::vec3(0,privatePort_.getSize().y*0.925f,0), tfUnit_.get());
                fontProp_.get()->setLineWidth((float)privatePort_.getSize().x);
                fontProp_.get()->setTextAlignment(tgt::Font::Left);
                std::stringstream strstr;
                strstr << tfi->getDomain(0).x * scalingProp_.get();
                fontProp_.get()->render(tgt::vec3(privatePort_.getSize().x*0.3f,privatePort_.getSize().y*0.05f,0), strstr.str());
                strstr.clear();
                strstr.str("");
                strstr << (tfi->getDomain(0).x+((tfi->getDomain(0).y-tfi->getDomain(0).x)/2)) * scalingProp_.get();
                fontProp_.get()->render(tgt::vec3(privatePort_.getSize().x*0.3f,privatePort_.getSize().y*0.45f,0), strstr.str());
                strstr.clear();
                strstr.str("");
                strstr << tfi->getDomain(0).y * scalingProp_.get();
                fontProp_.get()->render(tgt::vec3(privatePort_.getSize().x*0.3f,privatePort_.getSize().y*0.85f,0), strstr.str());
            MatStack.popMatrix();
        glPopAttrib();
         // render border around overlay
        if (renderBorder_.get()) {
            glPushAttrib(GL_ALL_ATTRIB_BITS);
                glColor4f(borderColor_.get().r,borderColor_.get().g,borderColor_.get().b,borderColor_.get().a*overlayOpacity_.get());
                glLineWidth(borderWidth_.get());
                glDepthFunc(GL_ALWAYS);
                glBegin(GL_LINE_STRIP);
                    glVertex2f(-0.8f,-0.9f);
                    glVertex2f(-0.5f,-0.9f);
                    glVertex2f(-0.5f,0.7f);
                    glVertex2f(-0.8f,0.7f);
                    glVertex2f(-0.8f,-0.9f);
                glEnd();
            glPopAttrib();
        }
        LGL_ERROR;
    privatePort_.deactivateTarget();

    //same code as in ImageOverlay
    //          |
    //          v
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
    bool dimensionsValid = ( (usePixelCoordinates_.getValue()  && tgt::hand(tgt::greaterThan(overlayDimensions_.get(), tgt::ivec2(0)))) ||
                             (!usePixelCoordinates_.getValue() && tgt::hand(tgt::greaterThan(overlayDimensionsRelative_.get(), tgt::vec2(0.f)))) );
    if (renderOverlay_.get() && /*overlayInport_.isReady() &&*/ dimensionsValid) {
        // bind overlay to tex unit
        TextureUnit overlayUnit;
        tgt::Texture* overlayTex = privatePort_.getColorTexture();//overlayInport_.getColorTexture();
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
        //program_->setUniform("weightingFactor_", 1.f-overlayOpacity_.get());

        // determine overlay dimensions and bottom-left in float pixel coords
        tgt::vec2 outportDim = tgt::vec2(outport_.getSize());
        tgt::vec2 overlayDim, overlayBL;
        if (usePixelCoordinates_.getValue()) {
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
    }

    outport_.deactivateTarget();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

void TransFuncOverlay::onChangeUsePixelCoordinates(){
    if(usePixelCoordinates_.getValue()){
        setPropertyGroupVisible("overlayAbs",true);
        setPropertyGroupVisible("overlayPer",false);
    } else {
        setPropertyGroupVisible("overlayAbs",false);
        setPropertyGroupVisible("overlayPer",true);
    }
}


} // namespace voreen
