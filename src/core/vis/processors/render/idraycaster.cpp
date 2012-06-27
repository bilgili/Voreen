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

const Identifier IDRaycaster::firstHitPointsTexUnit_("firstHitPointsTexUnit");     

IDRaycaster::IDRaycaster(): VolumeRaycaster(),
    coarse_(false),
    penetrationDepth_("set.penetrationDepth", "penetration depth", 0.05f, 0.f, 0.5f)
{
    addProperty(&penetrationDepth_ );

    tm_.addTexUnit(entryParamsTexUnit_);
    tm_.addTexUnit(entryParamsTexUnit_);
    tm_.addTexUnit(firstHitPointsTexUnit_);
    tm_.addTexUnit(exitParamsTexUnit_);
    tm_.addTexUnit(exitParamsTexUnit_);

	createInport("volumehandle.segmentation");
    createInport("image.entrypoints");
    createInport("image.firsthitpoints");
    createInport("image.exitpoints");
    
    createOutport("image.idmap");
}

IDRaycaster::~IDRaycaster() {
    MsgDistr.remove(this);
}

const std::string IDRaycaster::getProcessorInfo() const {
	return "Writes color coded regions of a segmented dataset to the alpha channel of the rendering target. The three color channels are filled with the first-hit-positions.";
}

void IDRaycaster::setPropertyDestination(Identifier dest) {
    VolumeRaycaster::setPropertyDestination(dest);
    MsgDistr.insert(this);
    penetrationDepth_.setMsgDestination(dest);
}

void IDRaycaster::processMessage(Message* msg, const Identifier& dest) {
	VolumeRaycaster::processMessage(msg, dest);

    if (msg->id_ == VoreenPainter::switchCoarseness_)
        coarse_ = msg->getValue<bool>();
    else if (msg->id_ == "set.penetrationDepth") {
        penetrationDepth_.set( msg->getValue<float>());
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
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "rc_id.frag", generateHeader(), false);
	invalidateShader();
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
    int entryPoints = portMapping->getTarget("image.entrypoints");
    int firstHitPoints = portMapping->getTarget("image.firsthitpoints");
    int exitPoints = portMapping->getTarget("image.exitpoints");
	int dest = portMapping->getTarget("image.idmap");
    
    tc_->setActiveTarget(dest, "IDRaycaster::render");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    VolumeHandle* volumeHandle = portMapping->getVolumeHandle("volumehandle.segmentation");
    if (volumeHandle != 0) {
       if (!volumeHandle->isIdentical(currentVolumeHandle_))
           setVolumeHandle(volumeHandle);
    }
    else
       setVolumeHandle(0); 

    if ((currentVolumeHandle_ == 0) || (currentVolumeHandle_->getVolumeGL() == 0))
		return;

    // don't render when coarse
    if (coarse_)
        return;

    // bind entry points
    glActiveTexture(tm_.getGLTexUnit(entryParamsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(entryPoints), tc_->getGLTexID(entryPoints));
    glActiveTexture(tm_.getGLTexUnit(entryParamsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(entryPoints), tc_->getGLDepthTexID(entryPoints));

    // bind first hit points
    glActiveTexture(tm_.getGLTexUnit(firstHitPointsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(firstHitPoints), tc_->getGLTexID(firstHitPoints));

    // bind exit points
    glActiveTexture(tm_.getGLTexUnit(exitParamsTexUnit_));
    glBindTexture(tc_->getGLTexTarget(exitPoints), tc_->getGLTexID(exitPoints));
    glActiveTexture(tm_.getGLTexUnit(exitParamsDepthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(exitPoints), tc_->getGLDepthTexID(exitPoints));
    LGL_ERROR;

    // vector containing the volumes to bind
    std::vector<VolumeStruct> volumes;

    volumes.push_back(VolumeStruct(
        currentVolumeHandle_->getVolumeGL(),
        volTexUnit_,
        "segmentation_",
        "segmentationParameters_")
    );

    glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));
    currentVolumeHandle_->getVolumeGL()->getTexture()->bind();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // initialize shader
    raycastPrg_->activate();
    setGlobalShaderParameters(raycastPrg_);
    bindVolumes(raycastPrg_, volumes);
    raycastPrg_->setUniform("entryPoints_", tm_.getTexUnit(entryParamsTexUnit_));
    raycastPrg_->setUniform("entryPointsDepth_", tm_.getTexUnit(entryParamsDepthTexUnit_));
    raycastPrg_->setUniform("firstHitPoints_", tm_.getTexUnit(firstHitPointsTexUnit_));
    raycastPrg_->setUniform("exitPoints_", tm_.getTexUnit(exitParamsTexUnit_));
    raycastPrg_->setUniform("exitPointsDepth_", tm_.getTexUnit(exitParamsDepthTexUnit_));
    
    raycastPrg_->setUniform("penetrationDepth_", penetrationDepth_.get());
    renderQuad();

    raycastPrg_->deactivate();

    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

} // namespace voreen
