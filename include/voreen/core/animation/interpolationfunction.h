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

#ifndef VRN_INTERPOLATIONFUNCTION_H
#define VRN_INTERPOLATIONFUNCTION_H

#include <string>
#include <vector>
#include "voreen/core/animation/interpolationfunctionbase.h"
#include "voreen/core/properties/propertyowner.h"
#include "voreen/core/animation/propertykeyvalue.h"

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

class Property;
template <class T> class PropertyKeyValue;

/**
 * This class represents an interpolation function between two values.
 */
template <class T> class InterpolationFunction : public PropertyOwner, public InterpolationFunctionBase {
public:

    /**
     * Constructor.
     */
    InterpolationFunction<T>();
    virtual ~InterpolationFunction<T>() {}

    /**
     * returns a copy of itself.
     */
    virtual InterpolationFunction<T>* clone() const;

    /**
     * Interpolates between the given values: start value und endvalue in the time interval [0,1].
     */
    virtual T interpolate(T startvalue, T endvalue, float time) const;

    /**
     * returns getIdentifier() + getMode() as concanated string
     */
    virtual std::string getName() const;

    /**
     * Returns the mode of the interpolation (easing in, easing out...).
     */
    virtual std::string getMode() const;

    /**
     * Returns the identifier of the interpolation (linear, quadratic...).
     */
    virtual std::string getIdentifier() const;

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);
};

template <class T> class MultiPointInterpolationFunction : public InterpolationFunction<T> {
public:
    virtual InterpolationFunction<T>* clone() const = 0;

    /**
     * Interpolation (using several controlpoints).
     */
    virtual T interpolate(std::vector<PropertyKeyValue<T>*> controlpoints, float time) const = 0;

protected:
};

} // namespace voreen

#endif
