/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/entryexitpoints/entryexitpoints.h"
#include "tgt/glmath.h"

#ifndef VRN_SNAPSHOT
#include "voreen/core/vis/virtualclipping.h"
#endif
#include "voreen/core/vis/voreenpainter.h"

using tgt::vec3;
using tgt::mat4;

namespace voreen {

const Identifier EntryExitPoints::entryPointsTexUnit_      = "entryPointsTexUnit";
const Identifier EntryExitPoints::entryPointsDepthTexUnit_ = "entryPointsDepthTexUnit";
const Identifier EntryExitPoints::exitPointsTexUnit_       = "exitPointsTexUnit";
const Identifier EntryExitPoints::jitterTexUnit_           = "jitterTexUnit";

const std::string EntryExitPoints::loggerCat_("voreen.EntryExitPoints");

/*
    constructor
*/
EntryExitPoints::EntryExitPoints()
    : VolumeRenderer(),
      shaderProgram_(0),
      shaderProgramJitter_(0),
      shaderProgramClipping_(0),
      supportCameraInsideVolume_("set.supportCameraInsideVolume",
                                 "Support camera inside the volume", true),
      jitterEntryPoints_("set.jitterEntryPoints", "Jitter entry params", false),
      filterJitterTexture_("set.filterJitterTexture", "Filter jitter texture", true),
      jitterStepLength_("set.jitterStepLength", "Jitter step length",
                        0.005f, 0.0005f, 0.025f, true),
      jitterTexture_(0)
{
    setName("EntryExitPoints");

    std::vector<Identifier> units;
    units.push_back(entryPointsTexUnit_);
    units.push_back(entryPointsDepthTexUnit_);
    units.push_back(exitPointsTexUnit_);
    units.push_back(jitterTexUnit_);
    tm_.registerUnits(units);

    filterJitterTexture_.onChange(
        CallMemberAction<EntryExitPoints>(this, &EntryExitPoints::onFilterJitterTextureChange));

    addProperty(&supportCameraInsideVolume_);
    addProperty(&jitterEntryPoints_);

    createInport("volumehandle.volumehandle");
    createCoProcessorInport("coprocessor.proxygeometry");
    createPrivatePort("image.tmp");
    createOutport("image.entrypoints");
    createOutport("image.exitpoints");
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
}

int EntryExitPoints::initializeGL() {
    shaderProgram_ = ShdrMgr.load("eep_simple", generateHeader(), false);

    shaderProgramJitter_ = ShdrMgr.loadSeparate("pp_identity.vert", "eep_jitter.frag",
                                                generateHeader(), false);
    shaderProgramClipping_ = ShdrMgr.loadSeparate("eep_simple.vert", "eep_clipping.frag",
                                                  generateHeader() , false);

    if (shaderProgram_ && shaderProgramJitter_ && shaderProgramClipping_)
        return VRN_OK;
    else
        return VRN_ERROR;
}

void EntryExitPoints::process(LocalPortMapping* portMapping) {

    LGL_ERROR;

    if (!VolumeHandleValidator::checkVolumeHandle(currentVolumeHandle_,
            portMapping->getVolumeHandle("volumehandle.volumehandle")))
        return;

    int exitSource = portMapping->getTarget("image.exitpoints");
    int entrySource;
    
    // use temporary target if necessary
    if ( (supportCameraInsideVolume_.get() && jitterEntryPoints_.get()) ||
         (!supportCameraInsideVolume_.get() && !jitterEntryPoints_.get()) )     
        entrySource = portMapping->getTarget("image.entrypoints");
    else 
        entrySource = portMapping->getTarget("image.tmp");

    // retrieve proxy geometry via coprocessor port
    PortDataCoProcessor* pg = portMapping->getCoProcessorData("coprocessor.proxygeometry");

    // set modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    tgt::loadMatrix(camera_->getProjectionMatrix());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    tgt::loadMatrix(camera_->getViewMatrix());

    // enable culling
    glEnable(GL_CULL_FACE);

    // activate shader program
    shaderProgram_->activate();
    setGlobalShaderParameters(shaderProgram_);

    LGL_ERROR;

    //
    // render back texture
    //
    tc_->setActiveTarget(exitSource, "exit"); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    LGL_ERROR;
    pg->call("render");
    LGL_ERROR;

    //
    // render front texture
    //
    tc_->setActiveTarget(entrySource, "entry");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_BACK);
    LGL_ERROR;
    pg->call("render");
    LGL_ERROR;

    // deactivate shader program
    shaderProgram_->deactivate();

    // fill holes in entry points texture caused by near plane clipping
    if (supportCameraInsideVolume_.get())
        complementClippedEntryPoints(portMapping);

    // jittering of entry points
    if (jitterEntryPoints_.get())
        jitterEntryPoints(portMapping);

    // restore OpenGL context
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));

    // restore modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    LGL_ERROR;
}

void EntryExitPoints::processMessage(Message* msg, const Identifier& dest) {
    VolumeRenderer::processMessage(msg, dest);

//     if (msg->id_ == VoreenPainter::cameraChanged_ && msg->getValue<tgt::Camera*>() == camera_)
//         invalidate();
}

