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
 * This module contains all functions which can be used for performing
 * classifications of a voxel within a raycaster.
 * The functions below are referenced by RC_APPLY_CLASSIFICATION which
 * is used in the raycaster fragment shaders.
 */

#if defined(TF_INTENSITY)
    uniform sampler1D transferFunc_;	// 1D transfer function

	vec4 applyTF(float intensity) {
		return texture1D(transferFunc_, intensity);
	}

	vec4 applyTF(vec4 intensity) {
		return texture1D(transferFunc_, intensity.a);
	}

#elif defined(TF_INTENSITY_GRADIENT)
    uniform sampler2D transferFunc_;	// 2D transfer function

	vec4 applyTF(float intensity) {
		return texture2D(transferFunc_, vec2(intensity,intensity));
	}

	vec4 applyTF(float intensity, float gradientMagnitude) {
		return texture2D(transferFunc_, vec2(intensity, gradientMagnitude));
	}

	vec4 applyTF(vec4 intensityGradient) {
		return texture2D(transferFunc_, vec2(intensityGradient.a, length(intensityGradient.rgb))); 
	}
#endif


// These functions are used by the ambient occlusion raycaster

vec4 triPreClassFetch(vec3 sample, sampler3D volumeTex, VOLUME_PARAMETERS volumeTexParameters,
                      vec3 volumeDimensions, sampler1D classTex, float fetchFactor) {
	// perform trilinear interpolation with pre-classification
	vec3 volDimMinusOne = volumeDimensions;//-ivec3(1);
	vec3 min = floor(sample*volDimMinusOne)/volDimMinusOne;
	vec3 max = ceil(sample*volDimMinusOne)/volDimMinusOne;
	vec3 pos0 = vec3(min.x, min.y, min.z);
	vec3 pos1 = vec3(max.x, min.y, min.z);
	vec3 pos2 = vec3(max.x, max.y, min.z);
	vec3 pos3 = vec3(min.x, max.y, min.z);
	vec3 pos4 = vec3(min.x, min.y, max.z);
	vec3 pos5 = vec3(max.x, min.y, max.z);
	vec3 pos6 = vec3(max.x, max.y, max.z);
	vec3 pos7 = vec3(min.x, max.y, max.z);
	vec4 normal0 = textureLookup3D(volumeTex, volumeTexParameters, pos0);
	vec4 normal1 = textureLookup3D(volumeTex, volumeTexParameters, pos1);
	vec4 normal2 = textureLookup3D(volumeTex, volumeTexParameters, pos2);
	vec4 normal3 = textureLookup3D(volumeTex, volumeTexParameters, pos3);
	vec4 normal4 = textureLookup3D(volumeTex, volumeTexParameters, pos4);
	vec4 normal5 = textureLookup3D(volumeTex, volumeTexParameters, pos5);
	vec4 normal6 = textureLookup3D(volumeTex, volumeTexParameters, pos6);
	vec4 normal7 = textureLookup3D(volumeTex, volumeTexParameters, pos7);

	vec4 voxColor0 = texture1D(classTex, normal0.a*fetchFactor);
	vec4 voxColor1 = texture1D(classTex, normal1.a*fetchFactor);
	vec4 voxColor2 = texture1D(classTex, normal2.a*fetchFactor);
	vec4 voxColor3 = texture1D(classTex, normal3.a*fetchFactor);
	vec4 voxColor4 = texture1D(classTex, normal4.a*fetchFactor);
	vec4 voxColor5 = texture1D(classTex, normal5.a*fetchFactor);
	vec4 voxColor6 = texture1D(classTex, normal6.a*fetchFactor);
	vec4 voxColor7 = texture1D(classTex, normal7.a*fetchFactor);

	vec4 lerpX0Bottom = mix(voxColor0, voxColor1, 1.0-fract(volDimMinusOne.x*sample.x));
	vec4 lerpX0Top    = mix(voxColor3, voxColor2, fract(volDimMinusOne.x*sample.x));

	vec4 lerpX1Bottom = mix(voxColor4, voxColor5, fract(volDimMinusOne.x*sample.x));
	vec4 lerpX1Top    = mix(voxColor7, voxColor6, fract(volDimMinusOne.x*sample.x));

	vec4 lerpY0 = mix(lerpX0Bottom, lerpX0Top, fract(volDimMinusOne.y*sample.y));
	vec4 lerpY1 = mix(lerpX1Bottom, lerpX1Top, fract(volDimMinusOne.y*sample.y));


	vec4 voxColor;// = mix(lerpY0, lerpY1, fract(volDimMinusOne.z*sample.z));
	voxColor = lerpX0Bottom;
	return voxColor;
}


vec4 planePreClassFetch(vec3 sample, sampler3D volumeTex, VOLUME_PARAMETERS volumeTexParameters, vec3 volumeDimensions,
                        sampler1D classTex, float fetchFactor, float weighting, float dist, vec3 normal) {
	vec3 voxelSize = vec3(1.0) * volumeTexParameters.datasetDimensionsRCP_;
	vec3 n = normalize(normal);

	vec3 u = normalize(vec3(-n.y, n.x, 0));
	vec3 v = normalize(cross(n,u));

	vec3 pos1 = sample + u*voxelSize*dist;
	vec3 pos2 = sample - u*voxelSize*dist;
	vec3 pos3 = sample + v*voxelSize*dist;
	vec3 pos4 = sample - v*voxelSize*dist;

	vec4 normal0 = textureLookup3D(volumeTex, volumeTexParameters, sample);
	vec4 normal1 = textureLookup3D(volumeTex, volumeTexParameters, pos1);
	vec4 normal2 = textureLookup3D(volumeTex, volumeTexParameters, pos2);
	vec4 normal3 = textureLookup3D(volumeTex, volumeTexParameters, pos3);
	vec4 normal4 = textureLookup3D(volumeTex, volumeTexParameters, pos4);

	vec4 voxColor0 = texture1D(classTex, normal0.a*fetchFactor);
	vec4 voxColor1 = texture1D(classTex, normal1.a*fetchFactor);
	vec4 voxColor2 = texture1D(classTex, normal2.a*fetchFactor);
	vec4 voxColor3 = texture1D(classTex, normal3.a*fetchFactor);
	vec4 voxColor4 = texture1D(classTex, normal4.a*fetchFactor);

	return weighting*voxColor0 + ((1.0-weighting)/4.0)*(voxColor1+voxColor2+voxColor3+voxColor4);
}
