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

#include "voreen/core/animation/interpolationfunctionfactory.h"

namespace voreen {

template class InterpolationFunctionFactory<float>;
template class InterpolationFunctionFactory<int>;
template class InterpolationFunctionFactory<bool>;
template class InterpolationFunctionFactory<tgt::ivec2>;
template class InterpolationFunctionFactory<tgt::ivec3>;
template class InterpolationFunctionFactory<tgt::ivec4>;
template class InterpolationFunctionFactory<tgt::vec2>;
template class InterpolationFunctionFactory<tgt::vec3>;
template class InterpolationFunctionFactory<tgt::vec4>;
template class InterpolationFunctionFactory<tgt::mat2>;
template class InterpolationFunctionFactory<tgt::mat3>;
template class InterpolationFunctionFactory<tgt::mat4>;
template class InterpolationFunctionFactory<tgt::Camera>;
template class InterpolationFunctionFactory<std::string>;
template class InterpolationFunctionFactory<ShaderSource>;
template class InterpolationFunctionFactory<TransFunc*>;

} // namespace voreen