void EntryExitPoints::complementClippedEntryPoints(LocalPortMapping* portMapping) {

    // note: since this a helper function which is only called internally,
    //       we assume that the modelview and projection matrices have already been set correctly
    //       and that a current dataset is available

    tgtAssert(currentVolumeHandle_, "No Volume active");
    tgtAssert(shaderProgramClipping_, "Clipping shader not available");

    int entrySource;
    int entryDest;
    // write to temporary target if jittering is active
    if (jitterEntryPoints_.get()) {
        entrySource = portMapping->getTarget("image.entrypoints");
        entryDest = portMapping->getTarget("image.tmp");
    }
    else {
        entrySource = portMapping->getTarget("image.tmp");
        entryDest = portMapping->getTarget("image.entrypoints");
    }
    int exitSource = portMapping->getTarget("image.exitpoints");

    glActiveTexture(tm_.getGLTexUnit(exitPointsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(exitSource), tc_->getGLTexID(exitSource));
    glActiveTexture(tm_.getGLTexUnit(entryPointsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(entrySource), tc_->getGLTexID(entrySource));
    glActiveTexture(tm_.getGLTexUnit(entryPointsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(entrySource), tc_->getGLDepthTexID(entrySource));

    LGL_ERROR;

    shaderProgramClipping_->activate();
    setGlobalShaderParameters(shaderProgramClipping_);
    shaderProgramClipping_->setUniform("entryTex_", tm_.getTexUnit(entryPointsTexUnit_));
    shaderProgramClipping_->setUniform("exitTex_", tm_.getTexUnit(exitPointsTexUnit_));
    shaderProgramClipping_->setUniform("entryTexDepth_", tm_.getTexUnit(entryPointsDepthTexUnit_));
    LGL_ERROR;

    // set volume parameters
    std::vector<VolumeStruct> volumes;
    volumes.push_back(VolumeStruct(
        currentVolumeHandle_->getVolumeGL(),
        "",                             // we do not need the volume itself...
        "",
        "volumeParameters_")            // ... but its parameters
    );
    bindVolumes(shaderProgramClipping_, volumes);

    glCullFace(GL_FRONT);

    tc_->setActiveTarget(entryDest, "entry filled");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PortDataCoProcessor* pg = portMapping->getCoProcessorData("coprocessor.proxygeometry");
    pg->call("render");

    shaderProgramClipping_->deactivate();

    glCullFace(GL_BACK);
    LGL_ERROR;

}

void EntryExitPoints::jitterEntryPoints(LocalPortMapping* portMapping) {

    tgtAssert(shaderProgramJitter_, "Jittering shader not available");

    int entrySource = portMapping->getTarget("image.tmp");
    int exitSource = portMapping->getTarget("image.exitpoints");

    int entryDest = portMapping->getTarget("image.entrypoints");

    // if canvas resolution has changed, regenerate jitter texture
    if (!jitterTexture_ ||
        (jitterTexture_->getDimensions().x != tc_->getSize().x) ||
        (jitterTexture_->getDimensions().y != tc_->getSize().y))
    {
        generateJitterTexture();
    }

    shaderProgramJitter_->activate();
    setGlobalShaderParameters(shaderProgramJitter_);

    // bind jitter texture
    glActiveTexture(tm_.getGLTexUnit(jitterTexUnit_));
    jitterTexture_->bind();
    jitterTexture_->uploadTexture();
    shaderProgramJitter_->setUniform("jitterTexture_", tm_.getTexUnit(jitterTexUnit_));

    // bind entry points texture and depth texture
    glActiveTexture(tm_.getGLTexUnit(entryPointsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(entrySource), tc_->getGLTexID(entrySource));
    shaderProgramJitter_->setUniform("entryPoints_", tm_.getTexUnit(entryPointsTexUnit_));
    glActiveTexture(tm_.getGLTexUnit(entryPointsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(entrySource), tc_->getGLDepthTexID(entrySource));
    shaderProgramJitter_->setUniform("entryPointsDepth_", tm_.getTexUnit(entryPointsDepthTexUnit_));

    // bind exit points texture
    glActiveTexture(tm_.getGLTexUnit(exitPointsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(exitSource), tc_->getGLTexID(exitSource));
    shaderProgramJitter_->setUniform("exitPoints_", tm_.getTexUnit(exitPointsTexUnit_));

    shaderProgramJitter_->setUniform("stepLength_", jitterStepLength_.get());

    tc_->setActiveTarget(entryDest, "EntryExitPoints: jitteredEntryParams");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render screen aligned quad
    renderQuad();

    shaderProgramJitter_->deactivate();
}

void EntryExitPoints::generateJitterTexture() {
    tgt::ivec2 screenDim_ = tc_->getSize();

    if (jitterTexture_)
        delete jitterTexture_;

    jitterTexture_ = new tgt::Texture(
        tgt::ivec3(screenDim_.x, screenDim_.y, 1),  // dimensions
        GL_LUMINANCE,                               // format
        GL_LUMINANCE8,                              // internal format
        GL_UNSIGNED_BYTE,                           // data type
        tgt::Texture::NEAREST,                      // filter
        (tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_RECTANGLE)
                                                    // is texture rectangle?
    );

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

void EntryExitPoints::onFilterJitterTextureChange() {
    generateJitterTexture();
    invalidate();
}

} // namespace voreen
