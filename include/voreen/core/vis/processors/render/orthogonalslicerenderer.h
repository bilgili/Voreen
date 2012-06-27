#ifndef VRN_ORTHOGONALSLICERENDERER_H
#define VRN_ORTHOGONALSLICERENDERER_H

#include "voreen/core/vis/processors/render/slicerendererbase.h"
#include "voreen/core/vis/properties/cameraproperty.h"
#include "voreen/core/vis/interaction/camerainteractionhandler.h"
#include "tgt/camera.h"

namespace voreen {

class OrthogonalSliceRenderer : public SliceRendererBase {
public:
    OrthogonalSliceRenderer();
    virtual ~OrthogonalSliceRenderer();

    virtual Processor* create() const;
    virtual std::string getCategory() const { return "Slice Rendering"; }
    virtual std::string getClassName() const { return "OrthogonalSliceRenderer"; }
    virtual CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual std::string getModuleName() const { return "core"; }
    virtual const std::string getProcessorInfo() const;

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

    GenericCoProcessorPort<OrthogonalSliceRenderer> cpInport_;
    GenericCoProcessorPort<OrthogonalSliceRenderer> cpOutport_;
};

}   // namespace

#endif
