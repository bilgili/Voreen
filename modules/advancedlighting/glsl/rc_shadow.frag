/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#include "modules/mod_sampler2d.frag"
#include "modules/mod_sampler3d.frag"
#include "modules/mod_transfunc.frag"

#include "modules/mod_raysetup.frag"
#include "modules/mod_depth.frag"
#include "modules/mod_compositing.frag"
#include "modules/mod_gradients.frag"
#include "modules/mod_shading.frag"

// variables for storing compositing results
vec4 result = vec4(0.0);

// declare entry and exit parameters
uniform sampler2D entryPoints_;            // ray entry points
uniform sampler2D entryPointsDepth_;       // ray entry points depth
uniform TextureParameters entryParameters_;

uniform sampler2D exitPoints_;                // ray exit points
uniform sampler2D exitPointsDepth_;        // ray exit points depth
uniform TextureParameters exitParameters_;

uniform float samplingStepSize_;

// declare volume
uniform VolumeParameters volumeStruct_;
uniform sampler3D volume_;    // volume data set

uniform TransFuncParameters transferFunc_;
uniform TF_SAMPLER_TYPE transferFuncTex_;
uniform float isoValue_;

// RGB to YUV color conversion
vec3 rgb2yuv(vec3 colorRGB) {
    vec3 colorYUV = vec3(0.0);
    // r=y, g=u, b=v
    colorYUV.r = 0.299 * colorRGB.r + 0.587 * colorRGB.g + 0.114 * colorRGB.b;
    colorYUV.g = 0.436 * (colorRGB.b - colorYUV.r) / (1.0 - 0.114);
    colorYUV.b = 0.615 * (colorRGB.r - colorYUV.r) / (1.0 - 0.299);
    return colorYUV;
}

// YUV to RGB color conversion
vec3 yuv2rgb(vec3 colorYUV) {
    vec3 colorRGB = vec3(0.0);
    // r=y, g=u, b=v
    colorRGB.r = colorYUV.r + 1.13983 * colorYUV.b;
    colorRGB.g = colorYUV.r - 0.39465 * colorYUV.g - 0.58060 * colorYUV.b;
    colorRGB.b = colorYUV.r + 2.03211 * colorYUV.g;
    return colorRGB;
}

/**
 * Performs the ray traversal
 * returns the final fragment color.
 */
void rayTraversal(in vec3 first, in vec3 last) {

    // calculate the required ray parameters
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    float lastIntensity = 0.0f; //used for pre-integrated transfer-functions
    vec3 rayDirection;
    raySetup(first, last, samplingStepSize_, rayDirection, tIncr, tEnd);

    float tDepth = -1.0f;
    bool finished = false;
    WHILE(!finished) {
        vec3 samplePos = first + t * rayDirection;
        vec4 voxel = getVoxel(volume_, volumeStruct_, samplePos);

        #ifdef SHADOWS_ACTIVE
            vec4 classified = RC_APPLY_CLASSIFICATION(transferFunc_, transferFuncTex_, voxel, lastIntensity);
            vec3 materialColor = classified.rgb;
            float opacity = classified.a;
            vec4 envColor = getShadowValues(samplePos);
            vec3 scatteringColor = envColor.rgb;
            float shadowing = 1.0-envColor.a;

            vec4 color;
            color.a = opacity;

            // compute gradient and surfaceness factor (approx. in [0,1])
            vec3 gradient;
            if(t == 0.0)
                gradient = fixClipBorderGradient(samplePos, rayDirection, entryPoints_, entryParameters_);
            else
                gradient = CALC_GRADIENT(volume_, volumeStruct_, samplePos);

            float surfaceness = length(gradient)*0.06;
            // compute diffuse and specular intensity by considering the shadowing
            float lambertIntensity = phongShadingD(gradient, texToPhysical(samplePos, volumeStruct_), volumeStruct_.lightPositionPhysical_, volumeStruct_.cameraPositionPhysical_, vec3(shadowing)).r;
            float specularIntensity = phongShadingS(gradient, texToPhysical(samplePos, volumeStruct_), volumeStruct_.lightPositionPhysical_, volumeStruct_.cameraPositionPhysical_, vec3(shadowing)).r;
            // interpolate between lambertIntensity and shadowing based
            // on surfaceness, which is given by the gradient length
            float luminance = surfaceness*lambertIntensity + (1.0-surfaceness)*shadowing;
            // add specular highlights dependent on surfaceness
            float specularity = 20.0;
            luminance += surfaceness*specularity*specularIntensity;
            luminance = clamp(luminance, 0.0, 1.0);
            // interpolate between materialColor and scatteringColor based
            // on surfaceness, which is given by the gradient length
            // the consideration of the luminance is done in YUV mode
            color.rgb = rgb2yuv(materialColor*scatteringColor);
            color.r = luminance;
            color.rgb = yuv2rgb(color.rgb);
            //color.rgb = shadowing * materialColor;
        #else
            // calculate gradients
            if(t == 0.0)
                voxel.xyz = fixClipBorderGradient(samplePos, rayDirection, entryPoints_, entryParameters_);
            else
                voxel.xyz = CALC_GRADIENT(volume_, volumeStruct_, samplePos);
            // apply classification
            vec4 color = RC_APPLY_CLASSIFICATION(transferFunc_, transferFuncTex_, voxel, lastIntensity);
            // apply shading
            color.rgb = APPLY_SHADING(voxel.xyz, texToPhysical(samplePos, volumeStruct_), volumeStruct_.lightPositionPhysical_, volumeStruct_.cameraPositionPhysical_, color.rgb, color.rgb, vec3(1.0, 1.0, 1.0));
        #endif

        // if opacity greater zero, apply compositing
        if (color.a > 0.0) {
            result = RC_APPLY_COMPOSITING(result, color, samplePos, voxel.xyz, t, samplingStepSize_, tDepth);
        }
        lastIntensity = voxel.a;

        finished = earlyRayTermination(result.a, EARLY_RAY_TERMINATION_OPACITY);
        t += tIncr;
        finished = finished || (t > tEnd);
    } END_WHILE
    gl_FragDepth = getDepthValue(tDepth, tEnd, entryPointsDepth_, entryParameters_, exitPointsDepth_, exitParameters_);
}

/**
 * The main method.
 */
void main() {
    vec2 p = gl_FragCoord.xy * screenDimRCP_;
    vec3 frontPos = textureLookup2Dnormalized(entryPoints_, entryParameters_, p).rgb;
    vec3 backPos = textureLookup2Dnormalized(exitPoints_, exitParameters_, p).rgb;

    // determine whether the ray has to be casted
    if (frontPos == backPos)
        // background needs no raycasting
        discard;
    else
        // fragCoords are lying inside the bounding box
        rayTraversal(frontPos, backPos);

    FragData0 = result;
}
