/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/render/entryexitpoints.h"
#include "voreen/core/vis/processors/portmapping.h"

#include "tgt/camera.h"
#include "tgt/glmath.h"

#ifndef VRN_SNAPSHOT
#include "voreen/core/vis/virtualclipping.h"
#endif
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/voreenpainter.h"

#include "voreen/core/volume/volumegl.h"

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
      scalingGroup_("entryexit.scaling", "Scaling of EEP"),
      scaleX_("set.scaleX", "X Scale", 1.f, 0.1f, 10.f, true),
      scaleY_("set.scaleY", "Y Scale", 1.f, 0.1f, 10.f, true),
      scaleZ_("set.scaleZ", "Z Scale", 1.f, 0.1f, 10.f, true),
      translationGroup_("entryexit.translation", "Translation of EEP"),
      translationX_("set.translationX", "X Translation", 0.f, -5.f, 5.f, true),
      translationY_("set.translationY", "Y Translation", 0.f, -5.f, 5.f, true),
      translationZ_("set.translationZ", "Z Translation", 0.f, -5.f, 5.f, true),
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

    addProperty(&supportCameraInsideVolume_);

    addProperty(&jitterEntryPoints_);
    condJitter_ = new ConditionProp("JitteringCond", &jitterEntryPoints_);
    jitterStepLength_.setConditioned("JitteringCond", 1);
    filterJitterTexture_.setConditioned("JitteringCond", 1);
    condJitter_->addCondProp(&jitterStepLength_);
    condJitter_->addCondProp(&filterJitterTexture_);

    addProperty(&scalingGroup_);
    scaleX_.setGrouped("entryexit.scaling");
    scaleY_.setGrouped("entryexit.scaling");
    scaleZ_.setGrouped("entryexit.scaling");
    addProperty(&scaleX_);
    addProperty(&scaleY_);
    addProperty(&scaleZ_);

    addProperty(&translationGroup_);
    translationX_.setGrouped("entryexit.translation");
    translationY_.setGrouped("entryexit.translation");
    translationZ_.setGrouped("entryexit.translation");
    addProperty(&translationX_);
    addProperty(&translationY_);
    addProperty(&translationZ_);
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

    if (MessageReceiver::getTag() != Message::all_)
        MsgDistr.remove(this);

    delete condJitter_;
}

