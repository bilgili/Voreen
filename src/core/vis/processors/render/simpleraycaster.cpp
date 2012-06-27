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

#include "voreen/core/vis/processors/render/simpleraycaster.h"

#include "voreen/core/vis/processors/portmapping.h"

namespace voreen {

SimpleRaycaster::SimpleRaycaster() :
    VolumeRaycaster() , transferFunc_(setTransFunc_, "not used", 0, true)
{
	setName("SimpleRaycaster");
    addProperty(raycastingQualitiesEnumProp_);

    // initialize transfer function
    TransFuncIntensityKeys* tf = new TransFuncIntensityKeys();
    tf->createStdFunc();
    transferFunc_.set(tf);
    addProperty(&transferFunc_);

    firstPass_ = true;

    createInport("volumehandle.volumehandle");
	createInport("image.entrypoints");
	createInport("image.exitpoints");
	createOutport("image.output");
}


SimpleRaycaster::~SimpleRaycaster() {
    if (MessageReceiver::getTag() != Message::all_)
        MsgDistr.remove(this);
}

const std::string SimpleRaycaster::getProcessorInfo() const {
	return "Performs a simple single pass raycasting with only some capabilites.";
}

void SimpleRaycaster::processMessage(Message* msg, const Identifier& dest) {
    VolumeRaycaster::processMessage(msg, dest);
    if (msg->id_ == setTransFunc_) {
        TransFunc* tf = msg->getValue<TransFunc*>();
        if (tf != transferFunc_.get()) {
            // shader has to be recompiled, if the transferfunc header has changed
            std::string definesOld = transferFunc_.get() ? transferFunc_.get()->getShaderDefines() : "";
            std::string definesNew = tf ? tf->getShaderDefines() : "";
            if ( definesOld != definesNew )
                invalidateShader();
            transferFunc_.set(tf);
        }
        invalidate();
	}
}

void SimpleRaycaster::setPropertyDestination(Identifier tag) {
    VolumeRaycaster::setPropertyDestination(tag);
    transferFunc_.setMsgDestination(tag);
    MsgDistr.insert(this);
}

int SimpleRaycaster::initializeGL() {
    loadShader();
    initStatus_ = raycastPrg_ ? VRN_OK : VRN_ERROR;;

    return initStatus_;
}

void SimpleRaycaster::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", this->fragmentShaderFilename_.c_str(), generateHeader(), false);
}

void SimpleRaycaster::compile() {
    raycastPrg_->setHeaders(generateHeader(), false);
	raycastPrg_->rebuild();
}

void SimpleRaycaster::process(LocalPortMapping* portMapping) 
{	
	int entryParams = portMapping->getTarget("image.entrypoints");
	int exitParams = portMapping->getTarget("image.exitpoints");

	int dest = portMapping->getTarget("image.output");	
    tc_->setActiveTarget(dest,"SimpleRaycaster::image.output");
    
    glViewport(0,0,(int)size_.x,(int)size_.y);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
	VolumeHandle* volumeHandle = portMapping->getVolumeHandle("volumehandle.volumehandle");
    if( (volumeHandle != 0) && (volumeHandle != currentVolumeHandle_) )
    {
        setVolumeHandle(volumeHandle);
    }
    if( currentVolumeHandle_ == 0 )
		return;

	// compile program
	if (firstPass_) {
		compileShader();
		LGL_ERROR;
	}

    // bind entry params
    glActiveTexture(tm_.getGLTexUnit(entryParamsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(entryParams), tc_->getGLTexID(entryParams));
    glActiveTexture(tm_.getGLTexUnit(entryParamsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(entryParams), tc_->getGLDepthTexID(entryParams));
    LGL_ERROR;

    // bind exit params
    glActiveTexture(tm_.getGLTexUnit(exitParamsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(exitParams), tc_->getGLTexID(exitParams));
    glActiveTexture(tm_.getGLTexUnit(exitParamsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(exitParams), tc_->getGLDepthTexID(exitParams));
    LGL_ERROR;

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

    // add main volume
    volumeTextures.push_back(VolumeStruct(
        currentVolumeHandle_->getVolumeGL(),
        volTexUnit_,
        "volume_",
        "volumeParameters_")
    );

   	// segmentation volume
    if (useSegmentation_.get() && volumeContainer_->getVolumeGL(Modality::MODALITY_SEGMENTATION)) {
        volumeTextures.push_back(VolumeStruct(
            volumeContainer_->getVolumeGL(Modality::MODALITY_SEGMENTATION),
            segmentationTexUnit_,
            "segmentation_",
            "segmentationParameters_")
        );

        // set texture filters for this volume/texunit
        glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));

        glPushAttrib(GL_TEXTURE_BIT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glPopAttrib();
    }

    // bind transfer function
    glActiveTexture(tm_.getGLTexUnit(transferTexUnit_));
    if (transferFunc_.get())
        transferFunc_.get()->bind();

	// initialize shader
    raycastPrg_->activate();
	if (firstPass_) {
		// set common uniforms used by all shaders
        setGlobalShaderParameters(raycastPrg_);
        // bind the volumes and pass the necessary information to the shader
        bindVolumes(raycastPrg_, volumeTextures);
        // pass the remaining uniforms to the shader
        raycastPrg_->setUniform("entryParams_", (GLint) tm_.getTexUnit(entryParamsTexUnit_));
		raycastPrg_->setUniform("entryParamsDepth_", (GLint) tm_.getTexUnit(entryParamsDepthTexUnit_));
		raycastPrg_->setUniform("exitParams_", (GLint) tm_.getTexUnit(exitParamsTexUnit_));
		raycastPrg_->setUniform("exitParamsDepth_", (GLint) tm_.getTexUnit(exitParamsDepthTexUnit_));
		raycastPrg_->setUniform("lowerThreshold_", lowerTH_.get());
		raycastPrg_->setUniform("upperThreshold_", upperTH_.get());
		raycastPrg_->setUniform("transferFunc_", (GLint) tm_.getTexUnit(transferTexUnit_));
		if (useSegmentation_.get() && volumeContainer_->getVolumeGL(Modality::MODALITY_SEGMENTATION))
            raycastPrg_->setUniform("segment_" , (float)segment_.get());
    }

    renderQuad();

    raycastPrg_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

std::string SimpleRaycaster::generateHeader() {
    std::string header = VolumeRenderer::generateHeader();

    header += transferFunc_.get()->getShaderDefines();

    if (useSegmentation_.get() && volumeContainer_->getVolumeGL(Modality::MODALITY_SEGMENTATION)){
        header += "#define USE_SEGMENTATION\n";
    }
    if( (currentVolumeHandle_ != 0) && (currentVolumeHandle_->getVolume()->getBitsStored() == 12) )
    {
        header += "#define BITDEPTH_12\n";
    }
    return header;
}



} // namespace
