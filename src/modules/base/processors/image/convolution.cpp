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

#include "voreen/modules/base/processors/image/convolution.h"

namespace voreen {

Convolution::Convolution()
    : ImageProcessor("image/convolution"),
      filterSize_("filtersize", "Filter Size (NxN)", 7),
      inport_(Port::INPORT, "inport"),
      filterPort_(Port::INPORT, "filterport"),
      outport_(Port::OUTPORT, "outport")
{
    // register properties and ports:
    addProperty(filterSize_);

    addPort(inport_);
    addPort(filterPort_);
    addPort(outport_);
}

Processor* Convolution::create() const {
    return new Convolution();
}

std::string Convolution::getProcessorInfo() const {
    return "Applies an arbitrary filter mask to an image. "
           "The mask is passed through the second RenderPort.";
}

void Convolution::process() {
    // activate and clear output render target
    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind result from previous processor
    inport_.bindTextures(GL_TEXTURE0, GL_TEXTURE1);
    filterPort_.bindColorTexture(GL_TEXTURE2);

    // activate shader and set uniforms:
    program_->activate();
    setGlobalShaderParameters(program_);
    LGL_ERROR;
    program_->setUniform("colorTex_", 0);
    program_->setUniform("depthTex_", 1);
    inport_.setTextureParameters(program_, "textureParameters_");
    LGL_ERROR;

    program_->setUniform("filter_", 2);
    LGL_ERROR;
    filterPort_.setTextureParameters(program_, "filterParameters_");
    LGL_ERROR;

    program_->setUniform("filterSize_", filterSize_.get());

    LGL_ERROR;
    // render screen aligned quad:
    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    program_->deactivate();
    outport_.deactivateTarget();
    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;
}

} // namespace
