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

#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE shadeTex0_;	     
uniform SAMPLER2D_TYPE depthTex0_; 
uniform SAMPLER2D_TYPE shadeTex1_;	     
uniform SAMPLER2D_TYPE depthTex1_;  

uniform vec4 backgroundColor_;

#ifdef COMBINE_SHOW_DIFFERENCES
uniform vec4 firstModifyColor_;
uniform vec4 secondModifyColor_;
#endif

#ifdef COMBINE_BLEND_PERCENTAGED
uniform float blendFactor_;
#endif

void main() {
	
    vec2 p = gl_FragCoord.xy;
    vec4 shadeCol0 = textureLookup2D(shadeTex0_, p);
	float depth0 = textureLookup2D(depthTex0_, p).z;
	vec4 shadeCol1 = textureLookup2D(shadeTex1_, p);
	float depth1 = textureLookup2D(depthTex1_, p).z;
	
	vec4 fragColor = vec4(0.0);
	float fragDepth;

#ifdef COMBINE_DEPTH_DEPENDENT
	if (depth0 < depth1) {
	    fragColor = shadeCol1*(1.0-shadeCol0.a) + shadeCol0*shadeCol0.a;
		fragDepth = depth0;
	} 
	else {
	    fragColor = shadeCol0*(1.0-shadeCol1.a) + shadeCol1*shadeCol1.a;
		fragDepth = depth1;
	}
#elif defined(COMBINE_SECOND_HAS_PRIORITY)
    if (shadeCol1.a > 0.0) {
		fragColor = shadeCol1;
		fragDepth = depth1;
	} 
	else {
		fragColor = shadeCol0;
		fragDepth = depth0;
	}
#elif defined(COMBINE_SHOW_DIFFERENCES)
    if (distance(shadeCol0, backgroundColor_) > 0.001)  {
        if (distance(shadeCol1, backgroundColor_) > 0.001)  {
			// both case
			fragColor = shadeCol1;
			fragDepth = depth1;
		}
		else {
			// first only case
			fragColor = shadeCol0*firstModifyColor_;
			fragDepth = depth0;
		}
	}
    else if (distance(shadeCol1, backgroundColor_) > 0.001)  {
		// second only case
		fragColor = shadeCol1*secondModifyColor_;
		fragDepth = depth1;
	}	
	else {
		// none case
		fragColor = shadeCol1;
		fragDepth = depth1;
		
	}
#elif defined(COMBINE_SHOW_OVERLAP)
    if (distance(shadeCol0, backgroundColor_) < 0.001) {
        if (distance(shadeCol1, backgroundColor_) < 0.001) {
            //both background
        	fragColor = backgroundColor_;
    		fragDepth = depth1;
        }
        else {
            //0 background, 1 not background
            fragColor = shadeCol1*vec4(1.0, 0.0, 0.0, 1.0);
            fragDepth = depth1;
        }
	} 
	else {
        if (distance(shadeCol1, backgroundColor_) < 0.001) {
            //0 not background, 1 background
        	fragColor = shadeCol0*vec4(1.0, 0.0, 0.0, 1.0);
    		fragDepth = depth0;
        }
        else {
            //0 not background, 1 not background
            fragColor = (shadeCol1+shadeCol0)*vec4(0.0, 1.0, 0.0, 1.0);
            fragDepth = depth0;
        }
    }
    if (all(lessThan(fragColor, vec4(0.2))))
        fragColor = fragColor*vec4(2.0);
#elif defined(COMBINE_BLEND_PERCENTAGED)
    if (blendFactor_ == 0.0)
        fragDepth = depth0;
    else if (blendFactor_ == 100.0)
        fragDepth = depth1;
    else
        fragDepth = min(depth0, depth1);

    fragColor = blendFactor_/100.0*shadeCol0+(1.0-blendFactor_/100.0)*shadeCol1;
#elif defined(COMBINE_MAXIMUM_ALPHA)
    fragDepth = min(depth0, depth1);
    fragColor.rgb = shadeCol0.rgb*shadeCol0.a + shadeCol1.rgb*shadeCol1.a;
    fragColor.a = max(shadeCol0.a, shadeCol1.a);
#elif defined(COMBINE_ALPHA_COMPOSITING)
    fragColor.rgb = shadeCol1.rgb * shadeCol1.a + shadeCol0.rgb * shadeCol0.a * (1.0 - shadeCol1.a);
	fragColor.a = shadeCol1.a + shadeCol0.a * (1.0 - shadeCol1.a);
    fragDepth = min(depth0, depth1);
#elif defined(COMBINE_DEPTH_ALPHA_COMPOSITING)
    if (depth1 < depth0) {
        fragColor.rgb = shadeCol1.rgb * shadeCol1.a + shadeCol0.rgb * shadeCol0.a * (1.0 - shadeCol1.a);
	    fragColor.a = shadeCol1.a + shadeCol0.a * (1.0 - shadeCol1.a);
    }
    else {
        fragColor = shadeCol0;
    }
    fragDepth = min(depth0, depth1);
#endif

	gl_FragColor = fragColor;
	gl_FragDepth = fragDepth;
}
