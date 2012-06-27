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

#include "voreen/core/vis/processors/image/combine.h"
#include "voreen/core/vis/processors/portmapping.h"

namespace voreen {

const Identifier Combine::shadeTexUnit1_ = "shadeTexUnit1";
const Identifier Combine::depthTexUnit1_ = "depthTexUnit1";

Combine::Combine()
    : Collect("pp_combine")
    , firstModifyColor_("Combine.set.firstModifyColor", "First modify color", tgt::Color(1.0f, 0.0f, 0.0f, 1.0f))
    , secondModifyColor_("Combine.set.secondModifyColor", "Second modify color", tgt::Color(0.0f, 1.0f, 0.0f, 1.0f))
    , blendFactor_("Combine.set.blendFactor", "BlendFactor in percent", 0.f, 0.f, 100.f)
{
    tm_.addTexUnit(shadeTexUnit1_);
    tm_.addTexUnit(depthTexUnit1_);

    blendModeMethods_.push_back("Depth dependent");
    blendModeMethodDefines_.push_back("COMBINE_DEPTH_DEPENDENT");  // constant for use in shader
    blendModeMethods_.push_back("Second has priority");
    blendModeMethodDefines_.push_back("COMBINE_SECOND_HAS_PRIORITY");
    blendModeMethods_.push_back("Show differences");
    blendModeMethodDefines_.push_back("COMBINE_SHOW_DIFFERENCES");
    blendModeMethods_.push_back("Show overlap");
    blendModeMethodDefines_.push_back("COMBINE_SHOW_OVERLAP");
    blendModeMethods_.push_back("Blend percentaged");
    blendModeMethodDefines_.push_back("COMBINE_BLEND_PERCENTAGED");
    blendModeMethods_.push_back("Maximum Alpha");
    blendModeMethodDefines_.push_back("COMBINE_MAXIMUM_ALPHA");
    blendModeMethods_.push_back("Alpha Compositing");
    blendModeMethodDefines_.push_back("COMBINE_ALPHA_COMPOSITING");
    blendModeMethods_.push_back("Alpha Depth Compositing");
    blendModeMethodDefines_.push_back("COMBINE_DEPTH_ALPHA_COMPOSITING");

    blendMode_ = new EnumProp("Combine.set.blendMode", "Set blend mode:", blendModeMethods_, &needRecompileShader_, 0, false);
    addProperty(blendMode_);
    cond_ = new ConditionProp("blendModeCond", blendMode_);
    addProperty(cond_);
    firstModifyColor_.setAutoChange(true);
    firstModifyColor_.setConditioned("blendModeCond", 2);
    secondModifyColor_.setAutoChange(true);
    secondModifyColor_.setConditioned("blendModeCond", 2);
    addProperty(&firstModifyColor_);
    addProperty(&secondModifyColor_);
    blendFactor_.setAutoChange(true);
    blendFactor_.setConditioned("blendModeCond", 4);
    addProperty(&blendFactor_);
	backgroundColor_.setAutoChange(true);
	addProperty(&backgroundColor_);

    setName("Combine PP");
}

Combine::~Combine() {
    delete blendMode_;
    delete cond_;
}

const std::string Combine::getProcessorInfo() const {
	return "Combines the result of two Renderer objects with a set of blending methods.";
}

void Combine::compile()
{
    std::string header;
    program_->setHeaders(generateHeader(), false);
    program_->rebuild();
}

std::string Combine::generateHeader() {
    std::string header = Collect::generateHeader();
    header += "#define " + blendModeMethodDefines_[blendMode_->get()] + "\n";
    return header;
}

void Combine::renderTwo(int source0, int source1, int /*pass*/) {
    // bind shading and depth result from first ray cast
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit_));
    glBindTexture(tc_->getGLTexTarget(source0), tc_->getGLTexID(source0));
    glActiveTexture(tm_.getGLTexUnit(depthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(source0), tc_->getGLDepthTexID(source0));
    LGL_ERROR;

    // bind shading and depth result from second ray cast
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit1_));
    glBindTexture(tc_->getGLTexTarget(source1), tc_->getGLTexID(source1));
    glActiveTexture(tm_.getGLTexUnit(depthTexUnit1_));
    glBindTexture(tc_->getGLDepthTexTarget(source1), tc_->getGLDepthTexID(source1));
    LGL_ERROR;

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex0_", (GLint) tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex0_", (GLint) tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("shadeTex1_", (GLint) tm_.getTexUnit(shadeTexUnit1_));
    program_->setUniform("depthTex1_", (GLint) tm_.getTexUnit(depthTexUnit1_));
	if (blendMode_->get() == 2 || blendMode_->get() == 3){
        program_->setUniform("backgroundColor_", backgroundColor_.get());
    }
    if (blendMode_->get() == 2) {
        program_->setUniform("firstModifyColor_", firstModifyColor_.get());
        program_->setUniform("secondModifyColor_", secondModifyColor_.get());
    }
    if (blendMode_->get() == 4) {
        program_->setUniform("blendFactor_", blendFactor_.get());
    }
    renderQuad();

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

void Combine::processMessage(Message* msg, const Identifier& dest) {
    Collect::processMessage(msg, dest);

    if (msg->id_ == "Combine.set.blendMode") {
        blendMode_->set(msg->getValue<int>());
        invalidate();
    }
    else if (msg->id_ == "Combine.set.firstModifyColor") {
        firstModifyColor_.set(msg->getValue<tgt::Color>());
        invalidate();
    }
    else if (msg->id_ == "Combine.set.secondModifyColor") {
        secondModifyColor_.set(msg->getValue<tgt::Color>());
        invalidate();
    }
}


} // voreen namespace

