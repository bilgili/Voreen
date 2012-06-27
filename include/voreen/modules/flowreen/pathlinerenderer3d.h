#ifdef VRN_MODULE_FLOWREEN
#ifndef VRN_PATHLINERENDERER3D_H

#include <string>
#include "voreen/core/vis/processors/renderprocessor.h"
#include "voreen/modules/flowreen/flowreenprocessor.h"

namespace voreen {

class CameraInteractionHandler;
class Modality;
class OrthogonalSliceRenderer;

/**
 * Processor for rendering pathlines from time-dependent flow data using
 * geometrical primitives like points, lines, tubes and arrows.
 */
class PathlineRenderer3D : public RenderProcessor, private FlowreenProcessor {
public:
    PathlineRenderer3D();
    virtual ~PathlineRenderer3D();

    virtual Processor* create() const { return new PathlineRenderer3D(); }
    virtual std::string getModuleName() const { return "flowreen"; }
    virtual std::string getCategory() const { return "Flow Visualization"; }
    virtual std::string getClassName() const { return "PathlineRenderer3D"; }
    virtual Processor::CodeState getCodeState() const { return Processor::CODE_STATE_STABLE; }
    virtual const std::string getProcessorInfo() const;
    virtual void initialize() throw (VoreenException);
    virtual void invalidate(InvalidationLevel inv = INVALID_RESULT);
    virtual void process();

private:
    bool applyThresholds(std::vector<tgt::vec3>& pathline, const float& length) const;

    /**
     * Marks the data in the supplied VolumeSeries containing the contextual data by
     * checking whether the intesities fall within the range of the threshold.
     * Therefore Volumes of the same size consiting of bool are created in
     * intensityMasks_, indicating the test result.
     */
    size_t markContextData(const tgt::vec2& thresholds);

    /**
     * Verfies whether all input flow volumes are of the correct type (VolumeFlow3D)
     * and whether they are all of the same dimensions. The pointers to valid flow
     * volumes are stored in flows_ member which is replaced.
     *
     * @param   forceUpdate Determines whether to replace the existing flow volumes in
     *                      member flows_ or not.
     * @return  The number of valid flow volumes within flows_. If this member is empty,
     *          rendering cannot be performed and has to be canceled.
     */
    size_t checkFlows(const bool forceUpdate = false);

    void adjustTimestepProperty();
    void clearPathlines();
    void initPathlines(const size_t numPoints);
    void initPathlinesGrid(const size_t spacing);
    void initPathlinesSliceGrid(const size_t spacing);
    void onIntensityChange();
    void onLineStyleChange();
    void onSeedingStrategyChange();
    void onTimestepChange();
    void onThresholdingChange();

    void renderAsArrows(const tgt::vec4& lineColor);
    void renderAsLines(const tgt::vec4& lineColor);
    void renderAsLineSegments(const tgt::vec4& lineColor);
    void renderAsPoints(const tgt::vec4& lineColor);
    void renderAsTubes(const tgt::vec4& lineColor);

    bool setupShader();

private:
    static const std::string loggerCat_;

    enum LineStyle {
        STYLE_POINTS, 
        STYLE_LINES, 
        STYLE_TUBES, 
        STYLE_ARROWS, 
        STYLE_SEGMENTS
    };

    enum SeedingStrategy {
        SEED_RANDOM, 
        SEED_GRID, 
        SEED_SLICES_RANDOM, 
        SEED_SLICES_GRID
    };
    
    enum Thresholding { 
        THRESHOLDING_NONE, 
        THRESHOLDING_LINELENGTH, 
        THRESHOLDING_INTENSITY,
        THRESHOLDING_OR, 
        THRESHOLDING_AND
    };

    OptionProperty<SeedingStrategy>* seedingStrategyProp_;
    IntProperty numSeedpointsProp_;
    BoolProperty seedOnXYSliceProp_;
    BoolProperty seedOnXZSliceProp_;
    BoolProperty seedOnYZSliceProp_;
    IntProperty gridSpacingProp_;
    IntProperty objectSizeProp_;
    OptionProperty<LineStyle>* lineStyleProp_;
    OptionProperty<Thresholding>* thresholdingProp_;
    FloatVec2Property lineLengthProp_;
    FloatVec2Property intensityProp_;
    ColorProperty lineColorProp_;
    IntProperty segmentLengthProp_;
    FloatProperty integrationStepProp_;
    FloatProperty timestepProp_;          /** intended to be linked with a clock processor. */
    CameraProperty camProp_;
    CameraInteractionHandler* cameraHandler_;

    /** All flow volumes */
    std::vector<const Flow3D*> flows_;
    VolumeCollection* contextCollection_;
    VolumeCollection* flowCollection_;

    /** Bool Volumes holding "true" if the intensity at the location in the volume is
     * within the threshold range or "false" otherwise. */
    std::vector<VolumeAtomic<bool>*> intensityMasks_;

    tgt::Shader* shader_;

    LineStyle currentStyle_;
    SeedingStrategy seedingStrategy_;
    Thresholding thresholding_;
    size_t numPathlines_;
    std::vector<tgt::vec3>* pathlines_;
    size_t currentTimestep_;
    size_t previousTimestep_;
    tgt::ivec3 slicePositions_;

    GenericCoProcessorPort<OrthogonalSliceRenderer> coInport_;
    RenderPort imgOutport_;
    VolumeCollectionPort inportContext_;
    VolumeCollectionPort inportFlows_;
};

}   // namespace

#endif  // VRN_PATHLINERENDERER3D_H
#endif  // VRN_MODULE_FLOWREEN
