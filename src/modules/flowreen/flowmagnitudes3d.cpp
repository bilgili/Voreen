#include "voreen/modules/flowreen/flowmagnitudes3d.h"
#include "voreen/modules/flowreen/volumeflow3d.h"
#include "voreen/modules/flowreen/volumeoperatorflowmagnitude.h"

#include "voreen/core/volume/volumehandle.h"

namespace voreen {

FlowMagnitudes3D::FlowMagnitudes3D()
    : Processor(),
    processedVolumeHandle_(0),
    volInport_(Port::INPORT, "volumehandle.input"),
    volOutport_(Port::OUTPORT, "volumehandle.output", true)
{
    addPort(volInport_);
    addPort(volOutport_);
    volOutport_.setData(processedVolumeHandle_);
}

FlowMagnitudes3D::~FlowMagnitudes3D() {
    if ((processedVolumeHandle_ != 0) && (processedVolumeHandle_ != currentVolumeHandle_))
        delete processedVolumeHandle_;
}

const std::string FlowMagnitudes3D::getProcessorInfo() const {
    return std::string("Calculates the magnitudes of the incomming Flow Volume and \
outputs them into a new Volume which can be used as a normal 3D texture.");
}

void FlowMagnitudes3D::process() {
    if (volInport_.isReady() && volInport_.hasChanged()) {
        currentVolumeHandle_ = volInport_.getData();
        if (dynamic_cast<VolumeFlow3D*>(currentVolumeHandle_->getVolume()) != 0)
            calculateMagnitudes();
        else
            LERROR("process(): supplied VolumeHandle seems to contain no flow data! Cannot proceed.");
    }
}

void FlowMagnitudes3D::initialize() throw (VoreenException) {
    FlowreenProcessor::init();
    Processor::initialize();
}

// private methods
//

void FlowMagnitudes3D::calculateMagnitudes() {
    VolumeOperatorFlowMagnitude magnitudeOp;
    Volume* input = currentVolumeHandle_->getVolume();
    Volume* output = magnitudeOp.apply<VolumeUInt8*>(input);

    if ((processedVolumeHandle_ != 0)
        && processedVolumeHandle_ != currentVolumeHandle_)
    {
            delete processedVolumeHandle_;
    }

    if (output != 0)
        processedVolumeHandle_ = new VolumeHandle(output, 0.0f);
    else
        processedVolumeHandle_ = 0;
    volOutport_.setData(processedVolumeHandle_);
}

}   // namespace
