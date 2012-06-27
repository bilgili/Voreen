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

#ifndef VRN_ORTHOGONALSLICERENDERER_H
#define VRN_ORTHOGONALSLICERENDERER_H

#include "voreen/modules/base/processors/render/slicerendererbase.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/interaction/camerainteractionhandler.h"
#include "tgt/camera.h"

namespace voreen {

class FlowOrthogonalSliceRenderer : public SliceRendererBase {
public:
    FlowOrthogonalSliceRenderer();
    virtual ~FlowOrthogonalSliceRenderer();

    virtual Processor* create() const;
    virtual std::string getCategory() const { return "Slice Rendering"; }
    virtual std::string getClassName() const { return "FlowOrthogonalSliceRenderer"; }
    virtual CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

    /**
     * The processor is ready when its inport and outports are connected. The
     * coprocessor ports are optional and can be ignored.
     */
    virtual bool isReady() const;
    virtual void process();

    const tgt::ivec3& getSlicePositions() const;

protected:
    enum SliceAlignment { SLICE_XY, SLICE_XZ, SLICE_ZY };

protected:
    static tgt::vec3 permuteComponents(const tgt::vec3& input, const tgt::ivec3& permutation);
    void onSlicePositionChange();
    virtual void renderSlice(const SliceAlignment& sliceAlign, const float sliceNo);
    void setSlicePropertiesVisible(const bool visible);
    void setupSliceProperties(const tgt::ivec3& positions);
    virtual void updateNumSlices();

protected:
    tgt::ivec3 volumeDimensions_;
    tgt::ivec3 slicePositions_;

    BoolProperty useXYSliceProp_;
    BoolProperty useXZSliceProp_;
    BoolProperty useZYSliceProp_;
    IntProperty sliceNoXYProp_;
    IntProperty sliceNoXZProp_;
    IntProperty sliceNoZYProp_;
    CameraProperty camProp_;
    CameraInteractionHandler* cameraHandler_;

    GenericCoProcessorPort<FlowOrthogonalSliceRenderer> cpInport_;
    GenericCoProcessorPort<FlowOrthogonalSliceRenderer> cpOutport_;
};

}   // namespace

#endif
