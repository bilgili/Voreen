#ifndef VRN_FLOWSTREAMLINES3D_H
#define VRN_FLOWSTREAMLINES3D_H

#include <string>
#include "voreen/core/vis/processors/processor.h"
#include "voreen/modules/flowreen/flowreenprocessor.h"
#include "voreen/core/vis/properties/boolproperty.h"
#include "voreen/core/vis/properties/intproperty.h"

namespace voreen {

class VolumeHandle;

class FlowStreamlinesTexture3D : public Processor, private FlowreenProcessor {
public:
    FlowStreamlinesTexture3D();
    virtual ~FlowStreamlinesTexture3D();

    virtual Processor* create() const { return new FlowStreamlinesTexture3D(); }
    virtual std::string getCategory() const { return "Flow Visualization"; }
    virtual std::string getClassName() const { return "StreamlineTexture3D"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual std::string getModuleName() const { return "flowreen"; }
    virtual const std::string getProcessorInfo() const;

    virtual void process();

private:
    void calculateStreamlines();

private:
    VolumeHandle* processedVolumeHandle_;
    IntProperty voxelSamplingProp_;

    VolumePort volInport_;
    VolumePort volOutport_;
};

}   // namespace

#endif
