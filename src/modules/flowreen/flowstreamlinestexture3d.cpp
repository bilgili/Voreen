#include "voreen/modules/flowreen/flowstreamlinestexture3d.h"
#include "voreen/modules/flowreen/streamlinetexture.h"
#include "voreen/modules/flowreen/volumeflow3d.h"

#include "voreen/core/volume/volumehandle.h"

namespace voreen {

FlowStreamlinesTexture3D::FlowStreamlinesTexture3D()
    : Processor(),
    processedVolumeHandle_(0),
    voxelSamplingProp_("voxelSampling", "voxel sampling: ", 10, 1, 100000),
    volInport_(Port::INPORT, "volumehandle.input"),
    volOutport_(Port::OUTPORT, "volumehandle.output", processedVolumeHandle_)
{

    CallMemberAction<FlowStreamlinesTexture3D> cma(this, &FlowStreamlinesTexture3D::calculateStreamlines);
    maxStreamlineLengthProp_.onChange(cma);
    thresholdProp_.onChange(cma);
    voxelSamplingProp_.onChange(cma);

    addProperty(maxStreamlineLengthProp_);
    addProperty(thresholdProp_);
    addProperty(voxelSamplingProp_);

    addPort(volInport_);
    addPort(volOutport_);
}

FlowStreamlinesTexture3D::~FlowStreamlinesTexture3D() {
    if ((processedVolumeHandle_ != 0) && (processedVolumeHandle_ != currentVolumeHandle_))
        delete processedVolumeHandle_;
}

const std::string FlowStreamlinesTexture3D::getProcessorInfo() const {
    return std::string("Calculates a texture containing streamlines of the incomming \
Flow Volume and outputs it into a new Volume which can be used as a normal 3D texture.");
}

void FlowStreamlinesTexture3D::process() {
    if (volInport_.isReady() && volInport_.hasChanged())
        calculateStreamlines();
}

// private methods
//

void FlowStreamlinesTexture3D::calculateStreamlines() {
    currentVolumeHandle_ = volInport_.getData();
    if (currentVolumeHandle_ == 0)
        return;

    VolumeFlow3D* input = dynamic_cast<VolumeFlow3D*>(currentVolumeHandle_->getVolume());
    if (input == 0) {
        LERROR("process(): supplied VolumeHandle seems to contain no flow data! Cannot proceed.");
        return;
    }

    const size_t voxelSampling = static_cast<size_t>(voxelSamplingProp_.get());
    const Flow3D& flow = input->getFlow3D();
    tgt::vec2 thresholds(flow.maxMagnitude_ * thresholdProp_.get() / 100.0f);

    const int textureScaling = 1;
    unsigned char* streamlineTexture =
        StreamlineTexture<unsigned char>::integrateDraw(flow, textureScaling, voxelSampling, thresholds);

    Volume* output = new VolumeUInt8(streamlineTexture, flow.dimensions_ * textureScaling);
    if ((processedVolumeHandle_ != 0)
        && (processedVolumeHandle_ != currentVolumeHandle_)) {
            delete processedVolumeHandle_;
    }

    if (output != 0)
        processedVolumeHandle_ = new VolumeHandle(output, 0.0f);
    else
        processedVolumeHandle_ = 0;
    volOutport_.setData(processedVolumeHandle_);
}

}   // namespace

