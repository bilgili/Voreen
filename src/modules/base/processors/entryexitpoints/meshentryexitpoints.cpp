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

#include "voreen/modules/base/processors/entryexitpoints/meshentryexitpoints.h"

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

/*
    constructor
*/
MeshEntryExitPoints::MeshEntryExitPoints()
    : RenderProcessor(),
      shaderProgram_(0),
      shaderProgramJitter_(0),
      supportCameraInsideVolume_("supportCameraInsideVolume",
                                 "Support camera in volume", true),
      jitterEntryPoints_("jitterEntryPoints", "Jitter entry params", false),
      filterJitterTexture_("filterJitterTexture", "Filter jitter texture", true),
      useFloatRenderTargets_("useFloatRenderTargets", "Use float render targets", false),
      jitterStepLength_("jitterStepLength", "Jitter step length",
                        0.005f, 0.0005f, 0.025f),
      jitterTexture_(0),
      camera_("camera", "Camera", new tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f))),
      entryPort_(Port::OUTPORT, "image.entrypoints"),
      exitPort_(Port::OUTPORT, "image.exitpoints"),
      inport_(Port::INPORT, "proxgeometry.geometry"),
      tmpPort_(Port::OUTPORT, "image.tmp", false)
{

    addProperty(supportCameraInsideVolume_);

    //
    // jittering
    //
    addProperty(jitterEntryPoints_);
    jitterEntryPoints_.onChange(CallMemberAction<MeshEntryExitPoints>(this, &MeshEntryExitPoints::onJitterEntryPointsChanged));
    onJitterEntryPointsChanged(); // set initial state
    jitterStepLength_.setStepping(0.001f);
    jitterStepLength_.setNumDecimals(3);
    addProperty(jitterStepLength_);
    addProperty(filterJitterTexture_);
    filterJitterTexture_.onChange(CallMemberAction<MeshEntryExitPoints>(this, &MeshEntryExitPoints::onFilterJitterTextureChanged));
    addProperty(useFloatRenderTargets_);

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

std::string MeshEntryExitPoints::getProcessorInfo() const {
    return "This is the standard processor for generating entry- and exit-points within Voreen. "
           "The generated image color-codes the ray parameters for a subsequent VolumeRaycaster.<br/>"
           "See CubeMeshProxyGeometry.";
}

void MeshEntryExitPoints::initialize() throw (VoreenException) {
    RenderProcessor::initialize();

    shaderProgram_ = ShdrMgr.load("eep_simple", generateHeader(), false);

    shaderProgramJitter_ = ShdrMgr.loadSeparate("passthrough.vert", "eep_jitter.frag",
                                                generateHeader(), false);

    if (!shaderProgram_ || !shaderProgramJitter_) {
        throw VoreenException("Failed to load shaders");
    }
}

void MeshEntryExitPoints::deinitialize() throw (VoreenException) {
    ShdrMgr.dispose(shaderProgram_);
    shaderProgram_ = 0;

    ShdrMgr.dispose(shaderProgramJitter_);
    shaderProgramJitter_ = 0;

    TexMgr.dispose(jitterTexture_);
    jitterTexture_ = 0;

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
    MeshListGeometry* meshListGeometry = dynamic_cast<MeshListGeometry*>(inport_.getData());
    MeshGeometry* meshGeometry = dynamic_cast<MeshGeometry*>(inport_.getData());
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

    // set modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    tgt::loadMatrix(camera_.get()->getProjectionMatrix());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    tgt::loadMatrix(camera_.get()->getViewMatrix());

    // enable culling
    glEnable(GL_CULL_FACE);

    // activate shader program
    shaderProgram_->activate();
    setGlobalShaderParameters(shaderProgram_, camera_.get());
    LGL_ERROR;

    //
    // render back texture
    //
    if (exitPort_.isReady()) {
        exitPort_.activateTarget();
        glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        glDepthFunc(GL_GREATER);

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
            float nearPlaneDistToOrigin = tgt::dot(camera_.get()->getPosition(), -camera_.get()->getLook()) - camera_.get()->getNearDist() - 0.0001f;
            MeshListGeometry closingFaces = geometry_.clip(tgt::vec4(-camera_.get()->getLook(), nearPlaneDistToOrigin));

            // render closing face separately, if not empty
            if (!closingFaces.empty()) {
                // project closing faces onto near-plane
                tgt::mat4 trafoMatrix = camera_.get()->getProjectionMatrix() * camera_.get()->getViewMatrix();
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
    // if canvas resolution has changed, regenerate jitter texture
    if (!jitterTexture_ ||
        (jitterTexture_->getDimensions().x != entryPort_.getSize().x) ||
        (jitterTexture_->getDimensions().y != entryPort_.getSize().y))
    {
        generateJitterTexture();
    }

    shaderProgramJitter_->activate();
    setGlobalShaderParameters(shaderProgramJitter_, camera_.get());

    // bind jitter texture
    TextureUnit jitterUnit, entryUnit, entryDepthUnit, exitUnit;
    jitterUnit.activate();
    jitterTexture_->bind();
    jitterTexture_->uploadTexture();
    shaderProgramJitter_->setUniform("jitterTexture_", jitterUnit.getUnitNumber());
    shaderProgramJitter_->setIgnoreUniformLocationError(true);
    shaderProgramJitter_->setUniform("jitterParameters_.dimensions_",
                                     tgt::vec2(jitterTexture_->getDimensions().xy()));
    shaderProgramJitter_->setUniform("jitterParameters_.dimensionsRCP_",
                                     tgt::vec2(1.0f) / tgt::vec2(jitterTexture_->getDimensions().xy()));
    shaderProgramJitter_->setUniform("jitterParameters_.matrix_", tgt::mat4::identity);
    shaderProgramJitter_->setIgnoreUniformLocationError(false);

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

void MeshEntryExitPoints::generateJitterTexture() {
    tgt::ivec2 screenDim_ = entryPort_.getSize();

    delete jitterTexture_;

    jitterTexture_ = new tgt::Texture(
            tgt::ivec3(screenDim_.x, screenDim_.y, 1),  // dimensions
            GL_LUMINANCE,                               // format
            GL_LUMINANCE8,                              // internal format
            GL_UNSIGNED_BYTE,                           // data type
            tgt::Texture::NEAREST,                      // filter
            (!GpuCaps.isNpotSupported())                // is texture rectangle?
    );

#ifdef VRN_NO_RANDOM
    srand(0);
#endif

    // create jitter values
    int* texData = new int[screenDim_.x * screenDim_.y];
    for (int i = 0; i < screenDim_.x * screenDim_.y; ++i)
         texData[i] = rand() % 256;

    // blur jitter texture
    GLubyte* texDataFiltered = new GLubyte[screenDim_.x * screenDim_.y];
    for (int x = 0; x < screenDim_.x; ++x) {
        for (int y = 0; y < screenDim_.y; ++y) {
            int value;
            if (!filterJitterTexture_.get() || x == 0 || x == screenDim_.x - 1 ||
                y == 0 || y == screenDim_.y - 1)
            {
                value = texData[y * screenDim_.x + x];
            } else {
                value = texData[y * screenDim_.x + x] * 4
                    + texData[(y + 1) * screenDim_.x +  x     ] * 2
                    + texData[(y - 1) * screenDim_.x +  x     ] * 2
                    + texData[ y      * screenDim_.x + (x + 1)] * 2
                    + texData[ y      * screenDim_.x + (x - 1)] * 2
                    + texData[(y + 1) * screenDim_.x + (x + 1)]
                    + texData[(y - 1) * screenDim_.x + (x + 1)]
                    + texData[(y + 1) * screenDim_.x + (x - 1)]
                    + texData[(y - 1) * screenDim_.x + (x - 1)];
                value /= 16;
            }
            texDataFiltered[y * screenDim_.x + x] = static_cast<GLubyte>(value);
        }
    }

    jitterTexture_->setPixelData(texDataFiltered);

    delete[] texData;
}

void MeshEntryExitPoints::onJitterEntryPointsChanged() {
    jitterStepLength_.setVisible(jitterEntryPoints_.get());
    filterJitterTexture_.setVisible(jitterEntryPoints_.get());
}

void MeshEntryExitPoints::onFilterJitterTextureChanged() {
    generateJitterTexture();
    invalidate();
}

} // namespace voreen
