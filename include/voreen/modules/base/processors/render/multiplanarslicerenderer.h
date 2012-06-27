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

#ifndef VRN_MULTIPLANARSLICERENDERER_H
#define VRN_MULTIPLANARSLICERENDERER_H

#include "voreen/modules/base/processors/render/slicerendererbase.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/interaction/camerainteractionhandler.h"
#include "tgt/glmath.h"
#include "tgt/camera.h"

namespace voreen {

class MultiplanarSliceRenderer : public SliceRendererBase {
public:
    MultiplanarSliceRenderer();
    virtual ~MultiplanarSliceRenderer();

    virtual Processor* create() const;
    virtual std::string getCategory() const { return "Slice Rendering"; }
    virtual std::string getClassName() const { return "MultiplanarSliceRenderer"; }
    virtual CodeState getCodeState() const { return CODE_STATE_TESTING; }
    virtual std::string getProcessorInfo() const;

    virtual void process();

protected:
    enum SliceAlignment { SLICE_XY, SLICE_XZ, SLICE_YZ };

protected:
    virtual void renderSlice(SliceAlignment sliceAlign, int sliceNo);
    virtual void updateNumSlices();

protected:
    BoolProperty renderXYSlice_;
    BoolProperty renderXZSlice_;
    BoolProperty renderYZSlice_;
    IntProperty sliceNumberXY_;
    IntProperty sliceNumberXZ_;
    IntProperty sliceNumberYZ_;
    CameraProperty camProp_;
    BoolProperty applyDatasetTransformationMatrix_;
    CameraInteractionHandler* cameraHandler_;
};

}   // namespace

#endif
