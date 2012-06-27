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

#include "voreen/modules/base/processors/image/grayscale.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

Grayscale::Grayscale()
    : ImageProcessor("pp_grayscale"), // loads fragment shader pp_grayscale.frag
      saturation_("saturation", "Saturation", 0.0f),
      inport_(Port::INPORT, "inport"),
      outport_(Port::OUTPORT, "outport")
{
    // register properties and ports
    addProperty(saturation_);

    addPort(inport_);
    addPort(outport_);
}

std::string Grayscale::getProcessorInfo() const {
    return "Converts a color image to grayscale.";
}

void Grayscale::process() {
    // activate and clear output render target
    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind result from previous processor
    TextureUnit shadeUnit, depthUnit;
    inport_.bindTextures(shadeUnit.getEnum(), depthUnit.getEnum());

    // activate shader and set uniforms:
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", shadeUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());
    program_->setUniform("saturation_", saturation_.get());

    // render screen aligned quad:
    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    // cleanup
    program_->deactivate();
    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;
}

} // namespace
