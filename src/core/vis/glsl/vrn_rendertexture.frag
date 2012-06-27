#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE tex_;

/***
 * Simply render a texture.
 ***/
void main() {
	vec2 coord = gl_TexCoord[0].xy;
	vec3 color = textureLookup2D(tex_, coord*screenDim_).rgb;
	
	gl_FragColor = vec4(color.r, color.g, color.b, 1.0);

}
