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

#include "voreen/modules/base/processors/image/imagemasking.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

ImageMasking::ImageMasking()
    : ImageProcessorDepth("image/mask")
    , inport_(Port::INPORT, "image.input")
    , inportMask_(Port::INPORT, "image.mask")
    , outport_(Port::OUTPORT, "image.output", true)
    , maskColor_("maskColor", "Mask Color", tgt::Color(0.0f, 0.0f, 0.0f, 0.0f))
{
    maskColor_.setViews(Property::COLOR);
    addPort(inport_);
    addPort(inportMask_);
    addPort(outport_);

    addProperty(maskColor_);
}

std::string ImageMasking::getProcessorInfo() const {
    return "Masks the input image: All fragments matching the mask color in the mask texture are cleared.";
}

Processor* ImageMasking::create() const {
    return new ImageMasking();
}

void ImageMasking::process() {
    outport_.activateTarget();
    outport_.clearTarget();

    TextureUnit colorUnit, depthUnit, maskUnit;
    inport_.bindTextures(colorUnit.getEnum(), depthUnit.getEnum());
    inportMask_.bindColorTexture(maskUnit.getEnum());

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    LGL_ERROR;

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);

    program_->setIgnoreUniformLocationError(true);
    program_->setUniform("colorTex_", colorUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());
    program_->setUniform("colorTexMask_", maskUnit.getUnitNumber());
    inport_.setTextureParameters(program_, "texParams_");
    inportMask_.setTextureParameters(program_, "texParamsMask_");
    program_->setIgnoreUniformLocationError(false);

    program_->setUniform("maskColor_", maskColor_.get());

    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    program_->deactivate();
    outport_.deactivateTarget();
    glActiveTexture(GL_TEXTURE0);
    LGL_ERROR;
}

} // voreen namespace
