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

#include "voreen/modules/base/processors/image/binaryimageprocessor.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

BinaryImageProcessor::BinaryImageProcessor()
    : ImageProcessorDepth("image/binary")
    , shader_("shader", "Fragment shader", "image/binary.frag")
    , inport0_(Port::INPORT, "image.inport0")
    , inport1_(Port::INPORT, "image.inport1")
    , outport_(Port::OUTPORT, "image.outport", true)
{
    addProperty(shader_);

    addPort(inport0_);
    addPort(inport1_);
    addPort(outport_);
}

std::string BinaryImageProcessor::getProcessorInfo() const {
    return "Image processor with two input images that is customizable by its shader property.";
}

Processor* BinaryImageProcessor::create() const {
    return new BinaryImageProcessor();
}

void BinaryImageProcessor::initialize() throw (VoreenException) {
    // assign header before initializing the shader property
    initialized_ = true;
    shader_.setHeader(generateHeader());
    shader_.rebuild();
    initialized_ = false;
    ImageProcessorDepth::initialize();  // initializes the shader property
    loadShader();
    initialized_ = true;
}

void BinaryImageProcessor::loadShader() {
    shader_.rebuild();
}

void BinaryImageProcessor::compileShader() {
    shader_.setHeader(generateHeader());
    shader_.rebuild();
}

void BinaryImageProcessor::process() {
    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compileShader();
    LGL_ERROR;

    tgt::Shader* sh = shader_.getShader();
    if(!sh || !sh->isLinked())
        return;

    outport_.activateTarget();
    outport_.clearTarget();

    TextureUnit colorUnit0, depthUnit0, colorUnit1, depthUnit1;
    inport0_.bindTextures(colorUnit0.getEnum(), depthUnit0.getEnum());
    inport1_.bindTextures(colorUnit1.getEnum(), depthUnit1.getEnum());

    // initialize shader
    sh->activate();
    setGlobalShaderParameters(sh);

    sh->setIgnoreUniformLocationError(true);
    sh->setUniform("colorTex0_", colorUnit0.getUnitNumber());
    sh->setUniform("depthTex0_", depthUnit0.getUnitNumber());
    sh->setUniform("colorTex1_", colorUnit1.getUnitNumber());
    sh->setUniform("depthTex1_", depthUnit1.getUnitNumber());
    inport0_.setTextureParameters(sh, "texParams0_");
    inport1_.setTextureParameters(sh, "texParams1_");
    sh->setIgnoreUniformLocationError(false);

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    sh->deactivate();
    outport_.deactivateTarget();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

} // voreen namespace
