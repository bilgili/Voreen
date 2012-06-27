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

#include "voreen/core/vis/processors/image/canvasrenderer.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/opengl/texturecontainer.h"

namespace voreen {

CanvasRenderer::CanvasRenderer() : CopyToScreenRenderer()
{
	createInport("image.input");
}

CanvasRenderer::~CanvasRenderer() {
}

const std::string CanvasRenderer::getProcessorInfo() const {
	return "A Canvas is the last processor in a network. Its only purpose is to copy its input to the finaltarget of texture container. It inherits from CopyToScreenRenderer because of the coarseness properties they both share.";
}

void CanvasRenderer::process(LocalPortMapping*  portMapping)
{
	if (useCoarseness_.get() && !ignoreCoarseness_)
    {
		CoarsenessStruct* cs = new CoarsenessStruct();
		cs->coarsenessFactor = (float) coarsenessFactor_.get();
		cs->processor = this;
		CoarsenessMsg* msg = new CoarsenessMsg(NetworkEvaluator::setSizeBackward_,cs);
		MsgDistr.postMessage(msg,"evaluator");
	} 
  
	int source = portMapping->getTarget("image.input");
	int dest = tc_->getFinalTarget();

    tc_->setActiveTarget(dest, "CanvasRenderer::process() dest");
		glViewport(0,0,(int)size_.x,(int)size_.y);

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
			glViewport(0, 0, (int)size_.x, (int)size_.y);
		}

	}
}

//---------------------------------------------------------------------------

CacheRenderer::CacheRenderer() : CopyToScreenRenderer()
{
	props_.clear();
	
	createInport("image.input");
	createOutport("image.output");
}

CacheRenderer::~CacheRenderer() {
}

const std::string CacheRenderer::getProcessorInfo() const {
	return "A CacheRenderer is the last processor in a network. Its only purpose is to copy its input to the finaltarget of texture container. It inherits from CopyToScreenRenderer because of the coarseness properties they both share.";
}

void CacheRenderer::process(LocalPortMapping*  portMapping)
{
    LGL_ERROR;
	glViewport(0,0,(int)size_.x,(int)size_.y);
  

//  
//    // look for cached image
//    int cacheRT = -1;
//    if (!stereo)
//        if (cachedImage_ > -1){
//			cacheRT = cachedImage_;
//        }
//	else { // FIXME reenable stereo support
////         if (camera_->getEye() == tgt::Camera::EYE_LEFT)
////             if (cachedStereoImageLeft_ > -1)
//// 				cacheRT = cachedStereoImageLeft_;
//// 		else if (camera_->getEye() == tgt::Camera::EYE_RIGHT)
////             if (cachedStereoImageRight_ > -1)
//// 				cacheRT = cachedStereoImageRight_;
//	}
//
//    // if cached image found, use this as source.
//    // otherwise let producer perform a rendering pass.
//    if (cacheRT == -1)
//    {
//        tc_->releaseTarget(cacheRT);
//
//        if (useCoarseness_.get() && !ignoreCoarseness_) {
//            // apply coarseness
//            producer_->setSize(tgt::ivec2((int)size_.x/coarsenessFactor_.get(), (int)size_.y/coarsenessFactor_.get()));
//            glViewport(0,0, (int)size_.x/coarsenessFactor_.get(), (int)size_.y/coarsenessFactor_.get());
//        }
//
//        LGL_ERROR;
//        producer_->render();
//        LGL_ERROR;
//
//        if (useCoarseness_.get() && !ignoreCoarseness_) {
//            // revoke coarseness
//            producer_->setSize(size_);
//            glViewport(0, 0, (int)size_.x, (int)size_.y);
//        }
//        source = tc_->findTarget(getTargetType(ttImage_));
//
//        if (caching_ && source != -1) {
//            // mark image for reuse
//            cachedImage_ = -1;
//            cachedStereoImageLeft_ = -1;
//            cachedStereoImageRight_ = -1;
//            if (!stereo) {
//                cachedImage_ = source;
//                tc_->changeType(cachedImage_, getTargetType(ttCachedImage_));
//                tc_->setPersistent(cachedImage_, true);
//            }
//            else {
////                 if(camera_->getEye() == tgt::Camera::EYE_LEFT) {
////                     cachedStereoImageLeft_ = source;
////                     tc_->changeType(cachedStereoImageLeft_, getTargetType(ttCachedImageStereoLeft_));
////                     tc_->setPersistent(cachedStereoImageLeft_, true);
////                 }
////                 else if (camera_->getEye() == tgt::Camera::EYE_RIGHT) {
////                     cachedStereoImageRight_ = source;
////                     tc_->changeType(cachedStereoImageRight_, getTargetType(ttCachedImageStereoRight_));
////                     tc_->setPersistent(cachedStereoImageRight_, true);
////                 }
//            }
//        }
//    }
//    else {
//        source = cacheRT;
//    }

    // render result
	int source= portMapping->getTarget("image.input");
    int dest = portMapping->getTarget("image.output");
    /*if (renderToScreen_)
        dest = tc_->getFinalTarget();
    else {
        dest = tc_->allocTarget(getTargetType(ttImage_));
    }
    tc_->setDebugLabel(dest, "CopyToScreenRenderer::render() dest");*/
    tc_->setActiveTarget(dest,"CacheRenderer::process() image.output");
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if (source != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));
        /*if (useCoarseness_.get() && !ignoreCoarseness_)
            glTexParameteri(tc_->getGLTexTarget(source), GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

        raycastPrg_->activate();
        setGlobalShaderParameters(raycastPrg_);
        raycastPrg_->setUniform("shadeTex_", 0);
        raycastPrg_->setUniform("depthTex_", 1);

        /*if (useCoarseness_.get() && !ignoreCoarseness_)
            raycastPrg_->setUniform("interactionCoarseness_", coarsenessFactor_.get());
        else*/
        raycastPrg_->setUniform("interactionCoarseness_", 1);

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        glDepthFunc(GL_LESS);
        raycastPrg_->deactivate();

        /*if (useCoarseness_.get() && !ignoreCoarseness_)
            glTexParameteri(tc_->getGLTexTarget(source), GL_TEXTURE_MAG_FILTER, GL_NEAREST);*/

        // if source does not contain a cached image, free target
        /*if (source != cachedImage_ && source != cachedStereoImageLeft_ && source != cachedStereoImageRight_)
            tc_->releaseTarget(source);*/
		MsgDistr.postMessage(new ProcessorPointerMsg(NetworkEvaluator::setCachedBackward_,this),"evaluator");
	}
    LGL_ERROR;
}

} // namespace voreen
