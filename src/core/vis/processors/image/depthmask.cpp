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

#include "voreen/core/vis/processors/image/depthmask.h"

namespace voreen {

DepthMask::DepthMask()
    : ImageProcessor("pp_depthmask"),
    lambda_("set.depthMaskLambda", "Lambda", 5.0f, 0.0f, 10.0f)
{
    setName("Depth Masking");
    addProperty(&lambda_);

    createInport("image.inport");
    createOutport("image.outport");
}

const std::string DepthMask::getProcessorInfo() const {
    return "Performs unsharp masking the depth buffer as presented by Luft et al. in 2006.";
}

void DepthMask::process(LocalPortMapping* portMapping) {
    int source = portMapping->getTarget("image.inport");
    int dest = portMapping->getTarget("image.outport");

    tc_->setActiveTarget(dest, "DepthMask::process");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    analyzeDepthBuffer(source);

    // bind shading result from previous ray cast
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit_));
    glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));
    LGL_ERROR;

    // bind depth result from previous ray cast
    glActiveTexture(tm_.getGLTexUnit(depthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
    LGL_ERROR;

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex_", tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("lambda_", lambda_.get());
    program_->setUniform("minDepth_", minDepth_.get());
    program_->setUniform("maxDepth_", maxDepth_.get());

    renderQuad();

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

void DepthMask::setLambda(float lambda) {
    lambda_.set(lambda);
}

} // voreen namespace
