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

#include "voreen/core/animation/interpolation/volumehandleinterpolationfunctions.h"

namespace voreen {

VolumeHandleStartInterpolationFunction::VolumeHandleStartInterpolationFunction() {
}
std::string VolumeHandleStartInterpolationFunction::getMode() const {
    return "focus on startvalue";
}
std::string VolumeHandleStartInterpolationFunction::getIdentifier() const {
    return "boolean";
}
VolumeHandle* VolumeHandleStartInterpolationFunction::interpolate(VolumeHandle* startvalue, VolumeHandle* endvalue, float time) const {
    if (time<1){
        return startvalue;
    }
    else{
        return endvalue;
    }
}
InterpolationFunction<VolumeHandle*>* VolumeHandleStartInterpolationFunction::clone() const{
    return new VolumeHandleStartInterpolationFunction();
}


VolumeHandleEndInterpolationFunction::VolumeHandleEndInterpolationFunction() {
}
std::string VolumeHandleEndInterpolationFunction::getMode() const {
    return "focus on endvalue";
}
std::string VolumeHandleEndInterpolationFunction::getIdentifier() const {
    return "boolean";
}
VolumeHandle* VolumeHandleEndInterpolationFunction::interpolate(VolumeHandle* startvalue, VolumeHandle* endvalue, float time) const {
    if (time>0){
        return endvalue;
    }
    else{
        return startvalue;
    }
}
InterpolationFunction<VolumeHandle*>* VolumeHandleEndInterpolationFunction::clone() const{
    return new VolumeHandleEndInterpolationFunction();
}


VolumeHandleStartEndInterpolationFunction::VolumeHandleStartEndInterpolationFunction() {
}
std::string VolumeHandleStartEndInterpolationFunction::getMode() const {
    return "bisection";
}
std::string VolumeHandleStartEndInterpolationFunction::getIdentifier() const {
    return "boolean";
}
VolumeHandle* VolumeHandleStartEndInterpolationFunction::interpolate(VolumeHandle* startvalue, VolumeHandle* endvalue, float time) const {
    if (time<0.5){
        return startvalue;
    }
    else{
        return endvalue;
    }
}
InterpolationFunction<VolumeHandle*>* VolumeHandleStartEndInterpolationFunction::clone() const{
    return new VolumeHandleStartEndInterpolationFunction();
}

} // namespace voreen

