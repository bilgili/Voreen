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

#include "voreen/core/animation/interpolation/shadersourceinterpolationfunctions.h"

namespace voreen {

ShaderSourceStartInterpolationFunction::ShaderSourceStartInterpolationFunction() {}

std::string ShaderSourceStartInterpolationFunction::getMode() const {
    return "focus on startvalue";
}

std::string ShaderSourceStartInterpolationFunction::getIdentifier() const {
    return "boolean";
}

ShaderSource ShaderSourceStartInterpolationFunction::interpolate(ShaderSource startvalue, ShaderSource endvalue, float time) const {
    if (time < 1.f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<ShaderSource>* ShaderSourceStartInterpolationFunction::clone() const {
    return new ShaderSourceStartInterpolationFunction();
}

ShaderSourceEndInterpolationFunction::ShaderSourceEndInterpolationFunction() {}

std::string ShaderSourceEndInterpolationFunction::getMode() const {
    return "focus on endvalue";
}

std::string ShaderSourceEndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

ShaderSource ShaderSourceEndInterpolationFunction::interpolate(ShaderSource startvalue, ShaderSource endvalue, float time) const {
    if (time > 0.f)
        return endvalue;
    else
        return startvalue;
}

InterpolationFunction<ShaderSource>* ShaderSourceEndInterpolationFunction::clone() const {
    return new ShaderSourceEndInterpolationFunction();
}

ShaderSourceStartEndInterpolationFunction::ShaderSourceStartEndInterpolationFunction() {}

std::string ShaderSourceStartEndInterpolationFunction::getMode() const {
    return "bisection";
}

std::string ShaderSourceStartEndInterpolationFunction::getIdentifier() const {
    return "boolean";
}

ShaderSource ShaderSourceStartEndInterpolationFunction::interpolate(ShaderSource startvalue, ShaderSource endvalue, float time) const {
    if (time < 0.5f)
        return startvalue;
    else
        return endvalue;
}

InterpolationFunction<ShaderSource>* ShaderSourceStartEndInterpolationFunction::clone() const {
    return new ShaderSourceStartEndInterpolationFunction();
}

} // namespace voreen
