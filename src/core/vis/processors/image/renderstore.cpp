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

#include "voreen/core/vis/processors/image/renderstore.h"

namespace voreen {

const Identifier RenderStore::storeTexUnit_        = "storeTexUnit";
const Identifier RenderStore::storeDepthTexUnit_   = "storeDepthTexUnit";

RenderStore::RenderStore()
    : Processor()
    , privateTargetID_(-1)
{
    setName("RenderStore");
    createInport("image.input");
    createPrivatePort("image.private");
    createCoProcessorOutport("coprocessor.renderstore", &Processor::call);

    // set texture unit identifiers and register
    std::vector<Identifier> units;
    units.push_back(storeTexUnit_);
    units.push_back(storeDepthTexUnit_);
    tm_.registerUnits(units);

}

RenderStore::~RenderStore() {
}

const Identifier RenderStore::getClassName() const {
    return "Miscellaneous.RenderStore";
}

Processor* RenderStore::create() const {
    return new RenderStore();
}

const std::string RenderStore::getProcessorInfo() const {
    return "A RenderStore stores a copy of its input image and makes it available to other processors via a coprocessor outport.\
           The image is stored in a persistent render target and can therefore be accessed at any time, not necessarily\
           within a rendering pass.";
}

void RenderStore::process(LocalPortMapping* portMapping) {

    LGL_ERROR;
    // render result
    int source = portMapping->getTarget("image.input");
    privateTargetID_ = portMapping->getTarget("image.private");

    tc_->setActiveTarget(privateTargetID_,"RenderStore::process() image.private");
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if (source != -1) {

        LGL_ERROR;
        glActiveTexture(tm_.getGLTexUnit(storeTexUnit_));
        glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));
        glActiveTexture(tm_.getGLTexUnit(storeDepthTexUnit_));
        glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
        shaderPrg_->activate();
        setGlobalShaderParameters(shaderPrg_);
        shaderPrg_->setUniform("shadeTex_", tm_.getTexUnit(storeTexUnit_));
        shaderPrg_->setUniform("depthTex_", tm_.getTexUnit(storeDepthTexUnit_));
        shaderPrg_->setUniform("interactionCoarseness_", 1);

        LGL_ERROR;

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        glDepthFunc(GL_LESS);

        shaderPrg_->deactivate();
    }

    tc_->setActiveTarget(tc_->getFinalTarget(), "RenderStore::process()");
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

Message* RenderStore::call(Identifier ident, LocalPortMapping* /*portMapping*/) {
    if (ident == "getStoredRenderTarget") {
        return new IntMsg("renderstore.storedRenderTarget", getStoredTargetID());
    }
    else {
        return 0;
    }
}

bool RenderStore::isEndProcessor() const {
    // By definition, the RenderStore is an EndProcessor, i.e. no processors
    // have to render before it. It is important to flag this, as the RenderStore has a co-processor outport,
    // which may otherwise lead to problems during the network analysis / evaluation.
    return true;
}

int RenderStore::initializeGL() {

    initStatus_ = Processor::initializeGL();
    if (initStatus_ != VRN_OK)
        return initStatus_;

    shaderPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "vrn_interactionmode.frag",
        generateHeader(), false);

    if (!shaderPrg_)
        initStatus_ = VRN_ERROR;

    return initStatus_;
}

tgt::vec4 RenderStore::getStoredTargetPixel(const tgt::ivec2 &pos) const {
    int storeTarget = getStoredTargetID();
    if (storeTarget < 0)
        return tgt::vec4(-1.f);
    else {
        float* f = tc_->getTargetAsFloats(storeTarget, pos.x, pos.y);
        return tgt::vec4(f);
    }
}

} // namespace voreen
