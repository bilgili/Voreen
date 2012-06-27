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

#include "voreen/modules/base/processors/image/unaryimageprocessor.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;


namespace voreen {

UnaryImageProcessor::UnaryImageProcessor()
    : ImageProcessorDepth("image/unary")
    , shader_("shader", "Fragment shader", "image/unary.frag")
    , inport_(Port::INPORT, "image.inport")
    , outport_(Port::OUTPORT, "image.outport", true)
{
    addProperty(shader_);

    addPort(inport_);
    addPort(outport_);
}

std::string UnaryImageProcessor::getProcessorInfo() const {
    return "Image processor with one input image that is customizable by its shader property.";
}

Processor* UnaryImageProcessor::create() const {
    return new UnaryImageProcessor();
}

void UnaryImageProcessor::initialize() throw (VoreenException) {
    // assign header before initializing the shader property
    initialized_ = true;
    shader_.setHeader(generateHeader());
    shader_.rebuild();
    initialized_ = false;
    ImageProcessorDepth::initialize();  // initializes the shader property
    loadShader();
    initialized_ = true;
}

void UnaryImageProcessor::loadShader() {
    shader_.rebuild();
}

void UnaryImageProcessor::compileShader() {
    shader_.setHeader(generateHeader());
    shader_.rebuild();
}

void UnaryImageProcessor::process() {
    outport_.activateTarget();

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compileShader();
    LGL_ERROR;

    // initialize shader
    tgt::Shader* sh = shader_.getShader();
    if (!sh || !sh->isLinked())
        return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextureUnit colorUnit, depthUnit;
    inport_.bindTextures(colorUnit.getEnum(), depthUnit.getEnum());

    sh->activate();
    setGlobalShaderParameters(sh);

    sh->setIgnoreUniformLocationError(true);
    sh->setUniform("colorTex_", colorUnit.getUnitNumber());
    sh->setUniform("depthTex_", depthUnit.getUnitNumber());
    inport_.setTextureParameters(sh, "texParams_");
    sh->setIgnoreUniformLocationError(false);

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    sh->deactivate();
    outport_.deactivateTarget();
    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;
}

} // voreen namespace
