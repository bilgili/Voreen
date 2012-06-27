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

#include "voreen/core/vis/processors/image/glow.h"

namespace voreen {

const Identifier Glow::setGlowColor("set.glowColor");
const Identifier Glow::setGlowRange("set.glowRange");

Glow::Glow()
    : ImageProcessor("pp_glow"),
    glowColor_(setGlowColor, "Glow Color", tgt::vec4(1.0f, 1.0f, 0.0f, 1.0f)),
    glowRange_(setGlowRange, "Glow Range", 30)
{
    setName("Glow Rendering");
    addProperty(&glowRange_);
    addProperty(&glowColor_);

    createInport("image.inport");
    createOutport("image.outport");
    createPrivatePort("image.tmp");
}

const std::string Glow::getProcessorInfo() const {
    return "Performs a glow.";
}

void Glow::renderInternalPass(int source, int pass) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind shading and depth result from previous ray cast
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit_));
    glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));
    glActiveTexture(tm_.getGLTexUnit(depthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
    LGL_ERROR;

    // get background color
    tgt::vec4 backColor;
    glGetFloatv(GL_COLOR_CLEAR_VALUE, backColor.elem);

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex_", tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("pass_", pass);
    program_->setUniform("color_", glowColor_.get());
    program_->setUniform("range_", static_cast<GLfloat>(glowRange_.get()));
    program_->setUniform("backgroundColor_", backColor);

    renderQuad();

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

void Glow::process(LocalPortMapping* portMapping) {
    int source = portMapping->getTarget("image.inport");
    int dest = portMapping->getTarget("image.outport");
    int tmp = portMapping->getTarget("image.tmp");

    if (source == -1) {
        tc_->setActiveTarget(dest, "Glow::render (final)");
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    glDisable(GL_DEPTH_TEST);
    // first rendering pass
    tc_->setActiveTarget(tmp, "Glow::process (intermediate)");
    renderInternalPass(source, 1);

    // second rendering pass
    source = dest;
    tc_->setActiveTarget(dest, "Glow::render (final)");
    renderInternalPass(source, 2);

    glEnable(GL_DEPTH_TEST);
}

} // voreen namespace
