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

#include "voreen/core/animation/interpolation/volumecollectioninterpolationfunctions.h"

namespace voreen {

VolumeCollectionStartInterpolationFunction::VolumeCollectionStartInterpolationFunction() {}

std::string VolumeCollectionStartInterpolationFunction::getMode() const {
    return "focus on startvalue";
}

std::string VolumeCollectionStartInterpolationFunction::getIdentifier() const {
    return "boolean";
}

VolumeCollection* VolumeCollectionStartInterpolationFunction::interpolate(VolumeCollection* startvalue, VolumeCollection* endvalue, float time) const {
    if (time < 1.f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<VolumeCollection*>* VolumeCollectionStartInterpolationFunction::clone() const {
    return new VolumeCollectionStartInterpolationFunction();
}

VolumeCollectionEndInterpolationFunction::VolumeCollectionEndInterpolationFunction() {}

std::string VolumeCollectionEndInterpolationFunction::getMode() const {
    return "focus on endvalue";
}

std::string VolumeCollectionEndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

VolumeCollection* VolumeCollectionEndInterpolationFunction::interpolate(VolumeCollection* startvalue, VolumeCollection* endvalue, float time) const {
    if (time > 0.f)
        return endvalue;
    else
        return startvalue;
}

InterpolationFunction<VolumeCollection*>* VolumeCollectionEndInterpolationFunction::clone() const {
    return new VolumeCollectionEndInterpolationFunction();
}

VolumeCollectionStartEndInterpolationFunction::VolumeCollectionStartEndInterpolationFunction() {}

std::string VolumeCollectionStartEndInterpolationFunction::getMode() const {
    return "bisection";
}

std::string VolumeCollectionStartEndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

VolumeCollection* VolumeCollectionStartEndInterpolationFunction::interpolate(VolumeCollection* startvalue, VolumeCollection* endvalue, float time) const {
    if (time < 0.5f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<VolumeCollection*>* VolumeCollectionStartEndInterpolationFunction::clone() const {
    return new VolumeCollectionStartEndInterpolationFunction();
}

} // namespace voreen
