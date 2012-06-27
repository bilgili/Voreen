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

#include "modules/vrn_shaderincludes.frag"

uniform SAMPLER2D_TYPE previousRes_;
uniform TEXTURE_PARAMETERS previousResParams_;
uniform SAMPLER2D_TYPE currentSlice_;
uniform TEXTURE_PARAMETERS currentSliceParams_;

/***
 * The main method.
 ***/
void main() {

    vec2 p = gl_FragCoord.xy * screenDimRCP_;
    vec4 previousRes = textureLookup2Dnormalized(previousRes_, previousResParams_, p);
    vec4 currentRes = textureLookup2Dnormalized(currentSlice_, currentSliceParams_, p);

    vec4 result;
    result.rgb = previousRes.rgb + (1.0-previousRes.a) * currentRes.a*currentRes.rgb;
    result.a = previousRes.a + (1.0-previousRes.a) * currentRes.a;
    FragData0 = result;
}
