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

#include "voreen/modules/base/processors/image/gaussian.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

Gaussian::Gaussian()
    : ImageProcessor("image/gaussian"),
      sigma_("sigma", "Sigma", 2.0f, 0.1f, 10.0f),
      blurRed_("blurRed", "Red channel", true),
      blurGreen_("blurGreen", "Green channel", true),
      blurBlue_("blurBlue", "Blue channel", true),
      blurAlpha_("blurAlpha", "Alpha channel", true),
      blurDepth_("blurDepth", "Depth channel", true),
      inport_(Port::INPORT, "image.inport"),
      outport_(Port::OUTPORT, "image.outport"),
      privatePort_(Port::OUTPORT, "image.privateport", true)
{
    addProperty(sigma_);
    addProperty(blurRed_);
    addProperty(blurGreen_);
    addProperty(blurBlue_);
    addProperty(blurAlpha_);
    addProperty(blurDepth_);

    addPort(inport_);
    addPort(outport_);
    addPrivateRenderPort(&privatePort_);
}

std::string Gaussian::getProcessorInfo() const {
    return "Performs a convolution with a Gaussian filter kernel. The kernel size is set to round(2.5*sigma).";
}

void Gaussian::process() {
    privatePort_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextureUnit colorUnit, depthUnit;
    inport_.bindTextures(colorUnit.getEnum(), depthUnit.getEnum());

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("colorTex_", colorUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());
    inport_.setTextureParameters(program_, "textureParameters_");
    program_->setUniform("dir_", tgt::vec2(1.0,0.0));
    program_->setUniform("sigma_", sigma_.get());
    program_->setUniform("blurChannels",  blurRed_.get() ? 1.f : 0.f,
        blurGreen_.get() ? 1.f : 0.f,
        blurBlue_.get() ? 1.f : 0.f,
        blurAlpha_.get() ? 1.f : 0.f);
    program_->setUniform("nblurChannels", blurRed_.get() ? 0.f : 1.f,
        blurGreen_.get() ? 0.f : 1.f,
        blurBlue_.get() ? 0.f : 1.f,
        blurAlpha_.get() ? 0.f : 1.f);
    program_->setUniform("blurDepth_", blurDepth_.get());
    renderQuad();
    program_->deactivate();
    privatePort_.deactivateTarget();

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    privatePort_.bindTextures(colorUnit.getEnum(), depthUnit.getEnum());

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("colorTex_", colorUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());
    inport_.setTextureParameters(program_, "textureParameters_");
    program_->setUniform("dir_", tgt::vec2(0.0,1.0));
    program_->setUniform("sigma_", sigma_.get());
    program_->setUniform("blurChannels",  blurRed_.get() ? 1.f : 0.f,
        blurGreen_.get() ? 1.f : 0.f,
        blurBlue_.get() ? 1.f : 0.f,
        blurAlpha_.get() ? 1.f : 0.f);
    program_->setUniform("nblurChannels", blurRed_.get() ? 0.f : 1.f,
        blurGreen_.get() ? 0.f : 1.f,
        blurBlue_.get() ? 0.f : 1.f,
        blurAlpha_.get() ? 0.f : 1.f);
    program_->setUniform("blurDepth_", blurDepth_.get());
    renderQuad();

    program_->deactivate();
    outport_.deactivateTarget();
    LGL_ERROR;
}

} // voreen namespace
