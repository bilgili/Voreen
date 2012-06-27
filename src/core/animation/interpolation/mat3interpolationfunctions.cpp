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
#include "voreen/core/animation/interpolation/mat3interpolationfunctions.h"

namespace voreen {

Mat3StartInterpolationFunction::Mat3StartInterpolationFunction() {}

std::string Mat3StartInterpolationFunction::getMode() const {
    return "focus on startvalue";
}

std::string Mat3StartInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::mat3 Mat3StartInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    if (time < 1.f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<tgt::mat3>* Mat3StartInterpolationFunction::clone() const {
    return new Mat3StartInterpolationFunction();
}

Mat3EndInterpolationFunction::Mat3EndInterpolationFunction() {}

std::string Mat3EndInterpolationFunction::getMode() const {
    return "focus on endvalue";
}

std::string Mat3EndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::mat3 Mat3EndInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    if (time > 0.f)
        return endvalue;
    else
        return startvalue;
}

InterpolationFunction<tgt::mat3>* Mat3EndInterpolationFunction::clone() const {
    return new Mat3EndInterpolationFunction();
}

Mat3StartEndInterpolationFunction::Mat3StartEndInterpolationFunction() {}

std::string Mat3StartEndInterpolationFunction::getMode() const {
    return "bisection";
}

std::string Mat3StartEndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

tgt::mat3 Mat3StartEndInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    if (time < 0.5f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<tgt::mat3>* Mat3StartEndInterpolationFunction::clone() const {
    return new Mat3StartEndInterpolationFunction();
}

Mat3LinearInterpolationFunction::Mat3LinearInterpolationFunction() {}

std::string Mat3LinearInterpolationFunction::getMode() const {
    return "linear interpolation";
}

std::string Mat3LinearInterpolationFunction::getIdentifier() const {
    return "linear";
}

tgt::mat3 Mat3LinearInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::linearInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3LinearInterpolationFunction::clone() const {
    return new Mat3LinearInterpolationFunction();
}

Mat3InQuadInterpolationFunction::Mat3InQuadInterpolationFunction() {}

std::string Mat3InQuadInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat3InQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::mat3 Mat3InQuadInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inQuadInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InQuadInterpolationFunction::clone() const {
    return new Mat3InQuadInterpolationFunction();
}

Mat3InCubicInterpolationFunction::Mat3InCubicInterpolationFunction() {}

std::string Mat3InCubicInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat3InCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::mat3 Mat3InCubicInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inCubicInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InCubicInterpolationFunction::clone() const {
    return new Mat3InCubicInterpolationFunction();
}

Mat3InQuartInterpolationFunction::Mat3InQuartInterpolationFunction() {}

std::string Mat3InQuartInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat3InQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::mat3 Mat3InQuartInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inQuartInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InQuartInterpolationFunction::clone() const {
    return new Mat3InQuartInterpolationFunction();
}

Mat3InQuintInterpolationFunction::Mat3InQuintInterpolationFunction() {}

std::string Mat3InQuintInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat3InQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::mat3 Mat3InQuintInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inQuintInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InQuintInterpolationFunction::clone() const {
    return new Mat3InQuintInterpolationFunction();
}

Mat3InSineInterpolationFunction::Mat3InSineInterpolationFunction() {}

std::string Mat3InSineInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat3InSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::mat3 Mat3InSineInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inSineInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InSineInterpolationFunction::clone() const {
    return new Mat3InSineInterpolationFunction();
}

Mat3InExponentInterpolationFunction::Mat3InExponentInterpolationFunction() {}

std::string Mat3InExponentInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat3InExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::mat3 Mat3InExponentInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inExponentInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InExponentInterpolationFunction::clone() const {
    return new Mat3InExponentInterpolationFunction();
}

Mat3InCircInterpolationFunction::Mat3InCircInterpolationFunction() {}

std::string Mat3InCircInterpolationFunction::getMode() const {
    return "easing in";
}

std::string Mat3InCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::mat3 Mat3InCircInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inCircInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InCircInterpolationFunction::clone() const {
    return new Mat3InCircInterpolationFunction();
}

Mat3OutQuadInterpolationFunction::Mat3OutQuadInterpolationFunction() {}

std::string Mat3OutQuadInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat3OutQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::mat3 Mat3OutQuadInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outQuadInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutQuadInterpolationFunction::clone() const {
    return new Mat3OutQuadInterpolationFunction();
}

Mat3OutCubicInterpolationFunction::Mat3OutCubicInterpolationFunction() {}

std::string Mat3OutCubicInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat3OutCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::mat3 Mat3OutCubicInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outCubicInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutCubicInterpolationFunction::clone() const {
    return new Mat3OutCubicInterpolationFunction();
}

Mat3OutQuartInterpolationFunction::Mat3OutQuartInterpolationFunction() {}

std::string Mat3OutQuartInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat3OutQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::mat3 Mat3OutQuartInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outQuartInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutQuartInterpolationFunction::clone() const {
    return new Mat3OutQuartInterpolationFunction();
}

Mat3OutQuintInterpolationFunction::Mat3OutQuintInterpolationFunction() {}

std::string Mat3OutQuintInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat3OutQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::mat3 Mat3OutQuintInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outQuintInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutQuintInterpolationFunction::clone() const {
    return new Mat3OutQuintInterpolationFunction();
}

Mat3OutSineInterpolationFunction::Mat3OutSineInterpolationFunction() {}

std::string Mat3OutSineInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat3OutSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::mat3 Mat3OutSineInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outSineInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutSineInterpolationFunction::clone() const {
    return new Mat3OutSineInterpolationFunction();
}

Mat3OutExponentInterpolationFunction::Mat3OutExponentInterpolationFunction() {}

std::string Mat3OutExponentInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat3OutExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::mat3 Mat3OutExponentInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outExponentInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutExponentInterpolationFunction::clone() const {
    return new Mat3OutExponentInterpolationFunction();
}

Mat3OutCircInterpolationFunction::Mat3OutCircInterpolationFunction() {}

std::string Mat3OutCircInterpolationFunction::getMode() const {
    return "easing out";
}

std::string Mat3OutCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::mat3 Mat3OutCircInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outCircInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutCircInterpolationFunction::clone() const {
    return new Mat3OutCircInterpolationFunction();
}

Mat3InOutQuadInterpolationFunction::Mat3InOutQuadInterpolationFunction() {}

std::string Mat3InOutQuadInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat3InOutQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::mat3 Mat3InOutQuadInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutQuadInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InOutQuadInterpolationFunction::clone() const {
    return new Mat3InOutQuadInterpolationFunction();
}

Mat3InOutCubicInterpolationFunction::Mat3InOutCubicInterpolationFunction() {}

std::string Mat3InOutCubicInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat3InOutCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::mat3 Mat3InOutCubicInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutCubicInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InOutCubicInterpolationFunction::clone() const {
    return new Mat3InOutCubicInterpolationFunction();
}

Mat3InOutQuartInterpolationFunction::Mat3InOutQuartInterpolationFunction() {}

std::string Mat3InOutQuartInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat3InOutQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::mat3 Mat3InOutQuartInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutQuartInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InOutQuartInterpolationFunction::clone() const {
    return new Mat3InOutQuartInterpolationFunction();
}

Mat3InOutQuintInterpolationFunction::Mat3InOutQuintInterpolationFunction() {}

std::string Mat3InOutQuintInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat3InOutQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::mat3 Mat3InOutQuintInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutQuintInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InOutQuintInterpolationFunction::clone() const {
    return new Mat3InOutQuintInterpolationFunction();
}

Mat3InOutSineInterpolationFunction::Mat3InOutSineInterpolationFunction() {}

std::string Mat3InOutSineInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat3InOutSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::mat3 Mat3InOutSineInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutSineInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InOutSineInterpolationFunction::clone() const {
    return new Mat3InOutSineInterpolationFunction();
}

Mat3InOutExponentInterpolationFunction::Mat3InOutExponentInterpolationFunction() {}

std::string Mat3InOutExponentInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat3InOutExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::mat3 Mat3InOutExponentInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutExponentInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InOutExponentInterpolationFunction::clone() const {
    return new Mat3InOutExponentInterpolationFunction();
}

Mat3InOutCircInterpolationFunction::Mat3InOutCircInterpolationFunction() {}

std::string Mat3InOutCircInterpolationFunction::getMode() const {
    return "first easing in, then easing out";
}

std::string Mat3InOutCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::mat3 Mat3InOutCircInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::inOutCircInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3InOutCircInterpolationFunction::clone() const {
    return new Mat3InOutCircInterpolationFunction();
}

Mat3OutInQuadInterpolationFunction::Mat3OutInQuadInterpolationFunction() {}

std::string Mat3OutInQuadInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat3OutInQuadInterpolationFunction::getIdentifier() const {
    return "quadratic";
}

tgt::mat3 Mat3OutInQuadInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInQuadInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutInQuadInterpolationFunction::clone() const {
    return new Mat3OutInQuadInterpolationFunction();
}

Mat3OutInCubicInterpolationFunction::Mat3OutInCubicInterpolationFunction() {}

std::string Mat3OutInCubicInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat3OutInCubicInterpolationFunction::getIdentifier() const {
    return "cubic";
}

tgt::mat3 Mat3OutInCubicInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInCubicInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutInCubicInterpolationFunction::clone() const {
    return new Mat3OutInCubicInterpolationFunction();
}

Mat3OutInQuartInterpolationFunction::Mat3OutInQuartInterpolationFunction() {}

std::string Mat3OutInQuartInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat3OutInQuartInterpolationFunction::getIdentifier() const {
    return "quartetic";
}

tgt::mat3 Mat3OutInQuartInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInQuartInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutInQuartInterpolationFunction::clone() const {
    return new Mat3OutInQuartInterpolationFunction();
}

Mat3OutInQuintInterpolationFunction::Mat3OutInQuintInterpolationFunction() {}

std::string Mat3OutInQuintInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat3OutInQuintInterpolationFunction::getIdentifier() const {
    return "quintic";
}

tgt::mat3 Mat3OutInQuintInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInQuintInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutInQuintInterpolationFunction::clone() const {
    return new Mat3OutInQuintInterpolationFunction();
}

Mat3OutInSineInterpolationFunction::Mat3OutInSineInterpolationFunction() {}

std::string Mat3OutInSineInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat3OutInSineInterpolationFunction::getIdentifier() const {
    return "sineousidal";
}

tgt::mat3 Mat3OutInSineInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInSineInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutInSineInterpolationFunction::clone() const {
    return new Mat3OutInSineInterpolationFunction();
}

Mat3OutInExponentInterpolationFunction::Mat3OutInExponentInterpolationFunction() {}

std::string Mat3OutInExponentInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat3OutInExponentInterpolationFunction::getIdentifier() const {
    return "exponential";
}

tgt::mat3 Mat3OutInExponentInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInExponentInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutInExponentInterpolationFunction::clone() const {
    return new Mat3OutInExponentInterpolationFunction();
}

Mat3OutInCircInterpolationFunction::Mat3OutInCircInterpolationFunction() {}

std::string Mat3OutInCircInterpolationFunction::getMode() const {
    return "first easing out, then easing in";
}

std::string Mat3OutInCircInterpolationFunction::getIdentifier() const {
    return "circular";
}

tgt::mat3 Mat3OutInCircInterpolationFunction::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::outInCircInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

InterpolationFunction<tgt::mat3>* Mat3OutInCircInterpolationFunction::clone() const {
    return new Mat3OutInCircInterpolationFunction();
}

} // namespace voreen
