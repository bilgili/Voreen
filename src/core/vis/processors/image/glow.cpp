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

const std::string Glow::setGlowColor("glowColor");
const std::string Glow::setGlowRange("glowRange");

Glow::Glow()
    : ImageProcessor("pp_glow")
    , glowColor_(setGlowColor, "Glow Color", tgt::vec4(1.0f, 1.0f, 0.0f, 1.0f))
    , glowRange_(setGlowRange, "Glow Range", 30)
    , inport_(Port::INPORT, "image.inport")
    , outport_(Port::OUTPORT, "image.outport", true)
    , tmpPort_(Port::OUTPORT, "image.tmp", true)
{
    addProperty(glowRange_);
    addProperty(glowColor_);

    addPort(inport_);
    addPort(outport_);
    addPort(tmpPort_);
}

const std::string Glow::getProcessorInfo() const {
    return "Performs a glow.";
}

Processor* Glow::create() const {
    return new Glow();}

void Glow::renderInternalPass(RenderPort* port, int pass) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind shading and depth result from previous ray cast
    port->bindTextures(tm_.getGLTexUnit(shadeTexUnit_), tm_.getGLTexUnit(depthTexUnit_));

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

void Glow::process() {
    outport_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //int source = getTarget("image.inport");
    //int dest = getTarget("image.outport");
    //int tmp = getTarget("image.tmp");

    //if (source == -1) {
    //    tc_->setActiveTarget(dest, "Glow::render (final)");
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //    return;
    //}

    glDisable(GL_DEPTH_TEST);
    // first rendering pass
    tmpPort_.activateTarget();
    renderInternalPass(&inport_, 1);
    //tc_->setActiveTarget(tmp, "Glow::process (intermediate)");
    //renderInternalPass(source, 1);

    // second rendering pass
    outport_.activateTarget();
    renderInternalPass(&inport_, 2);
    //source = dest;
    //tc_->setActiveTarget(dest, "Glow::render (final)");
    //renderInternalPass(source, 2);

    glEnable(GL_DEPTH_TEST);
}

} // voreen namespace
