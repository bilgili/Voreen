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
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/processors/portmapping.h"


namespace voreen {

CoarsenessRenderer::CoarsenessRenderer(tgt::Camera* camera, TextureContainer* tc)
  : CopyToScreenRenderer(camera, tc)
{
	createInport("image.inport");
	createOutport("image.outport");
}

CoarsenessRenderer::~CoarsenessRenderer() {
}

const std::string CoarsenessRenderer::getProcessorInfo() const {
	return "Renders in coarseness mode to ensure interactive data set handling";
}

void CoarsenessRenderer::process(LocalPortMapping*  portMapping) {
	glViewport(0,0,static_cast<GLsizei>(size_.x),static_cast<GLsizei>(size_.y));
	
	if (useCoarseness_.get() && !ignoreCoarseness_) {
        CoarsenessStruct* cs = new CoarsenessStruct();
		cs->coarsenessFactor = static_cast<float>(coarsenessFactor_.get());
		cs->processor = this;
		CoarsenessMsg* msg = new CoarsenessMsg(NetworkEvaluator::setSizeBackward_,cs);
		MsgDistr.postMessage(msg,"evaluator");
	} 

	int source=portMapping->getTarget("image.inport");
	int dest = portMapping->getTarget("image.outport");

    tc_->setActiveTarget(dest,"CoarsenessRenderer::image.outport");
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	if (source != -1) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));
		if (useCoarseness_.get() && !ignoreCoarseness_)
			glTexParameteri(tc_->getGLTexTarget(source), GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		raycastPrg_->activate();
		setGlobalShaderParameters(raycastPrg_);
		raycastPrg_->setUniform("shadeTex_", 0);
		raycastPrg_->setUniform("depthTex_", 1);

		if (useCoarseness_.get() && !ignoreCoarseness_)
			raycastPrg_->setUniform("interactionCoarseness_", coarsenessFactor_.get());
		else
			raycastPrg_->setUniform("interactionCoarseness_", 1);

		glDepthFunc(GL_ALWAYS);
		renderQuad();
		glDepthFunc(GL_LESS);
		raycastPrg_->deactivate();

		if (useCoarseness_.get() && !ignoreCoarseness_) {
			glTexParameteri(tc_->getGLTexTarget(source), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glViewport(0, 0, static_cast<int>(size_.x), static_cast<int>(size_.y));
		}
	}
}

} // namespace voreen
