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

#ifndef VRN_VEC2INTERPOLATIONFUNCTIONS_H
#define VRN_VEC2INTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

/**
 * This class offers an interpolation function for vec2-values. Interpolation: focus on startvalue.
 */
class Vec2StartInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2StartInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2-values. Interpolation: focus on endvalue.
 */
class Vec2EndInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2EndInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2-values. Interpolation: bisection.
 */
class Vec2StartEndInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2StartEndInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: linear.
 */
class Vec2LinearInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2LinearInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: spherical linear.
 */
class Vec2SphericalLinearInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2SphericalLinearInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quadratic (easing in).
 */
class Vec2InQuadInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InQuadInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: cubicular (easing in).
 */
class Vec2InCubicInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InCubicInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quartetic (easing in).
 */
class Vec2InQuartInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InQuartInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quintic (easing in).
 */
class Vec2InQuintInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InQuintInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: sineousidal (easing in).
 */
class Vec2InSineInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InSineInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: exponential (easing in).
 */
class Vec2InExponentInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InExponentInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: circular (easing in).
 */
class Vec2InCircInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InCircInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quadratic (easing out).
 */
class Vec2OutQuadInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutQuadInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: cubicular (easing out).
 */
class Vec2OutCubicInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutCubicInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quartetic (easing out).
 */
class Vec2OutQuartInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutQuartInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quintic (easing out).
 */
class Vec2OutQuintInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutQuintInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: sineousidal (easing out).
 */
class Vec2OutSineInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutSineInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: exponential (easing out).
 */
class Vec2OutExponentInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutExponentInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: circular (easing out).
 */
class Vec2OutCircInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutCircInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quadratic (easing in, then easing out).
 */
class Vec2InOutQuadInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InOutQuadInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: cubicular (easing in, then easing out).
 */
class Vec2InOutCubicInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InOutCubicInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quartetic (easing in, then easing out).
 */
class Vec2InOutQuartInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InOutQuartInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quintic (easing in, then easing out).
 */
class Vec2InOutQuintInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InOutQuintInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: sineousidal (easing in, then easing out).
 */
class Vec2InOutSineInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InOutSineInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: exponential (easing in, then easing out).
 */
class Vec2InOutExponentInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InOutExponentInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: circular (easing in, then easing out).
 */
class Vec2InOutCircInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2InOutCircInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quadratic (easing out, then easing in).
 */
class Vec2OutInQuadInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutInQuadInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: cubicular (easing out, then easing in).
 */
class Vec2OutInCubicInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutInCubicInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quartetic (easing out, then easing in).
 */
class Vec2OutInQuartInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutInQuartInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: quintic (easing out, then easing in).
 */
class Vec2OutInQuintInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutInQuintInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: sineousidal (easing out, then easing in).
 */
class Vec2OutInSineInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutInSineInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: exponential (easing out, then easing in).
 */
class Vec2OutInExponentInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutInExponentInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for vec2. Interpolation: circular (easing out, then easing in).
 */
class Vec2OutInCircInterpolationFunction : public InterpolationFunction<tgt::vec2> {

public:
    Vec2OutInCircInterpolationFunction();
    InterpolationFunction<tgt::vec2>* clone() const;
    tgt::vec2 interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
