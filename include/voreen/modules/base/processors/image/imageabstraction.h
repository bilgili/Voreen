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

#ifndef VRN_IMAGEABSTRACTION_H
#define VRN_IMAGEABSTRACTION_H

#include "voreen/core/processors/imageprocessordepth.h"

namespace voreen {

class ImageAbstraction : public ImageProcessorDepth {
public:
    ImageAbstraction();

    virtual std::string getCategory() const { return "Image Processing"; }
    virtual std::string getClassName() const { return "ImageAbstraction"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_EXPERIMENTAL; }
    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const {return new ImageAbstraction();}
    void process();

protected:
    FloatProperty minSigma_; ///< The minimal sigma value used for the Gaussian
    FloatProperty maxSigma_; ///< The maximal sigma value used for the Gaussian
    TransFuncProperty mappingFunc_;  ///< Mapping function to control the blur fall-off

    RenderPort inport_;
    RenderPort outport_;
};


} // namespace voreen

#endif //VRN_IMAGEABSTRACTION_H
