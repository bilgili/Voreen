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

#ifndef VRN_VOLUMESTRINGINTERPOLATIONFUNCTIONS_H
#define VRN_VOLUMESTRINGINTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

/**
 * This class offers an interpolation function for strings. Interpolation: focus on startvalue.
 */
class StringStartInterpolationFunction : public InterpolationFunction<std::string> {
public:
    StringStartInterpolationFunction();
    InterpolationFunction<std::string>* clone() const;
    std::string interpolate(std::string startvalue, std::string endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for strings. Interpolation: focus on endvalue.
 */
class StringEndInterpolationFunction : public InterpolationFunction<std::string> {
public:
    StringEndInterpolationFunction();
    InterpolationFunction<std::string>* clone() const;
    std::string interpolate(std::string startvalue, std::string endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for strings. Interpolation: bisection.
 */
class StringStartEndInterpolationFunction : public InterpolationFunction<std::string> {
public:
    StringStartEndInterpolationFunction();
    InterpolationFunction<std::string>* clone() const;
    std::string interpolate(std::string startvalue, std::string endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
