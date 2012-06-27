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

#include "voreen/modules/base/processors/image/loopcompositor.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

LoopCompositor::LoopCompositor()
    : RenderProcessor()
    , finalPass_("finalPass", "Final pass", 3, 1, 100)
    , shaderPrg_(0)
    , inport0_(Port::INPORT, "image.inport0")
    , inport1_(Port::INPORT, "image.inport1")
    , outport_(Port::OUTPORT, "image.outport")
{
    addProperty(finalPass_);
    currentPass_ = 1;

    addPort(inport0_);
    addPort(inport1_);
    addPort(outport_);
}

LoopCompositor::~LoopCompositor() {
    ShdrMgr.dispose(shaderPrg_);
}

std::string LoopCompositor::getProcessorInfo() const {
    return "";
}

void LoopCompositor::initialize() throw (VoreenException) {
    RenderProcessor::initialize();

    shaderPrg_ = ShdrMgr.loadSeparate("passthrough.vert", "copyimage.frag",
        generateHeader(), false);

    if (!shaderPrg_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }

    initialized_ = true;
}


void LoopCompositor::process() {

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextureUnit colorUnit, depthUnit;

    // initialize shader
    shaderPrg_->activate();
    setGlobalShaderParameters(shaderPrg_);
    shaderPrg_->setUniform("colorTex_", colorUnit.getUnitNumber());
    shaderPrg_->setUniform("depthTex_", depthUnit.getUnitNumber());
    if (currentPass_++ < finalPass_.get()) {
        inport0_.bindTextures(colorUnit.getEnum(), depthUnit.getEnum());
        inport0_.setTextureParameters(shaderPrg_, "textureParameters_");
    } else {
        currentPass_ = 1;
        inport1_.bindTextures(colorUnit.getEnum(), depthUnit.getEnum());
        inport1_.setTextureParameters(shaderPrg_, "textureParameters_");
    }

    renderQuad();

    shaderPrg_->deactivate();
    LGL_ERROR;
}

} // voreen namespace
