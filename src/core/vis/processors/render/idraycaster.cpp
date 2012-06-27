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

#include "voreen/core/vis/processors/render/idraycaster.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/processors/portmapping.h"

namespace voreen {

/*
    init statics
*/

const Identifier IDRaycaster::firstHitTexUnit_        = "firstHitTexUnit";
const Identifier IDRaycaster::firstHitDepthTexUnit_   = "firstHitDepthTexUnit";

IDRaycaster::IDRaycaster():
    VolumeRaycaster("rc_id.frag"),
    coarse_(false),
    standAlone_(true),
    useBlurring_("set.useBlurring", "use blurring", false),
    blurDelta_("set.blurDelta", "Delta", 2.0f, 0.1f, 10.f),
    penetrationDepth_("set.penetrationDepth", "penetration depth", 0.05f, 0.f, 0.5f),
    transferFunc_(setTransFunc_, "not used", 0, false)
{

	// initialize transfer function
	TransFuncIntensityKeys* tf = new TransFuncIntensityKeys();
	tf->createStdFunc();
    transferFunc_.set(tf);

    addProperty(&transferFunc_);
    addProperty( &penetrationDepth_ );
    addProperty( &useBlurring_ );
    addProperty( &blurDelta_ );

    tm_.addTexUnit(firstHitTexUnit_);
    tm_.addTexUnit(firstHitDepthTexUnit_);

    createInport("volumehandle.dataset");
	createInport("volumehandle.segmentation");
    
    createInport("image.entrypoints");
    createInport("image.exitpoints");
    createOutport("image.idmap");

    // temporary only
    //
    volumeContainer_ = 0;
    currentDataset_ = 0;
}

IDRaycaster::~IDRaycaster() {
    MsgDistr.remove(this);
}

const std::string IDRaycaster::getProcessorInfo() const {
	return "Writes color coded regions of a segmented dataset to the alpha channel of the rendering target. The three color channels are filled with the first-hit-positions.";
}

void IDRaycaster::setPropertyDestination(Identifier dest){
    VolumeRaycaster::setPropertyDestination(dest);
    MsgDistr.insert(this);
    transferFunc_.setMsgDestination(dest);
    penetrationDepth_.setMsgDestination(dest);
    blurDelta_.setMsgDestination(dest);
    useBlurring_.setMsgDestination(dest);
}

void IDRaycaster::processMessage(Message* msg, const Identifier& dest)
{
	VolumeRaycaster::processMessage(msg, dest);
    // moved here from super class VolumeRenderer. Eliminate on removing old VolumeContainer
    if (msg->id_ == setCurrentDataset_) {
        int oldDataset = currentDataset_;
        msg->getValue(currentDataset_);
        if (currentDataset_ != oldDataset) {
            invalidate();
        }
    }
    else if (msg->id_ == setVolumeContainer_) {
        VolumeContainer* oldContainer = volumeContainer_;
        msg->getValue(volumeContainer_);
        if (volumeContainer_ != oldContainer) {
            invalidate();
        }
    }

    if (msg->id_ == VoreenPainter::switchCoarseness_){
        coarse_ = msg->getValue<bool>();
    }
    else if (msg->id_ == "set.penetrationDepth"){
        penetrationDepth_.set( msg->getValue<float>());
        invalidate();
    }
    else if (msg->id_ == setTransFunc_) {
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
	} else if (msg->id_ == "set.useBlurring") {
		useBlurring_.set( msg->getValue<bool>() );
		invalidate();
	} else if (msg->id_ == "set.blurDelta") {
        blurDelta_.set( msg->getValue<float>() );
        invalidate();
    }
}

int IDRaycaster::initializeGL() {
    loadShader();
    return VRN_OK;
}

/**
 * Load the needed shaders.
 *
 */
void IDRaycaster::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", this->fragmentShaderFilename_.c_str(), generateHeader(), false);
    blurShader_ = ShdrMgr.loadSeparate("pp_identity.vert", "pp_blur.frag", generateHeader(), false);
	invalidateShader();
}

/**
 * If set to false, the entry-exit-param's RTs will
 * not be freed. Use this, if IDRaycaster is followed
 * by another raycaster.
 * Default is true.
 */
