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

uniform SAMPLER2D_TYPE entryPoints_;                    // ray entry points
uniform SAMPLER2D_TYPE entryPointsDepth_;               // ray entry points depth
uniform TEXTURE_PARAMETERS entryParameters_;
uniform SAMPLER2D_TYPE exitPoints_;                     // ray exit points
uniform SAMPLER2D_TYPE exitPointsDepth_;                // ray exit points depth
uniform TEXTURE_PARAMETERS exitParameters_;

uniform sampler3D volume_;                              // volume dataset
uniform VOLUME_PARAMETERS volumeParameters_;            // texture lookup parameters for volume_

uniform bool applyColorModulation_;

vec3 rgb2hsv(vec3 colorRGB) {
    float minComponent = min(colorRGB.r, min(colorRGB.g, colorRGB.b));
    float maxComponent = max(colorRGB.r, max(colorRGB.g, colorRGB.b));
    float delta = maxComponent - minComponent;

    vec3 result;
    result.b = maxComponent;
    if (maxComponent != 0.0) result.g = delta / maxComponent;
    else result.g = 0.0;
    if (result.g == 0.0) result.r = 0.0; // no hue
    else {
        if (colorRGB.r == maxComponent) result.r = (colorRGB.g - colorRGB.b) / delta;
        else if (colorRGB.g == maxComponent) result.r = 2 + (colorRGB.b - colorRGB.r) / delta;
        else if (colorRGB.b == maxComponent) result.r = 4 + (colorRGB.r - colorRGB.g) / delta;
        result.r *= 60.0;
        if (result.r < 0.0) result.r += 360.0;
        result.r /= 360.0;
    }
    return result;
}

/***
 * Performs direct volume rendering and
 * returns the final fragment color.
 ***/
vec4 directRendering(in vec3 first, in vec3 last, vec2 p) {

    vec4 result = vec4(0.0);
    float depthT = -1.0;
    bool finished = false;

    // calculate ray parameters
    float stepIncr = samplingStepSize_;
    float tend;
    float t = 0.0;
    vec3 direction = last.rgb - first.rgb;

    tend = length(direction);
    direction = normalize(direction);

    // 2 nested loops allow for more than 255 iterations,
    // but is slower than while (t < tend)
    for (int loop0=0; !finished && loop0<255; loop0++) {
        for (int loop1=0; !finished && loop1<255; loop1++) {

            vec3 sample = first.rgb + t * direction;
            vec4 voxel = getVoxel(volume_, volumeParameters_, sample);

            // no shading is applied
            vec4 color = voxel;
            vec4 tfColor = applyTF(transferFunc_, rgb2hsv(color.rgb).r);
            if (applyColorModulation_)
                color.rgb *= tfColor.rgb;
            color.a = tfColor.a;

            // perform compositing
            if (color.a > 0.0) {
                // multiply alpha by samplingStepSizeComposite_
                // to accomodate for variable slice spacing
                color.a *= samplingStepSizeComposite_;
                result.rgb = result.rgb + (1.0 - result.a) * color.a * color.rgb;
                result.a = result.a + (1.0 -result.a) * color.a;
            }

            // save first hit ray parameter for depth value calculation
            if (depthT < 0.0 && result.a > 0.0)
                depthT = t;

            // early ray termination
            if (result.a >= 1.0) {
                result.a = 1.0;
                finished = true;
            }

            t += stepIncr;
            finished = finished || (t > tend);
        }
    }

    // calculate depth value from ray parameter
    gl_FragDepth = 1.0;
    if (depthT >= 0.0)
        gl_FragDepth = calculateDepthValue(depthT/tend, textureLookup2Dnormalized(entryPointsDepth_, entryParameters_, p).z,
                                                        textureLookup2Dnormalized(exitPointsDepth_, exitParameters_, p).z);


    return result;
}

/***
 * The main method.
 ***/
void main() {
    vec2 p = gl_FragCoord.xy * screenDimRCP_;
    vec3 frontPos = textureLookup2Dnormalized(entryPoints_, entryParameters_, p).rgb;
    vec3 backPos = textureLookup2Dnormalized(exitPoints_, exitParameters_, p).rgb;

    //determine whether the ray has to be casted
    if (frontPos == backPos) {
        //background need no raycasting
        discard;
    } else {
        //fragCoords are lying inside the boundingbox
        gl_FragColor = directRendering(frontPos, backPos, p);
    }
}
