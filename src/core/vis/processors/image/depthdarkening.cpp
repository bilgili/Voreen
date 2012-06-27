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

#include "voreen/core/vis/processors/image/depthdarkening.h"

namespace voreen {

DepthDarkening::DepthDarkening()
    : ImageProcessorDepth("pp_depthdarkening")
    , sigma_("sigma", "Sigma", 2.0f, 0.1f, 10.0f)
    , lambda_("lambda", "Lambda", 10.0f, 0.0f, 20.0f)
    , inport_(Port::INPORT, "image.inport")
    , outport_(Port::OUTPORT, "image.outport", true)
{
    addProperty(sigma_);
    addProperty(lambda_);

    addPort(inport_);
    addPort(outport_);
}

const std::string DepthDarkening::getProcessorInfo() const {
    return "Performs depth darkening as presented by Luft et al. at Siggraph 2006. The technique is based on an unsharping mask applied to the depth buffer. The kernel size of the used Gaussian filter is set to round(2.5*sigma). Lambda controls the strength of the effect.";
}

Processor* DepthDarkening::create() const {
    return new DepthDarkening();
}

void DepthDarkening::process() {
    outport_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    analyzeDepthBuffer(&inport_);

    inport_.bindTextures(tm_.getGLTexUnit(shadeTexUnit_), tm_.getGLTexUnit(depthTexUnit_));

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex_", tm_.getTexUnit(depthTexUnit_));
    inport_.setTextureParameters(program_, "texParams_");
    program_->setUniform("sigma_", sigma_.get());
    program_->setUniform("lambda_", lambda_.get());
    program_->setUniform("minDepth_", minDepth_.get());
    program_->setUniform("maxDepth_", maxDepth_.get());

    renderQuad();

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

} // voreen namespace
