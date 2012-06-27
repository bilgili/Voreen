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
      shaderProgramClipping_(0),
      supportCameraInsideVolume_("set.supportCameraInsideVolume",
                                 "Support camera inside the volume", true),
      jitterEntryPoints_("set.jitterEntryPoints", "Jitter entry params", false),
      filterJitterTexture_("set.filterJitterTexture", "Filter jitter texture", true),
      jitterStepLength_("set.jitterStepLength", "Jitter step length",
                        0.005f, 0.0005f, 0.025f, true),
      jitterTexture_(0),
      transformationMatrix_(mat4::identity),
      switchFrontAndBackFaces_(false)
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
    bool handleChanged = false;
    const bool res = VolumeHandleValidator::checkVolumeHandle(currentVolumeHandle_,
        portMapping->getVolumeHandle("volumehandle.volumehandle"), &handleChanged);

    if (res && handleChanged)
        setTransformationMatrix(currentVolumeHandle_->getVolume()->meta().getTransformation());
}

void EntryExitPoints::processMessage(Message* msg, const Identifier& dest) {
    VolumeRenderer::processMessage(msg, dest);

    if (msg->id_ == VoreenPainter::cameraChanged_ && msg->getValue<tgt::Camera*>() == camera_)
        invalidate();
}

void EntryExitPoints::setTransformationMatrix(tgt::mat4 trans) {
    transformationMatrix_ = trans;

    float det = trans.t00 * trans.t11 * trans.t22
        + trans.t01 * trans.t12* trans.t20
        + trans.t02 * trans.t10* trans.t21
        - trans.t20 * trans.t11* trans.t02
        - trans.t21 * trans.t12* trans.t00
        - trans.t22 * trans.t10* trans.t01;
    switchFrontAndBackFaces_ = (det < 0.f);
}

//void EntryExitPoints::setScaling(tgt::vec3 scale) {
//    transformationMatrix_ *= mat4::createScale(scale);
//    if (currentVolumeHandle_ != 0) {
//        Volume* volume = currentVolumeHandle_->getVolume();
//        if (volume != 0)
//            volume->meta().setTransformation(transformationMatrix_);
//    }
//}
//
//void EntryExitPoints::setTranslation(tgt::vec3 trans) {
//    transformationMatrix_ *= mat4::createTranslation(trans);
//    if (currentVolumeHandle_ != 0) {
//        Volume* volume = currentVolumeHandle_->getVolume();
//        if (volume != 0)
//            volume->meta().setTransformation(transformationMatrix_);
//    }
//}

//void EntryExitPoints::setRotationX(float angle) {
//    mat4 mult = mat4::identity;
//    mult[0][0] = 1;
//    mult[1][1] = cosf(angle);
//    mult[1][2] = -sinf(angle);
//    mult[2][1] = sinf(angle);
//    mult[2][2] = cosf(angle);
//    transformationMatrix_ = transformationMatrix_*mult;
//    if (currentVolumeHandle_ != 0) {
//        Volume* volume = currentVolumeHandle_->getVolume();
//        if (volume != 0)
//            volume->meta().setTransformation(transformationMatrix_);
//    }
//}
//
//void EntryExitPoints::setRotationY(float angle) {
//    mat4 mult = mat4::identity;
//    mult[0][0] = cosf(angle);
//    mult[0][2] = sinf(angle);
//    mult[1][1] = 1.f;
//    mult[2][0] = -sinf(angle);
//    mult[2][2] = cosf(angle);
//    transformationMatrix_ = transformationMatrix_ * mult;
//    if (currentVolumeHandle_ != 0) {
//        Volume* volume = currentVolumeHandle_->getVolume();
//        if (volume != 0)
//            volume->meta().setTransformation(transformationMatrix_);
//    }
//}
//
//void EntryExitPoints::setRotationZ(float angle) {
//    mat4 mult = mat4::identity;
//    mult[0][0] = cosf(angle);
//    mult[0][1] = -sinf(angle);
//    mult[1][0] = sinf(angle);
//    mult[1][1] = cosf(angle);
//    mult[2][2] = 1;
//    transformationMatrix_ = transformationMatrix_*mult;
//    if (currentVolumeHandle_ != 0) {
//        Volume* volume = currentVolumeHandle_->getVolume();
//        if (volume != 0)
//            volume->meta().setTransformation(transformationMatrix_);
//    }
//}

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

    if (switchFrontAndBackFaces_)
        glCullFace(GL_BACK);
    else
        glCullFace(GL_FRONT);

    tc_->setActiveTarget(entryDest, "entry filled");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PortDataCoProcessor* pg = portMapping->getCoProcessorData("coprocessor.proxygeometry");
    pg->call("render", 0);

    shaderProgramClipping_->deactivate();

    glCullFace(GL_BACK);
}

void EntryExitPoints::jitterEntryPoints(LocalPortMapping* portMapping) {
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

// protected methods replacing messaging
//
void EntryExitPoints::onFilterJitterTextureChange() {
    generateJitterTexture();
    invalidate();
}

//void EntryExitPoints::onSetScaleChange() {
//    tgt::vec3 oldScale(transformationMatrix_.getScalingPart());
//    tgt::vec3 newScale(scaleProp_.get());
//    newScale.x /= oldScale.x;
//    newScale.y /= oldScale.y;
//    newScale.z /= oldScale.z;
//    setScaling(newScale);
//    invalidate();
//}
//
//void EntryExitPoints::onSetTranslationChange() {
//    tgt::vec3 oldTranslation(transformationMatrix_[0][3], transformationMatrix_[1][3], transformationMatrix_[2][3]);
//    tgt::vec3 newTranslation(translationProp_.get());
//    newTranslation -= oldTranslation;
//    newTranslation.x /= transformationMatrix_[0][0];
//    newTranslation.y /= transformationMatrix_[1][1];
//    newTranslation.z /= transformationMatrix_[2][2];
//    if ( newTranslation != tgt::vec3::zero )
//        setTranslation(newTranslation);
//
//    invalidate();
//}

} // namespace voreen
