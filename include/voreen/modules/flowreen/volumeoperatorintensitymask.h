#ifndef VRN_VOLUMEOPERATORINTENSITYMASK_H
#define VRN_VOLUMEOPERATORINTENSITYMASK_H

#include "tgt/vector.h"
#include "voreen/core/volume/volumeoperator.h"

namespace voreen {

/**
 * A VolumeOperator which returns all voxel positions having values within the
 * thresholds being passed to the ctor.
 * Note that this currently works only for volumes containing intensities, i.e.
 * the template parameter T for the underlying VolumeAtomic<T> may not be of any type
 * which cannot be compared to or casted to float.
 */
class VolumeOperatorIntensityMask : public VolumeOperatorUnary<VolumeOperatorIntensityMask> {
friend class VolumeOperatorUnary<VolumeOperatorIntensityMask>;

public:
    VolumeOperatorIntensityMask(const tgt::vec2& thresholds);

private:
    template<typename T>
    VolumeAtomic<bool>* apply_internal(const VolumeAtomic<T>* const volume) const;

    template<typename S>
    VolumeAtomic<bool>* apply_internal(const VolumeAtomic<tgt::Vector2<S> >* const) const {
        // method currently not really implemented because it is not required...
        return 0;
    }

    template<typename S>
    VolumeAtomic<bool>* apply_internal(const VolumeAtomic<tgt::Vector3<S> >* const) const {
        // method currently not really implemented because it is not required...
        return 0;
    }

    template<typename S>
    VolumeAtomic<bool>* apply_internal(const VolumeAtomic<tgt::Vector4<S> >* const) const {
        // method currently not really implemented because it is not required...
        return 0;
    }

    tgt::vec2 thresholds_;
};

}   // namespace

#endif
