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

#include "voreen/modules/flowreen/volumeoperatorflowmagnitude.h"

namespace voreen {

/*template<typename T>
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

template class VolumeOperatorUnary<VolumeOperatorFlowMagnitude>;*/

}   // namespace
