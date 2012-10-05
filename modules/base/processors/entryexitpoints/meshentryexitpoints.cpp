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

#include "meshentryexitpoints.h"

#include "voreen/core/interaction/camerainteractionhandler.h"

#include "tgt/glmath.h"
#include "tgt/gpucapabilities.h"
#include "tgt/texturemanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

using tgt::vec3;
using tgt::mat4;
using tgt::TextureUnit;

namespace voreen {

const std::string MeshEntryExitPoints::loggerCat_("voreen.MeshEntryExitPoints");

MeshEntryExitPoints::MeshEntryExitPoints()
    : RenderProcessor()
    , entryPort_(Port::OUTPORT, "image.entrypoints", "Entry-points Output", true, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER)
    , exitPort_(Port::OUTPORT, "image.exitpoints", "Exit-points Output", true, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER)
    , inport_(Port::INPORT, "proxgeometry.geometry", "Proxy Geometry Input")
    , tmpPort_(Port::OUTPORT, "image.tmp", "image.tmp", false)
    , supportCameraInsideVolume_("supportCameraInsideVolume", "Support camera in volume", true)
    , jitterEntryPoints_("jitterEntryPoints", "Jitter entry params", false)
    , useFloatRenderTargets_("useFloatRenderTargets", "Use float render targets", false)
    , useCulling_("useCulling", "Use culling", true)
    , jitterStepLength_("jitterStepLength", "Jitter step length", 0.005f, 0.0005f, 0.025f)
    , camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , shaderProgram_(0)
    , shaderProgramJitter_(0)
{
    addProperty(supportCameraInsideVolume_);

    // jittering
    addProperty(jitterEntryPoints_);
    jitterEntryPoints_.onChange(CallMemberAction<MeshEntryExitPoints>(this, &MeshEntryExitPoints::onJitterEntryPointsChanged));
    onJitterEntryPointsChanged(); // set initial state
    jitterStepLength_.setStepping(0.001f);
    jitterStepLength_.setNumDecimals(3);
    addProperty(jitterStepLength_);
    addProperty(useFloatRenderTargets_);
    addProperty(useCulling_);

    addProperty(camera_);

    cameraHandler_ = new CameraInteractionHandler("cameraHandler", "Camera", &camera_);
    addInteractionHandler(cameraHandler_);

    addPort(entryPort_);
    addPort(exitPort_);
    addPort(inport_);
    addPrivateRenderPort(&tmpPort_);
}

MeshEntryExitPoints::~MeshEntryExitPoints() {
    delete cameraHandler_;
}

Processor* MeshEntryExitPoints::create() const {
    return new MeshEntryExitPoints();
}

void MeshEntryExitPoints::initialize() throw (tgt::Exception) {
    RenderProcessor::initialize();

    shaderProgram_ = ShdrMgr.load("eep_simple", generateHeader(), false);

    shaderProgramJitter_ = ShdrMgr.loadSeparate("passthrough.vert", "eep_jitter.frag",
                                                 generateHeader(), false);
}

void MeshEntryExitPoints::deinitialize() throw (tgt::Exception) {
    ShdrMgr.dispose(shaderProgram_);
    shaderProgram_ = 0;

    ShdrMgr.dispose(shaderProgramJitter_);
    shaderProgramJitter_ = 0;

    RenderProcessor::deinitialize();
}

bool MeshEntryExitPoints::isReady() const {
    // We want to render if at least one of the outports is connected
    return ((entryPort_.isReady() || exitPort_.isReady()) && inport_.isReady());
}

void MeshEntryExitPoints::beforeProcess() {
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

void MeshEntryExitPoints::process() {
    // retrieve input geometry
    const MeshListGeometry* meshListGeometry = dynamic_cast<const MeshListGeometry*>(inport_.getData());
    const MeshGeometry* meshGeometry = dynamic_cast<const MeshGeometry*>(inport_.getData());
    if (meshListGeometry) {
        geometry_ = *meshListGeometry;
    }
    else if (meshGeometry) {
        geometry_.clear();
        geometry_.addMesh(*meshGeometry);
    }
    else {
        LERROR("Geometry of type MeshListGeometry/MeshGeometry expected.");
        return;
    }

    // A new volume was loaded
    if(inport_.hasChanged())
        cameraHandler_->adaptInteractionToScene(geometry_);

    // set modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    tgt::loadMatrix(camera_.get().getProjectionMatrix(entryPort_.isReady() ? entryPort_.getSize() : exitPort_.getSize()));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    tgt::loadMatrix(camera_.get().getViewMatrix());

    // enable culling
    if(useCulling_.get())
        glEnable(GL_CULL_FACE);

    // activate shader program
    shaderProgram_->activate();
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(shaderProgram_, &cam);
    LGL_ERROR;

    //
    // render back texture
    //
    if (exitPort_.isReady()) {
        exitPort_.activateTarget();
        glClearDepth(0.0f);
        glDepthFunc(GL_GREATER);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);

        geometry_.render();
        LGL_ERROR;
        exitPort_.deactivateTarget();
        glDepthFunc(GL_LESS);
        glClearDepth(1.0f);
        LGL_ERROR;
    }

    //
    // render front texture
    //
    // use temporary target if necessary
    if (entryPort_.isReady()) {
        if (!jitterEntryPoints_.get())
            entryPort_.activateTarget();
        else
            tmpPort_.activateTarget();
        LGL_ERROR;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);

        geometry_.render();
        LGL_ERROR;

        if (supportCameraInsideVolume_.get()) {
            // clip proxy geometry against near-plane
            float nearPlaneDistToOrigin = tgt::dot(camera_.get().getPosition(), -camera_.get().getLook()) - camera_.get().getNearDist() - 0.0001f;
            MeshListGeometry closingFaces;
            double epsilon = static_cast<double>(tgt::length(geometry_.getBoundingBox().diagonal())) * 1e-6;
            geometry_.clip(tgt::vec4(-camera_.get().getLook(), nearPlaneDistToOrigin), closingFaces, epsilon);

            // render closing face separately, if not empty
            if (!closingFaces.empty()) {
                // project closing faces onto near-plane
                tgt::mat4 trafoMatrix = camera_.get().getProjectionMatrix(entryPort_.getSize()) * camera_.get().getViewMatrix();
                closingFaces.transform(trafoMatrix);
                // set z-coord of closing face vertices to 0.0 in order to avoid near-plane clipping
                tgt::mat4 zTrafo = tgt::mat4::createIdentity();
                zTrafo[2][2] = 0.f;
                closingFaces.transform(zTrafo);

                //render closing faces with identity transformations
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                closingFaces.render();
                LGL_ERROR;
            }
        }

        if (!jitterEntryPoints_.get())
            entryPort_.deactivateTarget();
        else
            tmpPort_.deactivateTarget();
    }

