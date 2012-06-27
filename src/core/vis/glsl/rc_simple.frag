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

uniform SAMPLER2D_TYPE entryPoints_;	                // ray entry points
uniform SAMPLER2D_TYPE entryPointsDepth_;               // ray entry points depth
uniform SAMPLER2D_TYPE exitPoints_;	                    // ray exit points
uniform SAMPLER2D_TYPE exitPointsDepth_;                // ray exit points depth

uniform sampler3D volume_;                              // volume dataset
uniform VOLUME_PARAMETERS volumeParameters_;            // texture lookup parameters for volume_

/***
 * Performs direct volume rendering and
 * returns the final fragment color.
 ***/
vec4 directRendering(in vec3 first, in vec3 last) {

    vec4 result = vec4(0.0);
    float depthT = -1.0;
    bool finished = false;

    // calculate ray parameters
    float stepIncr = 0.005 * raycastingQualityFactorRCP_;
    float tend;
    float t = 0.0;
    vec3 direction = last.rgb - first.rgb;
    // if direction is a nullvector the entry- and exitparams are the same
    // so special handling for tend is needed, otherwise we divide by zero
    // furthermore both for-loops will cause only 1 pass overall.
    // The test whether last and first are nullvectors is already done in main-function
    // but however the framerates are higher with this test. (??? joerg)
    if (direction == vec3(0.0) && last.rgb != vec3(0.0) && first.rgb != vec3(0.0)) {
        tend = stepIncr/2.0;
    }
    else {
        tend = length(direction);
        direction = normalize(direction);
    }

    // 2 nested loops allow for more than 255 iterations
    // should not be slower than while (t < tend)
    for (int loop0=0; !finished && loop0<255; loop0++) {
        for (int loop1=0; !finished && loop1<255; loop1++) {

            vec3 sample = first.rgb + t * direction;
            vec4 voxel = textureLookup3D(volume_, volumeParameters_, sample);
            voxel.xyz -= 0.5;
			float intensity = voxel.a;
			#if defined(USE_SEGMENTATION)
				intensity *= applySegmentation(sample);
			#endif

			if (intensity >= lowerThreshold_ && intensity < upperThreshold_) {
				// no shading is applied
				vec4 color = applyTF(voxel);

				// perform compositing
				if (color.a > 0.0) {
					// multiply alpha by raycastingQualityFactorRCP_
					// to accomodate for variable slice spacing
					color.a *= raycastingQualityFactorRCP_;
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

			}
            t += stepIncr;
            finished = finished || (t > tend);
        }
    }

    // calculate depth value from ray parameter
	gl_FragDepth = 1.0;
    if (depthT >= 0.0)
        gl_FragDepth = calculateDepthValue(depthT / tend, entryPointsDepth_, exitPointsDepth_);


    return result;
}

/***
 * The main method.
 ***/
void main() {

    vec3 frontPos = textureLookup2D(entryPoints_, gl_FragCoord.xy).rgb;
	vec3 backPos = textureLookup2D(exitPoints_, gl_FragCoord.xy).rgb;

    //determine whether the ray has to be casted
    if ((frontPos == vec3(0.0)) && (backPos == vec3(0.0))) {
        //background need no raycasting
        discard;
    } else {
        //fragCoords are lying inside the boundingbox
        gl_FragColor = directRendering(frontPos, backPos);
    }
}
