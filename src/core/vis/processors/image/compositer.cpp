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

#include "voreen/core/vis/processors/image/compositer.h"

namespace voreen {

const std::string Compositer::shadeTexUnit1_ = "shadeTexUnit1";
const std::string Compositer::depthTexUnit1_ = "depthTexUnit1";

Compositer::Compositer()
    : ImageProcessor("pp_compositer")
    , compositingMode_("blendMode", "Blend mode", Processor::INVALID_PROGRAM)
    , blendFactor_("blendFactor", "Blend factor", 0.5f, 0.0f, 1.0f, true)
    , inport0_(Port::INPORT, "image.inport0")
    , inport1_(Port::INPORT, "image.inport1")
    , outport_(Port::OUTPORT, "image.outport")
{

    tm_.addTexUnit(shadeTexUnit1_);
    tm_.addTexUnit(depthTexUnit1_);

    compositingMode_.addOption("depth-test",            "Depth test",           "MODE_DEPTH_TEST");
    compositingMode_.addOption("alpha-compositing",     "Alpha compositing",    "MODE_ALPHA_COMPOSITING");
    compositingMode_.addOption("blend",                 "Blend",                "MODE_BLEND");
    compositingMode_.addOption("take-first",            "Take first",           "MODE_TAKE_FIRST");
    compositingMode_.addOption("take-second",           "Take second",          "MODE_TAKE_SECOND");
    compositingMode_.addOption("first-has-priority",    "First has priority",   "MODE_FIRST_HAS_PRIORITY");
    compositingMode_.addOption("second-has-priority",   "Second has priority",  "MODE_SECOND_HAS_PRIORITY");
    compositingMode_.addOption("maximum-alpha",         "Maximum alpha",        "MODE_MAXIMUM_ALPHA");
    compositingMode_.addOption("difference",            "Difference",           "MODE_DIFFERENCE");
    addProperty(compositingMode_);

    addProperty(blendFactor_);

    addPort(inport0_);
    addPort(inport1_);
    addPort(outport_);
}

Compositer::~Compositer() {
}

const std::string Compositer::getProcessorInfo() const {
    return "Composites two images with a selectable blending method.";
}

void Compositer::process() {

    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    inport0_.bindTextures(tm_.getGLTexUnit(shadeTexUnit_), tm_.getGLTexUnit(depthTexUnit_));
    inport1_.bindTextures(tm_.getGLTexUnit(shadeTexUnit1_), tm_.getGLTexUnit(depthTexUnit1_));

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    if (compositingMode_.get() != "take-second") {
        program_->setUniform("shadeTex0_", tm_.getTexUnit(shadeTexUnit_));
        program_->setUniform("depthTex0_", tm_.getTexUnit(depthTexUnit_));
        inport0_.setTextureParameters(program_, "textureParameters0_");
    }
    if (compositingMode_.get() != "take-first") {
        program_->setUniform("shadeTex1_", tm_.getTexUnit(shadeTexUnit1_));
        program_->setUniform("depthTex1_", tm_.getTexUnit(depthTexUnit1_));
        inport1_.setTextureParameters(program_, "textureParameters1_");
    }
    if (compositingMode_.get() == "blend")
        program_->setUniform("blendFactor_", blendFactor_.get());

    renderQuad();

    program_->deactivate();
    outport_.deactivateTarget();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

std::string Compositer::generateHeader() {
    std::string header = ImageProcessor::generateHeader();
    header += "#define " + compositingMode_.getValue() + "\n";
    return header;
}

void Compositer::compile() {
    if (program_)
        program_->setHeaders(generateHeader(), false);
    ImageProcessor::compile();
}

} // voreen namespace