    // deactivate shader program
    shaderProgram_->deactivate();

    // restore OpenGL state
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    LGL_ERROR;

    if (entryPort_.isReady()) {
        // jittering of entry points
        if (jitterEntryPoints_.get())
            jitterEntryPoints();

        entryPort_.deactivateTarget();
    }

    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

void MeshEntryExitPoints::jitterEntryPoints() {
    shaderProgramJitter_->activate();
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(shaderProgramJitter_, &cam);

    // bind texture
    TextureUnit entryUnit, entryDepthUnit, exitUnit;

    // bind entry points texture and depth texture (have been rendered to temporary port)
    tmpPort_.bindColorTexture(entryUnit.getEnum());
    shaderProgramJitter_->setUniform("entryPoints_", entryUnit.getUnitNumber());

    tmpPort_.bindDepthTexture(entryDepthUnit.getEnum());
    shaderProgramJitter_->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    tmpPort_.setTextureParameters(shaderProgramJitter_, "entryParameters_");

    // bind exit points texture
    exitPort_.bindColorTexture(exitUnit.getEnum());
    shaderProgramJitter_->setUniform("exitPoints_", exitUnit.getUnitNumber());
    exitPort_.setTextureParameters(shaderProgramJitter_, "exitParameters_");

    shaderProgramJitter_->setUniform("stepLength_", jitterStepLength_.get());

    entryPort_.activateTarget("jitteredEntryParams");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render screen aligned quad
    renderQuad();

    shaderProgramJitter_->deactivate();
}

void MeshEntryExitPoints::onJitterEntryPointsChanged() {
    jitterStepLength_.setVisible(jitterEntryPoints_.get());
}

} // namespace voreen
