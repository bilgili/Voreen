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

#include "voreen/core/vis/processors/image/canvasrenderer.h"

namespace voreen {

CanvasRenderer::CanvasRenderer()
    : Processor()
    , raycastPrg_(0)
    , useCaching_("set.useCaching", "Use caching?", true)
    , imageID_(-1)
{
    setName("CanvasRenderer");
    createInport("image.input");

    addProperty(&useCaching_);
}

CanvasRenderer::~CanvasRenderer() {
    if (raycastPrg_)
        ShdrMgr.dispose(raycastPrg_);
}

const std::string CanvasRenderer::getProcessorInfo() const {
    return "A CanvasRenderer is the last processor in a network. Its only purpose is to copy \
            its input to the finaltarget of texture container. Additionally the CanvasRenderer \
            is able to cache the rendering result, if no parameter in any processor in the network \
            has been changed since last rendering.";
}

const Identifier CanvasRenderer::getClassName() const {
    return "Miscellaneous.Canvas";
}

Processor* CanvasRenderer::create() const {
    return new CanvasRenderer();
}

void CanvasRenderer::process(LocalPortMapping* portMapping) {
    int source = portMapping->getTarget("image.input");
    imageID_ = source;
    int dest = tc_->getFinalTarget();

    tc_->setActiveTarget(dest, "CanvasRenderer::process()");
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
}

int CanvasRenderer::initializeGL() {
    initStatus_ = Processor::initializeGL();
    if (initStatus_ != VRN_OK)
        return initStatus_;

    raycastPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "vrn_interactionmode.frag",
        generateHeader(), false);

    if (!raycastPrg_)
        initStatus_ = VRN_ERROR;

    return initStatus_;
}

bool CanvasRenderer::isEndProcessor() const {
    return true;
}

bool CanvasRenderer::usesCaching() const {
    return useCaching_.get();
}

} // namespace voreen
