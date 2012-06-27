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

#include "voreen/core/processors/imageprocessordepth.h"

namespace voreen {

ImageProcessorDepth::ImageProcessorDepth(const std::string& shaderFilename)
    : ImageProcessor(shaderFilename)
    , minDepth_("minDepth", "Mindepth", 0.0f)
    , maxDepth_("maxDepth", "Maxdepth", 1.0f)
{}

/*
* Read back depth buffer and determine min and max depth value.
*/
void ImageProcessorDepth::analyzeDepthBuffer(RenderPort* port) {
    port->getDepthTexture()->downloadTexture();
    float* pixels = (float*)port->getDepthTexture()->getPixelData();
    float curDepth = *(pixels);
    float minDepth = curDepth;
    float maxDepth = curDepth;
    for (int i = 0; i < port->getSize().x*port->getSize().y; i++) {
        curDepth = *(pixels++);
        minDepth = std::min(minDepth, curDepth);
        maxDepth = std::max(maxDepth, curDepth);
    }
    minDepth_.set(minDepth);
    maxDepth_.set(maxDepth);
}

} // voreen namespace
