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

#include "voreen/core/vis/processors/image/unaryimageprocessor.h"

namespace voreen {

UnaryImageProcessor::UnaryImageProcessor()
    : ImageProcessorDepth("pp_unary")
	, shader_("shader", "Fragment shader", "pp_unary.frag", tgt::ShaderObject::FRAGMENT_SHADER, Processor::INVALID_PROGRAM)
	, inport_(Port::INPORT, "image.inport")
    , outport_(Port::OUTPORT, "image.outport", true)
{
	addProperty(shader_);

	addPort(inport_);
    addPort(outport_);
}

const std::string UnaryImageProcessor::getProcessorInfo() const {
    return "";
}

Processor* UnaryImageProcessor::create() const {
    return new UnaryImageProcessor();
}

void UnaryImageProcessor::initialize() throw (VoreenException) {

	// assign header before initializing the shader property
	initialized_ = true;
	shader_.setHeader(generateHeader());
	initialized_ = false;           //< prevents warning
	ImageProcessorDepth::initialize();  // initializes the shader property
	loadShader();
	if (!program_) {
		LERROR("Failed to load shaders!");
		initialized_ = false;
		throw VoreenException(getClassName() + ": Failed to load shaders!");
	}
}

void UnaryImageProcessor::loadShader() {
	program_ = new tgt::Shader();
	tgt::ShaderObject* vert = new tgt::ShaderObject(ShdrMgr.completePath("pp_identity.vert"), tgt::ShaderObject::VERTEX_SHADER);
	vert->setHeader(generateHeader());
	vert->loadSourceFromFile(ShdrMgr.completePath("pp_identity.vert"));
	vert->compileShader();
	program_->attachObject(shader_.get());
	program_->attachObject(vert);
	program_->linkProgram();
}

void UnaryImageProcessor::compileShader() {
	if (!program_)
        return;
	program_->setHeaders(generateHeader(), false);
	program_->rebuild();
}

void UnaryImageProcessor::process() {
    outport_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	inport_.bindTextures(tm_.getGLTexUnit(shadeTexUnit_), tm_.getGLTexUnit(depthTexUnit_));
	
	// compile program if needed
	if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
		compileShader();
	LGL_ERROR;

	// initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);

    program_->setIgnoreUniformLocationError(true);
    program_->setUniform("shadeTex_", tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex_", tm_.getTexUnit(depthTexUnit_));
	inport_.setTextureParameters(program_, "texParams_");
    program_->setIgnoreUniformLocationError(false);

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    program_->deactivate();
    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;
}

} // voreen namespace
