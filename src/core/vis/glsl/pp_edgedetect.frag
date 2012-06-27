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
#include "modules/mod_normdepth.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;

uniform vec4 edgeColor_;
uniform vec4 fillColor_;
uniform vec4 backgroundColor_;
uniform float edgeThreshold_;
uniform int showImage_;
uniform int blendMode_;
uniform int edgeStyle_;

/***
 * Performs an image based edge detection based on the depth value.
 *
 * @fragCoord - screen coordinates of the current fragment
 ***/
float edgeDetectionDepth(in vec2 fragCoord) {
    float sx = (normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(-1.0,-1.0))).z)+2.0*(normDepth(textureLookup2D(depthTex_, fragCoord.xy+vec2(-1.0,0.0)).z))+normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(-1.0,1.0))).z))
				-(normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(1.0,-1.0))).z)+2.0*(normDepth(textureLookup2D(depthTex_, fragCoord.xy+vec2(1.0,0.0)).z))+normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(1.0,1.0))).z));
	float sy = (normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(-1.0,-1.0))).z)+2.0*(normDepth(textureLookup2D(depthTex_, fragCoord.xy+vec2(0.0,-1.0)).z))+normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(1.0,-1.0))).z))
				-(normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(-1.0,1.0))).z)+2.0*(normDepth(textureLookup2D(depthTex_, fragCoord.xy+vec2(0.0,1.0)).z))+normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(1.0,1.0))).z));	
    
	return (abs(sx)+abs(sy))/4.0;
}

/***
 * Performs an image based edge detection based on the depth value.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @delta     - specifies the distance to the neighboor texels to be fetched
 ***/
float edgeDetectionDepthPersp(in vec2 fragCoord, float delta) {
    float sx = (normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(-delta,-delta))).z)+2.0*(normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(-delta,0.0))).z))+normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(-delta,delta))).z))
                -(normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(delta,-delta))).z)+2.0*(normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(delta,0.0))).z))+normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(delta,delta))).z));
    float sy = (normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(-delta,-delta))).z)+2.0*(normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(0.0,-delta))).z))+normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(delta,-delta))).z))
                -(normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(-delta,delta))).z)+2.0*(normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(0.0,delta))).z))+normDepth(textureLookup2D(depthTex_, (fragCoord.xy+vec2(delta,delta))).z));
	
	return length(sx+sy)/sqrt(32.0);
}

/***
 * Performs an image based edge detection based on the color channel.
 *
 * @fragCoord - screen coordinates of the current fragment
 ***/
vec3 edgeDetectionColor(in vec2 fragCoord) {
    vec3 sx = textureLookup2D(shadeTex_, (fragCoord.xy+vec2(-1.0,-1.0))).rgb
		   +2.0*textureLookup2D(shadeTex_, (fragCoord.xy+vec2(-1.0, 0.0))).rgb
		     +textureLookup2D(shadeTex_, (fragCoord.xy+vec2(-1.0, 1.0))).rgb
		     -(textureLookup2D(shadeTex_, (fragCoord.xy+vec2( 1.0,-1.0))).rgb
		   +2.0*textureLookup2D(shadeTex_, (fragCoord.xy+vec2( 1.0, 0.0))).rgb
		     +textureLookup2D(shadeTex_, (fragCoord.xy+vec2( 1.0, 1.0))).rgb);
	vec3 sy = textureLookup2D(shadeTex_, (fragCoord.xy+vec2(-1.0,-1.0))).rgb
	       +2.0*textureLookup2D(shadeTex_, (fragCoord.xy+vec2( 0.0,-1.0))).rgb
	         +textureLookup2D(shadeTex_, (fragCoord.xy+vec2( 1.0,-1.0))).rgb
	         -(textureLookup2D(shadeTex_, (fragCoord.xy+vec2(-1.0, 1.0))).rgb
	       +2.0*textureLookup2D(shadeTex_, (fragCoord.xy+vec2( 0.0, 1.0))).rgb
	         +textureLookup2D(shadeTex_, (fragCoord.xy+vec2( 1.0, 1.0))).rgb);
	
	return (abs(sx)+abs(sy))/6.0;
	//return length(sx+sy)/sqrt(32.0);
}

