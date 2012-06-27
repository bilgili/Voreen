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

#ifndef VRN_THRESHOLD_H
#define VRN_THRESHOLD_H

#include "voreen/core/vis/processors/image/imageprocessor.h"

namespace voreen {

/**
 * Performs a thresholding.
 *
 * The pixel color is used, when the surrounding pixel exceed a defined
 * threshold and the pixel is set to transparent otherwise.
 *
 * It's probably a slow filter because an if statement is used internally.
 */
class Threshold : public ImageProcessor {
public:
    /**
     * The Constructor.
     */
    Threshold();
    virtual std::string getCategory() const { return "Image Processing"; }
    virtual std::string getClassName() const { return "Threshold"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_TESTING; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const {return new Threshold();}

    void process();

protected:
    FloatProperty threshold_; ///< The threshold used to compare the sum of the lengths of the tested colors

    RenderPort inport_;
    RenderPort outport_;
};


} // namespace voreen

#endif //VRN_THRESHOLD_H
