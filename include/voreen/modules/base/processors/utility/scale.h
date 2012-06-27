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

#ifndef VRN_SCALE_H
#define VRN_SCALE_H

#include "voreen/core/processors/imageprocessor.h"

namespace voreen {

/**
 * Performs a scaling.
 */
class ScalingProcessor : public ImageProcessor {
public:
    ScalingProcessor();

    virtual std::string getCategory() const { return "Utility"; }
    virtual std::string getClassName() const { return "ScaleProcessor"; }
    virtual bool isUtility() const { return true; }

    virtual Processor* create() const = 0;

    void process() = 0;

    //Test if a textureContainerChanged on port p with tc would result in a conflict
    virtual bool testSizeOrigin(const RenderPort* p, void* so) const;

protected:

    void applyScalingMatrix(int scalingMode, RenderPort* inport, RenderPort* outport);
    tgt::MouseEvent* transformMouseCoordinates(tgt::MouseEvent* e, int scalingMode, RenderPort* inport, RenderPort* outport) const;

    virtual void onEvent(tgt::Event* e);

    BoolProperty distributeEvents_;

    RenderPort inport_;
};


class SingleScale : public ScalingProcessor {
public:
    SingleScale();

    virtual std::string getCategory() const { return "Utility"; }
    virtual std::string getClassName() const { return "SingleScale"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const {return new SingleScale();}

    void process();

    virtual void sizeOriginChanged(RenderPort* p);
    virtual void portResized(RenderPort* p, tgt::ivec2 newsize);
protected:

    virtual void onEvent(tgt::Event* e);

    IntOptionProperty scalingMode_;    ///< What compositing mode should be applied for second outport

    RenderPort outport_;
};
} // namespace voreen

#endif //VRN_SCALE_H
