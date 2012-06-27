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

/**
 * This module contains all functions which can be used for compositing
 * voxels within a raycaster.
 * The functions are referenced by RC_APPLY_COMPOSITING as used in the
 * raycaster fragment shaders.
 */

/**
 * Reciprocal value of the reference sampling interval used
 * for the opacity correction that is necessary to compensate
 * variable sampling rates.
 *
 * See Engel et. al.: "Real-Time Volume Graphics" - Ch 9.1.3
 */
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

/**
 * Performs regular DVR compositing. Expects the current voxels color
 * and the intermediate result. Returns the result after compositing.
 *
 */
vec4 compositeDVR(in vec4 curResult, in vec4 color, in float t, inout float tDepth) {
    vec4 result = curResult;

    // apply opacity correction to accomodate for variable sampling intervals
    color.a = 1.0 - pow(1.0 - color.a, samplingStepSize_ * SAMPLING_BASE_INTERVAL_RCP);

    result.rgb = result.rgb + (1.0 - result.a) * color.a * color.rgb;
    result.a = result.a + (1.0 -result.a) * color.a;
    // save first hit ray parameter for depth value calculation
    if (tDepth < 0.0)
        tDepth = t;
    return result;
}

/**
 * Performs MIP (maximum intensity projection) compositing. Expects the current
 * voxels color and the intermediate result. Returns the result after compositing.
 *
 */
vec4 compositeMIP(in vec4 curResult, in vec4 color, in float t, inout float tDepth) {
    vec4 result;
    if (color.a > curResult.a) {
        result = color;
        // save ray parameter for depth value calculation
        tDepth = t;
    }
    else result = curResult;
    return result;
}

/**
 * Performs isosurface rendering compositing. Expects the current voxels color
 * and the intermediate result. Returns the result after compositing.
 *
 */
vec4 compositeISO(in vec4 curResult, in vec4 color, in float t, inout float tDepth, in float isoValue) {
    vec4 result = curResult;
    float epsilon = 0.02;
    if (color.a >= isoValue-epsilon && color.a <= isoValue+epsilon) {
        result = color;
        result.a = 1.0;
        // save ray parameter for depth value calculation
        tDepth = t;
    }
    return result;
}

/**
 * Performs first hit point compositing.
 */
vec4 compositeFHP(in vec3 samplePos, in vec4 curResult, in float t, inout float tDepth) {
    vec4 result = curResult;
    // save first hit point
    if (result.xyz == vec3(0.0)) {
        result.xyz = samplePos;
        result.a = 1.0;
        // save first hit ray parameter for depth value calculation
        if (tDepth < 0.0)
            tDepth = t;
    }
    return result;
}

/**
 * Performs first hit normals (gradients) compositing.
 */
vec4 compositeFHN(in vec3 gradient, in vec4 curResult, in float t, inout float tDepth) {
    vec4 result = curResult;
    // save first hit normal
    if (result.xyz == vec3(0.0)) {
        result.xyz = normalize(gradient) / 2.0 + 0.5;
        result.a = 1.0;
        // save first hit ray parameter for depth value calculation
        if (tDepth < 0.0)
            tDepth = t;
    }
    return result;
}

/**
 * Performs first hit texture coordinate compositing.
 */
vec4 compositeFHT(in vec3 texCoords, in vec4 curResult, in float t, inout float tDepth) {
    vec4 result = curResult;
    // save first hit normal
    if (result.xyz == vec3(0.0)) {
        result.xyz = texCoords;
        result.a = 1.0;
        // save first hit ray parameter for depth value calculation
        if (tDepth < 0.0)
            tDepth = t;
    }
    return result;
}
