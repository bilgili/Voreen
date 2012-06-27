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

#ifndef VRN_MAT3INTERPOLATIONFUNCTIONS_H
#define VRN_MAT3INTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

/**
 * This class offers an interpolation function for mat3-values. Interpolation: focus on startvalue.
 */
class Mat3StartInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3StartInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3-values. Interpolation: focus on endvalue.
 */
class Mat3EndInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3EndInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3-values. Interpolation: bisection.
 */
class Mat3StartEndInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3StartEndInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: linear.
 */
class Mat3LinearInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3LinearInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quadratic (easing in).
 */
class Mat3InQuadInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InQuadInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: cubicular (easing in).
 */
class Mat3InCubicInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InCubicInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quartetic (easing in).
 */
class Mat3InQuartInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InQuartInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quintic (easing in).
 */
class Mat3InQuintInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InQuintInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: sineousidal (easing in).
 */
class Mat3InSineInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InSineInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: exponential (easing in).
 */
class Mat3InExponentInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InExponentInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: circular (easing in).
 */
class Mat3InCircInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InCircInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quadratic (easing out).
 */
class Mat3OutQuadInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutQuadInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: cubicular (easing out).
 */
class Mat3OutCubicInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutCubicInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quartetic (easing out).
 */
class Mat3OutQuartInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutQuartInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quintic (easing out).
 */
class Mat3OutQuintInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutQuintInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: sineousidal (easing out).
 */
class Mat3OutSineInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutSineInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: exponential (easing out).
 */
class Mat3OutExponentInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutExponentInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: circular (easing out).
 */
class Mat3OutCircInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutCircInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quadratic (easing in, then easing out).
 */
class Mat3InOutQuadInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InOutQuadInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: cubicular (easing in, then easing out).
 */
class Mat3InOutCubicInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InOutCubicInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quartetic (easing in, then easing out).
 */
class Mat3InOutQuartInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InOutQuartInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quintic (easing in, then easing out).
 */
class Mat3InOutQuintInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InOutQuintInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: sineousidal (easing in, then easing out).
 */
class Mat3InOutSineInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InOutSineInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: exponential (easing in, then easing out).
 */
class Mat3InOutExponentInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InOutExponentInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: circular (easing in, then easing out).
 */
class Mat3InOutCircInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3InOutCircInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quadratic (easing out, then easing in).
 */
class Mat3OutInQuadInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutInQuadInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: cubicular (easing out, then easing in).
 */
class Mat3OutInCubicInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutInCubicInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quartetic (easing out, then easing in).
 */
class Mat3OutInQuartInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutInQuartInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: quintic (easing out, then easing in).
 */
class Mat3OutInQuintInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutInQuintInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: sineousidal (easing out, then easing in).
 */
class Mat3OutInSineInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutInSineInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: exponential (easing out, then easing in).
 */
class Mat3OutInExponentInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutInExponentInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for mat3. Interpolation: circular (easing out, then easing in).
 */
class Mat3OutInCircInterpolationFunction : public InterpolationFunction<tgt::mat3> {
public:
    Mat3OutInCircInterpolationFunction();
    InterpolationFunction<tgt::mat3>* clone() const;
    tgt::mat3 interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
