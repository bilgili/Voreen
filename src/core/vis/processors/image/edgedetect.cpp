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

#include "voreen/core/vis/processors/image/edgedetect.h"


namespace voreen {

EdgeDetect::EdgeDetect()
    : ImageProcessorDepth("pp_edgedetect"),
      edgeThreshold_("edgeThreshold", "Edge threshold", 0.04f, 0.001f, 1.f, true),
      showImage_("showImage", "Show image", true),
      blendMode_("blendMode", "Blend mode", Processor::INVALID_PROGRAM),
      edgeStyle_("edgeStyle", "Edge style", Processor::INVALID_PROGRAM),  
      edgeColor_("edgeColor", "Edge color", tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)),
      inport_(Port::INPORT, "image.inport"),
      outport_(Port::OUTPORT, "image.outport")
{

    backgroundColor_.set(tgt::vec4(0.0f,0.0f,0.0f,1.0f));

    edgeThreshold_.setNumDecimals(3);
    blendMode_.addOption("replace", "Replace", 0);
    blendMode_.addOption("pseudo-chromadepth", "Pseudo chromadepth", 1);
    blendMode_.addOption("blend", "Blend", 2);
    edgeStyle_.addOption("contour", "Contour", 0);
    edgeStyle_.addOption("silhouette", "Silhouette", 1);
    edgeStyle_.addOption("contour-depth-based", "Contour (depth-based thickness)", 2);
    addProperty(edgeStyle_);
    addProperty(edgeThreshold_);
    addProperty(blendMode_);
    addProperty(edgeColor_);
    addProperty(backgroundColor_);
    addProperty(showImage_);

    addPort(inport_);
    addPort(outport_);
}

EdgeDetect::~EdgeDetect() {
}

const std::string EdgeDetect::getProcessorInfo() const {
    return "Performs an edge detection based on the Sobel operator. The detected edge is then drawn in selectable colors, \
           styles, blending modi etc.";
}

void EdgeDetect::process() {
    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    analyzeDepthBuffer(&inport_);

    inport_.bindTextures(tm_.getGLTexUnit(shadeTexUnit_), tm_.getGLTexUnit(depthTexUnit_));
    LGL_ERROR;

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("minDepth_", minDepth_.get());
    program_->setUniform("maxDepth_", maxDepth_.get());
    program_->setUniform("edgeColor_", edgeColor_.get());
    program_->setUniform("depthTex_", tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("backgroundColor_", backgroundColor_.get());
    program_->setUniform("edgeThreshold_", edgeThreshold_.get());
    program_->setUniform("showImage_", showImage_.get());
    program_->setUniform("blendMode_", blendMode_.getValue());
    program_->setUniform("edgeStyle_", edgeStyle_.getValue());

    renderQuad();

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

} // voreen namespace
