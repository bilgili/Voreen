#ifndef VRN_VOLUMEOPERATORFLOWMAGNITUDE_H
#define VRN_VOLUMEOPERATORFLOWMAGNITUDE_H

#include "voreen/core/volume/volumeoperator.h"

namespace voreen {

class VolumeOperatorFlowMagnitude : public VolumeOperatorUnary<VolumeOperatorFlowMagnitude> {

friend class VolumeOperatorUnary<VolumeOperatorFlowMagnitude>;

private:
    template<typename T>
    VolumeUInt8* apply_internal(const VolumeAtomic<T>* const volume) const;
};

}   // namespace

#endif
