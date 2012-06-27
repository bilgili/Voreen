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

#include "voreen/modules/base/processors/image/explosioncompositor.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

ExplosionCompositor::ExplosionCompositor()
    : ImageProcessor("image/compositor")
    , compositingMode_("blendMode", "Blend mode", Processor::INVALID_PROGRAM)
    , weightingFactor_("weightingFactor", "Weighting factor", 0.5f, 0.0f, 1.0f)
    , inport0_(Port::INPORT, "image.inport0")
    , interalPort_(Port::OUTPORT, "internalRenderPort", false)
    , outport_(Port::OUTPORT, "image.outport")
    , loopOutport_("loop.outport", Port::OUTPORT)
{
    loopOutport_.setLoopPort(true);

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
    compositingMode_.onChange(CallMemberAction<ExplosionCompositor>(this, &ExplosionCompositor::compositingModeChanged));
    compositingMode_.select("alpha-compositing");
    addProperty(compositingMode_);

    addProperty(weightingFactor_);

    addPort(inport0_);
    addPrivateRenderPort(interalPort_);
    //interalPort_.initialize();
    addPort(outport_);
    addPort(loopOutport_);

}

ExplosionCompositor::~ExplosionCompositor() {
}

std::string ExplosionCompositor::getProcessorInfo() const {
    return "Composites the partial renderings of an exploded view."
           "<p><b>See</b>: ExplosionProxyGeometry</p>";
}

Processor* ExplosionCompositor::create() const {
    return new ExplosionCompositor();
}

void ExplosionCompositor::process() {

    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();


    if (loopOutport_.getLoopIteration() == 0) {
        outport_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        interalPort_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    TextureUnit colorUnit0, depthUnit0, colorUnit1, depthUnit1;
    if ((loopOutport_.getLoopIteration() % 2) == 0) {
        // outport <-- inport + internal
        outport_.activateTarget();
        inport0_.bindTextures(colorUnit0.getEnum(), depthUnit0.getEnum());
        interalPort_.bindTextures(colorUnit1.getEnum(), depthUnit1.getEnum());
    }
    else {
        // internal <-- inport + outport
        interalPort_.activateTarget();
        inport0_.bindTextures(colorUnit0.getEnum(), depthUnit0.getEnum());
        outport_.bindTextures(colorUnit1.getEnum(), depthUnit1.getEnum());
    }


    if (loopOutport_.getLoopIteration() == (loopOutport_.getNumLoopIterations()-1)){
        outport_.activateTarget();
    }


    glDepthFunc(GL_ALWAYS);

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    if (compositingMode_.get() != "take-second") {
        program_->setUniform("colorTex0_", colorUnit0.getUnitNumber());
        program_->setUniform("depthTex0_", depthUnit0.getUnitNumber());
        //inport0_.setTextureParameters(program_, "textureParameters0_");
        outport_.setTextureParameters(program_, "textureParameters0_");
    }
    if (compositingMode_.get() != "take-first") {
        program_->setUniform("colorTex1_", colorUnit1.getUnitNumber());
        program_->setUniform("depthTex1_", depthUnit1.getUnitNumber());
        //interalPort_.setTextureParameters(program_, "textureParameters1_");
        outport_.setTextureParameters(program_, "textureParameters1_");
    }
    if (compositingMode_.get() == "weighted-average")
        program_->setUniform("weightingFactor_", weightingFactor_.get());

    renderQuad();

    glDepthFunc(GL_LESS);

    program_->deactivate();
    outport_.deactivateTarget();
    interalPort_.deactivateTarget();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string ExplosionCompositor::generateHeader() {
    std::string header = ImageProcessor::generateHeader();
    header += "#define " + compositingMode_.getValue() + "\n";
    return header;
}

void ExplosionCompositor::compile() {
    if (program_)
        program_->setHeaders(generateHeader());
    ImageProcessor::compile();
}

void ExplosionCompositor::compositingModeChanged() {
    weightingFactor_.setVisible(compositingMode_.get() == "weighted-average");
}


} // voreen namespace
