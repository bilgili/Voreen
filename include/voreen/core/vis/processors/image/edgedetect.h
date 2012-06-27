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

#ifndef VRN_EDGEDETECT_H
#define VRN_EDGEDETECT_H

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/image/imageprocessordepth.h"

namespace voreen {

/**
 * Performs an edge detection.
 *
 * The detected edge is then colored.
 */
class EdgeDetect : public ImageProcessorDepth {

public:

    /**
     * The Constructor.
     *
     */
    EdgeDetect();
    virtual std::string getCategory() const { return "Image Processing"; }
    virtual std::string getClassName() const { return "EdgeDetect"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new EdgeDetect(); }

    virtual ~EdgeDetect();

    void process();

protected:
    FloatProperty edgeThreshold_; ///< Threshold value for the gradient length to be considered as an edge
    BoolProperty showImage_; ///< Whether to show the original image or just the edge
    IntOptionProperty blendMode_; ///< Specifies how the edge is colored
    IntOptionProperty edgeStyle_; ///< The edge style can be: contour, silhouette, depth-dependent contour
    ColorProperty edgeColor_; ///< The color used for drawing the edges

    RenderPort inport_;
    RenderPort outport_;
};


} // namespace voreen

#endif //VRN_EDGEDETECT_H

