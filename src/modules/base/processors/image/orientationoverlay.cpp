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

#include "voreen/modules/base/processors/image/orientationoverlay.h"
#include "voreen/core/interaction/camerainteractionhandler.h"
#include "voreen/core/voreenapplication.h"

#include "tgt/texturemanager.h"
#include "tgt/gpucapabilities.h"
#include "tgt/glmath.h"
#include "tgt/textureunit.h"

using tgt::TextureUnit;
using tgt::vec3;

namespace voreen {

const std::string OrientationOverlay::loggerCat_("voreen.OrientationOverlay");

OrientationOverlay::OrientationOverlay()
    : ImageProcessor("pp_orientationoverlay")
    , inport_(Port::INPORT, "image.input")
    , outport_(Port::OUTPORT, "image.output")
    , privatePort_(Port::OUTPORT, "image.tmp", false)
    , drawCube_("drawCube", "Draw Cube", true)
    , drawAxes_("drawAxes", "Draw Axes", true)
    , drawTextures_("drawTextures", "Draw Cube Textures", true)
    , colorizeTextures_("colorizeTextures", "Colorize Textures", false)
    , filenameFront_("filenameFront", "Front Texture", "Select texture",
                VoreenApplication::app()->getTexturePath(), "*.jpg;*.png;*.bmp",
                FileDialogProperty::OPEN_FILE)
    , filenameBack_("filenameBack", "Back Texture", "Select texture",
                VoreenApplication::app()->getTexturePath(), "*.jpg;*.png;*.bmp",
                FileDialogProperty::OPEN_FILE)
    , filenameTop_("filenameTop", "Top Texture", "Select texture",
                VoreenApplication::app()->getTexturePath(), "*.jpg;*.png;*.bmp",
                FileDialogProperty::OPEN_FILE)
    , filenameBottom_("filenameBottom", "Bottom Texture", "Select texture",
                VoreenApplication::app()->getTexturePath(), "*.jpg;*.png;*.bmp",
                FileDialogProperty::OPEN_FILE)
    , filenameLeft_("filenameLeft", "Left Texture", "Select texture",
                VoreenApplication::app()->getTexturePath(), "*.jpg;*.png;*.bmp",
                FileDialogProperty::OPEN_FILE)
    , filenameRight_("filenameRight", "Right Texture", "Select texture",
                VoreenApplication::app()->getTexturePath(), "*.jpg;*.png;*.bmp",
                FileDialogProperty::OPEN_FILE)
    , shiftX_("shiftX", "Horizontal Position", -0.5f, -1.5f, 1.5f)
    , shiftY_("shiftY", "Vertical Position", -0.5f, -1.5f, 1.5f)
    , cubeSize_("cubeSize", "Cube Size", 0.2f, 0.1, 1)
    , axisLength_("axisLength", "Axes Length", 0.5f, 0.1, 4.f)
    , camera_("camera", "Camera", new tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , frontTex_(0)
    , backTex_(0)
    , topTex_(0)
    , leftTex_(0)
    , bottomTex_(0)
    , rightTex_(0)
    , loadingTextures_(false)
{
    addPort(inport_);
    addPort(outport_);
    addPrivateRenderPort(&privatePort_);

    addProperty(drawCube_);
    addProperty(drawAxes_);
    addProperty(drawTextures_);
    filenameFront_.onChange(CallMemberAction<OrientationOverlay>(this, &OrientationOverlay::loadTextures));
    addProperty(colorizeTextures_);
    addProperty(filenameFront_);
    filenameBack_.onChange(CallMemberAction<OrientationOverlay>(this, &OrientationOverlay::loadTextures));
    addProperty(filenameBack_);
    filenameTop_.onChange(CallMemberAction<OrientationOverlay>(this, &OrientationOverlay::loadTextures));
    addProperty(filenameTop_);
    filenameBottom_.onChange(CallMemberAction<OrientationOverlay>(this, &OrientationOverlay::loadTextures));
    addProperty(filenameBottom_);
    filenameLeft_.onChange(CallMemberAction<OrientationOverlay>(this, &OrientationOverlay::loadTextures));
    addProperty(filenameLeft_);
    filenameRight_.onChange(CallMemberAction<OrientationOverlay>(this, &OrientationOverlay::loadTextures));
    addProperty(filenameRight_);
    addProperty(shiftX_);
    addProperty(shiftY_);
    addProperty(cubeSize_);
    addProperty(axisLength_);
    addProperty(camera_);

    // set initial texture names
    std::string texturePath = VoreenApplication::app()->getTexturePath();
    textureNames_[0] = texturePath + "/axial_t.png";
    textureNames_[1] = texturePath + "/axial_b.png";
    textureNames_[2] = texturePath + "/coronal_f.png";
    textureNames_[3] = texturePath + "/coronal_b.png";
    textureNames_[4] = texturePath + "/sagittal_l.png";
    textureNames_[5] = texturePath + "/sagittal_r.png";
}

OrientationOverlay::~OrientationOverlay() {
}

std::string OrientationOverlay::getProcessorInfo() const {
    return "Adds a cube and a tripod to the input rendering indicating the current orientation of the camera.";
}

Processor* OrientationOverlay::create() const {
    return new OrientationOverlay();
}

void OrientationOverlay::initialize() throw (VoreenException) {
    ImageProcessor::initialize();
    loadTextures();
}

void OrientationOverlay::deinitialize() throw (VoreenException) {
    // dispose textures before sending this processor into nirvana
    if(frontTex_)
        TexMgr.dispose(frontTex_);
    if(backTex_)
        TexMgr.dispose(backTex_);
    if(bottomTex_)
        TexMgr.dispose(bottomTex_);
    if(leftTex_)
        TexMgr.dispose(leftTex_);
    if(topTex_)
        TexMgr.dispose(topTex_);
    if(rightTex_)
        TexMgr.dispose(rightTex_);

    ImageProcessor::deinitialize();
}

bool OrientationOverlay::isReady() const {
    return outport_.isReady();
}

void OrientationOverlay::process() {
    if (!inport_.isReady())
        outport_.activateTarget();
    else
        privatePort_.activateTarget();

    //draw the cube in the desired position
    // set modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    tgt::loadMatrix(camera_.get()->getProjectionMatrix());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(shiftX_.get(), shiftY_.get(), 0);
    tgt::mat4 view = camera_.get()->getViewMatrix();
    view.t23 = -2.7f;
    tgt::multMatrix(view);

    glClearDepth(1);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // render cube and axis overlays
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (drawCube_.get() == true)
        renderCube();
    if (drawAxes_.get() == true)
        renderAxes();

    if (!inport_.isReady())
        outport_.deactivateTarget();
    else
        privatePort_.deactivateTarget();

    // restore OpenGL state
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    LGL_ERROR;

    glEnable(GL_DEPTH_TEST);

    // now do the composition of the OrientationOverlay with the inport render data by shader
    if (inport_.isReady()) {
        outport_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // use the shader to draw cube and axis over inport render data
        // therefore bind rgba and depth values of both to textures
        TextureUnit shadeUnit0, shadeUnitDepth0, shadeUnit1, shadeUnitDepth1;
        privatePort_.bindTextures(shadeUnit0.getEnum(), shadeUnitDepth0.getEnum());
        inport_.bindTextures(shadeUnit1.getEnum(), shadeUnitDepth1.getEnum());

        // initialize shader
        program_->activate();
        setGlobalShaderParameters(program_);
        program_->setUniform("shadeTexMe_", shadeUnit0.getUnitNumber());
        program_->setUniform("depthTexMe_", shadeUnitDepth0.getUnitNumber());
        program_->setUniform("shadeTexIn_", shadeUnit1.getUnitNumber());
        program_->setUniform("depthTexIn_", shadeUnitDepth1.getUnitNumber());
        privatePort_.setTextureParameters(program_, "textureParametersMe_");
        inport_.setTextureParameters(program_, "textureParametersIn_");

        glDepthFunc(GL_ALWAYS);
        renderQuad(); // render quad primitive textured by fragment shader
        glDepthFunc(GL_LESS);
        outport_.deactivateTarget();

        program_->deactivate();
        glActiveTexture(GL_TEXTURE0);
    }

    LGL_ERROR;
}

void OrientationOverlay::renderCube() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // setup texture mapping
    if (drawTextures_.get()) {
        glEnable(GL_TEXTURE_2D);
        if (colorizeTextures_.get())
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        else
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }

    //top
    if (drawTextures_.get() && backTex_) {
        backTex_->bind();
        glColor3f(1, 0, 0);
        glBegin(GL_QUADS);
            glTexCoord2f(0.f, 0.f);
            glVertex3f( cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
            glTexCoord2f(1.f, 0.f);
            glVertex3f(-cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
            glTexCoord2f(1.f, 1.f);
            glVertex3f(-cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
            glTexCoord2f(0.f, 1.f);
            glVertex3f( cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
            glColor3f(1, 0, 0);
            glVertex3f( cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
            glVertex3f(-cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
            glVertex3f(-cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
            glVertex3f( cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
        glEnd();
    }
    //bottom
    if (drawTextures_.get() && frontTex_) {
        frontTex_->bind();
        glColor3f(0, 1, 1);
        glBegin(GL_QUADS);
            glTexCoord2f(1.f, 1.f);
            glVertex3f( cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
            glTexCoord2f(0.f, 1.f);
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
            glTexCoord2f(0.f, 0.f);
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
            glTexCoord2f(1.f, 0.f);
            glVertex3f( cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
            glColor3f(0, 1, 1);
            glVertex3f( cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
            glVertex3f( cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
        glEnd();
    }
    //front
    if (drawTextures_.get() && topTex_) {
        topTex_->bind();
        glColor3f(0, 1, 0);
        glBegin(GL_QUADS);
            glTexCoord2f(1.f, 1.f);
            glVertex3f( cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
            glTexCoord2f(0.f, 1.f);
            glVertex3f(-cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
            glTexCoord2f(0.f, 0.f);
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
            glTexCoord2f(1.f, 0.f);
            glVertex3f( cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
            glColor3f(0, 1, 0);
            glVertex3f( cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
            glVertex3f(-cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
            glVertex3f( cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
        glEnd();
    }
    //back
    if (drawTextures_.get() && bottomTex_) {
        bottomTex_->bind();
        glColor3f(1, 0, 1);
        glBegin(GL_QUADS);
            glTexCoord2f(1.f, 0.f);
            glVertex3f( cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
            glTexCoord2f(1.f, 1.f);
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
            glTexCoord2f(0.f, 1.f);
            glVertex3f(-cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
            glTexCoord2f(0.f, 0.f);
            glVertex3f( cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
            glColor3f(1, 0, 1);
            glVertex3f( cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
            glVertex3f(-cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
            glVertex3f( cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
        glEnd();
    }
    //left
    if (drawTextures_.get() && rightTex_) {
        rightTex_->bind();
        glColor3f(0, 0, 1);
        glBegin(GL_QUADS);
            glTexCoord2f(0.f, 1.f);
            glVertex3f(-cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
            glTexCoord2f(0.f, 0.f);
            glVertex3f(-cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
            glTexCoord2f(1.f, 0.f);
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
            glTexCoord2f(1.f, 1.f);
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
            glColor3f(0, 0, 1);
            glVertex3f(-cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
            glVertex3f(-cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
            glVertex3f(-cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
        glEnd();
    }
    //right
    if (drawTextures_.get() && leftTex_) {
        leftTex_->bind();
        glColor3f(1, 1, 0);
        glBegin(GL_QUADS);
            glTexCoord2f(1.f, 0.f);
            glVertex3f( cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
            glTexCoord2f(1.f, 1.f);
            glVertex3f( cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
            glTexCoord2f(0.f, 1.f);
            glVertex3f( cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
            glTexCoord2f(0.f, 0.f);
            glVertex3f( cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
            glColor3f(1, 1, 0);
            glVertex3f( cubeSize_.get(), cubeSize_.get(),-cubeSize_.get());
            glVertex3f( cubeSize_.get(), cubeSize_.get(), cubeSize_.get());
            glVertex3f( cubeSize_.get(),-cubeSize_.get(), cubeSize_.get());
            glVertex3f( cubeSize_.get(),-cubeSize_.get(),-cubeSize_.get());
        glEnd();
    }

    glPopAttrib();
    LGL_ERROR;
}

void OrientationOverlay::renderAxes() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    //x-axis
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3f( 0, 0, 0);
    glVertex3f( axisLength_.get(), 0, 0);
    glEnd();

    //y-axis
    glBegin(GL_LINES);
    glColor3f(0, 1, 0);
    glVertex3f( 0, 0, 0);
    glVertex3f(0, axisLength_.get(), 0);
    glEnd();

    //z-axis
    glBegin(GL_LINES);
    glColor3f(0, 0, 1);
    glVertex3f( 0, 0, 0);
    glVertex3f( 0,0, axisLength_.get());
    glEnd();

    glPopAttrib();
    LGL_ERROR;
}

#ifdef VRN_WITH_DEVIL
void OrientationOverlay::loadTextures() {

    if (loadingTextures_)
        return;

    if (isInitialized()) {

        loadingTextures_ = true;

        // first dispose textures
        TexMgr.dispose(topTex_);
        TexMgr.dispose(bottomTex_);
        TexMgr.dispose(frontTex_);
        TexMgr.dispose(backTex_);
        TexMgr.dispose(leftTex_);
        TexMgr.dispose(rightTex_);
        LGL_ERROR;

        // now try loading textures
        if (!filenameTop_.get().empty())
            topTex_ = TexMgr.load(filenameTop_.get());
        else if (textureNames_[0] != "") {
            topTex_ = TexMgr.load(textureNames_[0]);
            if (topTex_)
                filenameTop_.set(textureNames_[0]);
        }
        else
            topTex_ = 0;

        if (!filenameBottom_.get().empty())
            bottomTex_ = TexMgr.load(filenameBottom_.get());
        else if (textureNames_[1] != "") {
            bottomTex_ = TexMgr.load(textureNames_[1]);
            if (bottomTex_)
                filenameBottom_.set(textureNames_[1]);
        }
        else
            bottomTex_ = 0;

        if (!filenameFront_.get().empty())
            frontTex_ = TexMgr.load(filenameFront_.get());
        else if (textureNames_[2] != "") {
            frontTex_ = TexMgr.load(textureNames_[2]);
            if (frontTex_)
                filenameFront_.set(textureNames_[2]);
        }
        else
            frontTex_ = 0;

        if (!filenameBack_.get().empty())
            backTex_ = TexMgr.load(filenameBack_.get());
        else if (textureNames_[3] != "") {
            backTex_ = TexMgr.load(textureNames_[3]);
            if (backTex_)
                filenameBack_.set(textureNames_[3]);
        }
        else
            backTex_ = 0;

        if (!filenameLeft_.get().empty())
            leftTex_ = TexMgr.load(filenameLeft_.get());
        else if (textureNames_[4] != "") {
            leftTex_ = TexMgr.load(textureNames_[4]);
            if (leftTex_)
                filenameLeft_.set(textureNames_[4]);
        }
        else
            leftTex_ = 0;

        if (!filenameRight_.get().empty())
            rightTex_ = TexMgr.load(filenameRight_.get());
        else if (textureNames_[5] != "") {
            rightTex_ = TexMgr.load(textureNames_[5]);
            if (rightTex_)
                filenameRight_.set(textureNames_[5]);
        }
        else
            rightTex_ = 0;

        LGL_ERROR;
        loadingTextures_ = false;

        invalidate();
    }
}
#else
void OrientationOverlay::loadTextures() {
}


#endif //VRN_WITH_DEVIL


} // namespace
