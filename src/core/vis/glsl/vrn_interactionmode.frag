
#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;
uniform SAMPLER2D_TYPE overlayTex_;

uniform int interactionCoarseness_;
uniform bool useOverlay_;
uniform float overlayOpacity_;

/***
 * The main method.
 ***/
void main() {
	
    vec2 fragCoord = gl_FragCoord.xy;

    fragCoord.x /= float(interactionCoarseness_);
    fragCoord.y /= float(interactionCoarseness_);
    
    gl_FragColor = textureLookup2D(shadeTex_, fragCoord.xy);
    gl_FragDepth = textureLookup2D(depthTex_, fragCoord.xy).z;
	
}
