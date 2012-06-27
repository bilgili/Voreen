
#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE mirrorTex_;

uniform int pass_;
uniform float coarseMirror_;
uniform vec2 size_;

/***
 * The main method.
 ***/
void main() {
	if (pass_ == 0) {
		vec4 fragCoord = gl_FragCoord;
        gl_FragColor = textureLookup2D(mirrorTex_, vec2(gl_TexCoord[0])/(coarseMirror_*size_)*screenDim_).rgba;
        gl_FragDepth = fragCoord.z;
	}
	else {
		vec4 fragCoord = gl_FragCoord;
        gl_FragColor = textureLookup2D(mirrorTex_, fragCoord.xy/size_*screenDim_).rgba;
		gl_FragDepth = fragCoord.z;
	}
}
