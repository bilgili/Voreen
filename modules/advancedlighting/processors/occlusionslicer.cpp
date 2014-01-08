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

#include "occlusionslicer.h"

#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "tgt/glmath.h"
#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

OcclusionSlicer::OcclusionSlicer()
    : VolumeSlicer()
    , shaderProp_("slicing.shader", "Slicing shader", "sl_occlusion.frag", "sl_singlevolume.vert")
    , occlusionbuffer0_(Port::OUTPORT, "image.occlusionbuffer0", "OcclusionBuffer0 Output", false, Processor::INVALID_RESULT,
        //RenderPort::RENDERSIZE_DEFAULT, GL_RGBA16F_ARB)
        RenderPort::RENDERSIZE_DEFAULT)
    , occlusionbuffer1_(Port::OUTPORT, "image.occlusionbuffer1", "OcclusionBuffer1 Output", false, Processor::INVALID_RESULT,
        /*RenderPort::RENDERSIZE_DEFAULT, GL_RGBA16F_ARB)*/
        RenderPort::RENDERSIZE_DEFAULT)
    , outport_(Port::OUTPORT, "image.eyebuffer", "EyeBuffer Output", true, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER)
    , cameraHandler_("cameraHandler", "Camera Handler", &camera_)
    , sigma_("sigma", "Sigma", 100.0f, 0.0f, 1000.0f)
    , radius_("radius", "Radius", 5, 0, 50)
{

    addInteractionHandler(cameraHandler_);

    addPrivateRenderPort(occlusionbuffer0_);
    addPrivateRenderPort(occlusionbuffer1_);
    addPort(outport_);

    addProperty(shaderProp_);
    addProperty(sigma_);
    addProperty(radius_);
    addProperty(lightPosition_);
}

void OcclusionSlicer::initialize() throw (tgt::Exception) {
    VolumeSlicer::initialize();
    compile();

    if (!shaderProp_.hasValidShader()){
        LERROR("Failed to load shader!");
        processorState_ = PROCESSOR_STATE_NOT_INITIALIZED;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }
    processorState_ = PROCESSOR_STATE_NOT_READY;
}

void OcclusionSlicer::compile() {
    shaderProp_.setHeader(generateHeader());
    shaderProp_.rebuild();
}

bool OcclusionSlicer::isReady() const {
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

void OcclusionSlicer::process() {

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    LGL_ERROR;

    occlusionbuffer0_.activateTarget();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    occlusionbuffer1_.activateTarget();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    outport_.activateTarget();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

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
    TextureUnit occlusionUnit;
    transferFunc_.get()->setUniform(slicingPrg, "transferFunc_", "transferFuncTex_", transferUnit.getUnitNumber());
    slicingPrg->setUniform("occlusion_", occlusionUnit.getUnitNumber());
    occlusionbuffer1_.setTextureParameters(slicingPrg, "occlusionParams_");

    //clipping uniforms
    setupUniforms(slicingPrg);

    // set common uniforms used by all shaders
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(slicingPrg, &cam);

    slicingPrg->setUniform("sigma_", sigma_.get());
    slicingPrg->setUniform("radius_", radius_.get());
    slicingPrg->setUniform("lightPos_", lightPosition_.get());

    // bind the volumes and pass the necessary information to the shader
    bindVolumes(slicingPrg, volumeTextures, &cam, lightPosition_.get());

    glDisable(GL_DEPTH_TEST);

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadMatrix(camera_.get().getProjectionMatrix(outport_.getSize()));

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadMatrix(camera_.get().getViewMatrix());

    unsigned int numSlices = static_cast<unsigned int>(maxLength_ / sliceDistance_);

    slicingPrg->activate();

    for (unsigned int curSlice=0; curSlice<numSlices; curSlice++) {
        // first pass
        slicingPrg->setUniform("secondPass_", false);
        outport_.activateTarget();
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);

        occlusionbuffer0_.bindColorTexture(occlusionUnit.getEnum());

        glBegin(GL_POLYGON);
        for (unsigned int curPoint=0; curPoint<6; curPoint++)
            glVertex2i(curPoint, curSlice);
        glEnd();
        glDisable(GL_BLEND);

        outport_.deactivateTarget();

        // second pass
        slicingPrg->setUniform("secondPass_", true);

        occlusionbuffer1_.activateTarget();

        slicingPrg->setUniform("blurDirection_", tgt::vec2(1.f, 0.f));
        glBegin(GL_POLYGON);
        for (unsigned int curPoint=0; curPoint<6; curPoint++)
            glVertex2i(curPoint, curSlice);
        glEnd();

        occlusionbuffer1_.deactivateTarget();
        occlusionbuffer0_.activateTarget();
        occlusionbuffer1_.bindColorTexture(occlusionUnit.getEnum());

        slicingPrg->setUniform("blurDirection_", tgt::vec2(0.f, 1.f));
        glBegin(GL_POLYGON);
        for (unsigned int curPoint=0; curPoint<6; curPoint++)
            glVertex2i(curPoint, curSlice);
        glEnd();

        occlusionbuffer0_.deactivateTarget();
    }

    slicingPrg->deactivate();

    glEnable(GL_DEPTH_TEST);

    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);

    glBlendFunc(GL_ONE, GL_ZERO);
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string OcclusionSlicer::generateHeader() {
    std::string headerSource = VolumeRenderer::generateHeader();
    headerSource += transferFunc_.get()->getShaderDefines();
    return headerSource;
}

} // namespace
