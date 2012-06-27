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

#include "voreen/modules/base/processors/image/colordepth.h"

#include "tgt/texturemanager.h"
#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

ColorDepth::ColorDepth()
    : ImageProcessorDepth("pp_colordepth")
    , chromaDepthTex_(0)
    , colorMode_("mode", "Choose mode", INVALID_PROGRAM)
    , factor_("factor", "Factor", 1.0f, 0.0f, 10.0f)
    , inport_(Port::INPORT, "image.inport")
    , outport_(Port::OUTPORT, "image.outport")
{
    colorMode_.addOption("light-dark-replace", "Light-dark (replace)", 0);
    colorMode_.addOption("light-dark-modulate", "Light-dark (modulate)", 1);
    colorMode_.addOption("chromadepth", "Chromadepth", 2);
    colorMode_.addOption("pseudo-chromadepth", "Pseudo chromadepth", 3);
    addProperty(colorMode_);

    addProperty(factor_);

    addPort(inport_);
    addPort(outport_);
}

ColorDepth::~ColorDepth() {
    if (chromaDepthTex_)
        TexMgr.dispose(chromaDepthTex_);
}

std::string ColorDepth::getProcessorInfo() const {
    return "Performs a color adaptation based on the depth information. There are four modes available. "
        "'Light-dark' computes a gray value depending on the depth value. The current color can be modulated or "
        "replaced with this gray value. Additionally, a chromadepth or pseudo chromadepth color coding can be "
        "applied [Ropinski et al., Smartgraphics 2006].";
}

void ColorDepth::initialize() throw (VoreenException) {
    chromaDepthTex_ = TexMgr.load(VoreenApplication::app()->getTransFuncPath("chromadepthspectrum.bmp"));
    ImageProcessor::initialize();
}

void ColorDepth::process() {
    if (chromaDepthTex_ == 0)
        return;

    if (!interactionMode())
        analyzeDepthBuffer(&inport_);

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextureUnit shadeUnit, depthUnit;
    inport_.bindTextures(shadeUnit.getEnum(), depthUnit.getEnum());

    // bind chroma depth texture
    TextureUnit chromaDepthUnit;
    chromaDepthUnit.activate();
    //chromaDepthTex_ is 0 here
    chromaDepthTex_->bind();
    LGL_ERROR;

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", shadeUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());
    inport_.setTextureParameters(program_, "texParams_");
    program_->setUniform("chromadepthTex_", chromaDepthUnit.getUnitNumber());
    program_->setUniform("minDepth_", minDepth_.get());
    program_->setUniform("maxDepth_", maxDepth_.get());
    program_->setUniform("colorMode_", colorMode_.getValue());
    program_->setUniform("colorDepthFactor_", factor_.get());

    renderQuad();

    program_->deactivate();
    TextureUnit::setZeroUnit();
    outport_.deactivateTarget();
    LGL_ERROR;
}

} // voreen namespace
