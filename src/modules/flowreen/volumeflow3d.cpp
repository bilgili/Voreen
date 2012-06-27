#include "voreen/modules/flowreen/volumeflow3d.h"

namespace voreen {

VolumeFlow3D::VolumeFlow3D(tgt::vec3* const voxels, const tgt::ivec3& dimensions,
                           const float min, const float max, const float maxMagnitude)
    : VolumeAtomic<tgt::vec3>(voxels, dimensions),
    flow3D_(voxels, dimensions, Flow3D::XYZ, min, max, maxMagnitude)
{
}

}   // namespace
