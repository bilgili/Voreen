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

#include "voreen/core/animation/interpolation/basicfloatinterpolation.h"
#include "voreen/core/animation/interpolation/vec2interpolationfunctions.h"

namespace voreen {

Vec2StartInterpolationFunction::Vec2StartInterpolationFunction() {}

std::string Vec2StartInterpolationFunction::getMode() const {
    return "focus on startvalue";
}

std::string Vec2StartInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::vec2 Vec2StartInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    if (time < 1.f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<tgt::vec2>* Vec2StartInterpolationFunction::clone() const {
    return new Vec2StartInterpolationFunction();
}

Vec2EndInterpolationFunction::Vec2EndInterpolationFunction() {}

std::string Vec2EndInterpolationFunction::getMode() const {
    return "focus on endvalue";
}

std::string Vec2EndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::vec2 Vec2EndInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    if (time > 0.f)
        return endvalue;
    else
        return startvalue;
}

InterpolationFunction<tgt::vec2>* Vec2EndInterpolationFunction::clone() const {
    return new Vec2EndInterpolationFunction();
}

Vec2StartEndInterpolationFunction::Vec2StartEndInterpolationFunction() {}

std::string Vec2StartEndInterpolationFunction::getMode() const {
    return "bisection";
}

std::string Vec2StartEndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::vec2 Vec2StartEndInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    if (time < 0.5f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<tgt::vec2>* Vec2StartEndInterpolationFunction::clone() const {
    return new Vec2StartEndInterpolationFunction();
}

Vec2LinearInterpolationFunction::Vec2LinearInterpolationFunction() {}

std::string Vec2LinearInterpolationFunction::getMode() const {
    return "linear interpolation";
}

std::string Vec2LinearInterpolationFunction::getIdentifier() const {
    return "linear";
}

tgt::vec2 Vec2LinearInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::linearInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::linearInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2LinearInterpolationFunction::clone() const {
    return new Vec2LinearInterpolationFunction();
}

Vec2SphericalLinearInterpolationFunction::Vec2SphericalLinearInterpolationFunction() {}

std::string Vec2SphericalLinearInterpolationFunction::getMode() const {
        return "spherical linear interpolation";
}

std::string Vec2SphericalLinearInterpolationFunction::getIdentifier() const {
        return "linear";
}

tgt::vec2 Vec2SphericalLinearInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
        float eps = 0.0001f;
        float dotVal = dot(normalize(startvalue), normalize(endvalue));
        if (dotVal > 0.995f){
            // small angle => linear interpolation
            float compX = BasicFloatInterpolation::linearInterpolation(startvalue.x, endvalue.x, time);
            float compY = BasicFloatInterpolation::linearInterpolation(startvalue.y, endvalue.y, time);
            return tgt::vec2(compX, compY);
        }
        float theta = std::acos(dotVal);
        float compX = (sin((1.f - time) * theta)/(sin(theta) + eps)) * startvalue.x
                      + (sin(time * theta)/(sin(theta) + eps)) * endvalue.x;
        float compY = (sin((1.f - time) * theta)/(sin(theta) + eps)) * startvalue.y
                      + (sin(time * theta)/(sin(theta) + eps)) * endvalue.y;
        return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2SphericalLinearInterpolationFunction::clone() const {
        return new Vec2SphericalLinearInterpolationFunction();
}

Vec2InQuadInterpolationFunction::Vec2InQuadInterpolationFunction() {}

std::string Vec2InQuadInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Vec2InQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::vec2 Vec2InQuadInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inQuadInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inQuadInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InQuadInterpolationFunction::clone() const {
    return new Vec2InQuadInterpolationFunction();
}

Vec2InCubicInterpolationFunction::Vec2InCubicInterpolationFunction() {}

std::string Vec2InCubicInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Vec2InCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::vec2 Vec2InCubicInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inCubicInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inCubicInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InCubicInterpolationFunction::clone() const {
    return new Vec2InCubicInterpolationFunction();
}

Vec2InQuartInterpolationFunction::Vec2InQuartInterpolationFunction() {}

std::string Vec2InQuartInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Vec2InQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::vec2 Vec2InQuartInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inQuartInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inQuartInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InQuartInterpolationFunction::clone() const {
    return new Vec2InQuartInterpolationFunction();
}

Vec2InQuintInterpolationFunction::Vec2InQuintInterpolationFunction() {}

std::string Vec2InQuintInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Vec2InQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::vec2 Vec2InQuintInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inQuintInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inQuintInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InQuintInterpolationFunction::clone() const {
    return new Vec2InQuintInterpolationFunction();
}

Vec2InSineInterpolationFunction::Vec2InSineInterpolationFunction() {}

std::string Vec2InSineInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Vec2InSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::vec2 Vec2InSineInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inSineInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inSineInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InSineInterpolationFunction::clone() const {
    return new Vec2InSineInterpolationFunction();
}

Vec2InExponentInterpolationFunction::Vec2InExponentInterpolationFunction() {}

std::string Vec2InExponentInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Vec2InExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::vec2 Vec2InExponentInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inExponentInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inExponentInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InExponentInterpolationFunction::clone() const {
    return new Vec2InExponentInterpolationFunction();
}

Vec2InCircInterpolationFunction::Vec2InCircInterpolationFunction() {}

std::string Vec2InCircInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Vec2InCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::vec2 Vec2InCircInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inCircInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inCircInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InCircInterpolationFunction::clone() const {
    return new Vec2InCircInterpolationFunction();
}

Vec2OutQuadInterpolationFunction::Vec2OutQuadInterpolationFunction() {}

std::string Vec2OutQuadInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Vec2OutQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::vec2 Vec2OutQuadInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outQuadInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outQuadInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutQuadInterpolationFunction::clone() const {
    return new Vec2OutQuadInterpolationFunction();
}

Vec2OutCubicInterpolationFunction::Vec2OutCubicInterpolationFunction() {}

std::string Vec2OutCubicInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Vec2OutCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::vec2 Vec2OutCubicInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outCubicInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outCubicInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutCubicInterpolationFunction::clone() const {
    return new Vec2OutCubicInterpolationFunction();
}

Vec2OutQuartInterpolationFunction::Vec2OutQuartInterpolationFunction() {}

std::string Vec2OutQuartInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Vec2OutQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::vec2 Vec2OutQuartInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outQuartInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outQuartInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutQuartInterpolationFunction::clone() const {
    return new Vec2OutQuartInterpolationFunction();
}

Vec2OutQuintInterpolationFunction::Vec2OutQuintInterpolationFunction() {}

std::string Vec2OutQuintInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Vec2OutQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::vec2 Vec2OutQuintInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outQuintInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outQuintInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutQuintInterpolationFunction::clone() const {
    return new Vec2OutQuintInterpolationFunction();
}

Vec2OutSineInterpolationFunction::Vec2OutSineInterpolationFunction() {}

std::string Vec2OutSineInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Vec2OutSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::vec2 Vec2OutSineInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outSineInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outSineInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutSineInterpolationFunction::clone() const {
    return new Vec2OutSineInterpolationFunction();
}

Vec2OutExponentInterpolationFunction::Vec2OutExponentInterpolationFunction() {}

std::string Vec2OutExponentInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Vec2OutExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::vec2 Vec2OutExponentInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outExponentInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outExponentInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutExponentInterpolationFunction::clone() const {
    return new Vec2OutExponentInterpolationFunction();
}

Vec2OutCircInterpolationFunction::Vec2OutCircInterpolationFunction() {}

std::string Vec2OutCircInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Vec2OutCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::vec2 Vec2OutCircInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outCircInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outCircInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutCircInterpolationFunction::clone() const {
    return new Vec2OutCircInterpolationFunction();
}

Vec2InOutQuadInterpolationFunction::Vec2InOutQuadInterpolationFunction() {}

std::string Vec2InOutQuadInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Vec2InOutQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::vec2 Vec2InOutQuadInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inOutQuadInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inOutQuadInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InOutQuadInterpolationFunction::clone() const {
    return new Vec2InOutQuadInterpolationFunction();
}

Vec2InOutCubicInterpolationFunction::Vec2InOutCubicInterpolationFunction() {}

std::string Vec2InOutCubicInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Vec2InOutCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::vec2 Vec2InOutCubicInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inOutCubicInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inOutCubicInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InOutCubicInterpolationFunction::clone() const {
    return new Vec2InOutCubicInterpolationFunction();
}

Vec2InOutQuartInterpolationFunction::Vec2InOutQuartInterpolationFunction() {}

std::string Vec2InOutQuartInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Vec2InOutQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::vec2 Vec2InOutQuartInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inOutQuartInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inOutQuartInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InOutQuartInterpolationFunction::clone() const {
    return new Vec2InOutQuartInterpolationFunction();
}

Vec2InOutQuintInterpolationFunction::Vec2InOutQuintInterpolationFunction() {}

std::string Vec2InOutQuintInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Vec2InOutQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::vec2 Vec2InOutQuintInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inOutQuintInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inOutQuintInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InOutQuintInterpolationFunction::clone() const {
    return new Vec2InOutQuintInterpolationFunction();
}

Vec2InOutSineInterpolationFunction::Vec2InOutSineInterpolationFunction() {}

std::string Vec2InOutSineInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Vec2InOutSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::vec2 Vec2InOutSineInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inOutSineInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inOutSineInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InOutSineInterpolationFunction::clone() const {
    return new Vec2InOutSineInterpolationFunction();
}

Vec2InOutExponentInterpolationFunction::Vec2InOutExponentInterpolationFunction() {}

std::string Vec2InOutExponentInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Vec2InOutExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::vec2 Vec2InOutExponentInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inOutExponentInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inOutExponentInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InOutExponentInterpolationFunction::clone() const {
    return new Vec2InOutExponentInterpolationFunction();
}

Vec2InOutCircInterpolationFunction::Vec2InOutCircInterpolationFunction() {}

std::string Vec2InOutCircInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Vec2InOutCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::vec2 Vec2InOutCircInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::inOutCircInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::inOutCircInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2InOutCircInterpolationFunction::clone() const {
    return new Vec2InOutCircInterpolationFunction();
}

Vec2OutInQuadInterpolationFunction::Vec2OutInQuadInterpolationFunction() {}

std::string Vec2OutInQuadInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Vec2OutInQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::vec2 Vec2OutInQuadInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outInQuadInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outInQuadInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutInQuadInterpolationFunction::clone() const {
    return new Vec2OutInQuadInterpolationFunction();
}

Vec2OutInCubicInterpolationFunction::Vec2OutInCubicInterpolationFunction() {}

std::string Vec2OutInCubicInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Vec2OutInCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::vec2 Vec2OutInCubicInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outInCubicInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outInCubicInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutInCubicInterpolationFunction::clone() const {
    return new Vec2OutInCubicInterpolationFunction();
}

Vec2OutInQuartInterpolationFunction::Vec2OutInQuartInterpolationFunction() {}

std::string Vec2OutInQuartInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Vec2OutInQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::vec2 Vec2OutInQuartInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outInQuartInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outInQuartInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutInQuartInterpolationFunction::clone() const {
    return new Vec2OutInQuartInterpolationFunction();
}

Vec2OutInQuintInterpolationFunction::Vec2OutInQuintInterpolationFunction() {}

std::string Vec2OutInQuintInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Vec2OutInQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::vec2 Vec2OutInQuintInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outInQuintInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outInQuintInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutInQuintInterpolationFunction::clone() const {
    return new Vec2OutInQuintInterpolationFunction();
}

Vec2OutInSineInterpolationFunction::Vec2OutInSineInterpolationFunction() {}

std::string Vec2OutInSineInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Vec2OutInSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::vec2 Vec2OutInSineInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outInSineInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outInSineInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutInSineInterpolationFunction::clone() const {
    return new Vec2OutInSineInterpolationFunction();
}

Vec2OutInExponentInterpolationFunction::Vec2OutInExponentInterpolationFunction() {}

std::string Vec2OutInExponentInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Vec2OutInExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::vec2 Vec2OutInExponentInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outInExponentInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outInExponentInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutInExponentInterpolationFunction::clone() const {
    return new Vec2OutInExponentInterpolationFunction();
}

Vec2OutInCircInterpolationFunction::Vec2OutInCircInterpolationFunction() {}

std::string Vec2OutInCircInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Vec2OutInCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::vec2 Vec2OutInCircInterpolationFunction::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::outInCircInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::outInCircInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

InterpolationFunction<tgt::vec2>* Vec2OutInCircInterpolationFunction::clone() const {
    return new Vec2OutInCircInterpolationFunction();
}

} // namespace voreen
