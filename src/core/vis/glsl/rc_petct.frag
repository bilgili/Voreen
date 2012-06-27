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

// modified code snipped from mod_transfunc.frag
uniform sampler1D transferFuncPET_;	// 1D transfer function
uniform sampler1D transferFuncCT_;	// 1D transfer function

vec4 applyTF(float intensity, sampler1D transferFunc) {
	return texture1D(transferFunc, intensity);
}

vec4 applyTF(vec4 intensity, sampler1D transferFunc) {
	return texture1D(transferFunc, intensity.a);
}

vec4 applyTF(float intensity) {
	return texture1D(transferFuncCT_, intensity);
}

vec4 applyTF(vec4 intensity) {
	return texture1D(transferFuncCT_, intensity.a);
}
//

#include "modules/mod_sampler2d.frag"
#include "modules/mod_sampler3d.frag"
#include "modules/mod_raysetup.frag"
#include "modules/mod_masking.frag"
#include "modules/mod_gradients.frag"
#include "modules/mod_shading.frag"
#include "modules/mod_compositing.frag"
#include "modules/mod_depth.frag"


// variables for storing compositing results
vec4 result = vec4(0.0);


// declare entry and exit parameters
uniform SAMPLER2D_TYPE entryPointsPET_;	     // ray entry points PET
uniform SAMPLER2D_TYPE entryPointsDepthPET_; // ray entry points depth PET
uniform SAMPLER2D_TYPE exitPointsPET_;	     // ray exit points PET
uniform SAMPLER2D_TYPE exitPointsDepthPET_;  // ray exit points depth PET
uniform SAMPLER2D_TYPE entryPointsCT_;	     // ray entry points CT
uniform SAMPLER2D_TYPE entryPointsDepthCT_;  // ray entry points depth CT
uniform SAMPLER2D_TYPE exitPointsCT_;	     // ray exit points CT
uniform SAMPLER2D_TYPE exitPointsDepthCT_;   // ray exit points depth CT

// declare volumes
uniform sampler3D volumePET_;                   // volume data set
uniform VOLUME_PARAMETERS volumeParametersPET_;	// texture lookup parameters for volume_
uniform sampler3D volumeCT_;                    // volume data set
uniform VOLUME_PARAMETERS volumeParametersCT_;	// texture lookup parameters for volume_



vec3 calcGradients(int curVolume, vec3 oldGradient, vec3 samplePos, sampler3D volume, VOLUME_PARAMETERS volumeParameters, float t, vec3 rayDirection, SAMPLER2D_TYPE entryPoints) {
	if (curVolume == 0) {
		return RC_CALC_GRADIENTS(oldGradient, samplePos, volume, volumeParameters, t, rayDirection, entryPoints);
	} else {
		return RC_CALC_GRADIENTS_1(oldGradient, samplePos, volume, volumeParameters, t, rayDirection, entryPoints);
	}
}

vec3 applyShading(int curVolume, vec3 gradient, vec3 samplePos, VOLUME_PARAMETERS volumeParameters, vec3 colorD, vec3 colorS, vec3 colorA) {
	return colorS;
	if (curVolume == 0) {
		return RC_APPLY_SHADING(gradient, samplePos, volumeParameters, colorD, colorS, colorA);
	} else {
		return RC_APPLY_SHADING_1(gradient, samplePos, volumeParameters, colorD, colorS, colorA);
	}
}

vec4 applyCompositing(int curVolume, vec4 result, vec4 color, vec3 samplePos, vec3 gradient, float t, inout float tDepth) {
	if (curVolume == 0) {
		return RC_APPLY_COMPOSITING(result, color, samplePos, gradient, t);
	} else {
		return RC_APPLY_COMPOSITING_1(result, color, samplePos, gradient, t);
	}
}



/***
 * Performs the ray traversal
 * returns the final fragment color.
 ***/
