#ifndef VRN_FLOWMAGNITUDES3D_H
#define VRN_FLOWMAGNITUDES3D_H

#include "voreen/core/vis/processors/processor.h"
#include "voreen/modules/flowreen/flowreenprocessor.h"

#include <string>

namespace voreen {

class VolumeHandle;

/**
 * A volume pre-processor which takes a volume containing flow data for input
 * and calculates an output volume containing only the flow magnitudes.
 * The resulting output then can be treated as any other volume and can be
 * rendered using Raycaster or SliceRenderer.
 */
class FlowMagnitudes3D : public Processor, private FlowreenProcessor {
public:
    FlowMagnitudes3D();
    virtual ~FlowMagnitudes3D();

    virtual Processor* create() const { return new FlowMagnitudes3D(); }
    virtual std::string getCategory() const { return "Flow Visualization"; }
    virtual std::string getClassName() const { return "FlowMagnitudes3D"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual std::string getModuleName() const { return "flowreen"; }
    virtual const std::string getProcessorInfo() const;
    virtual void process();

protected:
    void initialize() throw (VoreenException);

private:
    void calculateMagnitudes();

private:
    VolumeHandle* processedVolumeHandle_;   /** The volume containing the magnitudes. */

    VolumePort volInport_;
    VolumePort volOutport_;
};

}   //namespace

#endif
