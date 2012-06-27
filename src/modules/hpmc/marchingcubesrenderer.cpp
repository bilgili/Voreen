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

#include "voreen/modules/hpmc/marchingcubesrenderer.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

const std::string MarchingCubesRenderer::loggerCat_("voreen.MarchingCubesRenderer");
const std::string MarchingCubesRenderer::volTexUnit_ = "volTexUnit";

/*
    constructor
*/
MarchingCubesRenderer::MarchingCubesRenderer()
    : GeometryRendererBase()
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle", false, Processor::INVALID_PROGRAM)
    , privatePort_(Port::OUTPORT, "image.output", true, Processor::INVALID_PROGRAM)
    , hpmcC_(0)
    , hpmcH_(0)
    , hpmcTraversalHandle_(0)
    , gridSize_(-1)
    , isoValue_("isoValue", "ISO value", 0.5f, 0.0f, 1.0f)
    , gridSizeOption_("gridSize", "HPMC grid size", Processor::INVALID_PROGRAM)
    , surfaceColor_("color", "ISO surface color", tgt::vec4(1.0f, 0.0f, 0.0f, 1.f))
    , surfaceAlpha_("surfaceAlpha", "ISO surface alpha", 0.75f, 0.0f, 1.0f)
    , useLighting_("useLighting", "Apply surface lighting", true, Processor::INVALID_PROGRAM)
    , lightPosRelativeToViewer_("cameraRelativeToViewer", "Camera relative to viewer", false)
    , lightPosition_("lightPosition", "Light source position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f),
            tgt::vec4(-10), tgt::vec4(10))
    , lightAmbient_("lightAmbient", "Ambient light", tgt::Color(0.4f, 0.4f, 0.4f, 1.f))
    , lightDiffuse_("lightDiffuse", "Diffuse light", tgt::Color(0.6f, 0.6f, 0.6f, 1.f))
    , lightSpecular_("lightSpecular", "Specular light", tgt::Color(0.4f, 0.4f, 0.4f, 1.f))
    , materialShininess_("materialShininess", "Shininess", 60.f, 0.1f, 128.f)
{

    gridSizeOption_.addOption("64", "64", 64);
    gridSizeOption_.addOption("96", "96", 96);
    gridSizeOption_.addOption("128", "128", 128);
    gridSizeOption_.addOption("192", "192", 192);
    gridSizeOption_.addOption("256", "256", 256);
    gridSizeOption_.addOption("384", "384", 384);
    gridSizeOption_.addOption("512", "512", 512);
    gridSizeOption_.addOption("volume-size", "volume size", 0);
    gridSizeOption_.selectByKey("192");

    addProperty(isoValue_);
    addProperty(gridSizeOption_),

    addProperty(surfaceColor_);
    addProperty(surfaceAlpha_);
    addProperty(useLighting_);
    addProperty(lightPosition_);
    addProperty(lightPosRelativeToViewer_);
    addProperty(lightAmbient_);
    addProperty(lightDiffuse_);
    addProperty(lightSpecular_);
    addProperty(materialShininess_);

    addPort(volumeInport_);
    addPrivateRenderPort(privatePort_);
}

MarchingCubesRenderer::~MarchingCubesRenderer() {
}

std::string MarchingCubesRenderer::getProcessorInfo() const {
    return "";
}

