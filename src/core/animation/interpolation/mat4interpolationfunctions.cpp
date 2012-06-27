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
#include "voreen/core/animation/interpolation/mat4interpolationfunctions.h"

namespace voreen {

Mat4StartInterpolationFunction::Mat4StartInterpolationFunction() {}

std::string Mat4StartInterpolationFunction::getMode() const {
    return "focus on startvalue";
}

std::string Mat4StartInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::mat4 Mat4StartInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    if (time < 1.f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<tgt::mat4>* Mat4StartInterpolationFunction::clone() const {
    return new Mat4StartInterpolationFunction();
}

Mat4EndInterpolationFunction::Mat4EndInterpolationFunction() {}

std::string Mat4EndInterpolationFunction::getMode() const {
    return "focus on endvalue";
}

std::string Mat4EndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::mat4 Mat4EndInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    if (time > 0.f)
        return endvalue;
    else
        return startvalue;
}

InterpolationFunction<tgt::mat4>* Mat4EndInterpolationFunction::clone() const {
    return new Mat4EndInterpolationFunction();
}

Mat4StartEndInterpolationFunction::Mat4StartEndInterpolationFunction() {}

std::string Mat4StartEndInterpolationFunction::getMode() const {
    return "bisection";
}

std::string Mat4StartEndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::mat4 Mat4StartEndInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    if (time < 0.5f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<tgt::mat4>* Mat4StartEndInterpolationFunction::clone() const {
    return new Mat4StartEndInterpolationFunction();
}

Mat4LinearInterpolationFunction::Mat4LinearInterpolationFunction() {}

std::string Mat4LinearInterpolationFunction::getMode() const {
    return "linear interpolation";
}

std::string Mat4LinearInterpolationFunction::getIdentifier() const {
    return "linear";
}

tgt::mat4 Mat4LinearInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::linearInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4LinearInterpolationFunction::clone() const {
    return new Mat4LinearInterpolationFunction();
}

Mat4InQuadInterpolationFunction::Mat4InQuadInterpolationFunction() {}

std::string Mat4InQuadInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat4InQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::mat4 Mat4InQuadInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inQuadInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InQuadInterpolationFunction::clone() const {
    return new Mat4InQuadInterpolationFunction();
}

Mat4InCubicInterpolationFunction::Mat4InCubicInterpolationFunction() {}

std::string Mat4InCubicInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat4InCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::mat4 Mat4InCubicInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inCubicInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InCubicInterpolationFunction::clone() const {
    return new Mat4InCubicInterpolationFunction();
}

Mat4InQuartInterpolationFunction::Mat4InQuartInterpolationFunction() {}

std::string Mat4InQuartInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat4InQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::mat4 Mat4InQuartInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inQuartInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InQuartInterpolationFunction::clone() const {
    return new Mat4InQuartInterpolationFunction();
}

Mat4InQuintInterpolationFunction::Mat4InQuintInterpolationFunction() {}

std::string Mat4InQuintInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat4InQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::mat4 Mat4InQuintInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inQuintInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InQuintInterpolationFunction::clone() const {
    return new Mat4InQuintInterpolationFunction();
}

Mat4InSineInterpolationFunction::Mat4InSineInterpolationFunction() {}

std::string Mat4InSineInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat4InSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::mat4 Mat4InSineInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inSineInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InSineInterpolationFunction::clone() const {
    return new Mat4InSineInterpolationFunction();
}

Mat4InExponentInterpolationFunction::Mat4InExponentInterpolationFunction() {}

std::string Mat4InExponentInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat4InExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::mat4 Mat4InExponentInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inExponentInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InExponentInterpolationFunction::clone() const {
    return new Mat4InExponentInterpolationFunction();
}

Mat4InCircInterpolationFunction::Mat4InCircInterpolationFunction() {}

std::string Mat4InCircInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat4InCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::mat4 Mat4InCircInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inCircInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InCircInterpolationFunction::clone() const {
    return new Mat4InCircInterpolationFunction();
}

Mat4OutQuadInterpolationFunction::Mat4OutQuadInterpolationFunction() {}

std::string Mat4OutQuadInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat4OutQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::mat4 Mat4OutQuadInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outQuadInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutQuadInterpolationFunction::clone() const {
    return new Mat4OutQuadInterpolationFunction();
}

Mat4OutCubicInterpolationFunction::Mat4OutCubicInterpolationFunction() {}

std::string Mat4OutCubicInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat4OutCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::mat4 Mat4OutCubicInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outCubicInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutCubicInterpolationFunction::clone() const {
    return new Mat4OutCubicInterpolationFunction();
}

Mat4OutQuartInterpolationFunction::Mat4OutQuartInterpolationFunction() {}

std::string Mat4OutQuartInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat4OutQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::mat4 Mat4OutQuartInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outQuartInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutQuartInterpolationFunction::clone() const {
    return new Mat4OutQuartInterpolationFunction();
}

Mat4OutQuintInterpolationFunction::Mat4OutQuintInterpolationFunction() {}

std::string Mat4OutQuintInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat4OutQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::mat4 Mat4OutQuintInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outQuintInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutQuintInterpolationFunction::clone() const {
    return new Mat4OutQuintInterpolationFunction();
}

Mat4OutSineInterpolationFunction::Mat4OutSineInterpolationFunction() {}

std::string Mat4OutSineInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat4OutSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::mat4 Mat4OutSineInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outSineInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutSineInterpolationFunction::clone() const {
    return new Mat4OutSineInterpolationFunction();
}

Mat4OutExponentInterpolationFunction::Mat4OutExponentInterpolationFunction() {}

std::string Mat4OutExponentInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat4OutExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::mat4 Mat4OutExponentInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outExponentInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutExponentInterpolationFunction::clone() const {
    return new Mat4OutExponentInterpolationFunction();
}

Mat4OutCircInterpolationFunction::Mat4OutCircInterpolationFunction() {}

std::string Mat4OutCircInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat4OutCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::mat4 Mat4OutCircInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outCircInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutCircInterpolationFunction::clone() const {
    return new Mat4OutCircInterpolationFunction();
}

Mat4InOutQuadInterpolationFunction::Mat4InOutQuadInterpolationFunction() {}

std::string Mat4InOutQuadInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat4InOutQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::mat4 Mat4InOutQuadInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutQuadInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InOutQuadInterpolationFunction::clone() const {
    return new Mat4InOutQuadInterpolationFunction();
}

Mat4InOutCubicInterpolationFunction::Mat4InOutCubicInterpolationFunction() {}

std::string Mat4InOutCubicInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat4InOutCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::mat4 Mat4InOutCubicInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutCubicInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InOutCubicInterpolationFunction::clone() const {
    return new Mat4InOutCubicInterpolationFunction();
}

Mat4InOutQuartInterpolationFunction::Mat4InOutQuartInterpolationFunction() {}

std::string Mat4InOutQuartInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat4InOutQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::mat4 Mat4InOutQuartInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutQuartInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InOutQuartInterpolationFunction::clone() const {
    return new Mat4InOutQuartInterpolationFunction();
}

Mat4InOutQuintInterpolationFunction::Mat4InOutQuintInterpolationFunction() {}

std::string Mat4InOutQuintInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat4InOutQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::mat4 Mat4InOutQuintInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutQuintInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InOutQuintInterpolationFunction::clone() const {
    return new Mat4InOutQuintInterpolationFunction();
}

Mat4InOutSineInterpolationFunction::Mat4InOutSineInterpolationFunction() {}

std::string Mat4InOutSineInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat4InOutSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::mat4 Mat4InOutSineInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutSineInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InOutSineInterpolationFunction::clone() const {
    return new Mat4InOutSineInterpolationFunction();
}

Mat4InOutExponentInterpolationFunction::Mat4InOutExponentInterpolationFunction() {}

std::string Mat4InOutExponentInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat4InOutExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::mat4 Mat4InOutExponentInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutExponentInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InOutExponentInterpolationFunction::clone() const {
    return new Mat4InOutExponentInterpolationFunction();
}

Mat4InOutCircInterpolationFunction::Mat4InOutCircInterpolationFunction() {}

std::string Mat4InOutCircInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat4InOutCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::mat4 Mat4InOutCircInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutCircInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4InOutCircInterpolationFunction::clone() const {
    return new Mat4InOutCircInterpolationFunction();
}

Mat4OutInQuadInterpolationFunction::Mat4OutInQuadInterpolationFunction() {}

std::string Mat4OutInQuadInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat4OutInQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::mat4 Mat4OutInQuadInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInQuadInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutInQuadInterpolationFunction::clone() const {
    return new Mat4OutInQuadInterpolationFunction();
}

Mat4OutInCubicInterpolationFunction::Mat4OutInCubicInterpolationFunction() {}

std::string Mat4OutInCubicInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat4OutInCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::mat4 Mat4OutInCubicInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInCubicInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutInCubicInterpolationFunction::clone() const {
    return new Mat4OutInCubicInterpolationFunction();
}

Mat4OutInQuartInterpolationFunction::Mat4OutInQuartInterpolationFunction() {}

std::string Mat4OutInQuartInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat4OutInQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::mat4 Mat4OutInQuartInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInQuartInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutInQuartInterpolationFunction::clone() const {
    return new Mat4OutInQuartInterpolationFunction();
}

Mat4OutInQuintInterpolationFunction::Mat4OutInQuintInterpolationFunction() {}

std::string Mat4OutInQuintInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat4OutInQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::mat4 Mat4OutInQuintInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInQuintInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutInQuintInterpolationFunction::clone() const {
    return new Mat4OutInQuintInterpolationFunction();
}

Mat4OutInSineInterpolationFunction::Mat4OutInSineInterpolationFunction() {}

std::string Mat4OutInSineInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat4OutInSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::mat4 Mat4OutInSineInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInSineInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutInSineInterpolationFunction::clone() const {
    return new Mat4OutInSineInterpolationFunction();
}

Mat4OutInExponentInterpolationFunction::Mat4OutInExponentInterpolationFunction() {}

std::string Mat4OutInExponentInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat4OutInExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::mat4 Mat4OutInExponentInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInExponentInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutInExponentInterpolationFunction::clone() const {
    return new Mat4OutInExponentInterpolationFunction();
}

Mat4OutInCircInterpolationFunction::Mat4OutInCircInterpolationFunction() {}

std::string Mat4OutInCircInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat4OutInCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::mat4 Mat4OutInCircInterpolationFunction::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInCircInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat4>* Mat4OutInCircInterpolationFunction::clone() const {
    return new Mat4OutInCircInterpolationFunction();
}

} // namespace voreen
