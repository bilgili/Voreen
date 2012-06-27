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
#include "voreen/core/animation/interpolation/ivec2interpolationfunctions.h"

namespace voreen {

IVec2StartInterpolationFunction::IVec2StartInterpolationFunction() {}

std::string IVec2StartInterpolationFunction::getMode() const {
    return "focus on startvalue";
}

std::string IVec2StartInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::ivec2 IVec2StartInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    if (time < 1.f)
        return startvalue;
    else
        return endvalue;
}
InterpolationFunction<tgt::ivec2>* IVec2StartInterpolationFunction::clone() const {
    return new IVec2StartInterpolationFunction();
}

IVec2EndInterpolationFunction::IVec2EndInterpolationFunction() {}

std::string IVec2EndInterpolationFunction::getMode() const {
    return "focus on endvalue";
}

std::string IVec2EndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::ivec2 IVec2EndInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    if (time > 0.f)
        return endvalue;
    else
        return startvalue;
}

InterpolationFunction<tgt::ivec2>* IVec2EndInterpolationFunction::clone() const {
    return new IVec2EndInterpolationFunction();
}

IVec2StartEndInterpolationFunction::IVec2StartEndInterpolationFunction() {}

std::string IVec2StartEndInterpolationFunction::getMode() const {
    return "bisection";
}

