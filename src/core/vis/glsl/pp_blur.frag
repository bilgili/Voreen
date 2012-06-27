
#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;

uniform float delta_;
uniform vec4 blurChannels;
uniform vec4 nblurChannels;

/***
 * Performs an image based blurring in the specified channels.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @blurChannels - specifies the channels to be blurred
 * @nblurChannels - specifies the channels not to be blurred
 * @delta - specifies the distance to the neighboor texels to be fetched
 ***/
vec4 blur(in vec2 fragCoord, in vec4 blurChannels, in vec4 nblurChannels, in float delta) {
    vec4 center = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y) ).rgba;
    vec4 west = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y) ).rgba;
    vec4 northwest = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y-delta) ).rgba;
    vec4 north = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y-delta) ).rgba;
    vec4 northeast = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y-delta) ).rgba;
    vec4 east = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y) ).rgba;
    vec4 southeast = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y+delta) ).rgba;
    vec4 south = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y+delta) ).rgba;
    vec4 southwest = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y+delta) ).rgba;
    
    return nblurChannels*center + blurChannels*(3.0*center + west + northwest + north + northeast + east + southeast + south + southwest)/11.0;
}


/***
 * The main method.
 ***/
void main() {
    vec4 fragCoord = gl_FragCoord;
    gl_FragColor = blur(fragCoord.xy, blurChannels, nblurChannels, delta_);
    
    gl_FragDepth = textureLookup2D(depthTex_, fragCoord.xy ).z;
    
 }
