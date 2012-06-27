
#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE tex_;

float f(float x, float center, float width) {
	return 1.0-abs((x-center)* 1.0/width);
}

/***
 * The main method.
 ***/
void main() {
	vec2 coord = gl_TexCoord[0].xy;
    // textureLookup2D expects fragment coordinates, so scale by screen dimensions
    float depth = textureLookup2D(tex_, coord*screenDim_).z;
    
#ifdef VRN_TEXTURE_CONTAINER_FBO
	depth = pow(depth, 50.0);
#endif
	gl_FragColor = vec4(depth, depth, depth, 1.0);
}
