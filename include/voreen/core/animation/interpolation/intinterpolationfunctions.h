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

#ifndef VRN_INTINTERPOLATIONFUNCTIONS_H
#define VRN_INTINTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API InterpolationFunction<int>;
#endif

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: focus on startvalue.
 */
class VRN_CORE_API IntStartInterpolationFunction : public InterpolationFunction<int> {
public:
    IntStartInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: focus on endvalue.
 */
class VRN_CORE_API IntEndInterpolationFunction : public InterpolationFunction<int> {
public:
    IntEndInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: bisection.
 */
class VRN_CORE_API IntStartEndInterpolationFunction : public InterpolationFunction<int> {
public:
    IntStartEndInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: linear.
 */
class VRN_CORE_API IntLinearInterpolationFunction : public InterpolationFunction<int> {
public:
    IntLinearInterpolationFunction();
    virtual InterpolationFunction<int>* clone() const;
    virtual int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: Catmull-Rom spline.
 */
class VRN_CORE_API IntCatmullRomInterpolationFunction : public MultiPointInterpolationFunction<int> {
public:
    IntCatmullRomInterpolationFunction();
    MultiPointInterpolationFunction<int>* clone() const;
    int interpolate(std::vector<PropertyKeyValue<int>*> controlpoints, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quadratic (easing in).
 */
class VRN_CORE_API IntInQuadInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInQuadInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: cubicular (easing in).
 */
class VRN_CORE_API IntInCubicInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInCubicInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quartetic (easing in).
 */
class VRN_CORE_API IntInQuartInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInQuartInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quintic (easing in).
 */
class VRN_CORE_API IntInQuintInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInQuintInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: sineousidal (easing in).
 */
class VRN_CORE_API IntInSineInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInSineInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: exponential (easing in).
 */
class VRN_CORE_API IntInExponentInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInExponentInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: circular (easing in).
 */
class VRN_CORE_API IntInCircInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInCircInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quadratic (easing out).
 */
class VRN_CORE_API IntOutQuadInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutQuadInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: cubicular (easing out).
 */
class VRN_CORE_API IntOutCubicInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutCubicInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quartetic (easing out).
 */
class VRN_CORE_API IntOutQuartInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutQuartInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quintic (easing out).
 */
class VRN_CORE_API IntOutQuintInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutQuintInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: sineousidal (easing out).
 */
class VRN_CORE_API IntOutSineInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutSineInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: exponential (easing out).
 */
class VRN_CORE_API IntOutExponentInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutExponentInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: circular (easing out).
 */
class VRN_CORE_API IntOutCircInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutCircInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quadratic (easing in, then easing out).
 */
class VRN_CORE_API IntInOutQuadInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutQuadInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: cubicular (easing in, then easing out).
 */
class VRN_CORE_API IntInOutCubicInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutCubicInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quartetic (easing in, then easing out).
 */
class VRN_CORE_API IntInOutQuartInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutQuartInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quintic (easing in, then easing out).
 */
class VRN_CORE_API IntInOutQuintInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutQuintInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: sineousidal (easing in, then easing out).
 */
class VRN_CORE_API IntInOutSineInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutSineInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: exponential (easing in, then easing out).
 */
class VRN_CORE_API IntInOutExponentInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutExponentInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: circular (easing in, then easing out).
 */
class VRN_CORE_API IntInOutCircInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutCircInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quadratic (easing out, then easing in).
 */
class VRN_CORE_API IntOutInQuadInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInQuadInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: cubicular (easing out, then easing in).
 */
class VRN_CORE_API IntOutInCubicInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInCubicInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quartetic (easing out, then easing in).
 */
class VRN_CORE_API IntOutInQuartInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInQuartInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: quintic (easing out, then easing in).
 */
class VRN_CORE_API IntOutInQuintInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInQuintInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: sineousidal (easing out, then easing in).
 */
class VRN_CORE_API IntOutInSineInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInSineInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: exponential (easing out, then easing in).
 */
class VRN_CORE_API IntOutInExponentInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInExponentInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for int-values. Interpolation: circular (easing out, then easing in).
 */
class VRN_CORE_API IntOutInCircInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInCircInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen

#endif