std::string IVec2StartEndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::ivec2 IVec2StartEndInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    if (time < 0.5f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<tgt::ivec2>* IVec2StartEndInterpolationFunction::clone() const {
    return new IVec2StartEndInterpolationFunction();
}

IVec2LinearInterpolationFunction::IVec2LinearInterpolationFunction() {}

std::string IVec2LinearInterpolationFunction::getMode() const {
    return "linear interpolation";
}

std::string IVec2LinearInterpolationFunction::getIdentifier() const {
    return "linear";
}

tgt::ivec2 IVec2LinearInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2LinearInterpolationFunction::clone() const {
    return new IVec2LinearInterpolationFunction();
}

IVec2InQuadInterpolationFunction::IVec2InQuadInterpolationFunction() {}

std::string IVec2InQuadInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec2InQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::ivec2 IVec2InQuadInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inQuadInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inQuadInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InQuadInterpolationFunction::clone() const {
    return new IVec2InQuadInterpolationFunction();
}

IVec2InCubicInterpolationFunction::IVec2InCubicInterpolationFunction() {}

std::string IVec2InCubicInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec2InCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::ivec2 IVec2InCubicInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inCubicInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inCubicInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InCubicInterpolationFunction::clone() const {
    return new IVec2InCubicInterpolationFunction();
}

IVec2InQuartInterpolationFunction::IVec2InQuartInterpolationFunction() {}

std::string IVec2InQuartInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec2InQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::ivec2 IVec2InQuartInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inQuartInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inQuartInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InQuartInterpolationFunction::clone() const {
    return new IVec2InQuartInterpolationFunction();
}

IVec2InQuintInterpolationFunction::IVec2InQuintInterpolationFunction() {}

std::string IVec2InQuintInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec2InQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::ivec2 IVec2InQuintInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inQuintInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inQuintInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InQuintInterpolationFunction::clone() const {
    return new IVec2InQuintInterpolationFunction();
}

IVec2InSineInterpolationFunction::IVec2InSineInterpolationFunction() {}

std::string IVec2InSineInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec2InSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::ivec2 IVec2InSineInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inSineInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inSineInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InSineInterpolationFunction::clone() const{
    return new IVec2InSineInterpolationFunction();
}

IVec2InExponentInterpolationFunction::IVec2InExponentInterpolationFunction() {}

std::string IVec2InExponentInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec2InExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::ivec2 IVec2InExponentInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inExponentInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inExponentInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InExponentInterpolationFunction::clone() const {
    return new IVec2InExponentInterpolationFunction();
}

IVec2InCircInterpolationFunction::IVec2InCircInterpolationFunction() {}

std::string IVec2InCircInterpolationFunction::getMode() const {
    return "easing in";
}

std::string IVec2InCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::ivec2 IVec2InCircInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inCircInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inCircInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InCircInterpolationFunction::clone() const {
    return new IVec2InCircInterpolationFunction();
}

IVec2OutQuadInterpolationFunction::IVec2OutQuadInterpolationFunction() {}

std::string IVec2OutQuadInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec2OutQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::ivec2 IVec2OutQuadInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outQuadInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outQuadInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutQuadInterpolationFunction::clone() const {
    return new IVec2OutQuadInterpolationFunction();
}

IVec2OutCubicInterpolationFunction::IVec2OutCubicInterpolationFunction() {}

std::string IVec2OutCubicInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec2OutCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::ivec2 IVec2OutCubicInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outCubicInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outCubicInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutCubicInterpolationFunction::clone() const {
    return new IVec2OutCubicInterpolationFunction();
}

IVec2OutQuartInterpolationFunction::IVec2OutQuartInterpolationFunction() {}

std::string IVec2OutQuartInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec2OutQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::ivec2 IVec2OutQuartInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outQuartInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outQuartInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutQuartInterpolationFunction::clone() const {
    return new IVec2OutQuartInterpolationFunction();
}

IVec2OutQuintInterpolationFunction::IVec2OutQuintInterpolationFunction() {}

std::string IVec2OutQuintInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec2OutQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::ivec2 IVec2OutQuintInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outQuintInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outQuintInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutQuintInterpolationFunction::clone() const {
    return new IVec2OutQuintInterpolationFunction();
}

IVec2OutSineInterpolationFunction::IVec2OutSineInterpolationFunction() {}

std::string IVec2OutSineInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec2OutSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::ivec2 IVec2OutSineInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outSineInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outSineInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutSineInterpolationFunction::clone() const {
    return new IVec2OutSineInterpolationFunction();
}

IVec2OutExponentInterpolationFunction::IVec2OutExponentInterpolationFunction() {}

std::string IVec2OutExponentInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec2OutExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::ivec2 IVec2OutExponentInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outExponentInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outExponentInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutExponentInterpolationFunction::clone() const {
    return new IVec2OutExponentInterpolationFunction();
}

IVec2OutCircInterpolationFunction::IVec2OutCircInterpolationFunction() {}

std::string IVec2OutCircInterpolationFunction::getMode() const {
    return "easing out";
}

std::string IVec2OutCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::ivec2 IVec2OutCircInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outCircInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outCircInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutCircInterpolationFunction::clone() const {
    return new IVec2OutCircInterpolationFunction();
}

IVec2InOutQuadInterpolationFunction::IVec2InOutQuadInterpolationFunction() {}

std::string IVec2InOutQuadInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec2InOutQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::ivec2 IVec2InOutQuadInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutQuadInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutQuadInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InOutQuadInterpolationFunction::clone() const {
    return new IVec2InOutQuadInterpolationFunction();
}

IVec2InOutCubicInterpolationFunction::IVec2InOutCubicInterpolationFunction() {}

std::string IVec2InOutCubicInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec2InOutCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::ivec2 IVec2InOutCubicInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutCubicInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutCubicInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InOutCubicInterpolationFunction::clone() const{
    return new IVec2InOutCubicInterpolationFunction();
}

IVec2InOutQuartInterpolationFunction::IVec2InOutQuartInterpolationFunction() {}

std::string IVec2InOutQuartInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec2InOutQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::ivec2 IVec2InOutQuartInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutQuartInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutQuartInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InOutQuartInterpolationFunction::clone() const {
    return new IVec2InOutQuartInterpolationFunction();
}

IVec2InOutQuintInterpolationFunction::IVec2InOutQuintInterpolationFunction() {}

std::string IVec2InOutQuintInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec2InOutQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::ivec2 IVec2InOutQuintInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutQuintInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutQuintInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InOutQuintInterpolationFunction::clone() const {
    return new IVec2InOutQuintInterpolationFunction();
}

IVec2InOutSineInterpolationFunction::IVec2InOutSineInterpolationFunction() {}

std::string IVec2InOutSineInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec2InOutSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::ivec2 IVec2InOutSineInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutSineInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutSineInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InOutSineInterpolationFunction::clone() const {
    return new IVec2InOutSineInterpolationFunction();
}

IVec2InOutExponentInterpolationFunction::IVec2InOutExponentInterpolationFunction() {}

std::string IVec2InOutExponentInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec2InOutExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::ivec2 IVec2InOutExponentInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutExponentInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutExponentInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InOutExponentInterpolationFunction::clone() const {
    return new IVec2InOutExponentInterpolationFunction();
}

IVec2InOutCircInterpolationFunction::IVec2InOutCircInterpolationFunction() {}

std::string IVec2InOutCircInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string IVec2InOutCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::ivec2 IVec2InOutCircInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::inOutCircInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::inOutCircInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2InOutCircInterpolationFunction::clone() const {
    return new IVec2InOutCircInterpolationFunction();
}

IVec2OutInQuadInterpolationFunction::IVec2OutInQuadInterpolationFunction() {}

std::string IVec2OutInQuadInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec2OutInQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::ivec2 IVec2OutInQuadInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInQuadInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInQuadInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutInQuadInterpolationFunction::clone() const {
    return new IVec2OutInQuadInterpolationFunction();
}

IVec2OutInCubicInterpolationFunction::IVec2OutInCubicInterpolationFunction() {}

std::string IVec2OutInCubicInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec2OutInCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::ivec2 IVec2OutInCubicInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInCubicInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInCubicInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutInCubicInterpolationFunction::clone() const {
    return new IVec2OutInCubicInterpolationFunction();
}

IVec2OutInQuartInterpolationFunction::IVec2OutInQuartInterpolationFunction() {}

std::string IVec2OutInQuartInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec2OutInQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::ivec2 IVec2OutInQuartInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInQuartInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInQuartInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutInQuartInterpolationFunction::clone() const {
    return new IVec2OutInQuartInterpolationFunction();
}

IVec2OutInQuintInterpolationFunction::IVec2OutInQuintInterpolationFunction() {}

std::string IVec2OutInQuintInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec2OutInQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::ivec2 IVec2OutInQuintInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInQuintInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInQuintInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutInQuintInterpolationFunction::clone() const {
    return new IVec2OutInQuintInterpolationFunction();
}

IVec2OutInSineInterpolationFunction::IVec2OutInSineInterpolationFunction() {}

std::string IVec2OutInSineInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec2OutInSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::ivec2 IVec2OutInSineInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInSineInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInSineInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutInSineInterpolationFunction::clone() const {
    return new IVec2OutInSineInterpolationFunction();
}

IVec2OutInExponentInterpolationFunction::IVec2OutInExponentInterpolationFunction() {}

std::string IVec2OutInExponentInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec2OutInExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::ivec2 IVec2OutInExponentInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInExponentInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInExponentInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutInExponentInterpolationFunction::clone() const {
    return new IVec2OutInExponentInterpolationFunction();
}

IVec2OutInCircInterpolationFunction::IVec2OutInCircInterpolationFunction() {}

std::string IVec2OutInCircInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string IVec2OutInCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::ivec2 IVec2OutInCircInterpolationFunction::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::outInCircInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::outInCircInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

InterpolationFunction<tgt::ivec2>* IVec2OutInCircInterpolationFunction::clone() const {
    return new IVec2OutInCircInterpolationFunction();
}

} // namespace voreen
