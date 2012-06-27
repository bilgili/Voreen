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

#ifndef VRN_GAUSSIAN_H
#define VRN_GAUSSIAN_H

#include "voreen/core/processors/imageprocessor.h"

namespace voreen {

/**
 * Performs a convolution with a Gaussian filter mask.
 */
class Gaussian : public ImageProcessor {
public:
    Gaussian();
    virtual Processor* create() const { return new Gaussian(); }

    virtual std::string getCategory() const { return "Image Processing"; }
    virtual std::string getClassName() const { return "Gaussian"; }
    virtual CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

protected:
    void process();

    FloatProperty sigma_; ///< Standard deviation of the Gaussian filter
    BoolProperty blurRed_; ///< Perform Gaussian filtering on R channel
    BoolProperty blurGreen_; ///< Perform Gaussian filtering on G channel
    BoolProperty blurBlue_; ///< Perform Gaussian filtering on B channel
    BoolProperty blurAlpha_; ///< Perform Gaussian filtering on A channel
    BoolProperty blurDepth_; ///< Perform Gaussian filtering on the depth buffer

    RenderPort inport_;
    RenderPort outport_;
    RenderPort privatePort_;
};


} // namespace voreen

#endif //VRN_GAUSSIAN_H
