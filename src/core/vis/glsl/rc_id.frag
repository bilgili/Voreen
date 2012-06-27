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

uniform SAMPLER2D_TYPE firstHitPoints_;	            // first hit points

uniform SAMPLER2D_TYPE entryPoints_;                // ray entry points
uniform SAMPLER2D_TYPE entryPointsDepth_;	        // ray entry points depth
uniform SAMPLER2D_TYPE exitPoints_;	                // ray exit points
uniform SAMPLER2D_TYPE exitPointsDepth_;	        // ray exit points depth

uniform sampler3D segmentation_;                    // segmented dataset
uniform VOLUME_PARAMETERS segmentationParameters_;

uniform float penetrationDepth_;


vec4 fillIDBuffer(in vec4 entry, in vec4 firstHitPoint, in vec4 exit) {

	vec4 result;
	float seg = textureLookup3D(segmentation_, segmentationParameters_, firstHitPoint.rgb).a;
	result.rgb = firstHitPoint.rgb;
	// if not hit any segment, penetrate volume until a segment is hit
	// or penetration-depth is reached	
	if (seg == 0.0) {
		vec3 direction = normalize(exit.rgb-entry.rgb);
		float stepIncr = 0.005;
		float t_add = 0.0;
		vec3 pos;
	    
		while ((seg == 0.0) && (t_add < penetrationDepth_)) {
			t_add += stepIncr;
			pos = firstHitPoint.rgb + t_add*direction;
			seg = textureLookup3D(segmentation_, segmentationParameters_, pos).a;
		}
		result.rgb = pos;
	}
	if (seg > 0.0)
		result.a = seg;
	else
		result.a = 1.0;
		
	return result;
}

void main() {

    vec2 p = gl_FragCoord.xy;
    vec4 firstHitPos = textureLookup2D(firstHitPoints_, p);
    vec4 entryPos = textureLookup2D(entryPoints_, p);
    vec4 exitPos = textureLookup2D(exitPoints_, p);
    float entryPointDepth = textureLookup2D(entryPointsDepth_, p).z;
    float exitPointDepth = textureLookup2D(exitPointsDepth_, p).z;
    
    vec4 result;
    if (firstHitPos.a != 0.0) {
   		result = fillIDBuffer(entryPos, firstHitPos, exitPos);
   		float t = length(result.rgb-entryPos.rgb)/length(exitPos.rgb-entryPos.rgb);
   		gl_FragDepth = calculateDepthValue(t, entryPointDepth, exitPointDepth);
	} 
	else {
		result=vec4(0.0);
		gl_FragDepth = 1.0;
    }
	
    gl_FragColor = result;
    
}
