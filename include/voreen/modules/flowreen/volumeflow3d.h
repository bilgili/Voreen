#ifdef VRN_MODULE_FLOWREEN
#ifndef VRN_FLOWVOLUME_H
#define VRN_FLOWVOLUME_H

#include "voreen/modules/flowreen/flow3d.h"
#include "voreen/core/volume/volumeatomic.h"

namespace voreen {

class VolumeFlow3D : public VolumeAtomic<tgt::vec3>
{
public:
    VolumeFlow3D(tgt::vec3* const voxels, const tgt::ivec3& dimensions,
        const float min = 0.0f, const float max = 1.0f, const float maxMagnitude = 1.0f);

    virtual ~VolumeFlow3D() {}

    const Flow3D& getFlow3D() const { return flow3D_; }

    float getMinValue() const { return flow3D_.minValue_; }
    float getMaxValue() const { return flow3D_.maxValue_; }
    float getMaxMagnitude() const { return flow3D_.maxMagnitude_; }

private:
    const Flow3D flow3D_;
};

}   // namespace

#endif  // VRN_FLOWVOLUME_H
#endif  // VRN_MODULES_FLOWREEN
