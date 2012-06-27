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

#include "voreen/core/datastructures/volume/volumeminmax.h"

namespace voreen {

VolumeMinMax::VolumeMinMax()
    : VolumeDerivedData()
    , min_(0.f)
    , max_(0.f)
    , minNorm_(0.f)
    , maxNorm_(0.f)
{}

VolumeMinMax::VolumeMinMax(float min, float max, float minNorm, float maxNorm)
    : VolumeDerivedData()
    , min_(min)
    , max_(max)
    , minNorm_(minNorm)
    , maxNorm_(maxNorm)
{}

VolumeDerivedData* VolumeMinMax::createFrom(const VolumeBase* handle) const {
    tgtAssert(handle, "no volume handle");

    const VolumeRAM* v = handle->getRepresentation<VolumeRAM>();
    tgtAssert(v, "no volume");

    float minNorm = v->minNormalizedValue();
    float maxNorm = v->maxNormalizedValue();
    float min = handle->getRealWorldMapping().normalizedToRealWorld(minNorm);
    float max = handle->getRealWorldMapping().normalizedToRealWorld(maxNorm);
    return new VolumeMinMax(min, max, minNorm, maxNorm);
}

void VolumeMinMax::serialize(XmlSerializer& s) const  {
    s.serialize("min", min_);
    s.serialize("max", max_);
    s.serialize("minNorm", minNorm_);
    s.serialize("maxNorm", maxNorm_);
}

void VolumeMinMax::deserialize(XmlDeserializer& s) {
    s.deserialize("min", min_);
    s.deserialize("max", max_);
    s.deserialize("minNorm", minNorm_);
    s.deserialize("maxNorm", maxNorm_);
}

} // namespace voreen
