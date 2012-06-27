/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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
