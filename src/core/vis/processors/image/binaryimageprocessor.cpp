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

#include "voreen/core/vis/processors/image/binaryimageprocessor.h"

namespace voreen {

const std::string BinaryImageProcessor::shadeTexUnit0_ = "shadeTex0";
const std::string BinaryImageProcessor::depthTexUnit0_ = "depthTex0";
const std::string BinaryImageProcessor::shadeTexUnit1_ = "shadeTex1";
const std::string BinaryImageProcessor::depthTexUnit1_ = "depthTex1";

BinaryImageProcessor::BinaryImageProcessor()
    : ImageProcessorDepth("pp_binary")
	, shader_("shader", "Fragment shader", "pp_binary.frag", tgt::ShaderObject::FRAGMENT_SHADER, Processor::INVALID_PROGRAM)
	, inport0_(Port::INPORT, "image.inport0")
	, inport1_(Port::INPORT, "image.inport1")
    , outport_(Port::OUTPORT, "image.outport", true)
{
	addProperty(shader_);

	addPort(inport0_);
	addPort(inport1_);
    addPort(outport_);

	std::vector<std::string> units;
	units.push_back(shadeTexUnit0_);
	units.push_back(depthTexUnit0_);
	units.push_back(shadeTexUnit1_);
	units.push_back(depthTexUnit1_);
	tm_.registerUnits(units);
}

const std::string BinaryImageProcessor::getProcessorInfo() const {
    return "";
}

Processor* BinaryImageProcessor::create() const {
    return new BinaryImageProcessor();
}

void BinaryImageProcessor::initialize() throw (VoreenException) {
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

void BinaryImageProcessor::loadShader() {
	program_ = new tgt::Shader();
	tgt::ShaderObject* vert = new tgt::ShaderObject(ShdrMgr.completePath("pp_identity.vert"), tgt::ShaderObject::VERTEX_SHADER);
	vert->setHeader(generateHeader());
	vert->loadSourceFromFile(ShdrMgr.completePath("pp_identity.vert"));
	vert->compileShader();
	program_->attachObject(shader_.get());
	program_->attachObject(vert);
	program_->linkProgram();
}

void BinaryImageProcessor::compileShader() {
	if (!program_)
        return;
	program_->setHeaders(generateHeader(), false);
	program_->rebuild();
}

void BinaryImageProcessor::process() {
    outport_.activateTarget();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	inport0_.bindTextures(tm_.getGLTexUnit(shadeTexUnit0_), tm_.getGLTexUnit(depthTexUnit0_));
	inport1_.bindTextures(tm_.getGLTexUnit(shadeTexUnit1_), tm_.getGLTexUnit(depthTexUnit1_));

	// compile program if needed
	if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
		compileShader();
	LGL_ERROR;

	// initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);

    program_->setIgnoreUniformLocationError(true);
    program_->setUniform("shadeTex0_", tm_.getTexUnit(shadeTexUnit0_));
    program_->setUniform("depthTex0_", tm_.getTexUnit(depthTexUnit0_));
    program_->setUniform("shadeTex1_", tm_.getTexUnit(shadeTexUnit1_));
    program_->setUniform("depthTex1_", tm_.getTexUnit(depthTexUnit1_));
	inport0_.setTextureParameters(program_, "texParams0_");
	inport1_.setTextureParameters(program_, "texParams1_");
    program_->setIgnoreUniformLocationError(false);

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    program_->deactivate();
    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;
}

} // voreen namespace
