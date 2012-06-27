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

#include "voreen/modules/base/processors/utility/renderloopfinalizer.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

RenderLoopFinalizer::RenderLoopFinalizer()
    : RenderProcessor(),
      shaderPrg_(0),
      inport_(Port::INPORT, "image.inport"),
      outport_(Port::OUTPORT, "image.outport"),
      loopOutport_(Port::OUTPORT, "loop.outport")
{
    loopOutport_.setLoopPort(true);

    addPort(inport_);
    addPort(outport_);
    addPort(loopOutport_);
}

std::string RenderLoopFinalizer::getProcessorInfo() const {
    return "In combination with RenderLoopInitiator, this processor is used to establish render loops.";
}

Processor* RenderLoopFinalizer::create() const {
    return new RenderLoopFinalizer();
}

void RenderLoopFinalizer::initialize() throw (VoreenException) {
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

void RenderLoopFinalizer::deinitialize() throw (VoreenException) {
    ShdrMgr.dispose(shaderPrg_);

    RenderProcessor::deinitialize();
}

bool RenderLoopFinalizer::isReady() const {
    return (inport_.isReady() && outport_.isReady());
}

void RenderLoopFinalizer::process() {
    if (loopOutport_.getLoopIteration() == loopOutport_.getNumLoopIterations()-1) {
        // last iteration: write input image to outport
        outport_.activateTarget();
    }
    else {
        // loop iteration: write input image to loop port
        loopOutport_.activateTarget();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextureUnit colorUnit, depthUnit;
    inport_.bindTextures(colorUnit.getEnum(), depthUnit.getEnum());
    LGL_ERROR;

    // initialize shader
    shaderPrg_->activate();

    // set common uniforms used by all shaders
    setGlobalShaderParameters(shaderPrg_);

    // pass the remaining uniforms to the shader
    shaderPrg_->setUniform("colorTex_", colorUnit.getUnitNumber());
    shaderPrg_->setUniform("depthTex_", depthUnit.getUnitNumber());
    inport_.setTextureParameters(shaderPrg_, "texParameters_");

    renderQuad();

    shaderPrg_->deactivate();
    outport_.deactivateTarget();
    TextureUnit::setZeroUnit();

    LGL_ERROR;
}

} // voreen namespace
