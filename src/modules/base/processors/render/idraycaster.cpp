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

#include "voreen/modules/base/processors/render/idraycaster.h"
#include "voreen/core/utils/voreenpainter.h"

#include "voreen/core/properties/cameraproperty.h"

#include "tgt/textureunit.h"

using tgt::vec3;
using tgt::TextureUnit;

namespace voreen {

IDRaycaster::IDRaycaster()
    : VolumeRaycaster()
    , volumePort_(Port::INPORT, "volumehandle.volumehandle")
    , entryPort_(Port::INPORT, "image.entrypoints")
    , exitPort_(Port::INPORT, "image.exitpoints")
    , firstHitpointsPort_(Port::INPORT, "image.firsthitpoints")
    , idMapPort_(Port::OUTPORT, "image.idmap", true)
    , raycastPrg_(0)
    , camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , penetrationDepth_("penetrationDepth", "Penetration Depth", 0.05f, 0.f, 0.5f)
{
    addPort(volumePort_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(firstHitpointsPort_);
    addPort(idMapPort_);

    addProperty(penetrationDepth_ );
    addProperty(camera_);
}

std::string IDRaycaster::getProcessorInfo() const {
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

void IDRaycaster::deinitialize() throw (VoreenException) {
    ShdrMgr.dispose(raycastPrg_);
    raycastPrg_ = 0;

    VolumeRaycaster::deinitialize();
}

/**
 * Load the needed shaders.
 *
 */
void IDRaycaster::loadShader() {
    raycastPrg_ = ShdrMgr.loadSeparate("passthrough.vert", "rc_id.frag", generateHeader(), false);
    invalidate(Processor::INVALID_PROGRAM);
}

/**
 * Compile and link the shader program
 */
void IDRaycaster::compile() {
    // evaluate state and configure shader appropriately
    raycastPrg_->setHeaders(generateHeader());
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
        idMapPort_.deactivateTarget();
        TextureUnit::setZeroUnit();
        return;
    }

    TextureUnit entryUnit, entryDepthUnit, exitUnit, exitDepthUnit, firstHitPointUnit;
    // bind entry params
    entryPort_.bindTextures(entryUnit.getEnum(), entryDepthUnit.getEnum());

    // bind first hit points
    firstHitpointsPort_.bindColorTexture(firstHitPointUnit.getEnum());

    // bind exit params
    exitPort_.bindTextures(exitUnit.getEnum(), exitDepthUnit.getEnum());

    // vector containing the volumes to bind
    std::vector<VolumeStruct> volumes;

    TextureUnit volUnit;
    volumes.push_back(VolumeStruct(
        volumePort_.getData()->getVolumeGL(),
        &volUnit,
        "segmentation_",
        "segmentationParameters_",
        true,
        GL_CLAMP_TO_EDGE,
        tgt::vec4(0.f),
        GL_NEAREST)
    );

    // initialize shader
    raycastPrg_->activate();
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(raycastPrg_, &cam);
    bindVolumes(raycastPrg_, volumes, &cam, lightPosition_.get());
    raycastPrg_->setUniform("entryPoints_", entryUnit.getUnitNumber());
    raycastPrg_->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    entryPort_.setTextureParameters(raycastPrg_, "entryParameters_");
    raycastPrg_->setUniform("firstHitPoints_", firstHitPointUnit.getUnitNumber());
    firstHitpointsPort_.setTextureParameters(raycastPrg_, "firstHitParameters_");
    raycastPrg_->setUniform("exitPoints_", exitUnit.getUnitNumber());
    raycastPrg_->setUniform("exitPointsDepth_", exitDepthUnit.getUnitNumber());
    entryPort_.setTextureParameters(raycastPrg_, "exitParameters_");

    raycastPrg_->setUniform("penetrationDepth_", penetrationDepth_.get());
    renderQuad();

    raycastPrg_->deactivate();
    idMapPort_.deactivateTarget();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}


} // namespace voreen