void MarchingCubesRenderer::initialize() throw (VoreenException) {
    GeometryRendererBase::initialize();

    loadShader();

    if (!mcExtractPrg_ || !mcExtractPrg_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }

    // set up buffer for feedback of MC triangles
    glGenBuffers(1, &mcTriVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, mcTriVBO_);
    mcTriVBON_ = 3*1000;
    glBufferData(GL_ARRAY_BUFFER, (3+3)*mcTriVBON_ * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glPolygonOffset(1.0f, 1.0f);

    initialized_ = true;
}

void MarchingCubesRenderer::deinitialize() throw (VoreenException) {
    releaseHPMCResources();
    ShdrMgr.dispose(mcExtractPrg_);
    ShdrMgr.dispose(mcRenderPrg_);
    mcExtractPrg_ = 0;
    mcRenderPrg_ = 0;

    GeometryRendererBase::deinitialize();
}

bool MarchingCubesRenderer::isReady() const {
    return (GeometryRendererBase::isReady() && mcExtractPrg_ && mcRenderPrg_ && hpmcTraversalHandle_);
}

void MarchingCubesRenderer::loadShader() {
    mcExtractPrg_ = ShdrMgr.loadSeparate("mc_extract.vert", "", generateExtractHeader(), false, false);
    mcRenderPrg_ = ShdrMgr.loadSeparate("mc_render.vert", "mc_render.frag", generateRenderHeader(), false, false);
}

void MarchingCubesRenderer::compile(VolumeHandle* volumeHandle) {

    tgtAssert(mcExtractPrg_ && mcRenderPrg_, "No shaders");
    tgtAssert(hpmcTraversalHandle_, "No hpmc traversal handle");

    // set varyings to active in order to not
    // get them optimized away
    glActiveVaryingNV(mcExtractPrg_->getID(), "normal");
    glActiveVaryingNV(mcExtractPrg_->getID(), "position");

    mcExtractPrg_->setHeaders(generateExtractHeader(volumeHandle), false);
    if (!hpmcTraversalHandle_)
        return;
    mcExtractPrg_->rebuild();

    mcRenderPrg_->setHeaders(generateRenderHeader(), false);
    mcRenderPrg_->rebuild();

    // determine the location of the varyings, and let OpenGL know
    GLint varyingLocs[2];
    varyingLocs[0] = glGetVaryingLocationNV(mcExtractPrg_->getID(), "normal");
    varyingLocs[1] = glGetVaryingLocationNV(mcExtractPrg_->getID(), "position");
    glTransformFeedbackVaryingsNV(mcExtractPrg_->getID(), 2, &varyingLocs[0], GL_INTERLEAVED_ATTRIBS_NV);

    // associate program with traversal handle
    tgtAssert(hpmcTraversalHandle_, "No hpmc traversal handle");
    HPMCsetTraversalHandleProgram(hpmcTraversalHandle_, mcExtractPrg_->getID(), 0, 1, 2);
}

std::string MarchingCubesRenderer::generateExtractHeader(VolumeHandle* volumeHandle) {
    std::string headerSource = GeometryRendererBase::generateHeader();

    tgt::ivec3 volDim = tgt::ivec3(128);
    GLint volID = 0;
    if (volumeHandle) {
        volDim = volumeHandle->getVolume()->getDimensions();
        volID = volumeHandle->getVolumeGL()->getTexture()->getId();
    }

    tgt::ivec3 newGridSize;
    if (gridSizeOption_.isSelected("volume-size"))
        newGridSize = volDim;
    else
        newGridSize = tgt::ivec3(gridSizeOption_.getValue());

    // recreate hpmc resources, if grid size has changed
    if (newGridSize != gridSize_ || !hpmcH_) {
        releaseHPMCResources();

        // create histogram pyramid
        hpmcC_ = HPMCcreateConstants();
        if (!hpmcC_) {
            LERROR("Could not create HPMC constants");
            gridSize_ = tgt::ivec3(-1);
            return headerSource;
        }
        hpmcH_ = HPMCcreateHistoPyramid(hpmcC_);
        if (!hpmcH_) {
            LERROR("Could not create HPMC histo pyramid");
            gridSize_ = tgt::ivec3(-1);
            return headerSource;
        }

        // create hpmc lattice
        tgtAssert(tgt::hand(tgt::greaterThan(newGridSize, tgt::ivec3(0))), "Invalid grid size");
        HPMCsetLatticeSize(hpmcH_, newGridSize.x, newGridSize.y, newGridSize.z);
        HPMCsetGridSize(hpmcH_, newGridSize.x-1, newGridSize.y-1, newGridSize.z-1);
        HPMCsetGridExtent(hpmcH_, 1.f, 1.f, 1.f);
        gridSize_ = newGridSize;
    }

    tgtAssert(hpmcH_, "No hpmc pyramid");
    HPMCsetFieldTexture3D(hpmcH_, volID, GL_FALSE);

    // create traversal vertex shader
    if (hpmcTraversalHandle_) {
        HPMCdestroyTraversalHandle(hpmcTraversalHandle_);
        hpmcTraversalHandle_ = 0;
    }
    hpmcTraversalHandle_ = HPMCcreateTraversalHandle(hpmcH_);
    if (!hpmcTraversalHandle_) {
        LERROR("Could not acquire HPMC traversal handle");
        gridSize_ = tgt::ivec3(-1);
        return headerSource;
    }

    char* shaderFunctions = HPMCgetTraversalShaderFunctions(hpmcTraversalHandle_);
    headerSource += shaderFunctions;
    free(shaderFunctions);

    return headerSource;
}

std::string MarchingCubesRenderer::generateRenderHeader() {
    std::string headerSource = GeometryRendererBase::generateHeader();
    if (useLighting_.get())
        headerSource += "#define APPLY_SURFACE_LIGHTING\n";
    return headerSource;
}

void MarchingCubesRenderer::setLightingParameters() {
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient_.get().elem);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse_.get().elem);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular_.get().elem);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, surfaceColor_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, surfaceColor_.get().elem);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, surfaceColor_.get().elem);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess_.get());
    LGL_ERROR;
}

