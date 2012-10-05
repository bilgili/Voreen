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

#include "shraycaster.h"

#include "voreen/core/interaction/camerainteractionhandler.h"
#include "tgt/camera.h"
#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

SHRaycaster::SHRaycaster()
    : VolumeRaycaster()
    , shaderProp_("shader.prop", "Shader program", "rc_sh.frag", "passthrough.vert")
    , transferFunc_("transferFunction", "Transfer function")
    , camera_("camera", "Camera", tgt::Camera(tgt::vec3(0.f, 0.f, 3.5f), tgt::vec3(0.f, 0.f, 0.f), tgt::vec3(0.f, 1.f, 0.f)))
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle", "volumehandle.volumehandle", false, Processor::INVALID_PROGRAM)
    , entryPort_(Port::INPORT, "image.entrypoints", "Entry-points Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , exitPort_(Port::INPORT, "image.exitpoints", "Exit-points Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , outport_(Port::OUTPORT, "image.output", "Image Output", true, Processor::INVALID_PROGRAM, RenderPort::RENDERSIZE_RECEIVER)
{

    addProperty(shaderProp_);
    addProperty(transferFunc_);
    addProperty(camera_);

    addProperty(gradientMode_);
    addProperty(compositingMode_);

    // lines needed for sh
    sh_ = new SHClass(this, &transferFunc_);

    std::vector<Property*> props = sh_->getProps();
    for(size_t i = 0; i < props.size(); i++)
        addProperty(props[i]);

    addInteractionHandler(sh_->getLightHandler());

    addPort(&volumeInport_);
    addPort(&entryPort_);
    addPort(&exitPort_);
    addPort(&outport_);
}

SHRaycaster::~SHRaycaster() {
    // line needed for sh
    delete sh_;
}

void SHRaycaster::initialize() throw (tgt::Exception) {
    sh_->initialize();
    VolumeRaycaster::initialize();
    compile();

    if(!shaderProp_.getShader()) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }

    portGroup_.initialize();
    portGroup_.addPort(&outport_);
}

void SHRaycaster::deinitialize() throw (tgt::Exception) {
    portGroup_.deinitialize();
    LGL_ERROR;

    VolumeRaycaster::deinitialize();
}

void SHRaycaster::compile() {
    shaderProp_.setHeader(generateHeader());
    shaderProp_.rebuild();
}

bool SHRaycaster::isReady() const {
    //check if all inports are connected:
    if(!entryPort_.isReady() || !exitPort_.isReady() || !volumeInport_.isReady())
        return false;

    //check if at least one outport is connected:
    if(!outport_.isReady())
        return false;

    if(!shaderProp_.hasValidShader())
        return false;

    return true;
}

void SHRaycaster::beforeProcess() {
    VolumeRaycaster::beforeProcess();

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM) {
        { // line needed for sh
            PROFILING_BLOCK("shcalc");
            sh_->initAndCalcCoeffs(volumeInport_.getData());
        }
        {
            PROFILING_BLOCK("compile");
            compile();
        }
    }
    LGL_ERROR;

    transferFunc_.setVolumeHandle(volumeInport_.getData());
}

void SHRaycaster::process() {

    // bind transfer function
    TextureUnit transferUnit;
    transferUnit.activate();
    if (transferFunc_.get())
        transferFunc_.get()->bind();

    portGroup_.activateTargets();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    //transferFunc_.setVolumeHandle(volumeInport_.getData());

    TextureUnit entryUnit, entryDepthUnit, exitUnit, exitDepthUnit;
    // bind entry params
    entryPort_.bindTextures(entryUnit.getEnum(), entryDepthUnit.getEnum());
    LGL_ERROR;

    // bind exit params
    exitPort_.bindTextures(exitUnit.getEnum(), exitDepthUnit.getEnum());
    LGL_ERROR;

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add main volume
    TextureUnit volUnit;
    volumeTextures.push_back(VolumeStruct(
        volumeInport_.getData(),
        &volUnit,
        "volume_","volumeStruct_")
    );

    // initialize shader
    tgt::Shader* raycastPrg = shaderProp_.getShader();
    raycastPrg->activate();

    // set common uniforms used by all shaders
    setGlobalShaderParameters(raycastPrg);

    // bind the volumes and pass the necessary information to the shader
    tgt::Camera cam = camera_.get();
    bindVolumes(raycastPrg, volumeTextures, &cam, lightPosition_.get());

    // line needed for sh
    sh_->setRCShaderUniforms(raycastPrg);

    // pass the remaining uniforms to the shader
    raycastPrg->setUniform("entryPoints_", entryUnit.getUnitNumber());
    //raycastPrg->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    entryPort_.setTextureParameters(raycastPrg, "entryParameters_");
    raycastPrg->setUniform("exitPoints_", exitUnit.getUnitNumber());
    //raycastPrg->setUniform("exitPointsDepth_", exitDepthUnit.getUnitNumber());
    exitPort_.setTextureParameters(raycastPrg, "exitParameters_");

    if (classificationMode_.get() != "none")
        transferFunc_.get()->setUniform(raycastPrg, "transferFunc_", "transferFuncTex_", transferUnit.getUnitNumber());

    LGL_ERROR;

    renderQuad();

    raycastPrg->deactivate();
    portGroup_.deactivateTargets();

    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string SHRaycaster::generateHeader() {
    std::string headerSource = VolumeRaycaster::generateHeader();

    headerSource += transferFunc_.get()->getShaderDefines();

    // line needed for sh
    headerSource += sh_->getShaderDefines();

    portGroup_.reattachTargets();
    headerSource += portGroup_.generateHeader(shaderProp_.getShader());
    return headerSource;
}

} // namespace
