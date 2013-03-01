/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

/// Base class for transfer function interpolation, provides commonly used methods.
class VRN_CORE_API TransFuncInterpolationFunctionBase : public InterpolationFunction<TransFunc*> {
protected:
    static GLubyte* convertTextureToRGBA(tgt::ivec3 dim, GLubyte* textur, GLuint inputformat);
    static GLubyte* changeTextureDimension(tgt::ivec3 in_dim, tgt::ivec3 out_dim, GLubyte* indata);
};

/// Default interpolation
class VRN_CORE_API TransFuncInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions. Interpolation: focus on startvalue.
 */
class VRN_CORE_API TransFuncStartInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncStartInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncStartInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions. Interpolation: focus on endvalue.
 */
class VRN_CORE_API TransFuncEndInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncEndInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncEndInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions. Interpolation: bisection.
 */
class VRN_CORE_API TransFuncStartEndInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncStartEndInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncStartEndInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys
 * the functions will be interpolated linearly (keywise).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuadInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuadInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuadInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuadOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuadOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuadOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuadInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuadInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuadInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quadratically (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuadOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuadOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuadOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCubicInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseCubicInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseCubicInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCubicOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseCubicOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseCubicOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCubicInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseCubicInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseCubicInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated cubicularly (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCubicOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseCubicOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseCubicOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuartInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuartInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuartInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuartOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuartOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuartOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuartInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuartInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuartInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quartetically (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuartOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuartOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuartOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuintInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuintInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuintInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuintOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuintOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuintOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuintInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuintInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuintInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated quintically (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseQuintOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseQuintOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseQuintOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseSineInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseSineInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseSineInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseSineOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseSineOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseSineOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseSineInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseSineInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseSineInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated sineousidally (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseSineOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseSineOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseSineOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseExponentInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseExponentInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseExponentInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseExponentOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseExponentOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseExponentOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseExponentInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseExponentInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseExponentInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated exponentially (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseExponentOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseExponentOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseExponentOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCircInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseCircInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseCircInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCircOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseCircOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseCircOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing in, then easing out).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCircInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseCircInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseCircInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * If the startfunction and the endfunction are both 1D-functions with the same number of keys,
 * the functions will be interpolated circularly (keywise, easing out, then easing in).
 * If not this functions falls back to a default function like TransFuncStartInterpolationFunction.
 */
class VRN_CORE_API TransFuncKeyWiseCircOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncKeyWiseCircOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncKeyWiseCircOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated linearly.
 */
class VRN_CORE_API TransFuncTextureLinearInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureLinearInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureLinearInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-in).
 */
class VRN_CORE_API TransFuncTextureQuadInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuadInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuadInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-out).
 */
class VRN_CORE_API TransFuncTextureQuadOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuadOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuadOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureQuadInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuadInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuadInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quadratically (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureQuadOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuadOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuadOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-in).
 */
class VRN_CORE_API TransFuncTextureCubicInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureCubicInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureCubicInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-out).
 */
class VRN_CORE_API TransFuncTextureCubicOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureCubicOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureCubicOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureCubicInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureCubicInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureCubicInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated cubicularly (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureCubicOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureCubicOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureCubicOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-in).
 */
class VRN_CORE_API TransFuncTextureQuartInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuartInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuartInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-out).
 */
class VRN_CORE_API TransFuncTextureQuartOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuartOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuartOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureQuartInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuartInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuartInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quartetically (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureQuartOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuartOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuartOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-in).
 */
class VRN_CORE_API TransFuncTextureQuintInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuintInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuintInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-out).
 */
class VRN_CORE_API TransFuncTextureQuintOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuintOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuintOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureQuintInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuintInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuintInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated quintically (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureQuintOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureQuintOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureQuintOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-in).
 */
class VRN_CORE_API TransFuncTextureSineInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureSineInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureSineInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-out).
 */
class VRN_CORE_API TransFuncTextureSineOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureSineOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureSineOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureSineInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureSineInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureSineInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated sineousidally (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureSineOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureSineOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureSineOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-in).
 */
class VRN_CORE_API TransFuncTextureExponentInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureExponentInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureExponentInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-out).
 */
class VRN_CORE_API TransFuncTextureExponentOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureExponentOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureExponentOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureExponentInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureExponentInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureExponentInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated exponentially (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureExponentOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureExponentOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureExponentOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-in).
 */
class VRN_CORE_API TransFuncTextureCircInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureCircInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureCircInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-out).
 */
class VRN_CORE_API TransFuncTextureCircOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureCircOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureCircOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-in, then easing-out).
 */
class VRN_CORE_API TransFuncTextureCircInOutInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureCircInOutInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureCircInOutInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

/**
 * This class offers an interpolation function for transfer functions.
 * The two textures of the transferfunctions are interpolated circularly (easing-out, then easing-in).
 */
class VRN_CORE_API TransFuncTextureCircOutInInterpolationFunction : public TransFuncInterpolationFunctionBase {
public:
    TransFuncTextureCircOutInInterpolationFunction();
    virtual std::string getClassName() const { return "TransFuncTextureCircOutInInterpolationFunction"; }
    InterpolationFunction<TransFunc*>* create() const;
    TransFunc* interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

    std::string getGuiName() const;
    std::string getCategory() const;
};

} // namespace voreen
#endif
