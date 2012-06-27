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

#ifndef VRN_IMAGEPROCESSORDEPTH_H
#define VRN_IMAGEPROCESSORDEPTH_H

#include "voreen/core/vis/processors/image/imageprocessor.h"

namespace voreen {


/**
 * Base class for post processors that analyze the depth buffer.
 */
class ImageProcessorDepth : public ImageProcessor {
public:
    /**
     * Constructor.
     *
     * @param shaderFilename The filename of the shader that will be used.
     */
    ImageProcessorDepth(const std::string& shaderFilename = "");

    virtual const Identifier getClassName() const { return "ImageProcessor.ImageProcessorDepth"; }

protected:
    virtual void analyzeDepthBuffer(int source); ///< Read back depth buffer and determine min and max depth value.

    FloatProp minDepth_; ///< Control the minimum depth value. (from what? (df))
    FloatProp maxDepth_; ///< Control the maximum depth value.
};

} // namespace voreen

#endif //VRN_IMAGEPROCESSORDEPTH_H
