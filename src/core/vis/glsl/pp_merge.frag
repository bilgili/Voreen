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

#define COMBINE_SECOND_HAS_PRIORITY	
#define COMBINE_SHOW_DIFFERENCES


/***
 * The main method.
 ***/
void main() {


	#ifdef COMBINE_SHOW_DIFFERENCES
		vec4 firstModifyColor = vec4(1.0, 0.0, 0.0, 1.0);
		vec4 secondModifyColor = vec4(0.0, 1.0, 0.0, 1.0);
	#endif

	vec2 fragCoord = gl_FragCoord.xy;
    vec4 source0 = textureLookup2D(shadeTex0_, fragCoord);
    vec4 source1 = textureLookup2D(shadeTex1_, fragCoord);
    
#ifdef COMBINE_SECOND_HAS_PRIORITY	
	if (any(notEqual(source1.rgb, backgroundColor_.rgb)))  {
		gl_FragColor = source1;
        gl_FragDepth = textureLookup2D(depthTex1_, fragCoord).a;
    } else {
		gl_FragColor = source0;
        gl_FragDepth = textureLookup2D(depthTex0_, fragCoord).a;
    }
#endif
#ifdef COMBINE_SHOW_DIFFERENCES
	float fragDepth;
	vec4 fragColor;
    if (any(notEqual(source0.rgb, backgroundColor_.rgb)))  {
        if (any(notEqual(source1.rgb, backgroundColor_.rgb)))  {
			// both case
			fragColor = source1;
            fragDepth = textureLookup2D(depthTex1_, fragCoord).a;
        }
		else {
			// first only case
			fragColor = source0*firstModifyColor;
            fragDepth = textureLookup2D(depthTex0_, fragCoord).a;
        }
	}
    else if (any(notEqual(source1.rgb, backgroundColor_.rgb)))  {
		// second only case
		fragColor = source1*secondModifyColor;
        fragDepth = textureLookup2D(depthTex1_, fragCoord).a;
    }	
	else {
		// none case
		fragColor = source1;
        fragDepth = textureLookup2D(depthTex1_, fragCoord);
    }
	gl_FragColor = fragColor;
    gl_FragDepth = fragDepth;
#endif
}
