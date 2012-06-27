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

#include "voreen/modules/base/processors/render/simpleraycaster.h"

#include "voreen/core/properties/cameraproperty.h"
#include "tgt/textureunit.h"

using tgt::vec3;
using tgt::TextureUnit;

namespace voreen {

SimpleRaycaster::SimpleRaycaster()
    : VolumeRaycaster()
    , volumePort_(Port::INPORT, "volumehandle.volumehandle")
    , entryPort_(Port::INPORT, "image.entrypoints")
    , exitPort_(Port::INPORT, "image.exitpoints")
    , outport_(Port::OUTPORT, "image.output", true)
    , transferFunc_("transferFunction", "Transfer Function", Processor::INVALID_RESULT,
        TransFuncProperty::Editors(TransFuncProperty::INTENSITY | TransFuncProperty::INTENSITY_RAMP))
    , camera_("camera", "Camera", new tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
{

    addPort(volumePort_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);

    addProperty(transferFunc_);
    addProperty(camera_);
    camera_.setVisible(false);
}

Processor* SimpleRaycaster::create() const {
    return new SimpleRaycaster();
}

std::string SimpleRaycaster::getProcessorInfo() const {
    return "Performs a simple single pass raycasting without lighting.";
}

void SimpleRaycaster::initialize() throw (VoreenException) {
    VolumeRaycaster::initialize();

    // load shader
    raycastPrg_ = ShdrMgr.loadSeparate("passthrough.vert", "rc_simple.frag",
        generateHeader(), false);
    if (!raycastPrg_)
        throw VoreenException("Failed to load shaders");

    // note: shader is disposed by VolumeRaycaster::deinitialize
}

void SimpleRaycaster::beforeProcess() {
    // assign volume to transfer function
    transferFunc_.setVolumeHandle(volumePort_.getData());
    LGL_ERROR;
}

void SimpleRaycaster::process() {
    // activate and clear output render target
    outport_.activateTarget();
    outport_.clearTarget();

    // activate shader and set common uniforms
    raycastPrg_->activate();
    setGlobalShaderParameters(raycastPrg_, camera_.get());

    // bind entry and exit params and pass texture units to the shader
    TextureUnit entryUnit, entryDepthUnit, exitUnit, exitDepthUnit;
    entryPort_.bindTextures(entryUnit, entryDepthUnit);
    raycastPrg_->setUniform("entryPoints_", entryUnit.getUnitNumber());
    raycastPrg_->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    entryPort_.setTextureParameters(raycastPrg_, "entryParameters_");

    exitPort_.bindTextures(exitUnit, exitDepthUnit);
    raycastPrg_->setUniform("exitPoints_", exitUnit.getUnitNumber());
    raycastPrg_->setUniform("exitPointsDepth_", exitDepthUnit.getUnitNumber());
    exitPort_.setTextureParameters(raycastPrg_, "exitParameters_");

    // bind volume texture and pass it to the shader
    std::vector<VolumeStruct> volumeTextures;
    TextureUnit volUnit;
     volumeTextures.push_back(VolumeStruct(
        volumePort_.getData()->getVolumeGL(),
        &volUnit,
        "volume_",
        "volumeParameters_",
        true)
    );
    bindVolumes(raycastPrg_, volumeTextures, camera_.get(), lightPosition_.get());

    // bind transfer function and pass it to the shader
    TextureUnit transferUnit;
    if (transferFunc_.get()) {
        transferUnit.activate();
        transferFunc_.get()->bind();
        raycastPrg_->setUniform("transferFunc_", transferUnit.getUnitNumber());
    }

    // render screen aligned quad
    renderQuad();

    // clean up
    raycastPrg_->deactivate();
    outport_.deactivateTarget();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string SimpleRaycaster::generateHeader(VolumeHandle* volumeHandle) {
    std::string header = VolumeRaycaster::generateHeader(volumeHandle);

    header += transferFunc_.get()->getShaderDefines();
    return header;
}

} // namespace voreen
