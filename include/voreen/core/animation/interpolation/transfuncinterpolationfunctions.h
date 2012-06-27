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

#ifndef VRN_TRANSFUNCINTERPOLATIONFUNCTIONS_H
#define VRN_TRANSFUNCINTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"
#include "voreen/core/properties/transfuncproperty.h"

namespace voreen {

/**
 * This class offers an interpolation function for transfer functions. Interpolation: focus on startvalue.
 */
class TransFuncStartInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncStartInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions. Interpolation: focus on endvalue.
 */
class TransFuncEndInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncEndInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions. Interpolation: bisection.
 */
class TransFuncStartEndInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncStartEndInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys
 * the functions will be interpolated linearly (keywise).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};


/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuadInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuadInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuadOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuadOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuadInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuadInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuadOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuadOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseCubicInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCubicInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseCubicOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCubicOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseCubicInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCubicInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseCubicOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCubicOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuartInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuartInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuartOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuartOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuartInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuartInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuartOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuartOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuintInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuintInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuintOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuintOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuintInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuintInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseQuintOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseQuintOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseSineInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseSineInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseSineOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseSineOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseSineInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseSineInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseSineOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseSineOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseExponentInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseExponentInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseExponentOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseExponentOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseExponentInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseExponentInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseExponentOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseExponentOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseCircInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCircInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseCircOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCircOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseCircInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCircInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class TransFuncKeyWiseCircOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncKeyWiseCircOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated linearly.
 */
class TransFuncTextureLinearInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureLinearInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-in).
 */
class TransFuncTextureQuadInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuadInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-out).
 */
class TransFuncTextureQuadOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuadOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-in, then easing-out).
 */
class TransFuncTextureQuadInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuadInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-out, then easing-in).
 */
class TransFuncTextureQuadOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuadOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-in).
 */
class TransFuncTextureCubicInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCubicInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-out).
 */
class TransFuncTextureCubicOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCubicOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-in, then easing-out).
 */
class TransFuncTextureCubicInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCubicInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-out, then easing-in).
 */
class TransFuncTextureCubicOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCubicOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-in).
 */
class TransFuncTextureQuartInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuartInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-out).
 */
class TransFuncTextureQuartOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuartOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-in, then easing-out).
 */
class TransFuncTextureQuartInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuartInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-out, then easing-in).
 */
class TransFuncTextureQuartOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuartOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-in).
 */
class TransFuncTextureQuintInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuintInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-out).
 */
class TransFuncTextureQuintOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuintOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-in, then easing-out).
 */
class TransFuncTextureQuintInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuintInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-out, then easing-in).
 */
class TransFuncTextureQuintOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureQuintOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-in).
 */
class TransFuncTextureSineInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureSineInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-out).
 */
class TransFuncTextureSineOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureSineOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-in, then easing-out).
 */
class TransFuncTextureSineInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureSineInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-out, then easing-in).
 */
class TransFuncTextureSineOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureSineOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-in).
 */
class TransFuncTextureExponentInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureExponentInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-out).
 */
class TransFuncTextureExponentOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureExponentOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-in, then easing-out).
 */
class TransFuncTextureExponentInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureExponentInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-out, then easing-in).
 */
class TransFuncTextureExponentOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureExponentOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-in).
 */
class TransFuncTextureCircInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCircInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-out).
 */
class TransFuncTextureCircOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCircOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-in, then easing-out).
 */
class TransFuncTextureCircInOutInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCircInOutInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-out, then easing-in).
 */
class TransFuncTextureCircOutInInterpolationFunction : public InterpolationFunction<TransFunc*> {
public:
    TransFuncTextureCircOutInInterpolationFunction();
    InterpolationFunction<TransFunc*>* clone() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
