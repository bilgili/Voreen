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

#include "voreen/core/vis/processors/image/imageprocessordepth.h"

namespace voreen {

ImageProcessorDepth::ImageProcessorDepth(const std::string& shaderFilename)
    : ImageProcessor(shaderFilename)
    , minDepth_("set.minDepthPP", "Mindepth", 0.0f)
    , maxDepth_("set.maxDepthPP", "Maxdepth", 1.0f)
{
}

/*
* Read back depth buffer and determine min and max depth value.
*/
void ImageProcessorDepth::analyzeDepthBuffer(int source) {
    float* pixels = tc_->getDepthTargetAsFloats(source);
    float* depthImg = pixels;
    float curDepth = *(pixels);
    minDepth_.set(curDepth);
    maxDepth_.set(curDepth);
    for (int x = 0; x < tc_->getSize().x; ++x) {
        for (int y = 0; y < tc_->getSize().y; ++y) {
            curDepth = *(pixels++);
            if (minDepth_.get() == 0.0f)
                minDepth_.set(curDepth);
            else if ((curDepth != 0.0f) && (minDepth_.get() > curDepth))
                minDepth_.set(curDepth);
            if (maxDepth_.get() == 1.0f)
                maxDepth_.set(curDepth);
            else if ((curDepth != 1.0f) && (maxDepth_.get() < curDepth))
                maxDepth_.set(curDepth);
        }
    }

    //#define SAVE_DEPTH_BUFFER
#ifdef SAVE_DEPTH_BUFFER
    // save image for test purpose
    ILuint img;
    ilGenImages(1, &img);
    ilBindImage(img);
    //ilTexImage(tc_->getWidth(), tc_->getHeight(), 1, 1, IL_LUMINANCE, IL_FLOAT, NULL);
    //ilSetPixels(0,0,0,tc_->getWidth(),tc_->getHeight(),1,IL_LUMINANCE, IL_FLOAT, pixels);
    ilTexImage(tc_->getWidth(), tc_->getHeight(), 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, NULL);
    ilSetPixels(0, 0, 0, tc_->getWidth(), tc_->getHeight(), 1, IL_RGBA, IL_UNSIGNED_BYTE, depthImg);
    ilEnable(IL_FILE_OVERWRITE);
    ilSaveImage("depth.bmp");
    ilDeleteImages(1, &img);
#endif
    delete[] depthImg;
}


} // voreen namespace
