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

const std::string RenderStore::storeTexUnit_        = "storeTexUnit";
const std::string RenderStore::storeDepthTexUnit_   = "storeDepthTexUnit";

RenderStore::RenderStore()
    : RenderProcessor()
    , inport_(Port::INPORT, "image.input")
    , privatePort_(Port::OUTPORT, "image.private", true)
    , cpPort_(Port::OUTPORT, "coprocessor.renderstore", false)
{

    addPort(inport_);
    addPrivateRenderPort(&privatePort_);
    addPort(cpPort_);

    // set texture unit identifiers and register
    std::vector<std::string> units;
    units.push_back(storeTexUnit_);
    units.push_back(storeDepthTexUnit_);
    tm_.registerUnits(units);

}

RenderStore::~RenderStore() {
}

Processor* RenderStore::create() const {
    return new RenderStore();
}

const std::string RenderStore::getProcessorInfo() const {
    return "A RenderStore stores a copy of its input image and makes it available to other processors via a coprocessor outport.\
           The image is stored in a persistent render target and can therefore be accessed at any time, not necessarily\
           within a rendering pass.";
}

void RenderStore::process() {

    LGL_ERROR;

    if (!privatePort_.isReady()) {
        LWARNING("Private port not ready");
        return;
    }

    privatePort_.activateTarget("RenderStore::process() image.private");
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    if (!inport_.isReady()) {
        privatePort_.deactivateTarget();
        return;
    }

    LGL_ERROR;
    inport_.bindTextures(tm_.getGLTexUnit(storeTexUnit_), tm_.getGLTexUnit(storeDepthTexUnit_));
    LGL_ERROR;

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

    privatePort_.deactivateTarget();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

bool RenderStore::isEndProcessor() const {
    // By definition, the RenderStore is an EndProcessor, i.e. no processors
    // have to render before it. It is important to flag this, as the RenderStore has a co-processor outport,
    // which may otherwise lead to problems during the network analysis / evaluation.
    return true;
}

void RenderStore::initialize() throw (VoreenException) {

    RenderProcessor::initialize();

    shaderPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "vrn_interactionmode.frag",
        generateHeader(), false, false);

    if(!shaderPrg_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }
    initialized_ = true;
}

tgt::vec4 RenderStore::getStoredTargetPixel(const tgt::ivec2 &pos) {

    if (!privatePort_.isReady()) {
        LWARNING("Private port not ready");
        return tgt::vec4(-1.f);
    }
    else {
        return privatePort_.getData()->getColorAtPos(pos);
    }
}

} // namespace voreen
