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

#include "voreen/core/vis/processors/image/cacherenderer.h"

namespace voreen {

CacheRenderer::CacheRenderer()
    : RenderProcessor()
{
    setName("CacheRenderer");

    createInport("image.input");
    createOutport("image.output");
}

CacheRenderer::~CacheRenderer() {

        ShdrMgr.dispose(raycastPrg_);
}

const std::string CacheRenderer::getProcessorInfo() const {
    return "A CacheRenderer is a processor that caches the rendering image on its inport. \
            It will use the cached image as long as any parameter of a predecessing processor \
            is changed.";
}

const Identifier CacheRenderer::getClassName() const {
    return "Miscellaneous.CacheRenderer";
}

Processor* CacheRenderer::create() const {
    return new CacheRenderer();
}

void CacheRenderer::process(LocalPortMapping* portMapping) {
    LGL_ERROR;
    // render result
    int source= portMapping->getTarget("image.input");
    int dest = portMapping->getTarget("image.output");

    tc_->setActiveTarget(dest,"CacheRenderer::process()");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (source != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));
        raycastPrg_->activate();
        setGlobalShaderParameters(raycastPrg_);
        raycastPrg_->setUniform("shadeTex_", 0);
        raycastPrg_->setUniform("depthTex_", 1);

        raycastPrg_->setUniform("interactionCoarseness_", 1);

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        glDepthFunc(GL_LESS);
        raycastPrg_->deactivate();

    }
    LGL_ERROR;
}

int CacheRenderer::initializeGL() {
    initStatus_ = Processor::initializeGL();
    if (initStatus_ != VRN_OK)
        return initStatus_;

    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "vrn_interactionmode.frag",
        generateHeader(), false);

    if (!raycastPrg_)
        initStatus_ = VRN_ERROR;

    return initStatus_;
}

} // namespace voreen
