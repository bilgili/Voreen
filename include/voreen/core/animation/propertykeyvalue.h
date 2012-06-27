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

#ifndef VRN_PROPERTYKEYVALUE_H
#define VRN_PROPERTYKEYVALUE_H

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

template <class T> class InterpolationFunction;
template <class T> class MultiPointInterpolationFunction;
template <class T> class TemplatePropertyTimelineState;

/**
 * Non-templated base class for generic PropertyKeyValues.
 */
class PropertyKeyValueBase : public AbstractSerializable {
public:
    PropertyKeyValueBase();
    virtual float getTime() const = 0;
};

/**
 * Class for creating an animation.
 * The KeyValue stores one property value at one corresponding time.
 */
template <class T> class PropertyKeyValue : public PropertyKeyValueBase {
public:
    /**
     * The key value has to be initialized with its value.
     */
    PropertyKeyValue(T value, float time);

    /**
     * Destructor.
     */
    virtual ~PropertyKeyValue();

    /**
     * Function to get the time.
     */
    float getTime() const;

    /**
     * Function to get the value.
     */
    const T getValue() const;

    /**
     * Gets the interpolation function after the keyvalue.
     */
    const InterpolationFunction<T>* getFollowingInterpolationFunction() const;

    /**
     * Gets the interpolation function before the keyvalue.
     */
    const InterpolationFunction<T>* getForegoingInterpolationFunction() const;

    /**
     * Returns the variable smooth.
     * Smoothness is a flag of a key value enabling continuity of the respective interpolation function at that specific key value
     */
    bool getSmooth() const;

    /**
     * Returns true if smooth is true and the following and the foregoing interpolation function are of the same type of multipointinterpolation.
     */
    bool isSmoothed() const;

    /**
     * Returns a copy of the current keyvalue.
     * The value and the time will be copied, but not the linked interpolation functions.
     */
    PropertyKeyValue<T>* clone() const;

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

protected:
    friend class TemplatePropertyTimelineState<T>;
    friend class TransFuncPropertyTimelineState;
    friend class CameraPropertyTimelineState;
    friend class ShaderSourcePropertyTimelineState;
    friend class XmlDeserializer;

    /**
     * Default constructor.
     */
    PropertyKeyValue();

    /**
     * Function to set the value.
     */
    void setValue(T value);

    /**
     * Sets the inner variable smooth.
     * Explanation of this variable: see function isSmoothed().
     */
    void setSmooth(bool smooth);

    /**
     * Function to set the time.
     */
    void setTime(float time);

    /**
     * Sets the interpolation function before the key value.
     */
    void setForegoingInterpolationFunction(InterpolationFunction<T>* func);

    /**
     * Sets the interpolation function following the key value.
     */
    void setFollowingInterpolationFunction(InterpolationFunction<T>* func);

    float time_;
    T value_;
    InterpolationFunction<T>* before_;
    InterpolationFunction<T>* after_;
    bool smooth_;
};

} // namespace voreen

#endif
