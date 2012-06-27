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

#ifndef VRN_TRANSFUNCINTERPOLATIONFUNCTIONS_H
#define VRN_TRANSFUNCINTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"
#include "voreen/core/properties/transfuncproperty.h"

namespace voreen {

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API InterpolationFunction<TransFunc*>;
#endif

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions. Interpolation: focus on startvalue.
 */
class VRN_CORE_API TransFuncStartInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncStartInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions. Interpolation: focus on endvalue.
 */
class VRN_CORE_API TransFuncEndInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncEndInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions. Interpolation: bisection.
 */
class VRN_CORE_API TransFuncStartEndInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncStartEndInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys
 * the functions will be interpolated linearly (keywise).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};


/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuadInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuadInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuadOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuadOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuadInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuadInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuadOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuadOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCubicInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCubicInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCubicOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCubicOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCubicInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCubicInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCubicOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCubicOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuartInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuartInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuartOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuartOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuartInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuartInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuartOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuartOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuintInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuintInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuintOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuintOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuintInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuintInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuintOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuintOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseSineInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseSineInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseSineOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseSineOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseSineInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseSineInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseSineOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseSineOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseExponentInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseExponentInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseExponentOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseExponentOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseExponentInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseExponentInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseExponentOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseExponentOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCircInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCircInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCircOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCircOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCircInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCircInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCircOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCircOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated linearly.
 */
class VRN_CORE_API TransFuncTextureLinearInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureLinearInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-in).
 */
class VRN_CORE_API TransFuncTextureQuadInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuadInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-out).
 */
class VRN_CORE_API TransFuncTextureQuadOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuadOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureQuadInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuadInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureQuadOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuadOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-in).
 */
class VRN_CORE_API TransFuncTextureCubicInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCubicInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-out).
 */
class VRN_CORE_API TransFuncTextureCubicOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCubicOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureCubicInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCubicInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureCubicOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCubicOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-in).
 */
class VRN_CORE_API TransFuncTextureQuartInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuartInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-out).
 */
class VRN_CORE_API TransFuncTextureQuartOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuartOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureQuartInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuartInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureQuartOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuartOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-in).
 */
class VRN_CORE_API TransFuncTextureQuintInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuintInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-out).
 */
class VRN_CORE_API TransFuncTextureQuintOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuintOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureQuintInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuintInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureQuintOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuintOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-in).
 */
class VRN_CORE_API TransFuncTextureSineInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureSineInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-out).
 */
class VRN_CORE_API TransFuncTextureSineOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureSineOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureSineInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureSineInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureSineOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureSineOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-in).
 */
class VRN_CORE_API TransFuncTextureExponentInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureExponentInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-out).
 */
class VRN_CORE_API TransFuncTextureExponentOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureExponentOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureExponentInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureExponentInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureExponentOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureExponentOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-in).
 */
class VRN_CORE_API TransFuncTextureCircInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCircInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-out).
 */
class VRN_CORE_API TransFuncTextureCircOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCircOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureCircInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCircInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureCircOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCircOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
