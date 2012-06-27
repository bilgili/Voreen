#include "voreen/modules/flowreen/volumeoperatorintensitymask.h"

namespace voreen {

VolumeOperatorIntensityMask::VolumeOperatorIntensityMask(const tgt::vec2& thresholds)
    : VolumeOperatorUnary<VolumeOperatorIntensityMask>(),
    thresholds_(thresholds)
{
}

template<typename T>
VolumeAtomic<bool>* VolumeOperatorIntensityMask::apply_internal(const voreen::VolumeAtomic<T>* const volume) const
{
    if (volume == 0)
        return 0;

    const tgt::ivec3 dim = volume->getDimensions();
    const T* const data = volume->voxel();

    VolumeAtomic<bool>* subVolume = new VolumeAtomic<bool>(dim);
    bool* const binVoxels = subVolume->voxel();

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        float value = static_cast<float>(data[i]);
        if ((value >= thresholds_.x) && (value <= thresholds_.y))
            binVoxels[i] = true;
        else
            binVoxels[i] = false;
    }   // for (i
    return subVolume;
}

template class VolumeOperatorUnary<VolumeOperatorIntensityMask>;

}   // namespace
