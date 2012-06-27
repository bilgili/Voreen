/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

uniform float sketchExp_;
uniform float sketchTH_;
uniform vec4 sketchColor_;

vec4 sketchShading(in vec4 normal, in vec3 first, in float t, in vec3 dir, vec3 kd) {

    // obtain normal and voxel position in texspace
    normal.xyz = normal.xyz * 2.0 - vec3(1.0);
    normal.w = 0.0;
    vec4 vpos = vec4(first + t * dir, 1.0);

    // transform normal and voxel position to eyespace
    vpos.xyz = vpos.xyz * 2.0 - vec3(1.0);
    vpos = viewMatrix_ * vpos;
    normal = viewMatrix_ * normal;

    vec3 N = normalize(normal.xyz);
    vec3 V = normalize(-vpos.xyz);

    float NdotV = max(dot(N,V),0.0);
        
    if (length(normal) > sketchTH_)
		return sketchColor_*vec4(1.0-pow(NdotV, sketchExp_));
    else return vec4(0.0);
}