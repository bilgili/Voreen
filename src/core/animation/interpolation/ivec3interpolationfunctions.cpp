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

#include "voreen/core/animation/interpolation/basicintinterpolation.h"
#include "voreen/core/animation/interpolation/ivec3interpolationfunctions.h"

namespace voreen {

IVec3StartInterpolationFunction::IVec3StartInterpolationFunction() {}

std::string IVec3StartInterpolationFunction::getMode() const {
    return "focus on startvalue";
}

std::string IVec3StartInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::ivec3 IVec3StartInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    if (time < 1.f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<tgt::ivec3>* IVec3StartInterpolationFunction::clone() const {
    return new IVec3StartInterpolationFunction();
}

IVec3EndInterpolationFunction::IVec3EndInterpolationFunction() {}

std::string IVec3EndInterpolationFunction::getMode() const {
    return "focus on endvalue";
}

std::string IVec3EndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::ivec3 IVec3EndInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    if (time > 0.f)
        return endvalue;
    else
        return startvalue;
}

InterpolationFunction<tgt::ivec3>* IVec3EndInterpolationFunction::clone() const {
    return new IVec3EndInterpolationFunction();
}

IVec3StartEndInterpolationFunction::IVec3StartEndInterpolationFunction() {}

std::string IVec3StartEndInterpolationFunction::getMode() const {
    return "bisection";
}

std::string IVec3StartEndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::ivec3 IVec3StartEndInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    if (time < 0.5f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<tgt::ivec3>* IVec3StartEndInterpolationFunction::clone() const {
    return new IVec3StartEndInterpolationFunction();
}

IVec3LinearInterpolationFunction::IVec3LinearInterpolationFunction() {}

std::string IVec3LinearInterpolationFunction::getMode() const {
    return "linear interpolation";
}

std::string IVec3LinearInterpolationFunction::getIdentifier() const {
    return "linear";
}

tgt::ivec3 IVec3LinearInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3LinearInterpolationFunction::clone() const {
    return new IVec3LinearInterpolationFunction();
}

IVec3InQuadInterpolationFunction::IVec3InQuadInterpolationFunction() {}

std::string IVec3InQuadInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec3InQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::ivec3 IVec3InQuadInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inQuadInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inQuadInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inQuadInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InQuadInterpolationFunction::clone() const {
    return new IVec3InQuadInterpolationFunction();
}

IVec3InCubicInterpolationFunction::IVec3InCubicInterpolationFunction() {}

std::string IVec3InCubicInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec3InCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::ivec3 IVec3InCubicInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inCubicInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inCubicInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inCubicInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InCubicInterpolationFunction::clone() const {
    return new IVec3InCubicInterpolationFunction();
}

IVec3InQuartInterpolationFunction::IVec3InQuartInterpolationFunction() {}

std::string IVec3InQuartInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec3InQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::ivec3 IVec3InQuartInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inQuartInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inQuartInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inQuartInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InQuartInterpolationFunction::clone() const {
    return new IVec3InQuartInterpolationFunction();
}

IVec3InQuintInterpolationFunction::IVec3InQuintInterpolationFunction() {}

std::string IVec3InQuintInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec3InQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::ivec3 IVec3InQuintInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inQuintInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inQuintInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inQuintInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InQuintInterpolationFunction::clone() const {
    return new IVec3InQuintInterpolationFunction();
}

IVec3InSineInterpolationFunction::IVec3InSineInterpolationFunction() {}

std::string IVec3InSineInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec3InSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::ivec3 IVec3InSineInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inSineInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inSineInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inSineInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InSineInterpolationFunction::clone() const {
    return new IVec3InSineInterpolationFunction();
}

IVec3InExponentInterpolationFunction::IVec3InExponentInterpolationFunction() {}

std::string IVec3InExponentInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec3InExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::ivec3 IVec3InExponentInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inExponentInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inExponentInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inExponentInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InExponentInterpolationFunction::clone() const {
    return new IVec3InExponentInterpolationFunction();
}

IVec3InCircInterpolationFunction::IVec3InCircInterpolationFunction() {}

std::string IVec3InCircInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec3InCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::ivec3 IVec3InCircInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inCircInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inCircInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inCircInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InCircInterpolationFunction::clone() const {
    return new IVec3InCircInterpolationFunction();
}

IVec3OutQuadInterpolationFunction::IVec3OutQuadInterpolationFunction() {}

std::string IVec3OutQuadInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec3OutQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::ivec3 IVec3OutQuadInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outQuadInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outQuadInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outQuadInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutQuadInterpolationFunction::clone() const {
    return new IVec3OutQuadInterpolationFunction();
}

IVec3OutCubicInterpolationFunction::IVec3OutCubicInterpolationFunction() {}

std::string IVec3OutCubicInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec3OutCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::ivec3 IVec3OutCubicInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outCubicInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outCubicInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outCubicInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutCubicInterpolationFunction::clone() const {
    return new IVec3OutCubicInterpolationFunction();
}

IVec3OutQuartInterpolationFunction::IVec3OutQuartInterpolationFunction() {}

std::string IVec3OutQuartInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec3OutQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::ivec3 IVec3OutQuartInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outQuartInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outQuartInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outQuartInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutQuartInterpolationFunction::clone() const {
    return new IVec3OutQuartInterpolationFunction();
}

IVec3OutQuintInterpolationFunction::IVec3OutQuintInterpolationFunction() {}

std::string IVec3OutQuintInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec3OutQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::ivec3 IVec3OutQuintInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outQuintInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outQuintInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outQuintInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutQuintInterpolationFunction::clone() const {
    return new IVec3OutQuintInterpolationFunction();
}

IVec3OutSineInterpolationFunction::IVec3OutSineInterpolationFunction() {}

std::string IVec3OutSineInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec3OutSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::ivec3 IVec3OutSineInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outSineInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outSineInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outSineInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutSineInterpolationFunction::clone() const {
    return new IVec3OutSineInterpolationFunction();
}

IVec3OutExponentInterpolationFunction::IVec3OutExponentInterpolationFunction() {}

std::string IVec3OutExponentInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec3OutExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::ivec3 IVec3OutExponentInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outExponentInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outExponentInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outExponentInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutExponentInterpolationFunction::clone() const {
    return new IVec3OutExponentInterpolationFunction();
}

IVec3OutCircInterpolationFunction::IVec3OutCircInterpolationFunction() {}

std::string IVec3OutCircInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec3OutCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::ivec3 IVec3OutCircInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outCircInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outCircInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outCircInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutCircInterpolationFunction::clone() const {
    return new IVec3OutCircInterpolationFunction();
}

IVec3InOutQuadInterpolationFunction::IVec3InOutQuadInterpolationFunction() {}

std::string IVec3InOutQuadInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec3InOutQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::ivec3 IVec3InOutQuadInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutQuadInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutQuadInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inOutQuadInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InOutQuadInterpolationFunction::clone() const {
    return new IVec3InOutQuadInterpolationFunction();
}

IVec3InOutCubicInterpolationFunction::IVec3InOutCubicInterpolationFunction() {}

std::string IVec3InOutCubicInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec3InOutCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::ivec3 IVec3InOutCubicInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutCubicInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutCubicInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inOutCubicInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InOutCubicInterpolationFunction::clone() const {
    return new IVec3InOutCubicInterpolationFunction();
}

IVec3InOutQuartInterpolationFunction::IVec3InOutQuartInterpolationFunction() {}

std::string IVec3InOutQuartInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec3InOutQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::ivec3 IVec3InOutQuartInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutQuartInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutQuartInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inOutQuartInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InOutQuartInterpolationFunction::clone() const {
    return new IVec3InOutQuartInterpolationFunction();
}

IVec3InOutQuintInterpolationFunction::IVec3InOutQuintInterpolationFunction() {}

std::string IVec3InOutQuintInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec3InOutQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::ivec3 IVec3InOutQuintInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutQuintInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutQuintInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inOutQuintInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InOutQuintInterpolationFunction::clone() const {
    return new IVec3InOutQuintInterpolationFunction();
}

IVec3InOutSineInterpolationFunction::IVec3InOutSineInterpolationFunction() {}

std::string IVec3InOutSineInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec3InOutSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::ivec3 IVec3InOutSineInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutSineInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutSineInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inOutSineInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InOutSineInterpolationFunction::clone() const {
    return new IVec3InOutSineInterpolationFunction();
}

IVec3InOutExponentInterpolationFunction::IVec3InOutExponentInterpolationFunction() {}

std::string IVec3InOutExponentInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec3InOutExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::ivec3 IVec3InOutExponentInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutExponentInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutExponentInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inOutExponentInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InOutExponentInterpolationFunction::clone() const {
    return new IVec3InOutExponentInterpolationFunction();
}

IVec3InOutCircInterpolationFunction::IVec3InOutCircInterpolationFunction() {}

std::string IVec3InOutCircInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec3InOutCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::ivec3 IVec3InOutCircInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutCircInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutCircInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::inOutCircInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3InOutCircInterpolationFunction::clone() const {
    return new IVec3InOutCircInterpolationFunction();
}

IVec3OutInQuadInterpolationFunction::IVec3OutInQuadInterpolationFunction() {}

std::string IVec3OutInQuadInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec3OutInQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::ivec3 IVec3OutInQuadInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInQuadInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInQuadInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outInQuadInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutInQuadInterpolationFunction::clone() const {
    return new IVec3OutInQuadInterpolationFunction();
}

IVec3OutInCubicInterpolationFunction::IVec3OutInCubicInterpolationFunction() {}

std::string IVec3OutInCubicInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec3OutInCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::ivec3 IVec3OutInCubicInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInCubicInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInCubicInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outInCubicInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutInCubicInterpolationFunction::clone() const {
    return new IVec3OutInCubicInterpolationFunction();
}

IVec3OutInQuartInterpolationFunction::IVec3OutInQuartInterpolationFunction() {}

std::string IVec3OutInQuartInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec3OutInQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::ivec3 IVec3OutInQuartInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInQuartInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInQuartInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outInQuartInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutInQuartInterpolationFunction::clone() const {
    return new IVec3OutInQuartInterpolationFunction();
}

IVec3OutInQuintInterpolationFunction::IVec3OutInQuintInterpolationFunction() {}

std::string IVec3OutInQuintInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec3OutInQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::ivec3 IVec3OutInQuintInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInQuintInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInQuintInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outInQuintInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutInQuintInterpolationFunction::clone() const {
    return new IVec3OutInQuintInterpolationFunction();
}

IVec3OutInSineInterpolationFunction::IVec3OutInSineInterpolationFunction() {}

std::string IVec3OutInSineInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec3OutInSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::ivec3 IVec3OutInSineInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInSineInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInSineInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outInSineInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutInSineInterpolationFunction::clone() const {
    return new IVec3OutInSineInterpolationFunction();
}

IVec3OutInExponentInterpolationFunction::IVec3OutInExponentInterpolationFunction() {}

std::string IVec3OutInExponentInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec3OutInExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::ivec3 IVec3OutInExponentInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInExponentInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInExponentInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outInExponentInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutInExponentInterpolationFunction::clone() const {
    return new IVec3OutInExponentInterpolationFunction();
}

IVec3OutInCircInterpolationFunction::IVec3OutInCircInterpolationFunction() {}

std::string IVec3OutInCircInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec3OutInCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::ivec3 IVec3OutInCircInterpolationFunction::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInCircInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInCircInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::outInCircInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

InterpolationFunction<tgt::ivec3>* IVec3OutInCircInterpolationFunction::clone() const {
    return new IVec3OutInCircInterpolationFunction();
}

} // namespace voreen
