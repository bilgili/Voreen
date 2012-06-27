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

#ifndef VRN_INTERPOLATIONFUNCTION_H
#define VRN_INTERPOLATIONFUNCTION_H

#include <string>
#include <vector>
#include "voreen/core/animation/interpolationfunctionbase.h"
#include "voreen/core/properties/propertyowner.h"
//#include "voreen/core/animation/propertykeyvalue.h"
#include "tgt/camera.h"
#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/transfuncproperty.h"

namespace voreen {

class Property;
template <class T> class PropertyKeyValue;

/**
 * This class represents an interpolation function between two values.
 */
template <class T>
class InterpolationFunction : public PropertyOwner, public InterpolationFunctionBase {
public:
    /**
     * Constructor.
     */
    InterpolationFunction();
    virtual ~InterpolationFunction();

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

template <class T>
class MultiPointInterpolationFunction : public InterpolationFunction<T> {
public:
    virtual InterpolationFunction<T>* clone() const = 0;

    /**
     * Interpolation (using several controlpoints).
     */
    virtual T interpolate(std::vector<PropertyKeyValue<T>*> controlpoints, float time) const = 0;
};

template <class T>
InterpolationFunction<T>::InterpolationFunction() {
}

template <class T>
InterpolationFunction<T>::~InterpolationFunction() {}

template <class T>
std::string InterpolationFunction<T>::getName() const {
    return getIdentifier() + ": " + getMode();
}

template <>
VRN_CORE_API std::string InterpolationFunction<int>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<float>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<bool>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::vec2>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::vec3>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::vec4>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::quat>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::mat2>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::mat3>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::mat4>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::ivec2>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::ivec3>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::ivec4>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::Camera>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<std::string>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<ShaderSource>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<TransFunc*>::getMode() const;

template <>
VRN_CORE_API std::string InterpolationFunction<int>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<float>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<bool>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::vec2>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::vec3>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::vec4>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::quat>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::mat2>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::mat3>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::mat4>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::ivec2>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::ivec3>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::ivec4>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<tgt::Camera>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<std::string>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<ShaderSource>::getIdentifier() const;

template <>
VRN_CORE_API std::string InterpolationFunction<TransFunc*>::getIdentifier() const;

template <class T>
InterpolationFunction<T>* InterpolationFunction<T>::clone() const {
    return new InterpolationFunction<T>();
}

template <>
VRN_CORE_API int InterpolationFunction<int>::interpolate(int startvalue, int endvalue, float time) const;

template <>
VRN_CORE_API float InterpolationFunction<float>::interpolate(float startvalue, float endvalue, float time) const;

template <>
VRN_CORE_API tgt::vec2 InterpolationFunction<tgt::vec2>::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const;

template <>
VRN_CORE_API tgt::vec3 InterpolationFunction<tgt::vec3>::interpolate(tgt::vec3 startvalue, tgt::vec3 endvalue, float time) const;

template <>
VRN_CORE_API tgt::vec4 InterpolationFunction<tgt::vec4>::interpolate(tgt::vec4 startvalue, tgt::vec4 endvalue, float time) const;

template <>
VRN_CORE_API tgt::quat InterpolationFunction<tgt::quat>::interpolate(tgt::quat startvalue, tgt::quat endvalue, float time) const;

template <>
VRN_CORE_API tgt::mat2 InterpolationFunction<tgt::mat2>::interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const;

template <>
VRN_CORE_API tgt::mat3 InterpolationFunction<tgt::mat3>::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const;

template <>
VRN_CORE_API tgt::mat4 InterpolationFunction<tgt::mat4>::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const;

template <>
VRN_CORE_API tgt::ivec2 InterpolationFunction<tgt::ivec2>::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const;

template <>
VRN_CORE_API tgt::ivec3 InterpolationFunction<tgt::ivec3>::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const;

template <>
VRN_CORE_API tgt::ivec4 InterpolationFunction<tgt::ivec4>::interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const;

template <>
VRN_CORE_API bool InterpolationFunction<bool>::interpolate(bool startvalue, bool endvalue, float time) const;

template <>
VRN_CORE_API tgt::Camera InterpolationFunction<tgt::Camera>::interpolate(tgt::Camera startvalue, tgt::Camera endvalue, float time) const;

template <>
VRN_CORE_API std::string InterpolationFunction<std::string>::interpolate(std::string startvalue, std::string endvalue, float time) const;

template <>
VRN_CORE_API ShaderSource InterpolationFunction<ShaderSource>::interpolate(ShaderSource startvalue, ShaderSource endvalue, float time) const;

template <>
VRN_CORE_API TransFunc* InterpolationFunction<TransFunc*>::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const;

template <class T>
void InterpolationFunction<T>::serialize(XmlSerializer& s) const {
    s.serialize("properties", this->getProperties());
}

template <class T>
void InterpolationFunction<T>::deserialize(XmlDeserializer& s) {
    std::vector<Property*> props;
    s.deserialize("properties", props);
    for (size_t i = 0; i < props.size(); ++i)
        addProperty(props.at(i));
}

} // namespace voreen

#endif
