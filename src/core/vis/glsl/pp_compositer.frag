/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE shadeTex0_;
uniform SAMPLER2D_TYPE depthTex0_;
uniform SAMPLER2D_TYPE shadeTex1_;
uniform SAMPLER2D_TYPE depthTex1_;

uniform int compositingMode_;
uniform float blendFactor_;

void main() {

    // fetch input textures
    vec4 shadeCol0 = textureLookup2D(shadeTex0_, gl_FragCoord.xy);
    float depth0 = textureLookup2D(depthTex0_, gl_FragCoord.xy).z;
    vec4 shadeCol1 = textureLookup2D(shadeTex1_, gl_FragCoord.xy);
    float depth1 = textureLookup2D(depthTex1_, gl_FragCoord.xy).z;

    vec4 fragColor = vec4(0.0);
    float fragDepth = 1.0;

    if (compositingMode_ == 0) {
        // depth test
        if (depth0 < depth1) {
            fragColor = shadeCol0;
            fragDepth = depth0;
        } else {
            fragColor = shadeCol1;
            fragDepth = depth1;
        }
    } else if (compositingMode_ == 1) {
        // first has priority
        fragColor = shadeCol0;
        fragDepth = depth0;
    } else if (compositingMode_ == 2) {
        // second has priority
        fragColor = shadeCol1;
        fragDepth = depth1;
    } else if (compositingMode_ == 3) {
        // blend using blendFactor_
        if (blendFactor_ == 0.0) fragDepth = depth0;
        else if (blendFactor_ == 1.0) fragDepth = depth1;
        else fragDepth = min(depth0, depth1);
        fragColor = blendFactor_*shadeCol0+(1.0-blendFactor_)*shadeCol1;
    } else if (compositingMode_ == 4) {
        // alpha compositing
        if (depth1 < depth0) {
            fragColor.rgb = shadeCol1.rgb * shadeCol1.a + shadeCol0.rgb * shadeCol0.a * (1.0 - shadeCol1.a);
            fragColor.a = shadeCol1.a + shadeCol0.a * (1.0 - shadeCol1.a);
		} else {
            fragColor.rgb = shadeCol0.rgb * shadeCol0.a + shadeCol1.rgb * shadeCol1.a * (1.0 - shadeCol0.a);
            fragColor.a = shadeCol0.a + shadeCol1.a * (1.0 - shadeCol0.a);
        }
        fragDepth = min(depth0, depth1);
    } else if (compositingMode_ == 5) {
        fragDepth = min(depth0, depth1);
        fragColor.rgb = shadeCol0.rgb*shadeCol0.a + shadeCol1.rgb*shadeCol1.a;
        fragColor.a = max(shadeCol0.a, shadeCol1.a);
    }

    gl_FragColor = fragColor;
    gl_FragDepth = fragDepth;
}
