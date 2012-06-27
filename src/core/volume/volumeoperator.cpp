/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/volume/volumeoperator.h"

#include <limits>

namespace voreen {

// ============================================================================

template<typename T>
void VolumeOperatorIncrease::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i)
        voxel[i] += T(1);
}

// ============================================================================

template<typename T>
void VolumeOperatorDecrease::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i)
        voxel[i] -= T(1);
}

// ============================================================================

template<typename T>
T VolumeOperatorMinValue::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    T min = std::numeric_limits<T>::max();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        if (voxel[i] < min)
            min = voxel[i];
    }
    return min;
}

// specialized template version for tgt::VectorX clases which do no implement operator>().
//

template<typename S>
S VolumeOperatorMinValue::apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const {
    tgt::Vector2<S>* voxel = volume->voxel();
    S min = std::numeric_limits<S>::max();
    S voxelMin = min;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMin = tgt::min(voxel[i]);
        if (voxelMin < min)
            min = voxelMin;
    }
    return min;
}

template<typename S>
S VolumeOperatorMinValue::apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const {
    tgt::Vector3<S>* voxel = volume->voxel();
    S min = std::numeric_limits<S>::max();
    S voxelMin = min;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMin = tgt::min(voxel[i]);
        if (voxelMin < min)
            min = voxelMin;
    }
    return min;
}

template<typename S>
S VolumeOperatorMinValue::apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const {
    tgt::Vector4<S>* voxel = volume->voxel();
    S min = std::numeric_limits<S>::max();
    S voxelMin = min;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMin = tgt::min(voxel[i]);
        if (voxelMin < min)
            min = voxelMin;
    }
    return min;
}
// ============================================================================

template<typename T>
T VolumeOperatorMaxValue::apply_internal(VolumeAtomic<T>* volume) const {
    T* voxel = volume->voxel();
    T max = std::numeric_limits<T>::min();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        if (voxel[i] > max)
            max = voxel[i];
    }
    return max;
}

// specialized template version for tgt::VectorX clases which do no implement operator>().
//

template<typename S>
S VolumeOperatorMaxValue::apply_internal(VolumeAtomic<tgt::Vector2<S> >* volume) const {
    tgt::Vector2<S>* voxel = volume->voxel();
    S max = std::numeric_limits<S>::min();
    S voxelMax = max;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMax = tgt::max(voxel[i]);
        if (voxelMax > max)
            max = voxelMax;
    }
    return max;
}

template<typename S>
S VolumeOperatorMaxValue::apply_internal(VolumeAtomic<tgt::Vector3<S> >* volume) const {
    tgt::Vector3<S>* voxel = volume->voxel();
    S max = std::numeric_limits<S>::min();
    S voxelMax = max;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMax = tgt::max(voxel[i]);
        if (voxelMax > max)
            max = voxelMax;
    }
    return max;
}

template<typename S>
S VolumeOperatorMaxValue::apply_internal(VolumeAtomic<tgt::Vector4<S> >* volume) const {
    tgt::Vector4<S>* voxel = volume->voxel();
    S max = std::numeric_limits<S>::min();
    S voxelMax = max;

    for (size_t i = 0; i < volume->getNumVoxels(); ++i) {
        voxelMax = tgt::max(voxel[i]);
        if (voxelMax > max)
            max = voxelMax;
    }
    return max;
}

// ============================================================================

template<typename T>
void VolumeOperatorInvert::apply_internal(VolumeAtomic<T>* volume) const {
    VolumeOperatorMaxValue maxOp;
    //T max = maxOp.apply_internal(volume);
    T max = T(maxOp.apply<float>(volume));
    T* voxel = volume->voxel();
    for (size_t i = 0; i < volume->getNumVoxels(); ++i)
        voxel[i] = max - voxel[i];
}

template class VolumeOperatorUnary<VolumeOperatorIncrease>;
template class VolumeOperatorUnary<VolumeOperatorDecrease>;
template class VolumeOperatorUnary<VolumeOperatorMinValue>;
template class VolumeOperatorUnary<VolumeOperatorMaxValue>;
template class VolumeOperatorUnary<VolumeOperatorInvert>;

}   // namespace
