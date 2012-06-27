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

#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE shadeTex0_;
uniform SAMPLER2D_TYPE depthTex0_;
uniform TEXTURE_PARAMETERS textureParameters0_;
uniform SAMPLER2D_TYPE shadeTex1_;
uniform SAMPLER2D_TYPE depthTex1_;
uniform TEXTURE_PARAMETERS textureParameters1_;

uniform int compositingMode_;
uniform float weightingFactor_;

void main() {
    vec2 fragCoord = gl_FragCoord.xy;

    // fetch input textures
    vec4 shadeCol0 = textureLookup2Dscreen(shadeTex0_, textureParameters0_, fragCoord);
    float depth0 = textureLookup2Dscreen(depthTex0_, textureParameters0_, fragCoord).z;
    vec4 shadeCol1 = textureLookup2Dscreen(shadeTex1_, textureParameters1_, fragCoord);
    float depth1 = textureLookup2Dscreen(depthTex1_, textureParameters1_, fragCoord).z;

    vec4 fragColor = vec4(0.0);
    float fragDepth = 1.0;

    #if defined(MODE_DEPTH_TEST)
        if (depth0 < depth1) {
            fragColor = shadeCol0;
            fragDepth = depth0;
        }
        else {
            fragColor = shadeCol1;
            fragDepth = depth1;
        }
    #elif defined(MODE_ALPHA_COMPOSITING)
        if (depth1 < depth0) {
            fragColor.rgb = shadeCol1.rgb + shadeCol0.rgb * shadeCol0.a * (1.0 - shadeCol1.a);
            fragColor.a = shadeCol1.a + shadeCol0.a * (1.0 - shadeCol1.a);
        }
        else {
            fragColor.rgb = shadeCol0.rgb + shadeCol1.rgb * shadeCol1.a * (1.0 - shadeCol0.a);
            fragColor.a = shadeCol0.a + shadeCol1.a * (1.0 - shadeCol0.a);
        }
        fragDepth = min(depth0, depth1);
    #elif defined(MODE_ALPHA_BLENDING)
        fragColor.rgb = shadeCol0.a * shadeCol0.rgb + (1.0 - shadeCol0.a) * shadeCol1.rgb;
        fragColor.a = shadeCol0.a + shadeCol1.a * (1.0 - shadeCol0.a);
        if (shadeCol0.a > 0)
            fragDepth = depth0;
        else
            fragDepth = depth1;
    #elif defined(MODE_ALPHA_BLENDING_B_OVER_A)
        fragColor.rgb = shadeCol1.a * shadeCol1.rgb + (1.0 - shadeCol1.a) * shadeCol0.rgb;
        fragColor.a = shadeCol1.a + shadeCol0.a * (1.0 - shadeCol1.a);
        if (shadeCol1.a > 0)
            fragDepth = depth1;
        else
            fragDepth = depth0;
    #elif defined(MODE_WEIGHTED_AVERAGE)
        if (weightingFactor_ == 0.0)
            fragDepth = depth1;
        else if (weightingFactor_ == 1.0)
            fragDepth = depth0;
        else
            fragDepth = min(depth0, depth1);
        fragColor = weightingFactor_*shadeCol0 + (1.0-weightingFactor_)*shadeCol1;
    #elif defined(MODE_TAKE_FIRST)
        fragColor = shadeCol0;
        fragDepth = depth0;
    #elif defined(MODE_TAKE_SECOND)
        fragColor = shadeCol1;
        fragDepth = depth1;
    #elif defined(MODE_FIRST_HAS_PRIORITY)
        if (shadeCol0.a > 0.0) {
            fragColor = shadeCol0;
            fragDepth = depth0;
        }
        else {
            fragColor = shadeCol1;
            fragDepth = depth1;
        }
    #elif defined(MODE_SECOND_HAS_PRIORITY)
        // second has priority
        if (shadeCol1.a > 0.0) {
            fragColor = shadeCol1;
            fragDepth = depth1;
        }
        else {
            fragColor = shadeCol0;
            fragDepth = depth0;
        }
    #elif defined(MODE_MAXIMUM_ALPHA)
        fragDepth = min(depth0, depth1);
        fragColor.rgb = shadeCol0.rgb*shadeCol0.a + shadeCol1.rgb*shadeCol1.a;
        fragColor.a = max(shadeCol0.a, shadeCol1.a);
    #elif defined(MODE_DIFFERENCE)
        fragDepth = abs(depth0 - depth1);
        float diffR = 1 - abs(shadeCol0.r - shadeCol1.r);
        float diffG = 1 - abs(shadeCol0.g - shadeCol1.g);
        float diffB = 1 - abs(shadeCol0.b - shadeCol1.b);
        fragColor.rgba = vec4(diffR, diffG, diffB, max(shadeCol0.a,shadeCol1.a));
    #endif

    gl_FragColor = fragColor;
    gl_FragDepth = fragDepth;
}
