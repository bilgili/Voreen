/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_VOLUMEOPERATORSUBSET_H
#define VRN_VOLUMEOPERATORSUBSET_H

#include "voreen/core/datastructures/volume/volumeoperator.h"

namespace voreen {

/**
 * Returns a volume containing the subset [pos, pos+size[ of the passed input volume.
 */
class VolumeOperatorSubsetBase : public UnaryVolumeOperatorBase {
public:
    virtual Volume* apply(const VolumeBase* volume, tgt::ivec3 pos, tgt::ivec3 size, ProgressBar* progressBar = 0) const = 0;
};

// Generic implementation:
template<typename T>
class VolumeOperatorSubsetGeneric : public VolumeOperatorSubsetBase {
public:
    virtual Volume* apply(const VolumeBase* volume, tgt::ivec3 pos, tgt::ivec3 size, ProgressBar* progressBar = 0) const;
    //Implement isCompatible using a handy macro:
    IS_COMPATIBLE
};

template<typename T>
Volume* VolumeOperatorSubsetGeneric<T>::apply(const VolumeBase* vh, tgt::ivec3 pos, tgt::ivec3 size, ProgressBar* progressBar) const {
    const VolumeRAM* vol = vh->getRepresentation<VolumeRAM>();
    if(!vol)
        return 0;

    const VolumeAtomic<T>* volume = dynamic_cast<const VolumeAtomic<T>*>(vol);
    if(!volume)
        return 0;

    VolumeAtomic<T>* subset;
    try {
        subset = new VolumeAtomic<T>(size);
    }
    catch (std::bad_alloc) {
        LERRORC("voreen.VolumeOperatorCreateSubset", "Failed to create subset: bad allocation");
        throw; // throw it to the caller
    }

    LINFOC("voreen.VolumeOperatorCreateSubset", "Creating subset " << size << " from position " << pos);

    // create values for ranges less than zero and greater equal dimensions_
    subset->clear(); // TODO: This can be optomized by avoiding to clear the values in range

    // now the rest
    tgt::svec3 start = tgt::svec3(tgt::max(pos, tgt::ivec3::zero));// clamp values
    tgt::svec3 end   = tgt::svec3(tgt::min(pos + size, tgt::ivec3(volume->getDimensions())));    // clamp values
    tgt::svec3 diff  = end - start;

    VRN_FOR_EACH_VOXEL_WITH_PROGRESS(index, tgt::svec3(0, 0, 0), diff, progressBar)
        subset->voxel(index) = volume->voxel(index+start);

    Volume* newvh = new Volume(subset, vh);
    newvh->setOffset(vh->getOffset() + (tgt::vec3(start) * vh->getSpacing()));
    return newvh;
}

typedef UniversalUnaryVolumeOperatorGeneric<VolumeOperatorSubsetBase> VolumeOperatorSubset;

} // namespace

#endif // VRN_VOLUMEOPERATOR_H
