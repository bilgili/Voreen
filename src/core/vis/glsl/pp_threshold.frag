
#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;

uniform float delta_;
uniform float threshold_;

/***
 * Performs an image based thresholding on the red channel.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @delta - sepcifies the distance to the neighboor texels to be fetched
 ***/
vec4 threshold(in vec2 fragCoord, in float delta) {

    vec4 center = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y)).rgba;
	vec4 west = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y)).rgba;
	vec4 northwest = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y-delta)).rgba;
	vec4 north = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y-delta)).rgba;
	vec4 northeast = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y-delta)).rgba;
	vec4 east = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y)).rgba;
	vec4 southeast = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y+delta)).rgba;
	vec4 south = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y+delta)).rgba;
	vec4 southwest = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y+delta)).rgba;
	
	if(west.r + northwest.r + north.r + northeast.r + east.r + southeast.r + south.r + southwest.r >= threshold_)
		return center;
	else
		return vec4(0.0);
}

/***
 * The main method.
 ***/
void main() {

	vec4 fragCoord = gl_FragCoord;
	
	gl_FragColor = threshold(fragCoord.xy, delta_);
    gl_FragDepth = textureLookup2D(depthTex_, fragCoord.xy).z;    
}
