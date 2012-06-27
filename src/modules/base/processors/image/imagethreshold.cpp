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

#include "voreen/modules/base/processors/image/imagethreshold.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

ImageThreshold::ImageThreshold()
    : ImageProcessor("image/imagethreshold"),
    inport_(Port::INPORT, "image.inport"),
    outport_(Port::OUTPORT, "image.outport"),
    lowerThreshold_("lowerThreshold", "Lower Threshold", 0.f, 0.0f, 1.f),
    upperThreshold_("upperThreshold", "Upper Threshold", 1.f, 0.0f, 1.f),
    lowerMaskColor_("lowerMaskColor", "Lower Mask Color", tgt::Color(0.f)),
    upperMaskColor_("upperMaskColor", "Upper Mask Color", tgt::Color(0.f))

{
    lowerMaskColor_.setViews(Property::COLOR);
    upperMaskColor_.setViews(Property::COLOR);
    addPort(inport_);
    addPort(outport_);

    addProperty(lowerThreshold_);
    addProperty(upperThreshold_);
    addProperty(lowerMaskColor_);
    addProperty(upperMaskColor_);
}

Processor* ImageThreshold::create() const {
    return new ImageThreshold();
}

std::string ImageThreshold::getProcessorInfo() const {
    return "All pixels with intensities below or above the two thresholds "
           "are assigned the mask color. Pixels with intensities between "
           "the thresholds are passed through.";
}

void ImageThreshold::process() {
    outport_.activateTarget();
    outport_.clearTarget();

    TextureUnit colorUnit, depthUnit;
    inport_.bindTextures(colorUnit, depthUnit);

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("colorTex_", colorUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());
    inport_.setTextureParameters(program_, "texParams_");
    program_->setUniform("lowerThreshold_", lowerThreshold_.get());
    program_->setUniform("upperThreshold_", upperThreshold_.get());
    program_->setUniform("lowerMaskColor_", lowerMaskColor_.get());
    program_->setUniform("upperMaskColor_", upperMaskColor_.get());

    renderQuad();

    program_->deactivate();
    outport_.deactivateTarget();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

} // voreen namespace
