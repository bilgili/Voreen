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

uniform SAMPLER2D_TYPE backTex_;
uniform SAMPLER2D_TYPE frontTex_;

uniform sampler3D volume_;
uniform VOLUME_PARAMETERS volumeParameters_;

uniform sampler1D transferFunc_;

uniform float lowerThreshold_;
uniform float upperThreshold_;

const float Samplings = 250.0;


/***
 * returns the coord of the first hit. 
 ***/
vec4 hitpoints(in vec3 first, in vec3 last, in float threshold, in float upperThreshold) {
	
	vec3 direction = last - first;
	int steps = int(floor(Samplings * length(direction)));
	vec3 diff1 = direction / float(steps);
	vec4 value;
	vec4 result = vec4(0.0);
	float scale = 1.0/(upperThreshold - threshold); 

	for (int i=0; i<steps; i++) {
		value.a = textureLookup3D(volume_, volumeParameters_ ,first).a;
		first += diff1;
		if (value.a != 0.0 && value.a >= threshold && value.a <upperThreshold) {
		    result=vec4(first,1.0);
			i = steps;
		}
	}
	return result;
}

/***
 * The main method.
 ***/
void main() {
	vec4 fragCoord = vec4(gl_FragCoord.xy, 0.0, 0.0);
    vec3 frontPos = textureLookup2D(frontTex_, fragCoord.xy).rgb;
	vec3 backPos = textureLookup2D(backTex_, fragCoord.xy).rgb;
    
    gl_FragColor = hitpoints(frontPos, backPos, lowerThreshold_, upperThreshold_);
}