void IDRaycaster::setStandAlone(bool standAlone){
    standAlone_ = standAlone;
}


/**
 * Compile and link the shader program
 */
void IDRaycaster::compile() {
    // evaluate state and configure shader appropriately
    raycastPrg_->setHeaders(generateHeader(), false);
    raycastPrg_->rebuild();
}

/**
 * Performs the raycasting.
 *
 * Initialize two texture units with the entry and exit params and renders
 * a screen aligned quad.
 */
void IDRaycaster::process(LocalPortMapping* portMapping) {

    compileShader();
    LGL_ERROR;
    int entryParams = portMapping->getTarget("image.entrypoints");
    int exitParams = portMapping->getTarget("image.exitpoints");
    // get render target for first rendering pass: id-raycasting
	int tempDest = portMapping->getTarget("image.idmap");

    tc_->setActiveTarget(tempDest, "IDRaycaster::render");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // don't render when coarse
    if (coarse_) {
        return;
    }

    // bind entry params
    glActiveTexture(tm_.getGLTexUnit(entryParamsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(entryParams), tc_->getGLTexID(entryParams));
    glActiveTexture(tm_.getGLTexUnit(entryParamsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(entryParams), tc_->getGLDepthTexID(entryParams));

    // bind exit params
    glActiveTexture(tm_.getGLTexUnit(exitParamsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(exitParams), tc_->getGLTexID(exitParams));
    glActiveTexture(tm_.getGLTexUnit(exitParamsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(exitParams), tc_->getGLDepthTexID(exitParams));
    LGL_ERROR;

    // vector containing the volumes to bind
    std::vector<VolumeStruct> volumes;

    if (!getCurrentDataset())
        return;
	
    // add main volume
    int datasetNumber = portMapping->getVolumeNumber("volume.dataset");
    volumes.push_back(VolumeStruct(
        volumeContainer_->getVolumeGL(datasetNumber),
        volTexUnit_,
        "volume_",
        "volumeParameters_")
    );


    int segmentationNumber = portMapping->getVolumeNumber("volume.segmentation");
    if (volumeContainer_->getVolumeGL(segmentationNumber)) {
         // segmentation volume
         volumes.push_back(VolumeStruct(
            volumeContainer_->getVolumeGL(segmentationNumber),
            segmentationTexUnit_,
            "segmentation_",
            "segmentationParameters_")
        );
        // set texture filters for this volume/texunit
        glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));
        volumeContainer_->getVolumeGL(segmentationNumber)->getTexture()->bind();
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // set render target to type id-raycasting
    } else {
        
        LERROR("No segmentation volume");

        return;

    }

    // bind transfer function
    glActiveTexture(tm_.getGLTexUnit(transferTexUnit_));
    transferFunc_.get()->bind();
    // initialize shader
    raycastPrg_->activate();
    setGlobalShaderParameters(raycastPrg_);
    bindVolumes(raycastPrg_, volumes);
    raycastPrg_->setUniform("lowerThreshold_", lowerTH_.get());
    raycastPrg_->setUniform("upperThreshold_", upperTH_.get());
    raycastPrg_->setUniform("entryParams_", (GLint) tm_.getTexUnit(entryParamsTexUnit_));
    raycastPrg_->setUniform("entryParamsDepth_", (GLint) tm_.getTexUnit(entryParamsDepthTexUnit_));
    raycastPrg_->setUniform("exitParams_", (GLint) tm_.getTexUnit(exitParamsTexUnit_));
    raycastPrg_->setUniform("exitParamsDepth_", (GLint) tm_.getTexUnit(exitParamsDepthTexUnit_));
    if ( volumeContainer_->getVolumeGL(datasetNumber)->getVolume()->getBitsStored() == 12 )
        raycastPrg_->setUniform("volumeScaleFactor_", 16.f);
    else
        raycastPrg_->setUniform("volumeScaleFactor_", 1.f);
    
    raycastPrg_->setUniform("transferFunc_", (GLint) tm_.getTexUnit(transferTexUnit_));
    // raycastPrg_->setUniform("viewMatrix_", tgt::mat4::identity);
    raycastPrg_->setUniform("penetrationDepth_", penetrationDepth_.get());
    renderQuad();

    raycastPrg_->deactivate();

    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;

}

} // namespace voreen
