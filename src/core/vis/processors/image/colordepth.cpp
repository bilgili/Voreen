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

#include "voreen/core/vis/processors/image/colordepth.h"
#include "voreen/core/application.h"

#include "tgt/texturemanager.h"

namespace voreen {

const Identifier ColorDepth::chromadepthTexUnit_ = "chromadepthTexUnit";

ColorDepth::ColorDepth()
    : ImageProcessorDepth("pp_colordepth"),
      chromaDepthTex_(0),
      factor_("set.colordepth.factor", "Factor", 1.0f, 0.0f, 10.0f, false)
{
    setName("Color Depth");

    tm_.addTexUnit(chromadepthTexUnit_);

    colorModes_.push_back("Light-dark (replace)");
    colorModes_.push_back("Light-dark (modulate)");
    colorModes_.push_back("Chromadepth");
    colorModes_.push_back("Pseudo chromadepth");
    colorMode_ = new EnumProp("set.colordepth.mode", "Choose mode:", colorModes_, 1, true, true);
    addProperty(colorMode_);

    addProperty(&factor_);

    createInport("image.inport");
    createOutport("image.outport");
}

ColorDepth::~ColorDepth() {
    delete colorMode_;
    TexMgr.dispose(chromaDepthTex_);
}

const std::string ColorDepth::getProcessorInfo() const {
    return "Performs a color filtering which encodes depth information";
}

int ColorDepth::initializeGL() {
    chromaDepthTex_ = TexMgr.load(VoreenApplication::app()->getTransFuncPath("chromadepthspectrum.bmp"));
    return ImageProcessor::initializeGL();
}

void ColorDepth::process(LocalPortMapping* portMapping) {
    int source = portMapping->getTarget("image.inport");
    int dest = portMapping->getTarget("image.outport");

    analyzeDepthBuffer(source);

    tc_->setActiveTarget(dest, "ColorDepth::process");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind shading result from previous ray cast
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit_));
    glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));
    LGL_ERROR;

    // bind depth result from previous ray cast
    glActiveTexture(tm_.getGLTexUnit(depthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
    LGL_ERROR;

    // bind chroma depth texture
    glActiveTexture(tm_.getGLTexUnit(chromadepthTexUnit_));
    chromaDepthTex_->bind();
    LGL_ERROR;

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex_", tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("chromadepthTex_", tm_.getTexUnit(chromadepthTexUnit_));
    program_->setUniform("minDepth_", minDepth_.get());
    program_->setUniform("maxDepth_", maxDepth_.get());
    program_->setUniform("colorMode_", colorMode_->get());
    program_->setUniform("colorDepthFactor_", factor_.get());

    renderQuad();

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

} // voreen namespace
