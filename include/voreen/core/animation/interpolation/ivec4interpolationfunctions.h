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

#ifndef VRN_IVEC4INTERPOLATIONFUNCTIONS_H
#define VRN_IVEC4INTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

/**
 * This class offers an interpolation function for ivec4-values. Interpolation: focus on startvalue.
 */
class IVec4StartInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4StartInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4-values. Interpolation: focus on endvalue.
 */
class IVec4EndInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4EndInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4-values. Interpolation: bisection.
 */
class IVec4StartEndInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4StartEndInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: linear.
 */
class IVec4LinearInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4LinearInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quadratic (easing in).
 */
class IVec4InQuadInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: cubicular (easing in).
 */
class IVec4InCubicInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quartetic (easing in).
 */
class IVec4InQuartInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quintic (easing in).
 */
class IVec4InQuintInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: sineousidal (easing in).
 */
class IVec4InSineInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InSineInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: exponential (easing in).
 */
class IVec4InExponentInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: circular (easing in).
 */
class IVec4InCircInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InCircInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quadratic (easing out).
 */
class IVec4OutQuadInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: cubicular (easing out).
 */
class IVec4OutCubicInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quartetic (easing out).
 */
class IVec4OutQuartInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quintic (easing out).
 */
class IVec4OutQuintInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: sineousidal (easing out).
 */
class IVec4OutSineInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutSineInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: exponential (easing out).
 */
class IVec4OutExponentInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: circular (easing out).
 */
class IVec4OutCircInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutCircInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quadratic (easing in, then easing out).
 */
class IVec4InOutQuadInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: cubicular (easing in, then easing out).
 */
class IVec4InOutCubicInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quartetic (easing in, then easing out).
 */
class IVec4InOutQuartInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quintic (easing in, then easing out).
 */
class IVec4InOutQuintInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: sineousidal (easing in, then easing out).
 */
class IVec4InOutSineInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutSineInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: exponential (easing in, then easing out).
 */
class IVec4InOutExponentInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: circular (easing in, then easing out).
 */
class IVec4InOutCircInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutCircInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quadratic (easing out, then easing in).
 */
class IVec4OutInQuadInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: cubicular (easing out, then easing in).
 */
class IVec4OutInCubicInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quartetic (easing out, then easing in).
 */
class IVec4OutInQuartInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: quintic (easing out, then easing in).
 */
class IVec4OutInQuintInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: sineousidal (easing out, then easing in).
 */
class IVec4OutInSineInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInSineInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: exponential (easing out, then easing in).
 */
class IVec4OutInExponentInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for ivec4. Interpolation: circular (easing out, then easing in).
 */
class IVec4OutInCircInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInCircInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
