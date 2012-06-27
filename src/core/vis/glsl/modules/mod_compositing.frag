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

/**
 * This module contains all functions which can be used for compositing
 * voxels within a raycaster.
 * The functions are referenced by RC_APPLY_COMPOSITING as used in the
 * raycaster fragment shaders.
 */

/**
 * Performs regular DVR compositing. Expects the current voxels color
 * and the intermediate result. Returns the result after compositing.
 *
 */
vec4 compositeDVR(in vec4 color, in vec4 curResult, in float t, inout float tDepth) {
	vec4 result = curResult;
	// multiply alpha by raycastingQualityFactorRCP_
	// to accomodate for variable slice spacing
	color.a *= raycastingQualityFactorRCP_;
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
vec4 compositeMIP(in vec4 color, in vec4 curResult, in float t, inout float tDepth) {
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
vec4 compositeISO(in vec4 color, in vec4 curResult, in float t, inout float tDepth, in float isoValue) {
	vec4 result = curResult;
	if (color.a >= isoValue) {
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
 * Performs first hit gradient compositing.
 */
vec4 compositeFHN(in vec3 gradient, in vec4 curResult, in float t, inout float tDepth) {
	vec4 result = curResult;
	// save first hit normal
	if (result.xyz == vec3(0.0)) {
		result.xyz = gradient;
		result.a = 1.0;
		// save first hit ray parameter for depth value calculation
		if (tDepth < 0.0)
			tDepth = t;
	}
	return result;
}