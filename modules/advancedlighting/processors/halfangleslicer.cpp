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

#include "halfangleslicer.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "tgt/glmath.h"
#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

HalfAngleSlicer::HalfAngleSlicer()
    : VolumeSlicer()
    , shaderProp_("slicing.prg", "Slicing shader", "sl_halfslicing.frag", "sl_singlevolume.vert")
    , outport_(Port::OUTPORT, "image.output", "Image Output", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER, GL_RGBA16F_ARB)
    , lightport_(Port::OUTPORT, "image.lightport", "Light Image Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_DEFAULT, GL_RGBA16F_ARB)
    , lightCamera_(tgt::Camera(tgt::vec3(0.f, -3.5f, -3.5f), tgt::vec3(0.f, 0.f, 0.f), tgt::vec3(0.f, 1.f, 0.f)))
    , eyeCamera_("eyeCamera", "Eye Camera", tgt::Camera(tgt::vec3(0.f, 0.f, 3.5f), tgt::vec3(0.f, 0.f, 0.f), tgt::vec3(0.f, 1.f, 0.f)))
    , cameraHandler_("cameraHandler", "Camera Handler", &eyeCamera_)
    , halfLight_("halfLight", "Light source position", tgt::vec4(0.f, -3.5f, -3.5f, 1.f))
    , invert_(false)
{
    removeProperty(&camera_);

    addInteractionHandler(cameraHandler_);
    addProperty(shaderProp_);
    addProperty(eyeCamera_);
    addProperty(halfLight_);

    addPort(outport_);
    addPrivateRenderPort(lightport_);
}

Processor* HalfAngleSlicer::create() const {
    return new HalfAngleSlicer();
}

void HalfAngleSlicer::initialize() throw (tgt::Exception) {
    VolumeSlicer::initialize();
    compile();
    halfLight_.setCamera(&eyeCamera_);
}

void HalfAngleSlicer::compile() {
    shaderProp_.setHeader(generateHeader());
    shaderProp_.rebuild();
}

bool HalfAngleSlicer::isReady() const {
    //check if all inports are connected:
    if(!volumeInport_.isReady())
        return false;

    //check if outport is connected:
    if (!outport_.isReady())
        return false;

    if(!shaderProp_.hasValidShader())
        return false;

    return true;
}

void HalfAngleSlicer::beforeProcess() {
    VolumeSlicer::beforeProcess();

    lightCamera_.setPosition(halfLight_.get().xyz());
    lightCamera_.setFocus(tgt::vec3(0.f));
    tgt::vec3 camView = eyeCamera_.get().getLook();
    tgt::vec3 lightView = lightCamera_.getLook();

    tgt::vec3 halfAngleVec;

    if (dot(camView, lightView) > 0) {
        halfAngleVec = normalize(camView + lightView);
        invert_ = false;
    } else {
        halfAngleVec = normalize(-camView + lightView);
        invert_ = true;
    }

    tgt::vec3 camPos = -halfAngleVec * 10.f;
    tgt::vec3 up = tgt::vec3(0.f, 1.f, 0.f);
    if(std::fabs(dot(normalize(camPos), up)) < 0.005f)
        up = tgt::vec3(1.f, 0.f, 0.f);

    camera_.set(tgt::Camera(camPos, tgt::vec3(0.f), up));
}

void HalfAngleSlicer::process() {

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    LGL_ERROR;

    lightport_.activateTarget();
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lightport_.deactivateTarget();
    outport_.activateTarget();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    outport_.deactivateTarget();

    // bind transfer function
    TextureUnit transferUnit;
    transferUnit.activate();
    if (transferFunc_.get())
        transferFunc_.get()->bind();

    transferFunc_.setVolumeHandle(volumeInport_.getData());

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add main volume
    TextureUnit volUnit;
    volumeTextures.push_back(VolumeStruct(
        volumeInport_.getData(),
        &volUnit,
        "volume_","volumeStruct_")
    );

    // initialize slicing shader
    tgt::Shader* slicingPrg = shaderProp_.getShader();
    slicingPrg->activate();

    // fragment shader uniforms
    transferFunc_.get()->setUniform(slicingPrg, "transferFunc_", "transferFuncTex_", transferUnit.getUnitNumber());

    // set common uniforms used by all shaders
    tgt::Camera cam = eyeCamera_.get();
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(slicingPrg, volumeTextures, &cam, lightPosition_.get());

    setupUniforms(slicingPrg);

    // correct slice distance for this technique
    sliceDistance_ *= 0.5f*std::sqrt(1.f + dot(eyeCamera_.get().getLook(), lightCamera_.getLook()));
    slicingPrg->setUniform("dPlaneIncr_", sliceDistance_);

    glDisable(GL_DEPTH_TEST);

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadMatrix(eyeCamera_.get().getProjectionMatrix(outport_.getSize()));

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadMatrix(eyeCamera_.get().getViewMatrix());

    slicingPrg->activate();
    glEnable(GL_BLEND);

    unsigned int numSlices = static_cast<unsigned int>(maxLength_ / sliceDistance_);

    TextureUnit lightBufferUnit;

    slicingPrg->setUniform("lightBuf_", lightBufferUnit.getUnitNumber());
    slicingPrg->setUniform("lightMat_", lightCamera_.getViewMatrix());
    lightport_.setTextureParameters(slicingPrg, "lightBufParameters_");

    for (unsigned int curSlice = 0; curSlice < numSlices; curSlice++) {
        outport_.activateTarget();

        // FIRST PASS
        if(invert_)
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        else
            glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);

        lightBufferUnit.activate();
        glEnable(GL_TEXTURE_2D);
        lightport_.bindColorTexture();

        tgt::Camera cam = eyeCamera_.get();
        setGlobalShaderParameters(slicingPrg, &cam);
        slicingPrg->setUniform("secondPass_", false);

        glBegin(GL_POLYGON);
            for (unsigned int curPoint=0; curPoint<6; curPoint++)
                glVertex2i(curPoint, curSlice);
        glEnd();

        outport_.deactivateTarget();

        // SECOND PASS
        lightport_.activateTarget();

        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        MatStack.pushMatrix();
        MatStack.loadMatrix(lightCamera_.getViewMatrix());
        setGlobalShaderParameters(slicingPrg, &lightCamera_);
        slicingPrg->setUniform("secondPass_", true);

        glBegin(GL_POLYGON);
            for (unsigned int curPoint=0; curPoint<6; curPoint++)
                glVertex2i(curPoint, curSlice);
        glEnd();
        MatStack.popMatrix();

        lightport_.deactivateTarget();
    }

    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);

    slicingPrg->deactivate();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string HalfAngleSlicer::generateHeader() {
    std::string headerSource = VolumeRenderer::generateHeader();
    headerSource += transferFunc_.get()->getShaderDefines();
    return headerSource;
}

} // namespace
