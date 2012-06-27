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

#include "voreen/modules/base/processors/geometry/eepgeometryintegrator.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

EEPGeometryIntegrator::EEPGeometryIntegrator()
    : ImageProcessor("eep_geometry")
    , inport0_(Port::INPORT, "image.entry")
    , inport1_(Port::INPORT, "image.exit")
    , geometryPort_(Port::INPORT, "image.geometry")
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle", false, Processor::INVALID_RESULT)
    , entryPort_(Port::OUTPORT, "image.postentry", true, Processor::INVALID_PROGRAM)
    , exitPort_(Port::OUTPORT, "image.postexit", true, Processor::INVALID_PROGRAM)
    , tmpPort_(Port::OUTPORT, "image.tmp", false)
    , useFloatRenderTargets_("useFloatRenderTargets", "Use float rendertargets", false)
    , camera_("camera", "Camera", tgt::Camera(tgt::vec3(0.f, 0.f, 3.5f), tgt::vec3(0.f, 0.f, 0.f), tgt::vec3(0.f, 1.f, 0.f)))
{
    addPort(inport0_);
    addPort(inport1_);
    addPort(geometryPort_);
    addPort(volumeInport_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPrivateRenderPort(&tmpPort_);

    addProperty(useFloatRenderTargets_);
    addProperty(camera_);
}

std::string EEPGeometryIntegrator::getProcessorInfo() const {
    return "Modifies raycasting entry/exit points with regard to the input geometry rendering "
           "such that alpha compositing of the geometry rendering with the raycasted image "
           "gives correct results (for opaque geometry). "
           "The raycasted volume has to be passed to the volume inport.<br/>"
           "See Compositor.";
}

Processor* EEPGeometryIntegrator::create() const {
    return new EEPGeometryIntegrator();
}

void EEPGeometryIntegrator::beforeProcess() {
    RenderProcessor::beforeProcess();

    RenderPort& refPort = (entryPort_.isReady() ? entryPort_ : exitPort_);

    if (useFloatRenderTargets_.get()) {
        if (refPort.getRenderTarget()->getColorTexture()->getDataType() != GL_FLOAT) {
            entryPort_.changeFormat(GL_RGBA16F_ARB);
            exitPort_.changeFormat(GL_RGBA16F_ARB);
            tmpPort_.changeFormat(GL_RGBA16F_ARB);
        }
    }
    else {
        if (refPort.getRenderTarget()->getColorTexture()->getDataType() == GL_FLOAT) {
            entryPort_.changeFormat(GL_RGBA16);
            exitPort_.changeFormat(GL_RGBA16);
            tmpPort_.changeFormat(GL_RGBA16);
        }
    }
}

void EEPGeometryIntegrator::process() {

    entryPort_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextureUnit colorUnit0, depthUnit0, colorUnit1, depthUnit1, colorUnit2, depthUnit2;

    inport0_.bindTextures(colorUnit0.getEnum(), depthUnit0.getEnum());
    inport1_.bindTextures(colorUnit1.getEnum(), depthUnit1.getEnum());
    geometryPort_.bindTextures(colorUnit2.getEnum(), depthUnit2.getEnum());

    // initialize shader
    program_->activate();
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(program_, &cam);
    program_->setIgnoreUniformLocationError(true);
    program_->setUniform("entryParams_", colorUnit0.getUnitNumber());
    program_->setUniform("entryParamsDepth_", depthUnit0.getUnitNumber());
    program_->setUniform("exitParams_", colorUnit1.getUnitNumber());
    program_->setUniform("exitParamsDepth_", depthUnit1.getUnitNumber());
    program_->setUniform("geometryTex_", colorUnit2.getUnitNumber());
    program_->setUniform("geometryTexDepth_", depthUnit2.getUnitNumber());

    // assume cube-formed dataset if the port is not connected
    if(volumeInport_.isReady())
        program_->setUniform("volumeSize_", volumeInport_.getData()->getVolume()->getCubeSize());
    else {
        LWARNING("Volume inport not connected: assuming cubic volume");
        program_->setUniform("volumeSize_", tgt::vec3(2.f));
    }

    inport0_.setTextureParameters(program_, "entryInfo_");
    inport1_.setTextureParameters(program_, "exitInfo_");
    geometryPort_.setTextureParameters(program_, "geomInfo_");
    program_->setIgnoreUniformLocationError(false);

    program_->setUniform("entry_", true);

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    exitPort_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program_->setUniform("entry_", false);
    program_->setUniform("near_", camera_.get().getNearDist());
    program_->setUniform("far_", camera_.get().getFarDist());
    program_->setUniform("useFloatTarget_", useFloatRenderTargets_.get());

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    program_->deactivate();
    entryPort_.deactivateTarget();
    glActiveTexture(GL_TEXTURE0);

    LGL_ERROR;
}

bool EEPGeometryIntegrator::isReady() const {
    if (!isInitialized())
        return false;

    if (!inport0_.isReady() || !inport1_.isReady() || !geometryPort_.isReady())
        return false;

    if(!entryPort_.isReady() || !exitPort_.isReady())
        return false;

    return true;
}

} // voreen namespace
