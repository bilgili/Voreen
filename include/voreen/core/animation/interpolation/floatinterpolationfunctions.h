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

#ifndef VRN_FLOATINTERPOLATIONFUNCTIONS_H
#define VRN_FLOATINTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API InterpolationFunction<float>;
#endif

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: focus on startvalue.
 */
class VRN_CORE_API FloatStartInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatStartInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: focus on endvalue.
 */
class VRN_CORE_API FloatEndInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatEndInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: bisection.
 */
class VRN_CORE_API FloatStartEndInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatStartEndInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: linear.
 */
class VRN_CORE_API FloatLinearInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatLinearInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: Catmull-Rom spline.
 */
class VRN_CORE_API FloatCatmullRomInterpolationFunction : public MultiPointInterpolationFunction<float> {

public:
        FloatCatmullRomInterpolationFunction();
        MultiPointInterpolationFunction<float>* clone() const;
        float interpolate(std::vector<PropertyKeyValue<float>*> controlpoints, float time) const;

        std::string getMode() const;
        std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quadratic (easing in).
 */
class VRN_CORE_API FloatInQuadInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInQuadInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: cubicular (easing in).
 */
class VRN_CORE_API FloatInCubicInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInCubicInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quartetic (easing in).
 */
class VRN_CORE_API FloatInQuartInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInQuartInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quintic (easing in).
 */
class VRN_CORE_API FloatInQuintInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInQuintInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: sineousidal (easing in).
 */
class VRN_CORE_API FloatInSineInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInSineInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: exponential (easing in).
 */
class VRN_CORE_API FloatInExponentInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInExponentInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: circular (easing in).
 */
class VRN_CORE_API FloatInCircInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInCircInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quadratic (easing out).
 */
class VRN_CORE_API FloatOutQuadInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutQuadInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: cubicular (easing out).
 */
class VRN_CORE_API FloatOutCubicInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutCubicInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quartetic (easing out).
 */
class VRN_CORE_API FloatOutQuartInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutQuartInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quintic (easing out).
 */
class VRN_CORE_API FloatOutQuintInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutQuintInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: sineousidal (easing out).
 */
class VRN_CORE_API FloatOutSineInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutSineInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: exponential (easing out).
 */
class VRN_CORE_API FloatOutExponentInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutExponentInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: circular (easing out).
 */
class VRN_CORE_API FloatOutCircInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutCircInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quadratic (first easing in, then easing out).
 */
class VRN_CORE_API FloatInOutQuadInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutQuadInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: cubicular (first easing in, then easing out).
 */
class VRN_CORE_API FloatInOutCubicInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutCubicInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quartetic (first easing in, then easing out).
 */
class VRN_CORE_API FloatInOutQuartInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutQuartInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quintic (first easing in, then easing out).
 */
class VRN_CORE_API FloatInOutQuintInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutQuintInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: sineousidal (first easing in, then easing out).
 */
class VRN_CORE_API FloatInOutSineInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutSineInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: exponential (first easing in, then easing out).
 */
class VRN_CORE_API FloatInOutExponentInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutExponentInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: circular (first easing in, then easing out).
 */
class VRN_CORE_API FloatInOutCircInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatInOutCircInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quadratic (first easing out, then easing in).
 */
class VRN_CORE_API FloatOutInQuadInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInQuadInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: cubicular (first easing out, then easing in).
 */
class VRN_CORE_API FloatOutInCubicInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInCubicInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quartetic (first easing out, then easing in).
 */
class VRN_CORE_API FloatOutInQuartInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInQuartInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: quintic (first easing out, then easing in).
 */
class VRN_CORE_API FloatOutInQuintInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInQuintInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: sineousidal (first easing out, then easing in).
 */
class VRN_CORE_API FloatOutInSineInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInSineInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: exponential (first easing out, then easing in).
 */
class VRN_CORE_API FloatOutInExponentInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInExponentInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for float-values. Interpolation: circular (first easing out, then easing in).
 */
class VRN_CORE_API FloatOutInCircInterpolationFunction : public InterpolationFunction<float> {

public:
    FloatOutInCircInterpolationFunction();
    InterpolationFunction<float>* clone() const;
    float interpolate(float startvalue, float endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
