#ifdef VRN_MODULE_FLOWREEN
#ifndef VRN_STREAMLINERENDERER3D_H
#define VRN_STREAMLINERENDERER3D_H

#include "tgt/shadermanager.h"
#include "voreen/core/vis/processors/renderprocessor.h"
#include "voreen/core/volume/volumehandle.h"
#include "voreen/modules/flowreen/colorcodingability.h"
#include "voreen/modules/flowreen/flowreenprocessor.h"

namespace voreen {

class Flow3D;
class CameraInteractionHandler;

/**
 * Performs rendering of streamlines from a stationary input flow volume by
 * using geometric primitives like lines, tubes or arrows.
 */
class StreamlineRenderer3D : public RenderProcessor, public FlowreenProcessor
{
public:
    StreamlineRenderer3D();
    virtual ~StreamlineRenderer3D();

    virtual Processor* create() const { return new StreamlineRenderer3D(); }

    virtual std::string getCategory() const { return "Flow Visualization"; }
    virtual std::string getClassName() const { return "StreamlineRenderer3D"; }
    virtual std::string getModuleName() const { return "flowreen"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual const std::string getProcessorInfo() const;
    virtual void initialize() throw (VoreenException);
    virtual void process();

private:
    void buildDisplayLists(const Flow3D& flow, const tgt::vec2& thresholds);
    void buildDisplayListsArrowGrid(const Flow3D& flow);

    void initSeedingPositions();

    void invalidateRendering();

    void onColorCodingChange();
    void onStreamlineNumberChange();
    void onStyleChange();

    void renderStreamlineArrows(const std::vector<tgt::vec3>& streamline) const;
    void renderStreamlineLines(const std::vector<tgt::vec3>& streamline, const Flow3D& flow) const;
    void renderStreamlineTubes(const std::vector<tgt::vec3>& streamline) const;

    tgt::vec3 reseedPosition(const tgt::vec3& flowDimensions, const size_t validPositions = 0);

    void setPropertyVisibilities();

    bool setupShader(const Flow3D& flow, const tgt::vec2& thresholds);

private:
    enum StreamlineStyle {
        STYLE_LINES,
        STYLE_TUBES,
        STYLE_ARROWS,
        STYLE_ARROW_GRID
    };

    ColorCodingAbility colorCoding_;
    IntProperty numStreamlinesProp_;
    OptionProperty<StreamlineStyle>* styleProp_;
    IntProperty geometrySpacingProp_;
    IntProperty geometrySizeProp_;
    BoolProperty useAlphaBlendingProp_;
    CameraProperty camProp_;
    CameraInteractionHandler* cameraHandler_;

    StreamlineStyle currentStyle_;
    size_t numStreamlines_;
    bool rebuildDisplayLists_;
    bool reinitSeedingPositions_;
    GLuint displayLists_;
    tgt::Shader* shader_;
    tgt::vec3* seedingPositions_;

    VolumePort volInport_;
    RenderPort imgOutport_;
};

}   // namespace

#endif  // VRN_STREAMLINERENDERER3D_H
#endif  // VRN_MODULE_FLOWREEN