void MarchingCubesRenderer::process() {

    tgtAssert(mcExtractPrg_ && hpmcTraversalHandle_, "No shader or hpmc handle");
    tgtAssert(volumeInport_.getData()->getVolume(), "No volume");

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile(volumeInport_.getData());
    LGL_ERROR;

    privatePort_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    mcExtractPrg_->activate();
    mcExtractPrg_->setUniform("volumeCubeSize_", volumeInport_.getData()->getVolume()->getCubeSize());

    // bind volume texture
    TextureUnit volUnit;
    volUnit.activate();
    volumeInport_.getData()->getVolumeGL()->getTexture()->bind();

    HPMCbuildHistopyramid(hpmcH_, isoValue_.get());
    // transform feedback: resize buffer if needed
    numVertices_ = HPMCacquireNumberOfVertices(hpmcH_);
    if (mcTriVBON_ < numVertices_) {
        mcTriVBON_ = static_cast<GLsizei>(1.1f*static_cast<float>(numVertices_));
        LINFO("Resizing mcTriVBO_ to hold " << mcTriVBON_ << " vertices.");
        glBindBuffer(GL_ARRAY_BUFFER, mcTriVBO_);
        glBufferData(GL_ARRAY_BUFFER, (3+3) * mcTriVBON_ * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }
    glEnable(GL_RASTERIZER_DISCARD_NV);
    glBindBufferBaseNV(GL_TRANSFORM_FEEDBACK_BUFFER_NV, 0, mcTriVBO_);
    HPMCextractVerticesTransformFeedbackNV(hpmcTraversalHandle_);
    glDisable(GL_RASTERIZER_DISCARD_NV);

    mcExtractPrg_->deactivate();

    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

void MarchingCubesRenderer::render() {
    tgtAssert(mcRenderPrg_, "No shader");

    // setup shader
    mcRenderPrg_->activate();
    setGlobalShaderParameters(mcRenderPrg_, camera_);

    mcRenderPrg_->setUniform("surfaceColor_", tgt::vec4(surfaceColor_.get().xyz(), surfaceAlpha_.get()));
    if (useLighting_.get()) {
        setLightingParameters();
        if (lightPosRelativeToViewer_.get()) {
            // move light source with viewer
            tgtAssert(camera_, "No camera");
            mcRenderPrg_->setUniform("lightPosition_", camera_->getViewMatrixInverse()*lightPosition_.get().xyz());
        }
        else {
            // set light position in world coordinates
            mcRenderPrg_->setUniform("lightPosition_", lightPosition_.get().xyz());
        }
    }

    // render from buffermulti
    glBindBuffer(GL_ARRAY_BUFFER, mcTriVBO_);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glInterleavedArrays(GL_N3F_V3F, 0, NULL);
    glDrawArrays(GL_TRIANGLES, 0, numVertices_);
    glPopClientAttrib();
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mcRenderPrg_->deactivate();
    LGL_ERROR;
}

void MarchingCubesRenderer::releaseHPMCResources() {
    if (hpmcC_) {
        HPMCdestroyConstants(hpmcC_);
        hpmcC_ = 0;
    }
    if (hpmcH_) {
        //HPMCdestroyHandle(hpmcH_);
        hpmcH_ = 0;
    }
    if (hpmcTraversalHandle_) {
        HPMCdestroyTraversalHandle(hpmcTraversalHandle_);
        hpmcTraversalHandle_ = 0;
    }
}

Processor* MarchingCubesRenderer::create() const {
    return new MarchingCubesRenderer();
}



} // namespace voreen
