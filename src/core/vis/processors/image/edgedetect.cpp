/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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
#include "voreen/core/vis/processors/portmapping.h"
#include "voreen/core/vis/voreenpainter.h"

namespace voreen {

EdgeDetect::EdgeDetect()
    : GenericFragment("pp_edgedetect"),
      edgeThreshold_("set.edgeThreshold", "Edge threshold", 0.04f, 0.001f, 1.f, true),
      showImage_("set.showImage", "Show image", 1, 0, 1, true),
      edgeColor_("set.edgeColor", "Edge color", tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)),
      fillColor_("set.fillColor", "Fill color", tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f)),
      coarsnessOn_(false),
      labelMode_("set.labelMode", "Use label mode", false)
{
    setName("Edge detection");

    backgroundColor_.set(tgt::vec4(0.0f,0.0f,0.0f,1.0f));

    addProperty(&edgeColor_);
	addProperty(&backgroundColor_);
    addProperty(&fillColor_);
    addProperty(&edgeThreshold_);
    addProperty(&showImage_);
    blendModes_.push_back("Replace");
    blendModes_.push_back("Pseudo chromadepth");
    blendModes_.push_back("Blend");
    blendMode_ = new EnumProp("set.blendMode", "Blend mode:", blendModes_, &needRecompileShader_, 0, false);
	addProperty(blendMode_);
    edgeStyles_.push_back("Contour");
    edgeStyles_.push_back("Silhouette");
    edgeStyles_.push_back("Contour (depth-based thickness)");
    edgeStyle_ = new EnumProp("set.edgeStyle", "Edge style:", edgeStyles_, &needRecompileShader_, 0, false);
	addProperty(edgeStyle_);
    addProperty(&labelMode_);

	createInport("image.input");
	createOutport("image.output");
}

EdgeDetect::~EdgeDetect() {
    delete blendMode_;
    delete edgeStyle_;
}

const std::string EdgeDetect::getProcessorInfo() const {
	return "Performs an edge detection. The detected edge is then drawn in selectable colors, styles, modi of blending etc.";
}

void EdgeDetect::process(LocalPortMapping*  portMapping) {

	int source;
    int dest;
    if (labelMode_.get()){
        if (coarsnessOn_)
            return;
		source = portMapping->getTarget("image.input");
	    if (source == -1)
            return;
        dest = source;
        tc_->setActiveTarget(dest, "EdgeDetect::process");
        glDisable(GL_DEPTH_TEST);
        glColorMask(false, false, false, true);
    } else {
        source = portMapping->getTarget("image.input");
	    if (source == -1)
            return;
        dest = portMapping->getTarget("image.output");
        tc_->setActiveTarget(dest, "EdgeDetect::process");
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

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
    program_->setUniform("shadeTex_", (GLint) tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("minDepth_", minDepth_.get());
    program_->setUniform("maxDepth_", maxDepth_.get());
    program_->setUniform("edgeColor_", edgeColor_.get());
    program_->setUniform("fillColor_", fillColor_.get());
    program_->setUniform("depthTex_", (GLint) tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("backgroundColor_", backgroundColor_.get());
    program_->setUniform("edgeThreshold_", edgeThreshold_.get());
    if (labelMode_.get()){
        program_->setUniform("showImage_", 2);
        program_->setUniform("blendMode_", 3);
    } else {
        program_->setUniform("showImage_", showImage_.get());
        program_->setUniform("blendMode_", blendMode_->get());
    }
	program_->setUniform("edgeStyle_", edgeStyle_->get());

    renderQuad();

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;

    if (labelMode_.get()){
        glEnable(GL_DEPTH_TEST);
        glColorMask(true, true, true, true);
    }
}

void EdgeDetect::processMessage(Message* msg, const Identifier& dest/*=Message::all_*/)
{
	GenericFragment::processMessage(msg, dest);
	if (msg->id_ == VoreenPainter::switchCoarseness_) {
        coarsnessOn_ = msg->getValue<bool>();
    }
}


} // voreen namespace

