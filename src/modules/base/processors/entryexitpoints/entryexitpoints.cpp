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

#include "voreen/modules/base/processors/entryexitpoints/entryexitpoints.h"

#include "voreen/core/interaction/camerainteractionhandler.h"

#include "tgt/glmath.h"
#include "tgt/gpucapabilities.h"
#include "tgt/textureunit.h"

using tgt::vec3;
using tgt::mat4;
using tgt::TextureUnit;

namespace voreen {

const std::string EntryExitPoints::loggerCat_("voreen.EntryExitPoints");

/*
    constructor
*/
EntryExitPoints::EntryExitPoints()
    : VolumeRenderer(),
      shaderProgram_(0),
      shaderProgramJitter_(0),
      shaderProgramClipping_(0),
      supportCameraInsideVolume_("supportCameraInsideVolume",
                                 "Support camera in volume", true),
      jitterEntryPoints_("jitterEntryPoints", "Jitter entry params", false),
      filterJitterTexture_("filterJitterTexture", "Filter jitter texture", true),
      jitterStepLength_("jitterStepLength", "Jitter step length",
                        0.005f, 0.0005f, 0.025f),
      jitterTexture_(0),
      camera_("camera", "Camera", new tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f))),
      entryPort_(Port::OUTPORT, "image.entrypoints"),
      exitPort_(Port::OUTPORT, "image.exitpoints"),
      inport_(Port::INPORT, "volumehandle.volumehandle"),
      cpPort_(Port::INPORT, "coprocessor.proxygeometry"),
      tmpPort_(Port::OUTPORT, "image.tmp", false)
{
    addProperty(supportCameraInsideVolume_);

    //
    // jittering
    //
    addProperty(jitterEntryPoints_);
    jitterEntryPoints_.onChange(CallMemberAction<EntryExitPoints>(this, &EntryExitPoints::onJitterEntryPointsChanged));
    onJitterEntryPointsChanged(); // set initial state
    jitterStepLength_.setStepping(0.001f);
    jitterStepLength_.setNumDecimals(3);
    addProperty(jitterStepLength_);
    addProperty(filterJitterTexture_);
    filterJitterTexture_.onChange(CallMemberAction<EntryExitPoints>(this, &EntryExitPoints::onFilterJitterTextureChanged));

    addProperty(camera_);

    cameraHandler_ = new CameraInteractionHandler("cameraHandler", "Camera", &camera_);
    addInteractionHandler(cameraHandler_);

    addPort(entryPort_);
    addPort(exitPort_);
    addPort(inport_);
    addPort(cpPort_);
    addPrivateRenderPort(&tmpPort_);
}

std::string EntryExitPoints::getProcessorInfo() const {
    return "Generates entry- and exit-points for a subsequent VolumeRaycaster."
           "<p><span style=\"color: red; font-weight: bold;\">Deprecated:</span> "
           "Use CubeMeshProxyGeometry and MeshEntryExitPoints instead.</p>";
}

EntryExitPoints::~EntryExitPoints() {
    if (shaderProgram_)
        ShdrMgr.dispose(shaderProgram_);
    if (shaderProgramJitter_)
        ShdrMgr.dispose(shaderProgramJitter_);
    if (shaderProgramClipping_)
        ShdrMgr.dispose(shaderProgramClipping_);
    if (jitterTexture_)
        TexMgr.dispose(jitterTexture_);

    delete cameraHandler_;
}

void EntryExitPoints::initialize() throw (VoreenException) {
    VolumeRenderer::initialize();

    shaderProgram_ = ShdrMgr.load("eep_simple", generateHeader());

    shaderProgramJitter_ = ShdrMgr.loadSeparate("passthrough.vert", "eep_jitter.frag",
                                                generateHeader());
    shaderProgramClipping_ = ShdrMgr.loadSeparate("eep_simple.vert", "eep_clipping.frag",
                                                  generateHeader());

    if (!shaderProgram_ || !shaderProgramJitter_ || !shaderProgramClipping_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }

    initialized_ = true;
}

bool EntryExitPoints::isReady() const {
    // We want to render if at least one of the outports is connected
    return ((entryPort_.isReady() || exitPort_.isReady()) && inport_.isReady() && cpPort_.isReady());
}

