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

#include "simpleraycaster.h"

#include "voreen/core/properties/cameraproperty.h"
#include "tgt/textureunit.h"

using tgt::vec3;
using tgt::TextureUnit;

namespace voreen {

SimpleRaycaster::SimpleRaycaster()
    : VolumeRaycaster()
    , volumePort_(Port::INPORT, "volumehandle.volumehandle", "Volume Input")
    , entryPort_(Port::INPORT, "image.entrypoints", "Entry-points Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , exitPort_(Port::INPORT, "image.exitpoints", "Exit-points Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , outport_(Port::OUTPORT, "image.output", "Image Output", true, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER)
    , shader_("shader", "Shader", "rc_simple.frag", "passthrough.vert")
    , transferFunc_("transferFunction", "Transfer Function", Processor::INVALID_RESULT,
        TransFuncProperty::Editors(TransFuncProperty::INTENSITY | TransFuncProperty::INTENSITY_RAMP))
    , camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
{
    volumePort_.showTextureAccessProperties(true);
    addPort(volumePort_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);

    addProperty(shader_);
    addProperty(transferFunc_);

    // camera is required for depth value calculation
    addProperty(camera_);
}

Processor* SimpleRaycaster::create() const {
    return new SimpleRaycaster();
}

void SimpleRaycaster::initialize() throw (tgt::Exception) {
    VolumeRaycaster::initialize();
    rebuildShader();
}

void SimpleRaycaster::deinitialize() throw (tgt::Exception) {
    VolumeRaycaster::deinitialize();
}

void SimpleRaycaster::beforeProcess() {
    VolumeRaycaster::beforeProcess();

    // rebuild shader, if changed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        rebuildShader();

    // assign volume to transfer function
    transferFunc_.setVolumeHandle(volumePort_.getData());
    LGL_ERROR;
}

void SimpleRaycaster::process() {
    // activate and clear output render target
    outport_.activateTarget();
    outport_.clearTarget();

    // retrieve shader from shader property
    tgt::Shader* shader = shader_.getShader();
    if (!shader || !shader->isLinked()) {
        outport_.deactivateTarget();
        return;
    }

    // activate shader and set common uniforms
    shader->activate();
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(shader, &cam);

    // bind entry and exit params and pass texture units to the shader
    TextureUnit entryUnit, entryDepthUnit, exitUnit, exitDepthUnit;
    entryPort_.bindTextures(entryUnit, entryDepthUnit);
    shader->setUniform("entryPoints_", entryUnit.getUnitNumber());
    shader->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    entryPort_.setTextureParameters(shader, "entryParameters_");

    exitPort_.bindTextures(exitUnit, exitDepthUnit);
    shader->setUniform("exitPoints_", exitUnit.getUnitNumber());
    shader->setUniform("exitPointsDepth_", exitDepthUnit.getUnitNumber());
    exitPort_.setTextureParameters(shader, "exitParameters_");

    // bind volume texture and pass it to the shader
    std::vector<VolumeStruct> volumeTextures;
    TextureUnit volUnit;
     volumeTextures.push_back(VolumeStruct(
        volumePort_.getData(),
        &volUnit,
        "volume_",
        "volumeStruct_",
        volumePort_.getTextureClampModeProperty().getValue(),
        tgt::vec4(volumePort_.getTextureBorderIntensityProperty().get()),
        volumePort_.getTextureFilterModeProperty().getValue())
    );
    bindVolumes(shader, volumeTextures, &cam, lightPosition_.get());

    // bind transfer function and pass it to the shader
    TextureUnit transferUnit;
    if (transferFunc_.get()) {
        transferUnit.activate();
        transferFunc_.get()->bind();
        transferFunc_.get()->setUniform(shader, "transferFunc_", "transferFuncTex_", transferUnit.getUnitNumber());
    }

    // render screen aligned quad
    renderQuad();

    // clean up
    shader->deactivate();
    outport_.deactivateTarget();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

void SimpleRaycaster::rebuildShader() {
    shader_.setHeader(generateHeader());
    shader_.rebuild();
}

std::string SimpleRaycaster::generateHeader() {
    std::string header = VolumeRaycaster::generateHeader();

    header += transferFunc_.get()->getShaderDefines();
    return header;
}

} // namespace voreen
