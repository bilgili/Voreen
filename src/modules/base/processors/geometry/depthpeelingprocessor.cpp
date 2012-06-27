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

#include "voreen/modules/base/processors/geometry/depthpeelingprocessor.h"
//#include "voreen/modules/base/processors/geometry/geometryprocessor.h"
#include "voreen/core/processors/geometryrendererbase.h"
#include "voreen/core/interaction/camerainteractionhandler.h"

#include <typeinfo>

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/vector.h"
#include "tgt/quadric.h"
#include "tgt/textureunit.h"

using tgt::vec4;
using tgt::vec3;
using tgt::TextureUnit;

namespace voreen {

DepthPeelingProcessor::DepthPeelingProcessor()
    : RenderProcessor()
    , inport_(Port::INPORT, "image.input")
    , outport_(Port::OUTPORT, "image.output")
    , tempPort_(Port::OUTPORT, "image.temp")
    , cpPort_(Port::INPORT, "coprocessor.geometryrenderers", true)
    , shaderPrg_(0)
    , camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
{

    addProperty(camera_);
    cameraHandler_ = new CameraInteractionHandler("cameraHandler", "Camera Handler", &camera_);
    addInteractionHandler(cameraHandler_);

    addPort(inport_);
    addPort(outport_);
    addPrivateRenderPort(tempPort_);
    addPort(cpPort_);
}

DepthPeelingProcessor::~DepthPeelingProcessor() {
    delete cameraHandler_;
}

Processor* DepthPeelingProcessor::create() const {
    return new DepthPeelingProcessor();
}

std::string DepthPeelingProcessor::getProcessorInfo() const {
    return "This processor can be used instead of the regular GeometryProcessor in order "
           "to peel away layers within a rendering as described in the white paper "
           "'Order-Independent Transparency' by Cass Everitt. The peeling can be controlled "
           "by encapsulating the processor within a RenderLoopInitiator and a RenderLoopFinalizer, "
           "whereby the number of iterations controls the number of layers peeled away. "
           "Since the peeling is performed within a pair of vertex and fragment shader, "
           "the attached GeometryRenderes must not use these shaders themselves. "
           "Since the input received through the inport is used as first layer, "
           "the depth values should be set accordingly.";
}

bool DepthPeelingProcessor::isReady() const {
    if (!inport_.isConnected() || !outport_.isReady())
        return false;
    return true;
}

void DepthPeelingProcessor::initialize() throw (VoreenException) {
    RenderProcessor::initialize();

    shaderPrg_ = ShdrMgr.loadSeparate("image/depthpeeling.vert", "image/depthpeeling.frag",
        generateHeader(), false);

    if (!shaderPrg_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }

    initialized_ = true;
}

void DepthPeelingProcessor::deinitialize() throw (VoreenException) {
    if (shaderPrg_)
        ShdrMgr.dispose(shaderPrg_);
    shaderPrg_ = 0;

    RenderProcessor::deinitialize();
}

void DepthPeelingProcessor::process() {
    outport_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    tgt::loadMatrix(camera_.get().getProjectionMatrix());
    glMatrixMode(GL_MODELVIEW);
    tgt::loadMatrix(camera_.get().getViewMatrix());
    LGL_ERROR;

    TextureUnit depthTexUnit;
    inport_.bindDepthTexture(depthTexUnit.getEnum());
    LGL_ERROR;

    // initialize shader
    shaderPrg_->activate();

    // set common uniforms used by all shaders
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(shaderPrg_, &cam);

    // pass the remaining uniforms to the shader
    shaderPrg_->setUniform("depthTex_", depthTexUnit.getUnitNumber());
    inport_.setTextureParameters(shaderPrg_, "depthTexParameters_");

    std::vector<GeometryRendererBase*> portData = cpPort_.getConnectedProcessors();
    for (size_t i=0; i<portData.size(); i++) {
        GeometryRendererBase* pdcp = portData.at(i);
        if(pdcp->isReady()) {
            pdcp->setCamera(camera_.get());
            pdcp->setViewport(outport_.getSize());
            pdcp->render();
            LGL_ERROR;
        }
    }

    shaderPrg_->deactivate();
    outport_.deactivateTarget();

    // restore matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    LGL_ERROR;
}

} // namespace voreen
