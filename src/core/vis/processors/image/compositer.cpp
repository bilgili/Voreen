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

const Identifier Compositer::shadeTexUnit1_ = "shadeTexUnit1";
const Identifier Compositer::depthTexUnit1_ = "depthTexUnit1";

Compositer::Compositer()
    : ImageProcessor("pp_compositer")
    , blendFactor_("Compositer.set.blendFactor", "Blend factor", 0.5f, 0.0f, 1.0f, true)
{
    setName("Compositer");

    tm_.addTexUnit(shadeTexUnit1_);
    tm_.addTexUnit(depthTexUnit1_);

    compositingModes_.push_back("Depth test");
    compositingModes_.push_back("First has priority");
    compositingModes_.push_back("Second has priority");
    compositingModes_.push_back("Blend");
    compositingModes_.push_back("Alpha Compositing");
    compositingModes_.push_back("Maximum Alpha");

    compositingMode_ = new EnumProp("Compositer.set.blendMode", "Set blend mode:", compositingModes_, 0, true, true);
    addProperty(compositingMode_);
    addProperty(&blendFactor_);

    createInport("image.inport0");
    createInport("image.inport1");
    createOutport("image.outport");
}

Compositer::~Compositer() {
    delete compositingMode_;
}

const std::string Compositer::getProcessorInfo() const {
    return "Composites two images with a selectable blending method.";
}

void Compositer::process(LocalPortMapping* portMapping) {
    int source0 = portMapping->getTarget("image.inport0");
    int source1 = portMapping->getTarget("image.inport1");
    int dest = portMapping->getTarget("image.outport");

    tc_->setActiveTarget(dest, "Compositer::outport");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind shading results
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit_));
    glBindTexture(tc_->getGLTexTarget(source0), tc_->getGLTexID(source0));
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit1_));
    glBindTexture(tc_->getGLTexTarget(source1), tc_->getGLTexID(source1));
    // bind depth results
    glActiveTexture(tm_.getGLTexUnit(depthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(source0), tc_->getGLDepthTexID(source0));
    glActiveTexture(tm_.getGLTexUnit(depthTexUnit1_));
    glBindTexture(tc_->getGLDepthTexTarget(source1), tc_->getGLDepthTexID(source1));

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex0_", tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex0_", tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("shadeTex1_", tm_.getTexUnit(shadeTexUnit1_));
    program_->setUniform("depthTex1_", tm_.getTexUnit(depthTexUnit1_));
    program_->setUniform("compositingMode_", compositingMode_->get());
    program_->setUniform("blendFactor_", blendFactor_.get());

    renderQuad();

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

} // voreen namespace