void rayTraversal(in vec3 first0, in vec3 last0, in float first0depth, in float last0depth, sampler1D transFunc0,
				  in vec3 first1, in vec3 last1, in float first1depth, in float last1depth, sampler1D transFunc1,
				  sampler3D volume0, VOLUME_PARAMETERS volumeParameters0,
				  sampler3D volume1, VOLUME_PARAMETERS volumeParameters1) {

	// calculate the required ray parameters for both rays
    float t0, t1         = 0.0;
    float t0Incr, t1Incr = 0.0;
    float t0End, t1End   = 1.0;
    vec3 ray0Direction, ray1Direction;
	raySetup(first0, last0, ray0Direction, t0Incr, t0End);
	raySetup(first1, last1, ray1Direction, t1Incr, t1End);

	t0Incr = 0.002;
	t1Incr = 0.002;

	float tDepth = -1.0;
	bool finished = false;
	
	// cast the rays through the front volume
	for (int loop0=0; !finished && loop0<255; loop0++) {
		for (int loop1=0; !finished && loop1<255; loop1++) {
			vec3 samplePos0 = first0 + t0 * ray0Direction;
			vec4 voxel0 = getVoxel(volume0, volumeParameters0, samplePos0);

			// apply masking
			//if (RC_NOT_MASKED(samplePos0, voxel0.a)) {
				// calculate gradients
				//voxel.xyz = RC_CALC_GRADIENTS(voxel.xyz, samplePos, volume0, volumeParameters0, t, rayDirection, entryPoints_);
			// FIXME: entry points need to be replaced by a variable
				voxel0.xyz = calcGradients(0, voxel0.xyz, samplePos0, volume0, volumeParameters0, t0, ray0Direction, entryPointsPET_);

				// apply classification
				vec4 color0 = applyTF(voxel0.a, transFunc0);

				// apply shading
				color0.rgb = applyShading(0, voxel0.xyz, samplePos0, volumeParameters0, color0.rgb, color0.rgb, color0.rgb);

				// if opacity greater zero, apply compositing
				if (color0.a > 0.0) {
					result = applyCompositing(0, result, color0, samplePos0, voxel0.xyz, t0, tDepth);
				}
			//}
			finished = earlyRayTermination(result.a, 0.9);
			if (getDepthValue(t0, t0End, first0depth, last0depth) >= first1depth) finished = true;
			t0 += t0Incr;
			finished = finished || (t0 > t0End);
		}
	}
	
	/*
	float epsilon = t0Incr/3.0;
	float t0Delta = t0Incr;
	float sign = -1.0;
	float curDepth = getDepthValue(t0, t0End, first0depth, last0depth);
	while (abs(curDepth-first1depth) > epsilon) {
		t0Delta = t0Delta/2.0;
		if (curDepth > first1depth) sign = -1.0;
		else sign = 1.0;
		t0 += sign*t0Delta; 
		curDepth = getDepthValue(t0, t0End, first0depth, last0depth);
	}
	*/
	

	float length0 = last0depth-first0depth;
	float length1 = last1depth-first1depth;
	t0Incr *= (length1/length0);


	//if (!earlyRayTermination(result.a, 0.9)) { 
		// cast the rays through the overlapping region
		finished = false;
		for (int loop0=0; !finished && loop0<255; loop0++) {
			for (int loop1=0; !finished && loop1<255; loop1++) {
				vec3 samplePos0 = first0 + t0 * ray0Direction;
				vec3 samplePos1 = first1 + t1 * ray1Direction;
				vec4 voxel0 = getVoxel(volume0, volumeParameters0, samplePos0);
				vec4 voxel1 = getVoxel(volume1, volumeParameters1, samplePos1);

				// apply masking
				//if (RC_NOT_MASKED(samplePos0, voxel0.a)) {
					// calculate gradients
					voxel0.xyz = calcGradients(0, voxel0.xyz, samplePos0, volume0, volumeParameters0, t0, ray0Direction, entryPointsPET_);
					voxel1.xyz = calcGradients(1, voxel1.xyz, samplePos1, volume1, volumeParameters1, t1, ray1Direction, entryPointsCT_);

					// apply classification
					vec4 color0 = applyTF(voxel0.a, transFunc0);
					vec4 color1 = applyTF(voxel1.a, transFunc1);
					//color0.rgb *= vec3(1,0,0);
					//color1.rgb *= vec3(0,1,0);

					//color0 = vec4(0.0);
					//color1 = vec4(0.0);

					// apply shading
					color0.rgb = applyShading(0, voxel0.xyz, samplePos0, volumeParameters0, color0.rgb, color0.rgb, color0.rgb);
					color1.rgb = applyShading(1, voxel1.xyz, samplePos1, volumeParameters1, color1.rgb, color1.rgb, color1.rgb);

					// if opacity greater zero, apply compositing
					///if (color0.a > 0.0)
					///	result0 = RC_APPLY_COMPOSITING(result0, color0, samplePos0, voxel0.xyz, t0);
					///if (color1.a > 0.0)
					///	result1 = RC_APPLY_COMPOSITING(result1, color1, samplePos1, voxel1.xyz, t1);
					
					
					vec4 colorSum;
					colorSum.rgb = color0.rgb * color0.a + color1.rgb * color1.a;
					colorSum.a = max(color0.a, color1.a);
					// FIXME: !!!!
					result = RC_APPLY_COMPOSITING(result, colorSum, samplePos1, voxel1.xyz, t1);
					
					//result = RC_APPLY_COMPOSITING(result, color1, samplePos1, voxel1.xyz, t1);
					//result = RC_APPLY_COMPOSITING(result, color0, samplePos0, voxel0.xyz, t0);
				//}
				//finished = earlyRayTermination(result.a, 0.9);
				//finished = getDepthValue(tDepth, tEnd, first0depth, last0depth) > first1depth;
				t0 += t0Incr;
				t1 += t1Incr;
				finished = finished || (t1 > t1End);
				/*
				if (last0depth >= last1depth)
					finished = finished || (t0 > t0End);
				else //(last0depth < last1depth)
					finished = finished || (t1 > t1End);
					*/
			}

			// render everything behind t1
		//}
	
	}

	// FIXME: calculate correct depth values
	gl_FragDepth = getDepthValue(tDepth, t0End, first0depth, last0depth);
}

