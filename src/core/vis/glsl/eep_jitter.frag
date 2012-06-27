
#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE entryParams_;	 // ray entry points
uniform SAMPLER2D_TYPE entryParamsDepth_; // depth values of entry points
uniform SAMPLER2D_TYPE exitParams_;	     // ray exit points
uniform SAMPLER2D_TYPE jitterTexture_;   // 8 bit random values

uniform float stepLength_;		     // raycasting step length

/**
 * Jitter entry points in ray direction
 */
void main() {
    vec2 p = gl_FragCoord.xy;
    vec4 frontPos = textureLookup2D(entryParams_, p);
    vec4 backPos = textureLookup2D(exitParams_, p);
	
	vec4 fragColor;
	float fragDepth;
		
	if ( (backPos.a == 0.0) || (frontPos.a == 0.0) ) {
		fragColor = frontPos;
		fragDepth = 1.0;
	} else {
		float rayLength = length(backPos.rgb - frontPos.rgb);
		// do not jitter very short rays
		if (rayLength <= stepLength_) {
			fragColor = frontPos;
			fragDepth = textureLookup2D(entryParamsDepth_, p ).z;
		}
		else {
			vec3 dir = (backPos.rgb - frontPos.rgb)/rayLength;
			float jitterValue = textureLookup2D(jitterTexture_, p).x;
			vec3 frontPosNew = frontPos.rgb + (jitterValue*stepLength_)*dir;
			// save jitter value in alpha channel (for inverting jittering later if necessary)
			fragColor = vec4(frontPosNew, 1.0 - jitterValue*stepLength_);
			fragDepth = textureLookup2D(entryParamsDepth_, p ).z;
		}
	}
	
	gl_FragColor = fragColor;
	gl_FragDepth = fragDepth;

}
