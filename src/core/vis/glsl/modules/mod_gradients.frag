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

/**
 * This module contains all functions which can be used for calculating
 * gradients on the fly within a raycaster.
 * The functions below are referenced by RC_CALC_GRADIENTS which is
 * used in the raycaster fragment shaders.
 */

/**
 * Calculates a correct gradient for clipping and volume boundaries.
 *
 * @param samplePos the sample's position in texture space
 * @param rayDirection the ray direction
 */
vec3 fixClipBorderGradient(vec3 samplePos, vec3 rayDirection, SAMPLER2D_TYPE entryPoints) {
    vec3 v0 = normalize(textureLookup2D(entryPoints, vec2(gl_FragCoord.x+2.0, gl_FragCoord.y) ).rgb - samplePos);
    vec3 v1 = normalize(textureLookup2D(entryPoints, vec2(gl_FragCoord.x, gl_FragCoord.y+2.0) ).rgb - samplePos);
    //FIXME: error handling if v0 or v1 is (0,0,0)
    vec3 gradient = cross(v0, v1);
    gradient = faceforward(gradient, rayDirection, gradient);
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the red
 * channel using forward differences.
 *
 * @param volume the voxel's volume
 * @param volumeParameters additional information about the passed volume
 * @param samplePos the sample's position in texture space
 * @param t the ray parameter, needed to fix gradients on clipping and
 *          volume borders
 * @param rayDirection the ray direction
 */
vec3 calcGradientRFD(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 samplePos, float t, vec3 rayDirection, SAMPLER2D_TYPE entryPoints) {
    vec3 gradient;
    if (t == 0.0)
        gradient = fixClipBorderGradient(samplePos, rayDirection, entryPoints);
    else {
           vec3 offset = volumeParameters.datasetDimensionsRCP_;
        float v = textureLookup3DUnnormalized(volume, volumeParameters, samplePos).r;
        float v0 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(offset.x, 0.0, 0.0)).r;
        float v1 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, offset.y, 0)).r;
        float v2 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, offset.z)).r;
        gradient = vec3(v - v0, v - v1, v - v2);
        gradient *= volumeParameters.datasetSpacingRCP_;
        gradient *= volumeParameters.bitDepthScale_;
    }
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the green
 * channel using forward differences.
 *
 * @param volume the voxel's volume
 * @param volumeParameters additional information about the passed volume
 * @param samplePos the sample's position in texture space
 * @param t the ray parameter, needed to fix gradients on clipping and
 *          volume borders
 * @param rayDirection the ray direction
 */
vec3 calcGradientGFD(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 samplePos, float t, vec3 rayDirection, SAMPLER2D_TYPE entryPoints) {
    vec3 gradient;
    if (t == 0.0)
        gradient = fixClipBorderGradient(samplePos, rayDirection, entryPoints);
    else {
        vec3 offset = volumeParameters.datasetDimensionsRCP_;
        float v = textureLookup3DUnnormalized(volume, volumeParameters, samplePos).g;
        float v0 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(offset.x, 0.0, 0.0)).g;
        float v1 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, offset.y, 0)).g;
        float v2 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, offset.z)).g;
        gradient = vec3(v - v0, v - v1, v - v2);
        gradient *= volumeParameters.datasetSpacingRCP_;
        gradient *= volumeParameters.bitDepthScale_;
    }
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the blue
 * channel using forward differences.
 *
 * @param volume the voxel's volume
 * @param volumeParameters additional information about the passed volume
 * @param samplePos the sample's position in texture space
 * @param t the ray parameter, needed to fix gradients on clipping and
 *          volume borders
 * @param rayDirection the ray direction
 */
