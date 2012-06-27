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

#include "voreen/core/properties/numericproperty.h"
#include <sstream>

namespace voreen {

template<typename T>
void NumericProperty<T>::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("value", value_);
}

template<typename T>
void NumericProperty<T>::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    T value;
    s.deserialize("value", value);
    try {
        set(value);
    }
    catch (Condition::ValidationFailed& e) {
        s.addError(e);
    }
}

// explicit template instantiation to enable distribution of
// implementation of template class methods over .h and .cpp files
//
template class NumericProperty<int>;
template class NumericProperty<float>;
template class NumericProperty<tgt::vec2>;
template class NumericProperty<tgt::vec3>;
template class NumericProperty<tgt::vec4>;
template class NumericProperty<tgt::ivec2>;
template class NumericProperty<tgt::ivec3>;
template class NumericProperty<tgt::ivec4>;
template class NumericProperty<tgt::mat2>;
template class NumericProperty<tgt::mat3>;
template class NumericProperty<tgt::mat4>;

} // namespace voreen
