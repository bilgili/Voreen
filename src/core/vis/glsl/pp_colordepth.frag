
#include "modules/mod_sampler2d.frag"
#include "modules/mod_normdepth.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;

uniform sampler1D chromadepthTex_;
uniform float colorDepthFactor_;
uniform int colorMode_;

/***
 * The main method.
 ***/
void main() {
	vec4 fragCoord = gl_FragCoord;
		
	// read and normalize depth value
	float depth = textureLookup2D(depthTex_, fragCoord.xy).z;
    float depthNorm = normDepth(depth) * (1.0 / colorDepthFactor_);

	vec4 result = vec4(0.0);
	
	if (colorMode_ == 0) {
		// light-dark (replace)
		result = vec4(vec3(1.0 - depthNorm), 1.0);
    } else if (colorMode_ == 1) {
		// light-dark (modulate)
		result = vec4(vec3(1.0-depthNorm), 1.0)*textureLookup2D(shadeTex_, fragCoord.xy);
	} else if (colorMode_ == 2) {
		// chromadepth
		result = texture1D(chromadepthTex_, vec2(depthNorm, 0.5).x)*textureLookup2D(shadeTex_, fragCoord.xy);
	} else if (colorMode_ == 3) {
		// pseudochromadepth
		result = vec4(1.0-depthNorm, 0.0, depthNorm, 1.0)*textureLookup2D(shadeTex_, fragCoord.xy);
    }

	gl_FragColor = result;
	gl_FragDepth = depth;
}
