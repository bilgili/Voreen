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

#include "voreen/core/vis/processors/image/regionmodifier.h"

namespace voreen {

using tgt::Color;

const std::string RegionModifier::shadeTexUnit1_ = "shadeTexUnit1";
const std::string RegionModifier::depthTexUnit1_ = "depthTexUnit1";

RegionModifier::RegionModifier()
    : ImageProcessor("pp_regionmodifier")
    , mode_(MODE_BLEND)
    , modeProp_("mode", "Set Mode", Processor::INVALID_PROGRAM)
    , segmentId_("segmentId", "Segment-ID", Color(1.0, 0.0, 0.0, 1.0))
    , destColor_("destColor", "color", Color(0.0, 0.0, 1.0, 1.0))
    , inport_(Port::INPORT, "image.input")
    , maskPort_(Port::INPORT, "image.mask")
    , outport_(Port::OUTPORT, "image.outport", true)
{
    tm_.addTexUnit(shadeTexUnit1_);
    tm_.addTexUnit(depthTexUnit1_);
    // init Properties
    modeProp_.addOption("replace", "replace");
    modeProp_.addOption("blend", "blend");
    modeProp_.onChange(CallMemberAction<RegionModifier>(this, &RegionModifier::setRegionModifierModeEvt));
    addProperty(modeProp_);
    addProperty(segmentId_);
    addProperty(destColor_);
    // init Modemap
    modeDefinesMap_[MODE_REPLACE] = "RV_MODE_REPLACE";
    modeDefinesMap_[MODE_BLEND] = "RV_MODE_BLEND";
    // Ports
    addPort(inport_);
    addPort(maskPort_);
    addPort(outport_);
}

const std::string RegionModifier::getProcessorInfo() const {
    return "Highlights a part of an image using a segmentation image.";
}

Processor* RegionModifier::create() const {
    return new RegionModifier();
}

void RegionModifier::compile() {
    program_->setHeaders(generateHeader(), false);
    program_->rebuild();
}

std::string RegionModifier::generateHeader() {
    std::string header = ImageProcessor::generateHeader();
    header += "#define " + modeDefinesMap_[mode_] + "\n";
    return header;
}

void RegionModifier::process() {
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile(); // need this because of conditioned compilation

    outport_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind shading and depth result from previous ray cast
    inport_.bindTextures(tm_.getGLTexUnit(shadeTexUnit_), tm_.getGLTexUnit(depthTexUnit_));

    // bind shading and (not) depth from mask
    maskPort_.bindColorTexture(tm_.getGLTexUnit(shadeTexUnit1_));
    //maskPort_.bindDepthTexture(tm_.getGLTexUnit(depthTexUnit1_));

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex_", tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("shadeTexMask_", tm_.getTexUnit(shadeTexUnit1_));
    //program_->setUniform("depthTexMask_", tm_.getTexUnit(depthTexUnit1_));
    program_->setUniform("segmentId_", segmentId_.get());
    program_->setUniform("destColor_", destColor_.get());
    //program_->setUniform("mode_", modeMap_[mode_]);

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

void RegionModifier::setRegionModifierModeEvt() {

    if (modeProp_.get() == "replace")
        mode_ = MODE_REPLACE;
    else if (modeProp_.get() == "blend")
        mode_ = MODE_BLEND;
}

} // voreen namespace
