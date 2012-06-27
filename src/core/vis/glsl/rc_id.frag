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

uniform SAMPLER2D_TYPE entryPoints_;	            // ray entry points
uniform SAMPLER2D_TYPE entryPointsDepth_;           // ray entry points depth
uniform SAMPLER2D_TYPE exitPoints_;	                // ray exit points
uniform SAMPLER2D_TYPE exitPointsDepth_;	        // ray exit points depth

uniform sampler3D volume_;                          // volume dataset
uniform VOLUME_PARAMETERS volumeParameters_;        
uniform sampler3D segmentation_;                    // segmented dataset
uniform VOLUME_PARAMETERS segmentationParameters_;

uniform float volumeScaleFactor_;                   // the main volume's intensities are scaled by this factor
                                                    // (necessary for 12-bit datasets)

uniform sampler1D transferFunc_; // transfer function
uniform float lowerThreshold_;
uniform float upperThreshold_;

uniform float penetrationDepth_;
uniform float visibilityThreshold_;

varying float a_;
varying float b_;

float calcDepth(float z) {
    return (a_ + (b_/z));
}

vec4 fillIDBuffer(inout vec4 first, in vec4 last) {
    vec3 direction = last.rgb - first.rgb;
    float tend = length(direction);
    direction = normalize(direction);

    float t = 0.0;
    float stepIncr = 0.005;
    vec4 result = vec4(0.0);

    bool finished = false;

    // 2 nested loops allow for more than 255 iterations,
    // should not be slower than while (t < tend)
    for (int loop0=0; !finished && loop0<255; loop0++) {
        for (int loop1=0; !finished && loop1<255; loop1++) {

            gl_FragDepth = 1.0;
            
            vec3 p = first.rgb + t * direction;
            float intensity = textureLookup3D(volume_, volumeParameters_, p).a;
            intensity *= volumeScaleFactor_;

            //if (intensity != 0.0 && intensity >= lowerThreshold_ && intensity < upperThreshold_) {
            if ( intensity >= lowerThreshold_ && intensity <= upperThreshold_ && texture1D(transferFunc_, intensity).a > 0.0 ) {
                  
                // interpolate and write depth value
                gl_FragDepth = calculateDepthValue(t/tend);
                
                // write first hit position to rgb channels
                result.rgb = p.rgb;
                
			    float seg = textureLookup3D(segmentation_, segmentationParameters_, p).a;
			   
			    // if not hit any segment, penetrate volume until a segment is hit
			    // or penetration-depth is reached
			    float t_add = 0.0;
			    while ((seg == 0.0) && (t_add < penetrationDepth_)){
			        t_add += 2.0*stepIncr;
			        p = first.rgb + (t+t_add)*direction;
			        seg = textureLookup3D(segmentation_, segmentationParameters_, p).a;
			    }
			   
				if (seg > 0.0)
			        result.a = seg;
			    else
			        result.a = 1.0;
                               
               
                                
                finished = true;
            }
            t += stepIncr;
            if (t > tend) finished = true;
        }
    }
    return result;
}

void main() {

    vec2 p = gl_FragCoord.xy;
    vec4 frontPos = textureLookup2D(entryPoints_, p);
    vec4 backPos = textureLookup2D(exitPoints_, p);
    
    vec4 result=vec4(0.0);
    if ( !((length(backPos.rgb)==0.0) || (length(frontPos.rgb)==0.0) || (backPos.rgb == frontPos.rgb)) ){
   		result = fillIDBuffer(frontPos, backPos);
	} else {
		gl_FragDepth = 1.0;
    }
	
    gl_FragColor = result;
    
}
