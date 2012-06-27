/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/image/blur.h"
#include "voreen/core/vis/processors/portmapping.h"

namespace voreen {

Blur::Blur()
    : GenericFragment("pp_blur"),
      delta_("set.blurDelta", "Delta", 1.0f),
      blurRed_("set.blurRed", "red channel", true),
      blurGreen_("set.blurGreen", "green channel", true),
      blurBlue_("set.blurBlue", "blue channel", true),
      blurAlpha_("set.blurAlpha", "alpha channel", true)
{
    setName("Blur");
    addProperty(&delta_);
    addProperty(&blurRed_);
    addProperty(&blurGreen_);
    addProperty(&blurBlue_);
    addProperty(&blurAlpha_);
    createInport("image.inport");
    createOutport("image.outport");
}

const std::string Blur::getProcessorInfo() const {
	return "Performs a blurring";
}

void Blur::process(LocalPortMapping* portMapping) {
    glViewport(0,0,static_cast<GLsizei>(size_.x),static_cast<GLsizei>(size_.y));

	int source =  portMapping->getTarget("image.inport");
    int dest = portMapping->getTarget("image.outport");

    tc_->setActiveTarget(dest, "Blur::process");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind shading result from previous ray cast
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit_));
    glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));
    LGL_ERROR;

    // bind depth result from previous ray cast
    glActiveTexture(tm_.getGLTexUnit(depthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
    LGL_ERROR;

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", (GLint) tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex_", (GLint) tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("delta_", delta_.get());
    program_->setUniform("blurChannels",    blurRed_.get() ? 1.f : 0.f,
						                    blurGreen_.get() ? 1.f : 0.f,
                                            blurBlue_.get() ? 1.f : 0.f,
                                            blurAlpha_.get() ? 1.f : 0.f);
    program_->setUniform("nblurChannels",   blurRed_.get() ? 0.f : 1.f,
						                    blurGreen_.get() ? 0.f : 1.f,
                                            blurBlue_.get() ? 0.f : 1.f,
                                            blurAlpha_.get() ? 0.f : 1.f);
    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

void Blur::setDelta(float delta) {
    delta_.set(delta);
}

void Blur::processMessage(Message* msg, const Identifier& dest){
	GenericFragment::processMessage(msg, dest);
    if (msg->id_ == "set.blurDelta") {
        delta_.set(msg->getValue<float>());
        invalidate();
    }
    else if (msg->id_ == "set.blurRed") {
        blurRed_.set(msg->getValue<bool>());
        invalidate();
    }
    else if (msg->id_ == "set.blurGreen") {
        blurGreen_.set(msg->getValue<bool>());
        invalidate();
    }
    else if (msg->id_ == "set.blurBlue") {
        blurBlue_.set(msg->getValue<bool>());
        invalidate();
    }
    else if (msg->id_ == "set.blurAlpha") {
        blurAlpha_.set(msg->getValue<bool>());
        invalidate();
    }
}


} // voreen namespace