vec3 calcGradientBFD(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 samplePos, float t, vec3 rayDirection, SAMPLER2D_TYPE entryPoints) {
    vec3 gradient;
    if (t == 0.0)
        gradient = fixClipBorderGradient(samplePos, rayDirection, entryPoints);
    else {
        vec3 offset = volumeParameters.datasetDimensionsRCP_;
        float v = textureLookup3DUnnormalized(volume, volumeParameters, samplePos).b;
        float v0 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(offset.x, 0.0, 0.0)).b;
        float v1 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, offset.y, 0)).b;
        float v2 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, offset.z)).b;
        gradient = vec3(v - v0, v - v1, v - v2);
        gradient *= volumeParameters.datasetSpacingRCP_;
        gradient *= volumeParameters.bitDepthScale_;
    }
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the alpha
 * channel using forward differences.
 *
 * @param volume the voxel's volume
 * @param volumeParameters additional information about the passed volume
 * @param samplePos the sample's position in texture space
 * @param t the ray parameter, needed to fix gradients on clipping and
 *          volume borders
 * @param rayDirection the ray direction
 */
vec3 calcGradientAFD(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 samplePos, float t, vec3 rayDirection, SAMPLER2D_TYPE entryPoints) {
    vec3 gradient;
    if (t == 0.0)
        gradient = fixClipBorderGradient(samplePos, rayDirection, entryPoints);
    else {
        vec3 offset = volumeParameters.datasetDimensionsRCP_;
        float v = textureLookup3DUnnormalized(volume, volumeParameters, samplePos).a;
        float v0 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(offset.x, 0.0, 0.0)).a;
        float v1 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, offset.y, 0)).a;
        float v2 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, offset.z)).a;
        gradient = vec3(v - v0, v - v1, v - v2);
        gradient *= volumeParameters.datasetSpacingRCP_;
        gradient *= volumeParameters.bitDepthScale_;
    }
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the red
 * channel using central differences.
 *
 * @param volume the voxel's volume
 * @param volumeParameters additional information about the passed volume
 * @param samplePos the sample's position in texture space
 * @param t the ray parameter, needed to fix gradients on clipping and
 *          volume borders
 * @param rayDirection the ray direction
 */
vec3 calcGradientR(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 samplePos, float t, vec3 rayDirection, SAMPLER2D_TYPE entryPoints) {
    vec3 gradient;
    if (t == 0.0)
        gradient = fixClipBorderGradient(samplePos, rayDirection, entryPoints);
    else {
        vec3 offset = volumeParameters.datasetDimensionsRCP_;
        float v0 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(offset.x, 0.0, 0.0)).r;
        float v1 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, offset.y, 0)).r;
        float v2 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, offset.z)).r;
        float v3 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(-offset.x, 0, 0)).r;
        float v4 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, -offset.y, 0)).r;
        float v5 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, -offset.z)).r;
        gradient = vec3(v3 - v0, v4 - v1, v5 - v2) * 0.5;
        gradient *= volumeParameters.datasetSpacingRCP_;
        gradient *= volumeParameters.bitDepthScale_;
    }
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the green
 * channel using central differences.
 *
 * @param volume the voxel's volume
 * @param volumeParameters additional information about the passed volume
 * @param samplePos the sample's position in texture space
 * @param t the ray parameter, needed to fix gradients on clipping and
 *          volume borders
 * @param rayDirection the ray direction
 */
vec3 calcGradientG(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 samplePos, float t, vec3 rayDirection, SAMPLER2D_TYPE entryPoints) {
    vec3 gradient;
    if (t == 0.0)
        gradient = fixClipBorderGradient(samplePos, rayDirection, entryPoints);
    else {
        vec3 offset = volumeParameters.datasetDimensionsRCP_;
        float v0 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(offset.x, 0.0, 0.0)).g;
        float v1 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, offset.y, 0)).g;
        float v2 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, offset.z)).g;
        float v3 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(-offset.x, 0, 0)).g;
        float v4 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, -offset.y, 0)).g;
        float v5 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, -offset.z)).g;
        gradient = vec3(v3 - v0, v4 - v1, v5 - v2) * 0.5;
        gradient *= volumeParameters.datasetSpacingRCP_;
        gradient *= volumeParameters.bitDepthScale_;
    }
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the blue
 * channel using central differences.
 *
 * @param volume the voxel's volume
 * @param volumeParameters additional information about the passed volume
 * @param samplePos the sample's position in texture space
 * @param t the ray parameter, needed to fix gradients on clipping and
 *          volume borders
 * @param rayDirection the ray direction
 */
