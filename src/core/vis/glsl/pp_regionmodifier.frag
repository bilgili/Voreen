
#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;
uniform SAMPLER2D_TYPE shadeTexMask_;
//uniform SAMPLER2D_TYPE depthTexMask_;

uniform vec4 segmentId_;
uniform vec4 destColor_;
//uniform int mode_;

/***
 * The main method.
 ***/
void main() {
    vec4 fragCoord = gl_FragCoord;
    vec4 sourceColor = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y) ).rgba;
    vec4 maskColor = textureLookup2D(shadeTexMask_, vec2(fragCoord.x, fragCoord.y) ).rgba;
    vec4 destColor = sourceColor;
	// this does not reliably work due to limited accuracy of floating-point numbers:
    //   if (maskColor == segmentId_) {
    if ( round(maskColor.a*255.0) == round(segmentId_.a*255.0) ) {
        #ifdef RV_MODE_REPLACE
        //if ( mode_ == 0 /* REPLACE */ ) {
            destColor = destColor_;
        //}
        #endif
        #ifdef RV_MODE_BLEND
        //else if (mode_ == 1 /* BLEND */ ) {
            destColor = destColor_*destColor_.a + destColor*(1.0 - destColor_.a);
        //}
        #endif
    }
    gl_FragColor = destColor;
    gl_FragDepth = textureLookup2D(depthTex_, fragCoord.xy ).z;
 }
