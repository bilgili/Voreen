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

#ifndef VRN_IMAGEPROCESSOR_H
#define VRN_IMAGEPROCESSOR_H

#include "voreen/core/vis/processors/processor.h"

namespace voreen {

/**
 * Base class for fragment shader based post processors.
 *
 * Normally in a derived class you have only to call the ctor with the
 * appropriate shader name and overwrite the render method.
 */
class ImageProcessor : public Processor, public HasShader {
public:
    /**
     * Constructor.
     *
     * @param shaderFilename The filename of the shader that will be used.
     */
    ImageProcessor(const std::string& shaderFilename = "");
    virtual ~ImageProcessor();

    virtual const Identifier getClassName() const { return "ImageProcessor.ImageProcessor"; }
    virtual int initializeGL();

protected:
    /**
     * Load the needed shader.
     */
    virtual void compile();

    virtual void analyzeDepthBuffer(int source); ///< Read back depth buffer and determine min and max depth value.

    FloatProp minDepth_; ///< Control the minimum depth value. (from what? (df))
    FloatProp maxDepth_; ///< Control the maximum depth value.

    tgt::Shader* program_;
    std::string shaderFilename_;

    static const Identifier shadeTexUnit_;
    static const Identifier depthTexUnit_;
};

} // namespace voreen

#endif //VRN_IMAGEPROCESSOR_H
