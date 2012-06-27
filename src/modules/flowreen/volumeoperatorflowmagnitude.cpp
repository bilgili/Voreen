#include "voreen/modules/flowreen/volumeoperatorflowmagnitude.h"

namespace voreen {

template<typename T>
VolumeUInt8* VolumeOperatorFlowMagnitude::apply_internal(const VolumeAtomic<T>* const volume) const {
    const VolumeFlow3D* const v = dynamic_cast<const VolumeFlow3D* const>(volume);
    if (v == 0)
        return 0;

    const Flow3D& flow = v->getFlow3D();
    const size_t numVoxels = v->getNumVoxels();
    unsigned char* magnitudes = new unsigned char[numVoxels];
    for (size_t i = 0; i < numVoxels; ++i) {
        float m = (tgt::length(flow.flow3D_[i]) / flow.maxMagnitude_) * 255.0f;
        magnitudes[i] = static_cast<unsigned char>(m);
    }

    return new VolumeUInt8(magnitudes, flow.dimensions_);
}

template class VolumeOperatorUnary<VolumeOperatorFlowMagnitude>;

}   // namespace
