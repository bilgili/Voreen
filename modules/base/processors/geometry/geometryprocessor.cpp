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

#include "geometryprocessor.h"
#include "voreen/core/interaction/idmanager.h"
#include "voreen/core/interaction/camerainteractionhandler.h"
#include "voreen/core/processors/geometryrendererbase.h"

#include "tgt/textureunit.h"
#include "tgt/glmath.h"
#include "tgt/vector.h"

using tgt::vec4;
using tgt::vec3;
using tgt::TextureUnit;

namespace voreen {

GeometryProcessor::GeometryProcessor()
    : RenderProcessor()
    , shaderPrg_(0)
    , renderGeometries_("renderGeometries", "Render Geometries", true)
    , camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , inport_(Port::INPORT, "image.input", "Image Input", false, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_ORIGIN)
    , outport_(Port::OUTPORT, "image.output", "Image Output", true, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER)
    , tempPort_(Port::OUTPORT, "image.temp")
    , pickingPort_(Port::OUTPORT, "pickingTarget")
    , cpPort_(Port::INPORT, "coprocessor.geometryrenderers", "GeometryRenderers", true)
{
    addProperty(renderGeometries_);
    addProperty(camera_);
    cameraHandler_ = new CameraInteractionHandler("cameraHandler", "Camera", &camera_);
    cameraHandler_->setEnabled(false); // not needed most of the time
    addInteractionHandler(cameraHandler_);

    addPort(inport_);
    addPort(outport_);
    outport_.onSizeReceiveChange<GeometryProcessor>(this, &GeometryProcessor::passThroughSizeRequest);

    addPrivateRenderPort(tempPort_);
    addPrivateRenderPort(pickingPort_);
    addPort(cpPort_);
}

GeometryProcessor::~GeometryProcessor() {
    delete cameraHandler_;
}

void GeometryProcessor::initialize() throw (tgt::Exception) {
    RenderProcessor::initialize();

    idManager_.setRenderTarget(pickingPort_.getRenderTarget());
    idManager_.initializeTarget();

    shaderPrg_ = ShdrMgr.loadSeparate("passthrough.vert", "image/compositor.frag",
        generateHeader(), false);
}

void GeometryProcessor::deinitialize() throw (tgt::Exception) {
    if (shaderPrg_)
        ShdrMgr.dispose(shaderPrg_);
    shaderPrg_ = 0;

    RenderProcessor::deinitialize();
}

Processor* GeometryProcessor::create() const {
    return new GeometryProcessor();
}

bool GeometryProcessor::isReady() const {
    if (!outport_.isReady())
        return false;
    return true;
}

void GeometryProcessor::process() {

    // set modelview and projection matrices
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.loadMatrix(camera_.get().getProjectionMatrix(outport_.getSize()));
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.loadMatrix(camera_.get().getViewMatrix());
    LGL_ERROR;

    //
    // render geometry
    //

    //if there is no data on the inport we can render directly into the outport:
    if (!inport_.isReady())
        outport_.activateTarget();
    else
        tempPort_.activateTarget("internal");

    LGL_ERROR;
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    std::vector<GeometryRendererBase*> portData = cpPort_.getConnectedProcessors();
    if (renderGeometries_.get()) {
        for (size_t i=0; i<portData.size(); i++) {
            GeometryRendererBase* geomRenderer = portData.at(i);
            if(geomRenderer->isReady()) {
                geomRenderer->setCamera(camera_.get());
                geomRenderer->setViewport(outport_.getSize());
                geomRenderer->render();
                LGL_ERROR;
            }
        }
    }
    if (!inport_.isReady())
        outport_.deactivateTarget();
    else
        tempPort_.deactivateTarget();
    LGL_ERROR;

    //
    // render picking objects
    //
    idManager_.activateTarget(getID());
    idManager_.clearTarget();
    if (renderGeometries_.get()) {
        for (size_t i=0; i<portData.size(); i++) {
            GeometryRendererBase* geomRenderer = portData.at(i);
            if (geomRenderer->isReady()) {
                geomRenderer->setIDManager(&idManager_);
                geomRenderer->renderPicking();
                LGL_ERROR;
            }
        }
    }
    idManager_.deactivateTarget();

    // restore matrices
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.loadIdentity();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.loadIdentity();
    LGL_ERROR;

    // if inport is connected, combine both results:
    if (inport_.isReady()) {
        outport_.activateTarget();

        glClearDepth(1.0);
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        LGL_ERROR;

        inport_.bindTextures(GL_TEXTURE0, GL_TEXTURE1);
        tempPort_.bindTextures(GL_TEXTURE2, GL_TEXTURE3);

        shaderPrg_->activate();

        tgt::Camera cam = camera_.get();
        setGlobalShaderParameters(shaderPrg_, &cam);
        shaderPrg_->setUniform("colorTex0_", 0);
        shaderPrg_->setUniform("depthTex0_", 1);
        inport_.setTextureParameters(shaderPrg_, "textureParameters0_");

        shaderPrg_->setUniform("colorTex1_", 2);
        shaderPrg_->setUniform("depthTex1_", 3);
        tempPort_.setTextureParameters(shaderPrg_, "textureParameters1_");

        renderQuad();
        shaderPrg_->deactivate();
        outport_.deactivateTarget();
        LGL_ERROR;
    }

    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string GeometryProcessor::generateHeader() {
    std::string header = RenderProcessor::generateHeader();
    header += "#define MODE_ALPHA_COMPOSITING\n";
    return header;
}

void GeometryProcessor::passThroughSizeRequest() {
    inport_.requestSize(outport_.getReceivedSize());
}

} // namespace voreen
