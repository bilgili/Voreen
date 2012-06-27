
/* applies a label texture and sets the depth value to 0. */

#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE labelTex_;		// label texture
uniform vec2 texCoordScale_;			// scale factor for texture coordinates, depending on texture target type

/***
 * The main method.
 ***/
void main() {

    // texture coordinates are normalized but textureLookup2D expects
    // fragment coordinates => scale
    gl_FragColor = textureLookup2D(labelTex_, gl_TexCoord[0].st*texCoordScale_);
	
    gl_FragDepth = 0.0;
	
}
