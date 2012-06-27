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

#ifndef VRN_IVEC3INTERPOLATIONFUNCTIONS_H
#define VRN_IVEC3INTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

/**
 * This class offers an interpolation function for ivec3-values. Interpolation: focus on startvalue.
 */
class IVec3StartInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3StartInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3-values. Interpolation: focus on endvalue.
 */
class IVec3EndInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3EndInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3-values. Interpolation: bisection.
 */
class IVec3StartEndInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3StartEndInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: linear.
 */
class IVec3LinearInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3LinearInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quadratic (easing in).
 */
class IVec3InQuadInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: cubicular (easing in).
 */
class IVec3InCubicInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quartetic (easing in).
 */
class IVec3InQuartInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quintic (easing in).
 */
class IVec3InQuintInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: sineousidal (easing in).
 */
class IVec3InSineInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InSineInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: exponential (easing in).
 */
class IVec3InExponentInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: circular (easing in).
 */
class IVec3InCircInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InCircInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quadratic (easing out).
 */
class IVec3OutQuadInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: cubicular (easing out).
 */
class IVec3OutCubicInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quartetic (easing out).
 */
class IVec3OutQuartInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quintic (easing out).
 */
class IVec3OutQuintInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: sineousidal (easing out).
 */
class IVec3OutSineInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutSineInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: exponential (easing out).
 */
class IVec3OutExponentInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: circular (easing out).
 */
class IVec3OutCircInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutCircInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quadratic (easing in, then easing out).
 */
class IVec3InOutQuadInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InOutQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: cubicular (easing in, then easing out).
 */
class IVec3InOutCubicInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InOutCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quartetic (easing in, then easing out).
 */
class IVec3InOutQuartInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InOutQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quintic (easing in, then easing out).
 */
class IVec3InOutQuintInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InOutQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: sineousidal (easing in, then easing out).
 */
class IVec3InOutSineInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InOutSineInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: exponential (easing in, then easing out).
 */
class IVec3InOutExponentInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InOutExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: circular (easing in, then easing out).
 */
class IVec3InOutCircInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3InOutCircInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quadratic (easing out, then easing in).
 */
class IVec3OutInQuadInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutInQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: cubicular (easing out, then easing in).
 */
class IVec3OutInCubicInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutInCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quartetic (easing out, then easing in).
 */
class IVec3OutInQuartInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutInQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: quintic (easing out, then easing in).
 */
class IVec3OutInQuintInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutInQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: sineousidal (easing out, then easing in).
 */
class IVec3OutInSineInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutInSineInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: exponential (easing out, then easing in).
 */
class IVec3OutInExponentInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutInExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec3. Interpolation: circular (easing out, then easing in).
 */
class IVec3OutInCircInterpolationFunction : public InterpolationFunction<tgt::ivec3> {
public:
    IVec3OutInCircInterpolationFunction();
    InterpolationFunction<tgt::ivec3>* clone() const;
    tgt::ivec3 interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
