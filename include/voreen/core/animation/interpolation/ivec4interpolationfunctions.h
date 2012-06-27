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

#ifndef VRN_IVEC4INTERPOLATIONFUNCTIONS_H
#define VRN_IVEC4INTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API InterpolationFunction<tgt::ivec4>;
#endif

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4-values. Interpolation: focus on startvalue.
 */
class VRN_CORE_API IVec4StartInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4StartInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4-values. Interpolation: focus on endvalue.
 */
class VRN_CORE_API IVec4EndInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4EndInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4-values. Interpolation: bisection.
 */
class VRN_CORE_API IVec4StartEndInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4StartEndInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: linear.
 */
class VRN_CORE_API IVec4LinearInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4LinearInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quadratic (easing in).
 */
class VRN_CORE_API IVec4InQuadInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: cubicular (easing in).
 */
class VRN_CORE_API IVec4InCubicInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quartetic (easing in).
 */
class VRN_CORE_API IVec4InQuartInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quintic (easing in).
 */
class VRN_CORE_API IVec4InQuintInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: sineousidal (easing in).
 */
class VRN_CORE_API IVec4InSineInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InSineInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: exponential (easing in).
 */
class VRN_CORE_API IVec4InExponentInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: circular (easing in).
 */
class VRN_CORE_API IVec4InCircInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InCircInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quadratic (easing out).
 */
class VRN_CORE_API IVec4OutQuadInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: cubicular (easing out).
 */
class VRN_CORE_API IVec4OutCubicInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quartetic (easing out).
 */
class VRN_CORE_API IVec4OutQuartInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quintic (easing out).
 */
class VRN_CORE_API IVec4OutQuintInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: sineousidal (easing out).
 */
class VRN_CORE_API IVec4OutSineInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutSineInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: exponential (easing out).
 */
class VRN_CORE_API IVec4OutExponentInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: circular (easing out).
 */
class VRN_CORE_API IVec4OutCircInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutCircInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quadratic (easing in, then easing out).
 */
class VRN_CORE_API IVec4InOutQuadInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: cubicular (easing in, then easing out).
 */
class VRN_CORE_API IVec4InOutCubicInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quartetic (easing in, then easing out).
 */
class VRN_CORE_API IVec4InOutQuartInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quintic (easing in, then easing out).
 */
class VRN_CORE_API IVec4InOutQuintInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: sineousidal (easing in, then easing out).
 */
class VRN_CORE_API IVec4InOutSineInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutSineInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: exponential (easing in, then easing out).
 */
class VRN_CORE_API IVec4InOutExponentInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: circular (easing in, then easing out).
 */
class VRN_CORE_API IVec4InOutCircInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4InOutCircInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quadratic (easing out, then easing in).
 */
class VRN_CORE_API IVec4OutInQuadInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInQuadInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: cubicular (easing out, then easing in).
 */
class VRN_CORE_API IVec4OutInCubicInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInCubicInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quartetic (easing out, then easing in).
 */
class VRN_CORE_API IVec4OutInQuartInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInQuartInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: quintic (easing out, then easing in).
 */
class VRN_CORE_API IVec4OutInQuintInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInQuintInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: sineousidal (easing out, then easing in).
 */
class VRN_CORE_API IVec4OutInSineInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInSineInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: exponential (easing out, then easing in).
 */
class VRN_CORE_API IVec4OutInExponentInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInExponentInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for ivec4. Interpolation: circular (easing out, then easing in).
 */
class VRN_CORE_API IVec4OutInCircInterpolationFunction : public InterpolationFunction<tgt::ivec4> {
public:
    IVec4OutInCircInterpolationFunction();
    InterpolationFunction<tgt::ivec4>* clone() const;
    tgt::ivec4 interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
