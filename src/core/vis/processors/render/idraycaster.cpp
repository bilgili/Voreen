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

#include "voreen/core/vis/processors/render/idraycaster.h"
#include "voreen/core/vis/voreenpainter.h"

#include "voreen/core/vis/properties/cameraproperty.h"

using tgt::vec3;

namespace voreen {

const std::string IDRaycaster::firstHitPointsTexUnit_("firstHitPointsTexUnit");

IDRaycaster::IDRaycaster()
    : VolumeRaycaster()
    , camera_("camera", "Camera", new tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , penetrationDepth_("penetrationDepth", "penetration depth", 0.05f, 0.f, 0.5f)
    , volumePort_(Port::INPORT, "volumehandle.volumehandle")
    , entryPort_(Port::INPORT, "image.entrypoints")
    , exitPort_(Port::INPORT, "image.exitpoints")
    , firstHitpointsPort_(Port::INPORT, "image.firsthitpoints")
    , idMapPort_(Port::OUTPORT, "image.idmap", true)
{
    
    addProperty(penetrationDepth_ );
    addProperty(camera_);

    tm_.addTexUnit(entryParamsTexUnit_);
    tm_.addTexUnit(entryParamsTexUnit_);
    tm_.addTexUnit(firstHitPointsTexUnit_);
    tm_.addTexUnit(exitParamsTexUnit_);
    tm_.addTexUnit(exitParamsTexUnit_);

    addPort(volumePort_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(firstHitpointsPort_);
    addPort(idMapPort_);
}

IDRaycaster::~IDRaycaster() {
}

const std::string IDRaycaster::getProcessorInfo() const {
    return "Writes color coded regions of a segmented dataset to the alpha channel of the "
        "rendering target. The three color channels are filled with the first-hit-positions.";
}

Processor* IDRaycaster::create() const {
    return new IDRaycaster();
}

void IDRaycaster::initialize() throw (VoreenException) {
    VolumeRaycaster::initialize();
    loadShader();
}

/**
 * Load the needed shaders.
 *
 */
void IDRaycaster::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "rc_id.frag", generateHeader(), false, false);
    invalidate(Processor::INVALID_PROGRAM);
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
void IDRaycaster::process() {

    if (!volumePort_.isReady())
        return;

    if(getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    LGL_ERROR;

    idMapPort_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // don't render when in interaction mode
    if (interactionMode()) {
        glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
        return;
    }

    // bind entry points
    entryPort_.bindTextures(tm_.getGLTexUnit(entryParamsTexUnit_), tm_.getGLTexUnit(entryParamsDepthTexUnit_));

    // bind first hit points
    firstHitpointsPort_.bindColorTexture(tm_.getGLTexUnit(firstHitPointsTexUnit_));

    // bind exit points
    exitPort_.bindTextures(tm_.getGLTexUnit(exitParamsTexUnit_), tm_.getGLTexUnit(exitParamsDepthTexUnit_));

    // vector containing the volumes to bind
    std::vector<VolumeStruct> volumes;

    volumes.push_back(VolumeStruct(
        volumePort_.getData()->getVolumeGL(),
        volTexUnit_,
        "segmentation_",
        "segmentationParameters_")
    );

    glActiveTexture(tm_.getGLTexUnit(segmentationTexUnit_));
    volumePort_.getData()->getVolumeGL()->getTexture()->bind();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // initialize shader
    raycastPrg_->activate();
    setGlobalShaderParameters(raycastPrg_, camera_.get());
    bindVolumes(raycastPrg_, volumes);
    raycastPrg_->setUniform("entryPoints_", tm_.getTexUnit(entryParamsTexUnit_));
    raycastPrg_->setUniform("entryPointsDepth_", tm_.getTexUnit(entryParamsDepthTexUnit_));
    entryPort_.setTextureParameters(raycastPrg_, "entryParameters_");
    raycastPrg_->setUniform("firstHitPoints_", tm_.getTexUnit(firstHitPointsTexUnit_));
    firstHitpointsPort_.setTextureParameters(raycastPrg_, "firstHitParameters_");
    raycastPrg_->setUniform("exitPoints_", tm_.getTexUnit(exitParamsTexUnit_));
    raycastPrg_->setUniform("exitPointsDepth_", tm_.getTexUnit(exitParamsDepthTexUnit_));
    entryPort_.setTextureParameters(raycastPrg_, "exitParameters_");

    raycastPrg_->setUniform("penetrationDepth_", penetrationDepth_.get());
    renderQuad();

    raycastPrg_->deactivate();
    idMapPort_.deactivateTarget();

    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

} // namespace voreen