vec3 calcGradientB(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 samplePos, float t, vec3 rayDirection, SAMPLER2D_TYPE entryPoints) {
    vec3 gradient;
    if (t == 0.0)
        gradient = fixClipBorderGradient(samplePos, rayDirection, entryPoints);
    else {
        vec3 offset = volumeParameters.datasetDimensionsRCP_;
        float v0 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(offset.x, 0.0, 0.0)).b;
        float v1 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, offset.y, 0)).b;
        float v2 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, offset.z)).b;
        float v3 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(-offset.x, 0, 0)).b;
        float v4 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, -offset.y, 0)).b;
        float v5 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, -offset.z)).b;
        gradient = vec3(v3 - v0, v4 - v1, v5 - v2) * 0.5;
        gradient *= volumeParameters.datasetSpacingRCP_;
        gradient *= volumeParameters.bitDepthScale_;
    }
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space based on the alpha
 * channel using central differences.
 *
 * @param volume the voxel's volume
 * @param volumeParameters additional information about the passed volume
 * @param samplePos the sample's position in texture space
 * @param t the ray parameter, needed to fix gradients on clipping and
 *          volume borders
 * @param rayDirection the ray direction
 */
vec3 calcGradientA(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 samplePos, float t, vec3 rayDirection, SAMPLER2D_TYPE entryPoints) {
    vec3 gradient;
    if (t == 0.0)
        gradient = fixClipBorderGradient(samplePos, rayDirection, entryPoints);
    else {
        vec3 offset = volumeParameters.datasetDimensionsRCP_;
        float v0 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(offset.x, 0.0, 0.0)).a;
        float v1 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, offset.y, 0)).a;
        float v2 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, offset.z)).a;
        float v3 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(-offset.x, 0, 0)).a;
        float v4 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, -offset.y, 0)).a;
        float v5 = textureLookup3DUnnormalized(volume, volumeParameters, samplePos + vec3(0, 0, -offset.z)).a;
        gradient = vec3(v3 - v0, v4 - v1, v5 - v2) * 0.5;
        gradient *= volumeParameters.datasetSpacingRCP_;
        gradient *= volumeParameters.bitDepthScale_;
    }
    return gradient;
}

/**
 * Calculates a voxel's gradient in volume object space by calculating the
 * gradients for all eight neighbours based on the alpha channel using
 * central differences and filtering between the reuslts.
 *
 * @param volume the voxel's volume
 * @param volumeParameters additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
vec3 calcGradientFiltered(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 samplePos, SAMPLER2D_TYPE entryPoints) {
    vec3 delta = volumeParameters.datasetDimensionsRCP_;
    vec3 g0 = calcGradientA(volume, volumeParameters, samplePos, 0.5, vec3(0.0), entryPoints);
    vec3 g1 = calcGradientA(volume, volumeParameters, samplePos+vec3(-delta.x, -delta.y, -delta.z), 0.5, vec3(0.0), entryPoints);
    vec3 g2 = calcGradientA(volume, volumeParameters, samplePos+vec3( delta.x,  delta.y,  delta.z), 0.5, vec3(0.0), entryPoints);
    vec3 g3 = calcGradientA(volume, volumeParameters, samplePos+vec3(-delta.x,  delta.y, -delta.z), 0.5, vec3(0.0), entryPoints);
    vec3 g4 = calcGradientA(volume, volumeParameters, samplePos+vec3( delta.x, -delta.y,  delta.z), 0.5, vec3(0.0), entryPoints);
    vec3 g5 = calcGradientA(volume, volumeParameters, samplePos+vec3(-delta.x, -delta.y,  delta.z), 0.5, vec3(0.0), entryPoints);
    vec3 g6 = calcGradientA(volume, volumeParameters, samplePos+vec3( delta.x,  delta.y, -delta.z), 0.5, vec3(0.0), entryPoints);
    vec3 g7 = calcGradientA(volume, volumeParameters, samplePos+vec3(-delta.x,  delta.y,  delta.z), 0.5, vec3(0.0), entryPoints);
    vec3 g8 = calcGradientA(volume, volumeParameters, samplePos+vec3( delta.x, -delta.y, -delta.z), 0.5, vec3(0.0), entryPoints);
    vec3 mix0 = mix(mix(g1, g2, 0.5), mix(g3, g4, 0.5), 0.5);
    vec3 mix1 = mix(mix(g5, g6, 0.5), mix(g7, g8, 0.5), 0.5);
    return mix(g0, mix(mix0, mix1, 0.5), 0.75);
}

/**
 * Calculates a voxel's gradient in volume object space based on the alpha
 * channel using central differences.
 *
 * @param volume the voxel's volume
 * @param volumeParameters additional information about the passed volume
 * @param samplePos the sample's position in texture space
 */
