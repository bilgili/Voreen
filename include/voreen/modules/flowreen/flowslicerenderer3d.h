#ifdef VRN_MODULE_FLOWREEN
#ifndef VRN_FLOWSLICERENDERER3D_H
#define VRN_FLOWSLICERENDERER3D_H

#include "voreen/modules/flowreen/flowslicerenderer.h"

namespace voreen {

class CameraInteractionHandler;

/**
 * A processor for rendering 2D flow images on orthogonal slices in 3D using
 * various 2D flow visualization techniques.
 */
class FlowSliceRenderer3D : public FlowSliceRenderer {
public:
    FlowSliceRenderer3D();
    virtual ~FlowSliceRenderer3D();

    virtual Processor* create() const { return new FlowSliceRenderer3D(); }
    virtual std::string getModule() const { return "flowreen"; }
    virtual std::string getCategory() const { return "Flow Visualization"; }
    virtual std::string getClassName() const { return "FlowSliceRenderer3D"; }
    virtual CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual const std::string getProcessorInfo() const;
    virtual void process();

protected:
    virtual std::string generateShaderHeader() const;

private:
    inline void invalidateXYTexture() { rebuildTextureXY_ = true; }
    inline void invalidateXZTexture() { rebuildTextureXZ_ = true; }
    inline void invalidateZYTexture() { rebuildTextureZY_ = true; }

    static tgt::vec3 permuteComponents(const tgt::vec3& input, const tgt::ivec3& permutation);

    void renderSlice(const Flow3D& flow3D, const float sliceNo, const tgt::vec2& viewportSize,
        const SliceAlignment& sliceAlignment);

    void updateNumSlices();

private:
    BoolProperty useXYSliceProp_;
    BoolProperty useXZSliceProp_;
    BoolProperty useZYSliceProp_;
    IntProperty sliceNoXYProp_;
    IntProperty sliceNoXZProp_;
    IntProperty sliceNoZYProp_;
    CameraProperty camProp_;
    CameraInteractionHandler* cameraHandler_;

    tgt::Texture* textureXY_;
    tgt::Texture* textureXZ_;
    tgt::Texture* textureZY_;

    bool rebuildTextureXY_;
    bool rebuildTextureXZ_;
    bool rebuildTextureZY_;

    FlowSliceRenderer::SliceAlignment alignment_;
};

}   // namespace

#endif  // VRN_FLOWSLICERENDERER3D_H
#endif  // VRN_MODULE_FLOWREEN
