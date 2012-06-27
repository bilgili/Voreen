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

#ifndef VRN_BOOLINTERPOLATIONFUNCTIONS_H
#define VRN_BOOLINTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

/**
 * This class offers an interpolation function for bool-values. Interpolation: focus on startvalue.
 */
class BoolStartInterpolationFunction : public InterpolationFunction<bool> {

public:
    BoolStartInterpolationFunction();
    InterpolationFunction<bool>* clone() const;
    bool interpolate(bool startvalue, bool endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for bool-values. Interpolation: focus on endvalue.
 */
class BoolEndInterpolationFunction : public InterpolationFunction<bool> {

public:
    BoolEndInterpolationFunction();
    InterpolationFunction<bool>* clone() const;
    bool interpolate(bool startvalue, bool endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class offers an interpolation function for bool-values. Interpolation: bisection.
 */
class BoolStartEndInterpolationFunction : public InterpolationFunction<bool> {

public:
    BoolStartEndInterpolationFunction();
    InterpolationFunction<bool>* clone() const;
    bool interpolate(bool startvalue, bool endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