void EntryExitPoints::process() {
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);

        cpPort_.getConnectedProcessor()->render();
        LGL_ERROR;
        exitPort_.deactivateTarget();
    }

    //
    // render front texture
    //
    // use temporary target if necessary
    if (entryPort_.isReady()) {
        if ((supportCameraInsideVolume_.get() && jitterEntryPoints_.get()) ||
            ((!supportCameraInsideVolume_.get() || !exitPort_.isReady()) && !jitterEntryPoints_.get()))
            entryPort_.activateTarget();
        else
            tmpPort_.activateTarget();
        LGL_ERROR;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);

        cpPort_.getConnectedProcessor()->render();
        LGL_ERROR;

        if ((supportCameraInsideVolume_.get() && jitterEntryPoints_.get()) ||
            ((!supportCameraInsideVolume_.get() || !exitPort_.isReady()) && !jitterEntryPoints_.get()))
            entryPort_.deactivateTarget();
        else
            tmpPort_.deactivateTarget();
    }

    // deactivate shader program
    shaderProgram_->deactivate();

    // fill holes in entry points texture caused by near plane clipping
    if (supportCameraInsideVolume_.get() && entryPort_.isReady() && exitPort_.isReady())
        complementClippedEntryPoints();

    // restore OpenGL state
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    if (entryPort_.isReady()) {
        // jittering of entry points
        if (jitterEntryPoints_.get())
            jitterEntryPoints();

        entryPort_.deactivateTarget();
    }

    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

void EntryExitPoints::complementClippedEntryPoints() {
    // note: since this a helper function which is only called internally,
    //       we assume that the modelview and projection matrices have already been set correctly
    //       and that a current dataset is available

    tgtAssert(inport_.getData(), "No Volume active");
    tgtAssert(shaderProgramClipping_, "Clipping shader not available");

    RenderPort& activePort = jitterEntryPoints_.get() ? entryPort_ : tmpPort_;
    TextureUnit entryUnit, entryDepthUnit, exitUnit;
    activePort.bindTextures(entryUnit.getEnum(), entryDepthUnit.getEnum());
    exitPort_.bindColorTexture(exitUnit.getEnum());

    LGL_ERROR;

    shaderProgramClipping_->activate();
    setGlobalShaderParameters(shaderProgramClipping_, camera_.get());
    shaderProgramClipping_->setUniform("entryTex_", entryUnit.getUnitNumber());
    activePort.setTextureParameters(shaderProgramClipping_, "entryParameters_");
    shaderProgramClipping_->setUniform("exitTex_", exitUnit.getUnitNumber());
    exitPort_.setTextureParameters(shaderProgramClipping_, "exitParameters_");
    shaderProgramClipping_->setUniform("entryTexDepth_", entryDepthUnit.getUnitNumber());
    shaderProgramClipping_->setUniform("volumeCubeSizeRCP_", 1.f / inport_.getData()->getVolume()->getCubeSize());
    LGL_ERROR;

    glCullFace(GL_FRONT);

    // write to temporary target if jittering is active
    if (jitterEntryPoints_.get())
        tmpPort_.activateTarget("entry filled");
    else
        entryPort_.activateTarget("entry filled");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cpPort_.getConnectedProcessor()->render();

    shaderProgramClipping_->deactivate();

    glCullFace(GL_BACK);
    LGL_ERROR;
}

void EntryExitPoints::jitterEntryPoints() {
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
    TextureUnit jitterUnit;
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
    TextureUnit entryParams, exitParams, entryParamsDepth;
    tmpPort_.bindColorTexture(entryParams.getEnum());
    shaderProgramJitter_->setUniform("entryPoints_", entryParams.getUnitNumber());

    tmpPort_.bindDepthTexture(entryParamsDepth.getEnum());
    shaderProgramJitter_->setUniform("entryPointsDepth_", entryParamsDepth.getUnitNumber());
    tmpPort_.setTextureParameters(shaderProgramJitter_, "entryParameters_");

    // bind exit points texture
    exitPort_.bindColorTexture(exitParams.getEnum());
    shaderProgramJitter_->setUniform("exitPoints_", exitParams.getUnitNumber());
    exitPort_.setTextureParameters(shaderProgramJitter_, "exitParameters_");

    shaderProgramJitter_->setUniform("stepLength_", jitterStepLength_.get());

    entryPort_.activateTarget("jitteredEntryParams");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render screen aligned quad
    renderQuad();

    shaderProgramJitter_->deactivate();
}

void EntryExitPoints::generateJitterTexture() {
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

void EntryExitPoints::onJitterEntryPointsChanged() {
    jitterStepLength_.setVisible(jitterEntryPoints_.get());
    filterJitterTexture_.setVisible(jitterEntryPoints_.get());
}

void EntryExitPoints::onFilterJitterTextureChanged() {
    generateJitterTexture();
    invalidate();
}

} // namespace voreen
