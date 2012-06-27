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

#ifndef VRN_MAT2INTERPOLATIONFUNCTIONS_H
#define VRN_MAT2INTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

/**
 * This class offers an interpolation function for mat2-values. Interpolation: focus on startvalue.
 */
class Mat2StartInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2StartInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2-values. Interpolation: focus on endvalue.
 */
class Mat2EndInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2EndInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2-values. Interpolation: bisection.
 */
class Mat2StartEndInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2StartEndInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: linear.
 */
class Mat2LinearInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2LinearInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quadratic (easing in).
 */
class Mat2InQuadInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InQuadInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: cubicular (easing in).
 */
class Mat2InCubicInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InCubicInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quartetic (easing in).
 */
class Mat2InQuartInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InQuartInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quintic (easing in).
 */
class Mat2InQuintInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InQuintInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: sineousidal (easing in).
 */
class Mat2InSineInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InSineInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: exponential (easing in).
 */
class Mat2InExponentInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InExponentInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: circular (easing in).
 */
class Mat2InCircInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InCircInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quadratic (easing out).
 */
class Mat2OutQuadInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutQuadInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: cubicular (easing out).
 */
class Mat2OutCubicInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutCubicInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quartetic (easing out).
 */
class Mat2OutQuartInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutQuartInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quintic (easing out).
 */
class Mat2OutQuintInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutQuintInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: sineousidal (easing out).
 */
class Mat2OutSineInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutSineInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: exponential (easing out).
 */
class Mat2OutExponentInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutExponentInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: circular (easing out).
 */
class Mat2OutCircInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutCircInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quadratic (easing in, then easing out).
 */
class Mat2InOutQuadInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InOutQuadInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: cubicular (easing in, then easing out).
 */
class Mat2InOutCubicInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InOutCubicInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quartetic (easing in, then easing out).
 */
class Mat2InOutQuartInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InOutQuartInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quintic (easing in, then easing out).
 */
class Mat2InOutQuintInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InOutQuintInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: sineousidal (easing in, then easing out).
 */
class Mat2InOutSineInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InOutSineInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: exponential (easing in, then easing out).
 */
class Mat2InOutExponentInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InOutExponentInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: circular (easing in, then easing out).
 */
class Mat2InOutCircInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2InOutCircInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quadratic (easing out, then easing in).
 */
class Mat2OutInQuadInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutInQuadInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: cubicular (easing out, then easing in).
 */
class Mat2OutInCubicInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutInCubicInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quartetic (easing out, then easing in).
 */
class Mat2OutInQuartInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutInQuartInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: quintic (easing out, then easing in).
 */
class Mat2OutInQuintInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutInQuintInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: sineousidal (easing out, then easing in).
 */
class Mat2OutInSineInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutInSineInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: exponential (easing out, then easing in).
 */
class Mat2OutInExponentInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutInExponentInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat2. Interpolation: circular (easing out, then easing in).
 */
class Mat2OutInCircInterpolationFunction : public InterpolationFunction<tgt::mat2> {

public:
    Mat2OutInCircInterpolationFunction();
    InterpolationFunction<tgt::mat2>* clone() const;
    tgt::mat2 interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
