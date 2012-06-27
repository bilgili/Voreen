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

#include "voreen/core/vis/processors/image/regionmodifier.h"

namespace voreen {

using tgt::Color;

//---------------------------------------------------------------------------

const Identifier RegionModifier::shadeTexUnit1_ = "shadeTexUnit1";
const Identifier RegionModifier::depthTexUnit1_ = "depthTexUnit1";

RegionModifier::RegionModifier()
    : GenericFragment("pp_regionmodifier"),
      mode_(MODE_BLEND),
      segmentId_("set.segmentId", "Segment-ID", Color(1.0, 0.0, 0.0, 1.0)),
      destColor_("set.destColor", "color", Color(0.0, 0.0, 1.0, 1.0))
{
    tm_.addTexUnit(shadeTexUnit1_);
    tm_.addTexUnit(depthTexUnit1_);
    setName("RegionModifier");
    // init Properties
    std::vector<std::string> modes;
    modes.push_back("replace");
    modes.push_back("blend");
    modeProp_ = new EnumProp("set.RegionModifierMode", "Set Mode", modes, &needRecompileShader_, 0);
    addProperty(modeProp_);
    addProperty(&segmentId_);
    addProperty(&destColor_);
    // init Modemap
    modeDefinesMap_[MODE_REPLACE] = "RV_MODE_REPLACE";
    modeDefinesMap_[MODE_BLEND] = "RV_MODE_BLEND";
    // Ports
    createInport("image.input");
    createInport("image.mask");
    createOutport("image.outport");
}

const std::string RegionModifier::getProcessorInfo() const {
	return "Highlights a part of an image using a segmentation image.";
}

void RegionModifier::compile()
{
    program_->setHeaders(generateHeader(), false);
    program_->rebuild();
}

std::string RegionModifier::generateHeader() {
    std::string header = GenericFragment::generateHeader();
    header += "#define " + modeDefinesMap_[mode_] + "\n";
    return header;
}

void RegionModifier::process(LocalPortMapping* portMapping) {
    glViewport(0,0,static_cast<GLsizei>(size_.x),static_cast<GLsizei>(size_.y));

    compileShader(); // need this because of conditioned compilation

	int source =  portMapping->getTarget("image.input");
	int mask =  portMapping->getTarget("image.mask");
    int dest = portMapping->getTarget("image.outport");

    tc_->setActiveTarget(dest, "RegionModifier::process");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind shading and depth result from previous ray cast
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit_));
    glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));
    glActiveTexture(tm_.getGLTexUnit(depthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
    LGL_ERROR;

    // bind shading and (not) depth from mask 
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit1_));
    glBindTexture(tc_->getGLTexTarget(mask), tc_->getGLTexID(mask));
    //glActiveTexture(tm_.getGLTexUnit(depthTexUnit1_));
    //glBindTexture(tc_->getGLDepthTexTarget(mask), tc_->getGLDepthTexID(mask));
    LGL_ERROR;

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", (GLint) tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex_", (GLint) tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("shadeTexMask_", (GLint) tm_.getTexUnit(shadeTexUnit1_));
    //program_->setUniform("depthTexMask_", (GLint) tm_.getTexUnit(depthTexUnit1_));
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

void RegionModifier::processMessage(Message* msg, const Identifier& dest){
	GenericFragment::processMessage(msg, dest);
    if (msg->id_ == "set.segmentId") {
        segmentId_.set(msg->getValue<Color>());
        invalidate();
    }
    else if (msg->id_ == "set.destColor") {
        destColor_.set(msg->getValue<Color>());
        invalidate();
    }
    else if (msg->id_ == "set.RegionModfifierMode") {
        std::string modeStr = msg->getValue<std::string>();
        if (modeStr == "replace") mode_ = MODE_REPLACE;
        else if (modeStr == "blend") mode_ = MODE_BLEND;
        needRecompileShader_ = true;
        invalidate();
    }
}


} // voreen namespace