/***
 * The main method.
 ***/
void main() {

	// initialize light and material parameters
    matParams = gl_FrontMaterial;

	vec3 frontPosPET = textureLookup2D(entryPointsPET_, gl_FragCoord.xy).rgb;
	vec3 backPosPET = textureLookup2D(exitPointsPET_, gl_FragCoord.xy).rgb;
	vec3 frontPosCT = textureLookup2D(entryPointsCT_, gl_FragCoord.xy).rgb;
	vec3 backPosCT = textureLookup2D(exitPointsCT_, gl_FragCoord.xy).rgb;

    // determine whether the ray has to be casted
    if (frontPosPET == backPosPET && frontPosCT == backPosCT)
        // background needs no raycasting
        discard;
	else {
		float frontDepthPET = textureLookup2D(entryPointsDepthPET_, gl_FragCoord.xy).z;
		float backDepthPET = textureLookup2D(exitPointsDepthPET_, gl_FragCoord.xy).z;
		float frontDepthCT = textureLookup2D(entryPointsDepthCT_, gl_FragCoord.xy).z;
		float backDepthCT = textureLookup2D(exitPointsDepthCT_, gl_FragCoord.xy).z;
		if (frontDepthPET <= frontDepthCT) {
			rayTraversal(frontPosPET, backPosPET, frontDepthPET, backDepthPET, transferFuncPET_,
						 frontPosCT, backPosCT, frontDepthCT, backDepthCT, transferFuncCT_,
						  // FIXME: Why are the volumes swapped?
						 volumeCT_, volumeParametersCT_, volumePET_, volumeParametersPET_);
		} else {
			result = vec4(1.0,0.0,0.0,1.0);
			gl_FragDepth = 0.0;
/*
			rayTraversal(frontPosCT, backPosCT, frontDepthCT, backDepthCT, transferFuncCT_,
						 frontPosPET, backPosPET, frontDepthPET, backDepthPET, transferFuncPET_,
						 volumePET_, volumeParametersPET_, volumeCT_, volumeParametersCT_);*/
		}
	}

	/*
	#ifdef TONE_MAPPING_ENABLED
		result.r = 1.0 - exp(-result.r * TONE_MAPPING_VALUE);
		result.g = 1.0 - exp(-result.g * TONE_MAPPING_VALUE);
		result.b = 1.0 - exp(-result.b * TONE_MAPPING_VALUE);
	#endif
	*/

	// show proxy geometry for debugging purposes
	if (result.a == 0.0) {
		result.rgb = frontPosCT;
		result.a = 1.0;
		gl_FragDepth = textureLookup2D(entryPointsDepthCT_, gl_FragCoord.xy).z;
	}

	/*
	// show transfer functions for debugging purposes
	if (gl_FragCoord.y > 200) {
		result = texture1D(transferFuncCT_, gl_FragCoord.x/300.0);
	} else {
		result = texture1D(transferFuncPET_, gl_FragCoord.x/300.0);
	}
	result.a = 1.0;
	*/

	gl_FragColor = result;
}