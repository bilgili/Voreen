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

#ifndef VRN_INTINTERPOLATIONFUNCTIONS_H
#define VRN_INTINTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

/**
 * This class offers an interpolation function for int-values. Interpolation: focus on startvalue.
 */
class IntStartInterpolationFunction : public InterpolationFunction<int> {
public:
    IntStartInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: focus on endvalue.
 */
class IntEndInterpolationFunction : public InterpolationFunction<int> {
public:
    IntEndInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: bisection.
 */
class IntStartEndInterpolationFunction : public InterpolationFunction<int> {
public:
    IntStartEndInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: linear.
 */
class IntLinearInterpolationFunction : public InterpolationFunction<int> {
public:
    IntLinearInterpolationFunction();
    virtual InterpolationFunction<int>* clone() const;
    virtual int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: Catmull-Rom spline.
 */
class IntCatmullRomInterpolationFunction : public MultiPointInterpolationFunction<int> {
public:
    IntCatmullRomInterpolationFunction();
    MultiPointInterpolationFunction<int>* clone() const;
    int interpolate(std::vector<PropertyKeyValue<int>*> controlpoints, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quadratic (easing in).
 */
class IntInQuadInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInQuadInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: cubicular (easing in).
 */
class IntInCubicInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInCubicInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quartetic (easing in).
 */
class IntInQuartInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInQuartInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quintic (easing in).
 */
class IntInQuintInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInQuintInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: sineousidal (easing in).
 */
class IntInSineInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInSineInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: exponential (easing in).
 */
class IntInExponentInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInExponentInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: circular (easing in).
 */
class IntInCircInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInCircInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quadratic (easing out).
 */
class IntOutQuadInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutQuadInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: cubicular (easing out).
 */
class IntOutCubicInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutCubicInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quartetic (easing out).
 */
class IntOutQuartInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutQuartInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quintic (easing out).
 */
class IntOutQuintInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutQuintInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: sineousidal (easing out).
 */
class IntOutSineInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutSineInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: exponential (easing out).
 */
class IntOutExponentInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutExponentInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: circular (easing out).
 */
class IntOutCircInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutCircInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quadratic (easing in, then easing out).
 */
class IntInOutQuadInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutQuadInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: cubicular (easing in, then easing out).
 */
class IntInOutCubicInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutCubicInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quartetic (easing in, then easing out).
 */
class IntInOutQuartInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutQuartInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quintic (easing in, then easing out).
 */
class IntInOutQuintInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutQuintInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: sineousidal (easing in, then easing out).
 */
class IntInOutSineInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutSineInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: exponential (easing in, then easing out).
 */
class IntInOutExponentInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutExponentInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: circular (easing in, then easing out).
 */
class IntInOutCircInterpolationFunction : public InterpolationFunction<int> {
public:
    IntInOutCircInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quadratic (easing out, then easing in).
 */
class IntOutInQuadInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInQuadInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: cubicular (easing out, then easing in).
 */
class IntOutInCubicInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInCubicInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quartetic (easing out, then easing in).
 */
class IntOutInQuartInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInQuartInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: quintic (easing out, then easing in).
 */
class IntOutInQuintInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInQuintInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: sineousidal (easing out, then easing in).
 */
class IntOutInSineInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInSineInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: exponential (easing out, then easing in).
 */
class IntOutInExponentInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInExponentInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for int-values. Interpolation: circular (easing out, then easing in).
 */
class IntOutInCircInterpolationFunction : public InterpolationFunction<int> {
public:
    IntOutInCircInterpolationFunction();
    InterpolationFunction<int>* clone() const;
    int interpolate(int startvalue, int endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen

#endif