void EntryExitPoints::setPropertyDestination(Identifier tag) {
    MessageReceiver::setTag(tag);
    jitterEntryPoints_.setMsgDestination(tag);
    jitterStepLength_.setMsgDestination(tag);
    scaleX_.setMsgDestination(tag);
    scaleY_.setMsgDestination(tag);
    scaleZ_.setMsgDestination(tag);
    scalingGroup_.setMsgDestination(tag);
    translationX_.setMsgDestination(tag);
    translationY_.setMsgDestination(tag);
    translationZ_.setMsgDestination(tag);
    translationGroup_.setMsgDestination(tag);
    jitterEntryPoints_.setMsgDestination(tag);
    MsgDistr.insert(this);
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

void EntryExitPoints::processMessage(Message* msg, const Identifier& dest) {
    VolumeRenderer::processMessage(msg, dest);

    if (msg->id_ == VoreenPainter::cameraChanged_ && msg->getValue<tgt::Camera*>() == camera_)
        invalidate();
    else if (msg->id_ == "set.jitterEntryPoints") {
		jitterEntryPoints_.set(msg->getValue<bool>());
		invalidate();
	}
    else if (msg->id_ == "set.jitterStepLength") {
		jitterStepLength_.set(msg->getValue<float>());
		invalidate();
	}
    else if (msg->id_ == "set.filterJitterTexture") {
		filterJitterTexture_.set(msg->getValue<bool>());
        generateJitterTexture();
		invalidate();
	}
    else if (msg->id_ == "set.scaleX") {
        scaleX_.set(msg->getValue<float>());
        tgt::vec3 oldScale = transformationMatrix_.getScalingPart();
        setScaling(tgt::vec3(scaleX_.get()/oldScale.x, 1.f, 1.f));
        invalidate();
    }
    else if (msg->id_ == "set.scaleY") {
        scaleY_.set(msg->getValue<float>());
        tgt::vec3 oldScale = transformationMatrix_.getScalingPart();
        setScaling(tgt::vec3(1.f, scaleY_.get()/oldScale.y, 1.f));
        invalidate();
    }
    else if (msg->id_ == "set.scaleZ") {
        scaleZ_.set(msg->getValue<float>());
        tgt::vec3 oldScale = transformationMatrix_.getScalingPart();
        setScaling(tgt::vec3(1.f, 1.f, scaleZ_.get()/oldScale.z));
        invalidate();
    }
    else if (msg->id_ == "set.translationX") {
        translationX_.set(msg->getValue<float>());
        float oldTranslation = transformationMatrix_[0][3];
        if (oldTranslation != translationX_.get()) {
            setTranslation(tgt::vec3((translationX_.get() - oldTranslation)
                                     / transformationMatrix_[0][0], 0.f, 0.f));
            invalidate();
        }
    }
    else if (msg->id_ == "set.translationY") {
        translationY_.set(msg->getValue<float>());
        float oldTranslation = transformationMatrix_[1][3];
        if (oldTranslation != translationY_.get()) {
            setTranslation(tgt::vec3(0.f, (translationY_.get() - oldTranslation)
                                     / transformationMatrix_[1][1], 0.f));
            invalidate();
        }
    }
    else if (msg->id_ == "set.translationZ") {
        translationZ_.set(msg->getValue<float>());
        float oldTranslation = transformationMatrix_[2][3];
        if (oldTranslation != translationZ_.get()) {
            setTranslation(tgt::vec3(0.f, 0.f, (translationZ_.get() - oldTranslation)
                                     / transformationMatrix_[3][3]));
            invalidate();
        }
    }
    else if (msg->id_ == "set.supportCameraInsideVolume") {
        supportCameraInsideVolume_.set(msg->getValue<bool>());
		invalidate();
    }
}

void EntryExitPoints::setVolumeHandle(VolumeHandle* const handle) {
    VolumeRenderer::setVolumeHandle(handle);

    if (currentVolumeHandle_ != 0 && currentVolumeHandle_->getVolume() != 0)
        setTransformationMatrix(currentVolumeHandle_->getVolume()->meta().getTransformation());
}

void EntryExitPoints::setTransformationMatrix(tgt::mat4 trans) {
    transformationMatrix_ = trans;
    translationX_.set(trans[0][3]);
    translationY_.set(trans[1][3]);
    translationZ_.set(trans[2][3]);
    scaleX_.set(trans[0][0]);
    scaleY_.set(trans[1][1]);
    scaleZ_.set(trans[2][2]);

    float det = trans.t00 * trans.t11* trans.t22
        + trans.t01* trans.t12* trans.t20
        + trans.t02* trans.t10* trans.t21
        - trans.t20* trans.t11* trans.t02
        - trans.t21* trans.t12* trans.t00
        - trans.t22* trans.t10* trans.t01;
    switchFrontAndBackFaces_ = (det < 0.f);
}

void EntryExitPoints::setScaling(tgt::vec3 scale) {
    transformationMatrix_ *= mat4::createScale(scale);
    if (currentVolumeHandle_ != 0) {
        Volume* volume = currentVolumeHandle_->getVolume();
        if (volume != 0)
            volume->meta().setTransformation(transformationMatrix_);
    }
}

void EntryExitPoints::setTranslation(tgt::vec3 trans) {
    transformationMatrix_ *= mat4::createTranslation(trans);
    if (currentVolumeHandle_ != 0) {
        Volume* volume = currentVolumeHandle_->getVolume();
        if (volume != 0)
            volume->meta().setTransformation(transformationMatrix_);
    }
}

void EntryExitPoints::setRotationX(float angle) {
    mat4 mult = mat4::identity;
    mult[0][0] = 1;
    mult[1][1] = cosf(angle);
    mult[1][2] = -sinf(angle);
    mult[2][1] = sinf(angle);
    mult[2][2] = cosf(angle);
    transformationMatrix_ = transformationMatrix_*mult;
    if (currentVolumeHandle_ != 0) {
        Volume* volume = currentVolumeHandle_->getVolume();
        if (volume != 0)
            volume->meta().setTransformation(transformationMatrix_);
    }
}

void EntryExitPoints::setRotationY(float angle) {
    mat4 mult = mat4::identity;
    mult[0][0] = cosf(angle);
    mult[0][2] = sinf(angle);
    mult[1][1] = 1.f;
    mult[2][0] = -sinf(angle);
    mult[2][2] = cosf(angle);
    transformationMatrix_ = transformationMatrix_ * mult;
    if (currentVolumeHandle_ != 0) {
        Volume* volume = currentVolumeHandle_->getVolume();
        if (volume != 0)
            volume->meta().setTransformation(transformationMatrix_);
    }
}

void EntryExitPoints::setRotationZ(float angle) {
    mat4 mult = mat4::identity;
    mult[0][0] = cosf(angle);
    mult[0][1] = -sinf(angle);
    mult[1][0] = sinf(angle);
    mult[1][1] = cosf(angle);
    mult[2][2] = 1;
    transformationMatrix_ = transformationMatrix_*mult;
    if (currentVolumeHandle_ != 0) {
        Volume* volume = currentVolumeHandle_->getVolume();
        if (volume != 0)
            volume->meta().setTransformation(transformationMatrix_);
    }
}

void EntryExitPoints::complementClippedEntryPoints(LocalPortMapping* portMapping) {
    // note: since this a helper function which is only called internally,
    //       we assume that the modelview and projection matrices have already been set correctly
    //       and that a current dataset is available

    tgtAssert(currentVolumeHandle_, "No Volume active");
    tgtAssert(shaderProgramClipping_, "Clipping shader not available");

    
    int entrySource = portMapping->getTarget("image.tmp");
    int exitSource = portMapping->getTarget("image.exitpoints");
    int entryDest = portMapping->getTarget("image.entrypoints");
    
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

    if (switchFrontAndBackFaces_) 
        glCullFace(GL_FRONT);
    else 
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
    int* texData = new int[screenDim_.x*screenDim_.y];
    for (int i=0; i<screenDim_.x*screenDim_.y; ++i)
         texData[i] = rand() % 256;

    // blur jitter texture
    GLubyte* texDataFiltered = new GLubyte[screenDim_.x*screenDim_.y];
    for (int x=0; x<screenDim_.x; ++x) {
        for (int y=0; y<screenDim_.y; ++y) {
            int value;
            if (!filterJitterTexture_.get() || x==0 || x==screenDim_.x-1 ||
                y==0 || y==screenDim_.y-1 )
            {
                value = texData[y*screenDim_.x+x];
            } else {
                value = texData[y*screenDim_.x+x]*4
                    + texData[(y+1)*screenDim_.x+x]*2 + texData[(y-1)*screenDim_.x+x]*2
                    + texData[y*screenDim_.x+(x+1)]*2 + texData[y*screenDim_.x+(x-1)]*2
                    + texData[(y+1)*screenDim_.x+(x+1)] + texData[(y-1)*screenDim_.x+(x+1)]
                    + texData[(y+1)*screenDim_.x+(x-1)] + texData[(y-1)*screenDim_.x+(x-1)];
                value /= 16;
            }
            texDataFiltered[y*screenDim_.x+x] = static_cast<GLubyte>(value);
        }
    }

    jitterTexture_->setPixelData(texDataFiltered);

    delete[] texData;
}

//---------------------------------------------------------------------------

CubeEntryExitPoints::CubeEntryExitPoints()
    : EntryExitPoints()
    , needToReRender_(true)
{
    createInport("volumehandle.volumehandle");
    createCoProcessorInport("coprocessor.proxygeometry");
    createPrivatePort("image.tmp");
    createOutport("image.entrypoints");
    createOutport("image.exitpoints");
}

CubeEntryExitPoints::~CubeEntryExitPoints() {
}

const std::string CubeEntryExitPoints::getProcessorInfo() const {
    return "Calculates cuboid entry/exit points for raycasting.";
}

std::string CubeEntryExitPoints::generateHeader() {
    std::string header = EntryExitPoints::generateHeader();

    header += "// START OF PROGRAM GENERATED DEFINES\n";
    header += "// END OF PROGRAM GENERATED DEFINES\n#line 0\n";

    return header;
}

void CubeEntryExitPoints::setNeedToReRender(bool needed) {
    needToReRender_ = needed;
}

void CubeEntryExitPoints::process(LocalPortMapping*  portMapping) {
    if (needToReRender_) {
        LGL_ERROR;
        VolumeHandle* volumeHandle = portMapping->getVolumeHandle("volumehandle.volumehandle");
        if (volumeHandle != 0) {
            if (!volumeHandle->isIdentical(currentVolumeHandle_))
                setVolumeHandle(volumeHandle);
        } else
            setVolumeHandle(0);
    
        if (currentVolumeHandle_ == 0)
            return;

        int exitSource = portMapping->getTarget("image.exitpoints");
        int entrySource;
        if (supportCameraInsideVolume_.get() || jitterEntryPoints_.get())
            // render into tmp texture and jitter afterwards
            entrySource = portMapping->getTarget("image.tmp"); 
        else
            // render directly into final texture
            entrySource = portMapping->getTarget("image.entrypoints");
        
        PortDataCoProcessor* pg = portMapping->getCoProcessorData("coprocessor.proxygeometry");

        // set volume parameters
        std::vector<VolumeStruct> volumes;
        volumes.push_back(VolumeStruct(
            currentVolumeHandle_->getVolumeGL(),
            "",                             // we do not need the volume itself...
            "",
            "volumeParameters_")            // ... but its parameters
        );
        
        tc_->setActiveTarget(exitSource, "exit");
        
        // sometimes set to GL_CW. We don't know where and why...
        // as workaround we overwrite it here. (jms)
        glFrontFace(GL_CCW);

        // set modelview and projection matrices
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        tgt::loadMatrix(camera_->getProjectionMatrix());
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();       
        tgt::loadMatrix(camera_->getViewMatrix() * transformationMatrix_);
        
        shaderProgram_->activate();
        setGlobalShaderParameters(shaderProgram_);
        bindVolumes(shaderProgram_, volumes);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        LGL_ERROR;
        
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        LGL_ERROR;

        // render back or front texture, depending on transformation matrix
        if (switchFrontAndBackFaces_)
            glCullFace(GL_BACK);
        else
            glCullFace(GL_FRONT);
		pg->call("render");

        shaderProgram_->deactivate();

        // render front texture
		// the second initialization is only necessary when using tc with rtt.
		// should we use conditional compilation for performance reasons here?
		// (jennis)
        //TODO: check if this is really necessary
        
        tc_->setActiveTarget(entrySource, "entry");
//        glViewport(0, 0, getSize().x, getSize().y);

        // set modelview and projection matrices
        glMatrixMode(GL_PROJECTION);
        tgt::loadMatrix(camera_->getProjectionMatrix());
        glMatrixMode(GL_MODELVIEW);
        tgt::loadMatrix(camera_->getViewMatrix() * transformationMatrix_);

        shaderProgram_->activate();
        setGlobalShaderParameters(shaderProgram_);
        bindVolumes(shaderProgram_, volumes);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        LGL_ERROR;
        
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        LGL_ERROR;

        if (switchFrontAndBackFaces_)
            glCullFace(GL_FRONT);
        else
            glCullFace(GL_BACK);

		pg->call("render");
        shaderProgram_->deactivate();

        // fill holes in entry points texture caused by near plane clipping
        if (supportCameraInsideVolume_.get())
            complementClippedEntryPoints(portMapping);
        
        // jittering of entry points
        if (jitterEntryPoints_.get())
            jitterEntryPoints(portMapping);

        // clean up
        if (switchFrontAndBackFaces_)
            glCullFace(GL_FRONT);
        else
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
}

//---------------------------------------------------------------------------

OutputProcessor::OutputProcessor() : Processor() {
	createInport("image.input");
    outputTarget_ = -1;
}

OutputProcessor::~OutputProcessor() {
}

void OutputProcessor::process(LocalPortMapping*  portMapping) {  
	outputTarget_ = portMapping->getTarget("image.input");
}

} // namespace voreen
