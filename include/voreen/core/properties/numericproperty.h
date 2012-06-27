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

#ifndef VRN_NUMERICPROPERTY_H
#define VRN_NUMERICPROPERTY_H

#include "voreen/core/properties/condition.h"
#include "voreen/core/properties/templateproperty.h"

namespace voreen {

/**
 * Template for properties that store numeric values.
 *
 * Besides the allowed value range, some GUI hints can be defined,
 * such as the stepping and number of decimals.
 */
template<typename T>
class NumericProperty : public TemplateProperty<T> {

template<typename> friend class NumericPropertyValidation;

public:
    typedef T ElemType;

    NumericProperty(const std::string& id, const std::string& guiText, const T& value,
                    const T& minValue, const T& maxValue, const T& stepping,
                    Processor::InvalidationLevel invalidationLevel=Processor::INVALID_RESULT);

    /**
     * Sets the minimum value the variable can take.
     * This is checked by a NumericPropertyValidation on each
     * set() call. Therefore, it is guaranteed that the property
     * never attains a value outside its valid range.
     */
    void setMinValue(const T& minValue);
    const T& getMinValue() const;

    /**
     * Sets the maximum value the variable can take.
     * This is checked by a NumericPropertyValidation on each
     * set() call. Therefore, it is guaranteed that the property
     * never attains a value outside its valid range.
     */
    void setMaxValue(const T& maxValue);
    const T& getMaxValue() const;

    /**
     * Sets the property's minimum increase/decrease,
     * which should be used in GUI widgets representing
     * the property, e.g., a spin box.
     *
     * @see increase, decrease
     */
    void setStepping(const T stepping);
    T getStepping() const;

    /**
     * Sets the number of decimals that should be
     * displayed by a GUI representation of the property.
     */
    void setNumDecimals(size_t numDecimals);
    size_t getNumDecimals() const;

    /**
     * If tracking is disabled, the property is not to be updated
     * during user interactions, e.g., while the user drags a slider.
     * Tracking is enabled by default.
     */
    void setTracking(bool tracking);
    bool hasTracking() const;

    /**
     * Increases the property's value by its stepping.
     * @see setStepping
     */
    void increase();

    /**
     * Decreases the property's value by its stepping.
     * @see setStepping
     */
    void decrease();

    /// @see Property::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Property::deserialize
    virtual void deserialize(XmlDeserializer& s);

protected:
    using TemplateProperty<T>::value_;

    T minValue_;
    T maxValue_;
    T stepping_;
    bool tracking_;
    size_t numDecimals_;
};

} // namespace voreen

#endif // VRN_NUMERICPROPERTY_H
