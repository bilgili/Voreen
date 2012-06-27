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

#include "modules/vrn_shaderincludes.frag"

// declare entry and exit parameters
uniform SAMPLER2D_TYPE entryPoints_;	        // ray entry points
uniform SAMPLER2D_TYPE entryPointsDepth_;       // ray entry points depth
// declare volume
uniform sampler3D volume_;                      // volume data set
uniform VOLUME_PARAMETERS volumeParameters_;	// texture lookup parameters for volume_


/***
 * ...
 ***/
vec4 getFirstHitColor(in vec3 firstHitPos) {
	vec4 result = vec4(0.0);
	vec4 voxel = getVoxel(volume_, volumeParameters_, firstHitPos);
	// apply masking
	//FIXME: segmentation should not be possible here (tr)
	if (RC_NOT_MASKED(samplePos, voxel.a)) {
		// calculate gradients
		voxel.xyz = RC_CALC_GRADIENTS(voxel.xyz, firstHitPos, volume_, volumeParameters_, 0.0, vec3(0.0), entryPoints_);

		// apply classification
		result = RC_APPLY_CLASSIFICATION(voxel);

		// apply shading
		result.rgb = RC_APPLY_SHADING(voxel.xyz, firstHitPos, volumeParameters_, result.rgb, result.rgb, result.rgb);
	}
	return result;
}

/***
 * The main method.
 ***/
void main() {

    vec3 firstHitPos = textureLookup2D(entryPoints_, gl_FragCoord.xy).rgb;

	// initialize light and material parameters
    matParams = gl_FrontMaterial;

    if (firstHitPos == vec3(0.0))
        // background needs no rendering
        discard;
    else {
        gl_FragColor = getFirstHitColor(firstHitPos);
        // FIXME: somehow the depth values seem to be not correct (tr)
		//gl_FragDepth = calculateDepthValue(0.5, entryPointsDepth_, entryPointsDepth_);
		//gl_FragDepth = textureLookup2D(entryPointsDepth_, gl_FragCoord.xy).z;

		// obtain coordinates of proxy front face in world coordinates
		vec4 front = vec4((gl_FragCoord.x*screenDimRCP_.x*2.0)-1.0,
			    		  (gl_FragCoord.y*screenDimRCP_.y*2.0)-1.0,
						  (textureLookup2D(entryPointsDepth_, gl_FragCoord.xy).z*2.0)-1.0,
						  1.0);
		front = gl_ModelViewProjectionMatrixInverse * front;

		front = gl_ModelViewProjectionMatrix * front;
		gl_FragDepth = ((front.z/front.w)+1.0)/2.0;

    }
}