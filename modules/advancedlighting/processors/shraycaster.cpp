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
    , camera_("camera", "Camera", tgt::Camera(tgt::vec3(0.f, 0.f, 3.5f), tgt::vec3(0.f, 0.f, 0.f), tgt::vec3(0.f, 1.f, 0.f)), true)
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle", "volumehandle.volumehandle", false, Processor::INVALID_PROGRAM)
    , entryPort_(Port::INPORT, "image.entrypoints", "Entry-points Input", false, Processor::INVALID_RESULT)
    , exitPort_(Port::INPORT, "image.exitpoints", "Exit-points Input", false, Processor::INVALID_RESULT)
    , outport_(Port::OUTPORT, "image.output", "Image Output", true, Processor::INVALID_PROGRAM)
    , internalRenderPort_(Port::OUTPORT, "internalRenderPort", "Internal Render Port")
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

    addPort(volumeInport_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);

    addPrivateRenderPort(internalRenderPort_);
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
        processorState_ = PROCESSOR_STATE_NOT_INITIALIZED;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }
}

void SHRaycaster::compile() {
    shaderProp_.setHeader(generateHeader());
    shaderProp_.rebuild();
}

bool SHRaycaster::isReady() const {
    //check if all inports are connected:
    if(!entryPort_.isReady() || !exitPort_.isReady() || !volumeInport_.isReady())
        return false;

    //check if at outport is connected
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

    if(volumeInport_.hasChanged() && volumeInport_.hasData())
        camera_.adaptInteractionToScene(volumeInport_.getData()->getBoundingBox().getBoundingBox());
}

void SHRaycaster::process() {

    const bool renderCoarse = interactionMode() && interactionCoarseness_.get() > 1;
    tgt::svec2 renderSize;
    if (renderCoarse) {
        renderSize = outport_.getSize() / interactionCoarseness_.get();
        internalRenderPort_.resize(renderSize);
    }
    else {
        renderSize = outport_.getSize();
    }

    RenderPort& renderDestination = (renderCoarse ? internalRenderPort_ : outport_);
    renderDestination.activateTarget();
    renderDestination.clearTarget();

    LGL_ERROR;

    //transferFunc_.setVolumeHandle(volumeInport_.getData());

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
    setGlobalShaderParameters(raycastPrg, 0, renderSize);

    // bind the volumes and pass the necessary information to the shader
    tgt::Camera cam = camera_.get();
    bindVolumes(raycastPrg, volumeTextures, &cam, lightPosition_.get());

    // line needed for sh
    sh_->setRCShaderUniforms(raycastPrg);

    TextureUnit entryUnit, entryDepthUnit, exitUnit, exitDepthUnit;
    entryPort_.bindTextures(entryUnit, entryDepthUnit, GL_NEAREST);
    exitPort_.bindTextures(exitUnit, exitDepthUnit, GL_NEAREST);
    LGL_ERROR;

    // pass the remaining uniforms to the shader
    raycastPrg->setUniform("entryPoints_", entryUnit.getUnitNumber());
    raycastPrg->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    entryPort_.setTextureParameters(raycastPrg, "entryParameters_");
    raycastPrg->setUniform("exitPoints_", exitUnit.getUnitNumber());
    raycastPrg->setUniform("exitPointsDepth_", exitDepthUnit.getUnitNumber());
    exitPort_.setTextureParameters(raycastPrg, "exitParameters_");

    TextureUnit transferUnit;
    if (classificationMode_.get() != "none") {
        // bind transfer function
        transferUnit.activate();
        if (transferFunc_.get())
            transferFunc_.get()->bind();

        transferFunc_.get()->setUniform(raycastPrg, "transferFunc_", "transferFuncTex_", transferUnit.getUnitNumber());
    }

    LGL_ERROR;

    renderQuad();

    raycastPrg->deactivate();
    renderDestination.deactivateTarget();

    // copy over rendered image from internal port to outport,
    // thereby rescaling it to outport dimensions
    if (renderCoarse && outport_.isConnected())
        rescaleRendering(internalRenderPort_, outport_);

    TextureUnit::setZeroUnit();
    LGL_ERROR;

    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string SHRaycaster::generateHeader() {
    std::string headerSource = VolumeRaycaster::generateHeader();

    headerSource += transferFunc_.get()->getShaderDefines();

    // line needed for sh
    headerSource += sh_->getShaderDefines();
    return headerSource;
}

} // namespace
