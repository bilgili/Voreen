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

#include "voreen/modules/base/processors/image/compositor.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

Compositor::Compositor()
    : ImageProcessor("pp_compositor")
    , compositingMode_("blendMode", "Blend mode", Processor::INVALID_PROGRAM)
    , weightingFactor_("weightingFactor", "Weighting factor", 0.5f, 0.0f, 1.0f, true)
    , inport0_(Port::INPORT, "image.inport0")
    , inport1_(Port::INPORT, "image.inport1")
    , outport_(Port::OUTPORT, "image.outport")
{

    compositingMode_.addOption("depth-test",                "Depth Test",                   "MODE_DEPTH_TEST");
    compositingMode_.addOption("alpha-compositing",         "Alpha Compositing",            "MODE_ALPHA_COMPOSITING");
    compositingMode_.addOption("alpha-blending",            "Alpha Blending",               "MODE_ALPHA_BLENDING");
    compositingMode_.addOption("alpha-blending-b-over-a",   "Alpha Blending (B over A)",    "MODE_ALPHA_BLENDING_B_OVER_A");
    compositingMode_.addOption("weighted-average",          "Weighted Average",             "MODE_WEIGHTED_AVERAGE");
    compositingMode_.addOption("take-first",                "Take First",                   "MODE_TAKE_FIRST");
    compositingMode_.addOption("take-second",               "Take Second",                  "MODE_TAKE_SECOND");
    compositingMode_.addOption("first-has-priority",        "First Has Priority",           "MODE_FIRST_HAS_PRIORITY");
    compositingMode_.addOption("second-has-priority",       "Second Has Priority",          "MODE_SECOND_HAS_PRIORITY");
    compositingMode_.addOption("maximum-alpha",             "Maximum Alpha",                "MODE_MAXIMUM_ALPHA");
    compositingMode_.addOption("difference",                "Difference",                   "MODE_DIFFERENCE");
    compositingMode_.set("alpha-compositing");
    compositingMode_.onChange(CallMemberAction<Compositor>(this, &Compositor::compositingModeChanged));
    addProperty(compositingMode_);

    addProperty(weightingFactor_);

    addPort(inport0_);
    addPort(inport1_);
    addPort(outport_);
}

Compositor::~Compositor() {
}

std::string Compositor::getProcessorInfo() const {
    return "Composites two images with a selectable blending method.";
}

void Compositor::process() {

    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_ALWAYS);

    TextureUnit shadeUnit, shadeUnitDepth;
    TextureUnit shadeUnit1, shadeUnitDepth1;
    inport0_.bindTextures(shadeUnit.getEnum(), shadeUnitDepth.getEnum());
    inport1_.bindTextures(shadeUnit1.getEnum(), shadeUnitDepth1.getEnum());

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    if (compositingMode_.get() != "take-second") {
        program_->setUniform("shadeTex0_", shadeUnit.getUnitNumber());
        program_->setUniform("depthTex0_", shadeUnitDepth.getUnitNumber());
        inport0_.setTextureParameters(program_, "textureParameters0_");
    }
    if (compositingMode_.get() != "take-first") {
        program_->setUniform("shadeTex1_", shadeUnit1.getUnitNumber());
        program_->setUniform("depthTex1_", shadeUnitDepth1.getUnitNumber());
        inport1_.setTextureParameters(program_, "textureParameters1_");
    }
    if (compositingMode_.get() == "weighted-average")
        program_->setUniform("weightingFactor_", weightingFactor_.get());

    renderQuad();

    glDepthFunc(GL_LESS);

    program_->deactivate();
    outport_.deactivateTarget();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string Compositor::generateHeader() {
    std::string header = ImageProcessor::generateHeader();
    header += "#define " + compositingMode_.getValue() + "\n";
    return header;
}

void Compositor::compile() {
    if (program_)
        program_->setHeaders(generateHeader(), false);
    ImageProcessor::compile();
}

void Compositor::compositingModeChanged() {
    weightingFactor_.setVisible(compositingMode_.get() == "weighted-average");
}

} // voreen namespace
