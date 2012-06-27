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
#include "modules/mod_filtering.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;
uniform TEXTURE_PARAMETERS textureParameters_;

#if defined(NON_MIN_MAX_ISOTROPIC)
uniform ivec2 kernelRadius_;
#else
uniform SAMPLER2D_TYPE sobelTex_;
uniform TEXTURE_PARAMETERS textureParametersSobel_;
#endif

const float PI = 3.14159;

vec4 nonMinMaxSuppression(in SAMPLER2D_TYPE texture, in TEXTURE_PARAMETERS texParams, in SAMPLER2D_TYPE textureSobel, in TEXTURE_PARAMETERS texParamsSobel, in vec2 fragCoord) {

    vec4 frag = textureLookup2Dscreen(texture, texParams, vec2(fragCoord.x, fragCoord.y));
    if (frag.a == 0.0)
        return frag;

    vec4 sobelFrag = textureLookup2Dscreen(textureSobel, texParamsSobel, vec2(fragCoord.x, fragCoord.y));

    // determine angle in degrees (0 indicates vertical edge, increasing in clock-wise direction)
    float angle = sobelFrag.a;
    angle *= 360.0;

    // exploit symmetry
    if (angle >= 180.0)
        angle -= 180.0;

    vec4 left;
    vec4 right;
    int angleQuant = int(floor(angle / 45.0 - 0.5));
    if (angleQuant == 0 || angleQuant == 4) {      // 0 (vertical)
        left = textureLookup2Dscreen(textureSobel, texParamsSobel, vec2(fragCoord.x+1.0, fragCoord.y));
        right = textureLookup2Dscreen(textureSobel, texParamsSobel, vec2(fragCoord.x-1.0, fragCoord.y));
        //return vec4(0.0, 0.0, 0.0, 0.0);
    }
    else if (angleQuant == 1) { // 45
        left = textureLookup2Dscreen(textureSobel, texParamsSobel, vec2(fragCoord.x-1.0, fragCoord.y+1.0));
        right = textureLookup2Dscreen(textureSobel, texParamsSobel, vec2(fragCoord.x+1.0, fragCoord.y-1.0));
        //return vec4(0.0, 1.0, 0.0, 1.0);
        //return vec4(0.0, 0.0, 0.0, 0.0);
    }
    else if (angleQuant == 2) { // 90 (horizontal)
        left = textureLookup2Dscreen(textureSobel, texParamsSobel, vec2(fragCoord.x, fragCoord.y+1.0));
        right = textureLookup2Dscreen(textureSobel, texParamsSobel, vec2(fragCoord.x, fragCoord.y-1.0));
        //return vec4(0.0, 0.0, 1.0, 1.0);

    }
    else if (angleQuant == 3 || angleQuant == -1) { // 135
        left = textureLookup2Dscreen(textureSobel, texParamsSobel, vec2(fragCoord.x+1.0, fragCoord.y-1.0));
        right = textureLookup2Dscreen(textureSobel, texParamsSobel, vec2(fragCoord.x-1.0, fragCoord.y+1.0));
        //return vec4(1.0, 1.0, 1.0, 1.0);
        //return vec4(0.0, 0.0, 0.0, 0.0);
    }
    else  {
        return vec4(1.0, 0.0, 0.0, 1.0);
    }

    float edgeStrength = sobelFrag.b;

#if defined(NON_MAX_SUPPRESSION)
    if (left.b > edgeStrength || right.b > edgeStrength)
        return vec4(0.0);
    else
        return frag;
#elif defined(NON_MIN_SUPPRESSION)
    if (left.b < edgeStrength || right.b < edgeStrength)
        return vec4(0.0);
    else
        return frag;

#endif

    return vec4(0.0);
}

vec4 nonMinMaxSuppressionIsotropic(in SAMPLER2D_TYPE texture, in TEXTURE_PARAMETERS texParams, in vec2 fragCoord, in ivec2 kernelRadius) {
    vec4 center = textureLookup2Dscreen(texture, texParams, vec2(fragCoord.x, fragCoord.y));
    float centerGray = rgbToGrayScale(center).x;
    for (int x=-kernelRadius.x; x<=kernelRadius.x; x++)
        for (int y=-kernelRadius.y; y<=kernelRadius.y; y++) {
            vec4 curValue = rgbToGrayScale(textureLookup2Dscreen(texture, texParams,
                                           vec2(fragCoord.x+float(x), fragCoord.y+float(y))));
#if defined(NON_MAX_SUPPRESSION)
            if (curValue.x > centerGray)
                return vec4(0.0);
#elif defined(NON_MIN_SUPPRESSION)
            if (curValue.x < centerGray)
                return vec4(0.0);
#endif
        }
    return center;
}


void main() {
    vec2 fragCoord = gl_FragCoord.xy;

#if defined(NON_MIN_MAX_ISOTROPIC)
    gl_FragColor = nonMinMaxSuppressionIsotropic(shadeTex_, textureParameters_, fragCoord, kernelRadius_);
#else
    gl_FragColor = nonMinMaxSuppression(shadeTex_, textureParameters_, sobelTex_, textureParametersSobel_, fragCoord);
#endif

    gl_FragDepth = textureLookup2Dscreen(depthTex_, textureParameters_, fragCoord).z;
}
