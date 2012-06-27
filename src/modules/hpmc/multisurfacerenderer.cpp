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

//TODO: avoid caching of normals
//TODO: support updates of isoValues_

#include "voreen/modules/hpmc/multisurfacerenderer.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

const std::string MultiSurfaceRenderer::loggerCat_("voreen.MultiSurfaceRenderer");

/*
    constructor
*/
MultiSurfaceRenderer::MultiSurfaceRenderer()
    : GeometryRenderer()
    , hpmcC_(0)
    , hpmcH_(0)
    , hpmcTraversalHandle_(0)
    , mcTriVBO_(0)
    , mcTriVBON_(0)
    , mcExtractPrg_(0)
    , mcRenderPrg_(0)
    , volumeInport_(Port::INPORT, "volumehandle.volumehandle", false, Processor::INVALID_PROGRAM)
    , privatePort_(Port::OUTPORT, "image.output", true, Processor::INVALID_PROGRAM)
{
    // configure iso values
    //isoValues_.push_back(0.75f);
    //isoValueHasChanged_.push_back(true);
    isoValues_.push_back(0.55f);
    isoValueHasChanged_.push_back(true);
    isoValues_.push_back(0.25f);
    isoValueHasChanged_.push_back(true);

    addPort(volumeInport_);
    addPrivateRenderPort(privatePort_);
}

MultiSurfaceRenderer::~MultiSurfaceRenderer() {
    delete[] mcTriVBO_;
    delete[] mcTriVBON_;
    //TODO: move to deinitialize
    if(mcExtractPrg_)
        ShdrMgr.dispose(mcExtractPrg_);
    if(mcRenderPrg_)
        ShdrMgr.dispose(mcRenderPrg_);
    if(hpmcH_)
        HPMCdestroyTraversalHandle(hpmcTraversalHandle_);
}

std::string MultiSurfaceRenderer::getProcessorInfo() const {
    return "";
}