/***
 * Draws a one pixel thick halo around the objects.
 *
 * @fragCoord - screen coordinates of the current fragment
 ***/
float renderHalo(in vec2 fragCoord) {

	float result = 0.0;

	vec4 maskColor = vec4(vec3(0.0), 0.0);
    vec4 center = textureLookup2D(shadeTex_, fragCoord);    
	if (center != maskColor) {
        vec4 east = textureLookup2D(shadeTex_, (fragCoord+vec2(1.0,0.0)));
		vec4 south = textureLookup2D(shadeTex_, (fragCoord+vec2(0.0,-1.0)));
		vec4 west = textureLookup2D(shadeTex_, (fragCoord+vec2(-1.0,0.0)));
		vec4 north = textureLookup2D(shadeTex_, (fragCoord+vec2(0.0,1.0)));
        
        if (east == maskColor || south == maskColor ||
		    west == maskColor || north == maskColor) {
			result = 1.0;
		} else {
            vec4 southeast = textureLookup2D(shadeTex_, (fragCoord+vec2(1.0,-1.0)));
			vec4 southwest = textureLookup2D(shadeTex_, (fragCoord+vec2(-1.0,-1.0)));
			vec4 northwest = textureLookup2D(shadeTex_, (fragCoord+vec2(-1.0,1.0)));
			vec4 northeast = textureLookup2D(shadeTex_, (fragCoord+vec2(1.0,1.0)));
			if (southeast == maskColor || southwest == maskColor ||
			    northwest == maskColor || northeast == maskColor) {
				result = 1.0;
			}
		}
	}
	return result;
}


/***
 * The main method.
 ***/
void main() {
	vec2 fragCoord = gl_FragCoord.xy;
    vec4 shadeCol = textureLookup2D(shadeTex_, fragCoord);
	
	// read and normalize depth value
    float depth = textureLookup2D(depthTex_, fragCoord).z;	
    float depthNorm = normDepth(depth);

	vec4 result;
	float edgeThickness;
	if (edgeStyle_ == 0) {
		// contour edges
		edgeThickness = edgeDetectionDepth(fragCoord);
	} else if (edgeStyle_ == 1) {
		// silhouette edges
		//edgeThickness = edgeDetectionColor(fragCoord.xy);
		edgeThickness = renderHalo(fragCoord.xy);
	} else if (edgeStyle_ == 2) {
		// contour edges where edge thickness decreases with increasing depth
		edgeThickness = edgeDetectionDepthPersp(fragCoord.xy, depth);
	}
	
	if (edgeThickness > edgeThreshold_) {
		if (blendMode_ == 0) {
			// use edge color
			result = edgeColor_;
		} else if (blendMode_ == 1) {
			// use pseudo chromadepth
			result = vec4(1.0-depthNorm, 0.0, depthNorm, 1.0)*2.0;
		} else if (blendMode_ == 2) {
			// blend edges
			result = vec4(shadeCol.rgb*2.0, 1.0);
			//result = (1.0-clamp(vec4(depthNorm),0.0,1.0))*0.5;
		
		// for labeling: set alpha-value in source to zero
		} else if (blendMode_ == 3) {
	        result = vec4(0.0);
	    }
	} else {
		if (showImage_== 0 ) {
			result = backgroundColor_;
			depth = 1.0;
		}
		else if (showImage_ == 1) {
			result = shadeCol;
		}	
		else if (showImage_ == 2) {
			discard;
		}
		else {
			if(any(notEqual(vec3(shadeCol), vec3(backgroundColor_))) ) {
				result = fillColor_;
			}
			else { 
				result = backgroundColor_;
				depth = 1.0;
			}
		}
	}

	gl_FragColor = result;
	gl_FragDepth = depth;
}
