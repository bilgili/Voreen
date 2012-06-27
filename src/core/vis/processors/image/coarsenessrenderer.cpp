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

#include "voreen/core/vis/processors/image/coarsenessrenderer.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/processors/portmapping.h"

#include "tgt/shadermanager.h"

namespace voreen {

const Identifier CoarsenessRenderer::setCoarseness_("set.Coarseness");

CoarsenessRenderer::CoarsenessRenderer()
    : Processor()
    , useCoarseness_(false)
    , coarsenessFactor_(setCoarseness_, "Interaction Coarseness", 4, 1, 20)
    , raycastPrg_(0)
{
    setName("CoarsenessRenderer");
	createInport("image.inport");
	createOutport("image.outport");
    addProperty(&coarsenessFactor_);
}

CoarsenessRenderer::~CoarsenessRenderer() {
    if (raycastPrg_)
        ShdrMgr.dispose(raycastPrg_);
}

const std::string CoarsenessRenderer::getProcessorInfo() const {
	return "Renders in coarseness mode to ensure interactive data set handling";
}

const Identifier CoarsenessRenderer::getClassName() const {
    return "Miscellaneous.CoarsenessRenderer";
}

Processor* CoarsenessRenderer::create() {
    return new CoarsenessRenderer();
}

int CoarsenessRenderer::initializeGL() {
    initStatus_ = Processor::initializeGL();
    if (initStatus_ != VRN_OK)
        return initStatus_;

    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "vrn_interactionmode.frag",
        generateHeader(), false);

    if (!raycastPrg_)
        initStatus_ = VRN_ERROR;

    return initStatus_;
}

void CoarsenessRenderer::setUseCoarseness(bool use) {
    useCoarseness_ = use;
}

bool CoarsenessRenderer::getUseCoarseness() {
    return useCoarseness_;
}

int CoarsenessRenderer::getCoarsenessFactor() {
    return coarsenessFactor_.get();
}

void CoarsenessRenderer::process(LocalPortMapping* portMapping) {
    LGL_ERROR;
	int source = portMapping->getTarget("image.inport");
	int dest   = portMapping->getTarget("image.outport");

    tc_->setActiveTarget(dest, "CoarsenessRenderer::image.outport");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (source != -1) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));
		if (useCoarseness_)
			glTexParameteri(tc_->getGLTexTarget(source), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // clamp to edge
		glTexParameteri(tc_->getGLTexTarget(source), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(tc_->getGLTexTarget(source), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

		raycastPrg_->activate();
		setGlobalShaderParameters(raycastPrg_);
		raycastPrg_->setUniform("shadeTex_", 0);
		raycastPrg_->setUniform("depthTex_", 1);

        if (useCoarseness_)
			raycastPrg_->setUniform("interactionCoarseness_", coarsenessFactor_.get());
        else
			raycastPrg_->setUniform("interactionCoarseness_", 1);

		glDepthFunc(GL_ALWAYS);
		renderQuad();
		glDepthFunc(GL_LESS);
		raycastPrg_->deactivate();

		if (useCoarseness_)
			glTexParameteri(tc_->getGLTexTarget(source), GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	}
    LGL_ERROR;
}

void CoarsenessRenderer::processMessage(Message* msg, const Identifier& dest) {
    Processor::processMessage(msg, dest);
    if (msg->id_ == setCoarseness_)
        coarsenessFactor_.set(msg->getValue<int>());
}

} // namespace voreen