void MultiSurfaceRenderer::initialize() throw (VoreenException) {
    GeometryRenderer::initialize();

    loadShader();

    if (!mcExtractPrg_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }

    // set up buffer for feedback of MC triangles
    mcTriVBO_ = new GLuint[isoValues_.size()];
    mcTriVBON_ = new GLsizei[isoValues_.size()];
    glGenBuffers(isoValues_.size(), mcTriVBO_);
    for (size_t i=0; i<isoValues_.size(); i++) {
        glBindBuffer(GL_ARRAY_BUFFER, mcTriVBO_[i]);
        mcTriVBON_[i] = 3*1000;
        glBufferData(GL_ARRAY_BUFFER, (3+3)*mcTriVBON_[i] * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glPolygonOffset(1.0f, 1.0f);

    initialized_ = true;
}

void MultiSurfaceRenderer::loadShader() {
    mcExtractPrg_ = ShdrMgr.loadSeparate("mc_extract.vert", "", generateHeader(), false, false);
    mcRenderPrg_ = ShdrMgr.loadSeparate("mc_render.vert", "", "", false, false);
}

void MultiSurfaceRenderer::compile(VolumeHandle* volumeHandle) {

    // set varyings to active in order to not
    // get them optimized away
    glActiveVaryingNV(mcExtractPrg_->getID(), "normal");
    glActiveVaryingNV(mcExtractPrg_->getID(), "position");

    mcExtractPrg_->setHeaders(generateHeader(volumeHandle), false);
    mcExtractPrg_->rebuild();

    // determine the location of the varyings, and let OpenGL know
    GLint varyingLocs[2];
    varyingLocs[0] = glGetVaryingLocationNV(mcExtractPrg_->getID(), "normal");
    varyingLocs[1] = glGetVaryingLocationNV(mcExtractPrg_->getID(), "position");
    glTransformFeedbackVaryingsNV(mcExtractPrg_->getID(), 2, &varyingLocs[0], GL_INTERLEAVED_ATTRIBS_NV);

    // associate program with traversal handle
    HPMCsetTraversalHandleProgram(hpmcTraversalHandle_, mcExtractPrg_->getID(), 0, 1, 2);
}

std::string MultiSurfaceRenderer::generateHeader(VolumeHandle* volumeHandle) {
    std::string headerSource = "";
    tgt::ivec3 dimensions = tgt::ivec3(128,128,128);
    GLint volID = 0;
    if (volumeHandle) {
        dimensions = volumeHandle->getVolume()->getDimensions();
        volID = volumeHandle->getVolumeGL()->getTexture()->getId();
    }
    // create histogram pyramid
    hpmcC_ = HPMCcreateConstants();
    if (!hpmcC_) {
        LERROR("Could not create HPMC constants");
        return headerSource;
    }
    hpmcH_ = HPMCcreateHistoPyramid(hpmcC_);
    if (!hpmcH_) {
        LERROR("Could not create HPMC histo pyramid");
        return headerSource;
    }
    HPMCsetLatticeSize(hpmcH_, dimensions.x, dimensions.y, dimensions.z);
    HPMCsetGridSize(hpmcH_, dimensions.x-1, dimensions.y-1, dimensions.z-1);
    float maxSize = static_cast<float>(std::max(dimensions.x, std::max(dimensions.y, dimensions.z)));
    HPMCsetGridExtent(hpmcH_, dimensions.x/maxSize, dimensions.y/maxSize, dimensions.z/maxSize);
    HPMCsetFieldTexture3D(hpmcH_, volID, GL_FALSE);
    // create traversal vertex shader
    hpmcTraversalHandle_ = HPMCcreateTraversalHandle(hpmcH_);
    if (hpmcTraversalHandle_) {
        char* shaderFunctions = HPMCgetTraversalShaderFunctions(hpmcTraversalHandle_);
        headerSource += shaderFunctions;
        free(shaderFunctions);
    }
    else {
        LERROR("Could not acquire HPMC traversal handle");
    }
    return headerSource;
}

void MultiSurfaceRenderer::process() {

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile(volumeInport_.getData());
    LGL_ERROR;

    privatePort_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    // bind volume texture
    TextureUnit volUnit;
    volUnit.activate();
    volumeInport_.getData()->getVolumeGL()->getTexture()->bind();

    for (size_t i=0; i<isoValues_.size(); i++) {
        HPMCbuildHistopyramid(hpmcH_, isoValues_[i]);
        // transform feedback: resize buffer if needed
        if (isoValueHasChanged_[i]) {
            isoValueHasChanged_[i] = false;
            GLsizei numVertices = HPMCacquireNumberOfVertices(hpmcH_);
            if (mcTriVBON_[i] < numVertices) {
                mcTriVBON_[i] = static_cast<GLsizei>(1.1f*static_cast<float>(numVertices));
                LINFO("Resizing mcTriVBO_[" << i << "] to hold " << mcTriVBON_[i] << " vertices.");
                glBindBuffer(GL_ARRAY_BUFFER, mcTriVBO_[i]);
                glBufferData(GL_ARRAY_BUFFER, (3+3) * mcTriVBON_[i] * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
        }
        glBindBufferBaseNV(GL_TRANSFORM_FEEDBACK_BUFFER_NV, 0, mcTriVBO_[i]);
        HPMCextractVerticesTransformFeedbackNV(hpmcTraversalHandle_);
    }

    mcExtractPrg_->deactivate();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

void MultiSurfaceRenderer::render() {
    // render from buffer
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    //glUseProgram( 0 );
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //mcRenderPrg_->activate();
    for (size_t i=0; i<isoValues_.size(); i++) {
        glColor3f(1.0f-isoValues_[i], 0.0f, isoValues_[i]);
        glBindBuffer(GL_ARRAY_BUFFER, mcTriVBO_[i]);
        glInterleavedArrays(GL_N3F_V3F, 0, NULL);
        glDrawArrays(GL_TRIANGLES, 0, mcTriVBON_[i]);
    }
    //glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //mcRenderPrg_->deactivate();
    glPopClientAttrib();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

} // namespace voreen
