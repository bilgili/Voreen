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

#ifndef VRN_FLOATINTERPOLATIONFUNCTIONS_H
#define VRN_FLOATINTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

/**
 * This class offers an interpolation function for float-values. Interpolation: focus on startvalue.
 */
class FloatStartInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatStartInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: focus on endvalue.
 */
class FloatEndInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatEndInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: bisection.
 */
class FloatStartEndInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatStartEndInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: linear.
 */
class FloatLinearInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatLinearInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: Catmull-Rom spline.
 */
class FloatCatmullRomInterpolationFunction : public MultiPointInterpolationFunction<float> {

public:
        FloatCatmullRomInterpolationFunction();
        MultiPointInterpolationFunction<float>* clone() const;
        float interpolate(std::vector<PropertyKeyValue<float>*> controlpoints, float time) const;

        std::string getMode() const;
        std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quadratic (easing in).
 */
class FloatInQuadInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInQuadInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: cubicular (easing in).
 */
class FloatInCubicInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInCubicInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quartetic (easing in).
 */
class FloatInQuartInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInQuartInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quintic (easing in).
 */
class FloatInQuintInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInQuintInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: sineousidal (easing in).
 */
class FloatInSineInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInSineInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: exponential (easing in).
 */
class FloatInExponentInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInExponentInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: circular (easing in).
 */
class FloatInCircInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInCircInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quadratic (easing out).
 */
class FloatOutQuadInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutQuadInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: cubicular (easing out).
 */
class FloatOutCubicInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutCubicInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quartetic (easing out).
 */
class FloatOutQuartInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutQuartInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quintic (easing out).
 */
class FloatOutQuintInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutQuintInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: sineousidal (easing out).
 */
class FloatOutSineInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutSineInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: exponential (easing out).
 */
class FloatOutExponentInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutExponentInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: circular (easing out).
 */
class FloatOutCircInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutCircInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quadratic (first easing in, then easing out).
 */
class FloatInOutQuadInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutQuadInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: cubicular (first easing in, then easing out).
 */
class FloatInOutCubicInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutCubicInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quartetic (first easing in, then easing out).
 */
class FloatInOutQuartInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutQuartInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quintic (first easing in, then easing out).
 */
class FloatInOutQuintInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutQuintInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: sineousidal (first easing in, then easing out).
 */
class FloatInOutSineInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutSineInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: exponential (first easing in, then easing out).
 */
class FloatInOutExponentInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutExponentInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: circular (first easing in, then easing out).
 */
class FloatInOutCircInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutCircInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quadratic (first easing out, then easing in).
 */
class FloatOutInQuadInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInQuadInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: cubicular (first easing out, then easing in).
 */
class FloatOutInCubicInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInCubicInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quartetic (first easing out, then easing in).
 */
class FloatOutInQuartInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInQuartInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: quintic (first easing out, then easing in).
 */
class FloatOutInQuintInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInQuintInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: sineousidal (first easing out, then easing in).
 */
class FloatOutInSineInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInSineInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: exponential (first easing out, then easing in).
 */
class FloatOutInExponentInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInExponentInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for float-values. Interpolation: circular (first easing out, then easing in).
 */
class FloatOutInCircInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInCircInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
