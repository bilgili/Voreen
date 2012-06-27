#ifdef VRN_MODULE_FLOWREEN
#ifndef VRN_FLOWSLICERENDERER2D_H
#define VRN_FLOWSLICERENDERER2D_H

#include "voreen/modules/flowreen/flowslicerenderer.h"

namespace voreen {

class FlowSliceRenderer2D : public FlowSliceRenderer {
public:
    FlowSliceRenderer2D();
    virtual ~FlowSliceRenderer2D();

    virtual Processor* create() const { return new FlowSliceRenderer2D(); }

    virtual std::string getModule() const { return "flowreen"; }
    virtual std::string getCategory() const { return "Flow Visualization"; }
    virtual std::string getClassName() const { return "FlowSliceRenderer2D"; }
    virtual CodeState getCodeState() const  { return CODE_STATE_STABLE; }
    virtual const std::string getProcessorInfo() const;
    virtual void process();

protected:
    virtual std::string generateShaderHeader() const;

private:
    float getTextureScalingFactor(const tgt::vec2& viewportSize, const tgt::vec2& textureSize);
    inline void invalidateTexture() { rebuildTexture_ = true; }
    void onSliceAlignmentChange();
    void onTextureZoomChange();
    void updateNumSlices();

private:
    /** Property containing the available alignments (axial, coronal, sagittal) */
    OptionProperty<SliceAlignment>* alignmentProp_;

    /** Property containing the currently selected slice */
    IntProperty sliceIndexProp_;

    /** Property defining the current texture magnification */
    OptionProperty<TextureZoom>* textureZoomProp_;

    tgt::Texture* sliceTexture_;    /** the current 2D texture containing the flow image */
    SliceAlignment alignment_;      /** the currently used slice alignment */
    int numSlices_;                 /** number of slices being available in the current alignment */
};

}   // namespace

#endif  // VRN_FLOWSLICERENDERER2D_H
#endif  // VRN_MODULE_FLOWREEN