vec3 calcGradient(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 samplePos, SAMPLER2D_TYPE entryPoints) {
    return calcGradientA(volume, volumeParameters, samplePos, 0.5, vec3(0.0), entryPoints);
}

// Are these functions obsolete?
/**
 * Calculates a normal for the ray entry point 'front' (and its corresponding exit point 'back') from the
 * neighbouring entry points. The normal points towards the camera's half-space and can thus be
 * used for lighting calculations. Additionally, the volume parameters have to be passed for coordinate transformations.
 */
/*
vec3 calcNormalFromEntryPoints(vec4 front, vec4 back, VOLUME_PARAMETERS volumeParams) {

    const float OFFSET = 5.0;

    vec3 v0;
    vec3 v1;

    // calculate x-tangent v0
    vec4 f0 = textureLookup2D(entryPoints_, vec2(gl_FragCoord.x+OFFSET, gl_FragCoord.y) );
    vec4 b0 = textureLookup2D(exitPoints_, vec2(gl_FragCoord.x+OFFSET, gl_FragCoord.y) );
    vec4 f1 = textureLookup2D(entryPoints_, vec2(gl_FragCoord.x-OFFSET, gl_FragCoord.y) );
    vec4 b1 = textureLookup2D(exitPoints_, vec2(gl_FragCoord.x-OFFSET, gl_FragCoord.y) );
    if (f0.a == 0.0 || b0.a == 0.0) {
        f0 = front;
        b0 = back;
    }
    else if (f1.a == 0.0 || b1.a == 0.0) {
        f1 = front;
        b1 = back;
    }
    // invert jittering of entry points
    f0.rgb = (f0.rgb + (f0.a-1.0)*b0.rgb)/f0.a;
    f1.rgb = (f1.rgb + (f1.a-1.0)*b1.rgb)/f1.a;
    v0 = f0.rgb - f1.rgb;

    // calculate y-tangent v1
    f0 = textureLookup2D(entryPoints_, vec2(gl_FragCoord.x, gl_FragCoord.y+OFFSET) );
    b0 = textureLookup2D(exitPoints_, vec2(gl_FragCoord.x, gl_FragCoord.y+OFFSET) );
    f1 = textureLookup2D(entryPoints_, vec2(gl_FragCoord.x, gl_FragCoord.y-OFFSET) );
    b1 = textureLookup2D(exitPoints_, vec2(gl_FragCoord.x, gl_FragCoord.y-OFFSET) );
    if (f0.a == 0.0 || b0.a == 0.0) {
        f0 = front;
        b0 = back;
    }
    else if (f1.a == 0.0 || b1.a == 0.0) {
        f1 = front;
        b1 = back;
    }
    // invert jittering of entry points
    f0.rgb = (f0.rgb + (f0.a-1.0)*b0.rgb)/f0.a;
    f1.rgb = (f1.rgb + (f1.a-1.0)*b1.rgb)/f1.a;
    v1 = f0.rgb - f1.rgb;

    return cross(v0, v1);
}

*/
/*
vec3 calcNormalFromEntryPoints(vec3 samplePos) {

    //FIXME: error handling if v0 or v1 is (0,0,0)

    vec3 v0 = textureLookup2D(entryPoints_, vec2(gl_FragCoord.x+2.0, gl_FragCoord.y) ).rgb - samplePos;
    vec3 v1 = textureLookup2D(entryPoints_, vec2(gl_FragCoord.x, gl_FragCoord.y+2.0) ).rgb - samplePos;

    return cross(v0, v1);
}
*/
