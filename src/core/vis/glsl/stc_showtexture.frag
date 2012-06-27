
#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE tex_;

/***
 * The main method.
 ***/
void main() {
	vec2 coord = gl_TexCoord[0].xy;
    
    // textureLookup2D expects fragment coordinates, so scale by screen dimensions
	float alpha = textureLookup2D(tex_, coord*screenDim_).a;
	
    gl_FragColor = vec4(alpha, alpha, alpha, 1.0);
}
