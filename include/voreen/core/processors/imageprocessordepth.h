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

#ifndef VRN_IMAGEPROCESSORDEPTH_H
#define VRN_IMAGEPROCESSORDEPTH_H

#include "voreen/core/processors/imageprocessor.h"

#include "voreen/core/properties/floatproperty.h"

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

    virtual std::string getCategory() const { return "Image Processing"; }
    virtual std::string getClassName() const { return "ImageProcessorDepth"; }

protected:
    virtual void analyzeDepthBuffer(RenderPort* port); ///< Read back depth buffer and determine min and max depth value.

    FloatProperty minDepth_; ///< Control the minimum depth value. (from what? (df))
    FloatProperty maxDepth_; ///< Control the maximum depth value.
};

} // namespace voreen

#endif //VRN_IMAGEPROCESSORDEPTH_H
