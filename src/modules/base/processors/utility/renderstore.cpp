/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/modules/base/processors/utility/renderstore.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

const std::string RenderStore::loggerCat_("voreen.RenderStore");

RenderStore::RenderStore()
    : RenderProcessor()
    , inport_(Port::INPORT, "image.input")
    , privatePort_(Port::OUTPORT, "image.stored", true)
    , outport_(Port::OUTPORT, "image.output")
    , shaderPrg_(0)
{

    addPort(inport_);
    addPrivateRenderPort(&privatePort_);
    addPort(outport_);
}

Processor* RenderStore::create() const {
    return new RenderStore();
}

std::string RenderStore::getProcessorInfo() const {
    return "A RenderStore stores a copy of its input image and makes it available to other processors via a render outport. "
           "It provides the rendering that had been assigned to its inport during the previous rendering pass. Its "
           "outport may be connected to a predecessing processor, thereby allowing it to access a previous rendering result. "
           ""
           "<p>Furthermore, the input image is stored in a persistent render target and can therefore be accessed at any time, "
           "not necessarily during a rendering pass.</p>";
}

bool RenderStore::isReady() const {
    return (inport_.isReady() && privatePort_.hasData());
}

void RenderStore::process() {

    tgtAssert(inport_.isReady(), "Inport not ready");
    tgtAssert(privatePort_.hasData(), "Private port has no data");

    //
    // render private port to outport, if connected
    //
    if (outport_.isConnected()) {

        // put out rendering currently stored in private port, if available
        // otherwise, pass-through input rendering
        RenderPort& storePort = privatePort_.hasValidResult() ? privatePort_ : inport_;

        // adjust size
        if (outport_.getSize() != storePort.getSize()) {
            outport_.resize(storePort.getSize());
            LGL_ERROR;
        }

        outport_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        LGL_ERROR;

        TextureUnit storeUnit, storeUnitDepth;
        storePort.bindTextures(storeUnit.getEnum(), storeUnitDepth.getEnum());
        LGL_ERROR;

        shaderPrg_->activate();
        setGlobalShaderParameters(shaderPrg_, 0, storePort.getSize());
        storePort.setTextureParameters(shaderPrg_, "texParams_");
        shaderPrg_->setUniform("colorTex_", storeUnit.getUnitNumber());
        shaderPrg_->setUniform("depthTex_", storeUnitDepth.getUnitNumber());
        LGL_ERROR;

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        glDepthFunc(GL_LESS);
        shaderPrg_->deactivate();
        outport_.deactivateTarget();
        LGL_ERROR;
    }


    //
    // render inport to private port
    //
    if (inport_.getSize() != privatePort_.getSize()) {
        privatePort_.resize(inport_.getSize());
        LGL_ERROR;
    }

    privatePort_.activateTarget("RenderStore::process() image.private");
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    TextureUnit storeUnit, storeUnitDepth;
    inport_.bindTextures(storeUnit.getEnum(), storeUnitDepth.getEnum());
    LGL_ERROR;

    shaderPrg_->activate();
    setGlobalShaderParameters(shaderPrg_, 0, inport_.getSize());
    inport_.setTextureParameters(shaderPrg_, "texParams_");
    shaderPrg_->setUniform("colorTex_", storeUnit.getUnitNumber());
    shaderPrg_->setUniform("depthTex_", storeUnitDepth.getUnitNumber());
    LGL_ERROR;

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);
    shaderPrg_->deactivate();
    privatePort_.deactivateTarget();
    LGL_ERROR;
}

void RenderStore::initialize() throw (VoreenException) {
    RenderProcessor::initialize();

    shaderPrg_ = ShdrMgr.loadSeparate("passthrough.vert", "copyimage.frag",
        generateHeader(), false, false);

    if (!shaderPrg_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }
    initialized_ = true;
}

void RenderStore::deinitialize() throw (VoreenException) {
    ShdrMgr.dispose(shaderPrg_);
    shaderPrg_ = 0;

    RenderProcessor::deinitialize();
}

tgt::vec4 RenderStore::getStoredTargetPixel(const tgt::ivec2 &pos) {

    if (!privatePort_.hasValidResult()) {
        LWARNING("Private port has no valid result");
        return tgt::vec4(-1.f);
    }
    else {
        return privatePort_.getData()->getColorAtPos(pos);
    }
}

void RenderStore::portResized(RenderPort* /*p*/, tgt::ivec2 /*newsize*/) {
    // nothing
}

void RenderStore::sizeOriginChanged(RenderPort* p) {
    if (!p->getSizeOrigin()) {
        const std::vector<Port*> outports = getOutports();
        for (size_t i=0; i<outports.size(); ++i) {
            RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
            if (rp) {
                if (rp->getSizeOrigin())
                    return;
            }
        }
    }

    // do not propagate to inport
    /*const std::vector<Port*> inports = getInports();
    for (size_t i=0; i<inports.size(); ++i) {
        RenderPort* rp = dynamic_cast<RenderPort*>(inports[i]);
        if (rp)
            rp->sizeOriginChanged(p->getSizeOrigin());
    } */
}

bool RenderStore::testSizeOrigin(const RenderPort* p, void* so) const {
    tgtAssert(p->isOutport(), "testSizeOrigin used with inport");

    if (so) {
        const std::vector<Port*> outports = getOutports();
        for (size_t i=0; i<outports.size(); ++i) {
            if(p == outports[i])
                continue;
            RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
            if (rp) {
                if (rp->getSizeOrigin() && (rp->getSizeOrigin() != so)) {
                    return false;
                }
            }
        }
    }

    // do not test inport
    /*const std::vector<Port*> inports = getInports();
    for (size_t i=0; i<inports.size(); ++i) {
        RenderPort* rp = dynamic_cast<RenderPort*>(inports[i]);
        if (rp) {
            if (rp->getSizeOrigin() && (rp->getSizeOrigin() != so) ) {
                testSizeOriginVisited_ = false;
                return false;
            }

            const std::vector<Port*>& connectedOutports = inports[i]->getConnected();
            for (size_t j=0; j<connectedOutports.size(); ++j) {
                RenderPort* op = static_cast<RenderPort*>(connectedOutports[j]);

                if (!static_cast<RenderProcessor*>(op->getProcessor())->testSizeOrigin(op, so)) {
                    testSizeOriginVisited_ = false;
                    return false;
                }
            }
        }
    } */

    return true;
}

} // namespace voreen
