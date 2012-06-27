
#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE tex_;

uniform bool showAlpha_;

/***
 * The main method.
 ***/
void main() {
	vec2 coord = gl_TexCoord[0].xy;
    // textureLookup2D expects fragment coordinates, so scale by screen dimensions
    vec4 fragment = textureLookup2D(tex_, coord*screenDim_);
    
	if(showAlpha_)
		gl_FragColor = vec4(fragment.a, 0.0, 0.0, 1.0);
	else 
		gl_FragColor = fragment;
}
