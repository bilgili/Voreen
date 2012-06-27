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

#include "voreen/modules/base/processors/image/imageabstraction.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

ImageAbstraction::ImageAbstraction()
    : ImageProcessorDepth("pp_imageabstraction"),
      minSigma_("minSigma", "Minimum sigma", 0.1f, 0.1f, 5.0f),
      maxSigma_("maxSigma", "Maximum sigma", 4.0f, 0.1f, 10.0f),
      mappingFunc_("mappingFunction", "Mapping function", Processor::INVALID_RESULT,
                    TransFuncProperty::Editors(TransFuncProperty::INTENSITY)),
      inport_(Port::INPORT, "image.inport"),
      outport_(Port::OUTPORT, "image.outport")
{
    addProperty(minSigma_);
    addProperty(maxSigma_);
    addProperty(mappingFunc_);

    addPort(inport_);
    addPort(outport_);
}

std::string ImageAbstraction::getProcessorInfo() const {
    return "";
}

void ImageAbstraction::process() {
    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    analyzeDepthBuffer(&inport_);

    TextureUnit shadeUnit, depthUnit;
    inport_.bindTextures(shadeUnit.getEnum(), depthUnit.getEnum());

    // bind mapping function
    TextureUnit mappingUnit;
    mappingUnit.activate();

    if (mappingFunc_.get())
        mappingFunc_.get()->bind();

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex_", shadeUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());
    program_->setUniform("minDepth_", minDepth_.get());
    program_->setUniform("maxDepth_", maxDepth_.get());
    inport_.setTextureParameters(program_, "textureParameters_");
    program_->setUniform("minSigma_", minSigma_.get());
    program_->setUniform("maxSigma_", maxSigma_.get());
    program_->setUniform("mappingFunc_", mappingUnit.getUnitNumber());

    renderQuad();

    program_->deactivate();
    LGL_ERROR;
}

} // voreen namespace
