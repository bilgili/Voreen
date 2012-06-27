
#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE shadeTex0_;
uniform SAMPLER2D_TYPE depthTex0_;
uniform SAMPLER2D_TYPE shadeTex1_;
uniform SAMPLER2D_TYPE depthTex1_;

uniform vec4 backgroundColor_;

#define COMBINE_SECOND_HAS_PRIORITY	
#define COMBINE_SHOW_DIFFERENCES


/***
 * The main method.
 ***/
void main() {


	#ifdef COMBINE_SHOW_DIFFERENCES
		vec4 firstModifyColor = vec4(1.0, 0.0, 0.0, 1.0);
		vec4 secondModifyColor = vec4(0.0, 1.0, 0.0, 1.0);
	#endif

	vec2 fragCoord = gl_FragCoord.xy;
    vec4 source0 = textureLookup2D(shadeTex0_, fragCoord);
    vec4 source1 = textureLookup2D(shadeTex1_, fragCoord);
    
#ifdef COMBINE_SECOND_HAS_PRIORITY	
	if (any(notEqual(source1.rgb, backgroundColor_.rgb)))  {
		gl_FragColor = source1;
        gl_FragDepth = textureLookup2D(depthTex1_, fragCoord).a;
    } else {
		gl_FragColor = source0;
        gl_FragDepth = textureLookup2D(depthTex0_, fragCoord).a;
    }
#endif
#ifdef COMBINE_SHOW_DIFFERENCES
	float fragDepth;
	vec4 fragColor;
    if (any(notEqual(source0.rgb, backgroundColor_.rgb)))  {
        if (any(notEqual(source1.rgb, backgroundColor_.rgb)))  {
			// both case
			fragColor = source1;
            fragDepth = textureLookup2D(depthTex1_, fragCoord).a;
        }
		else {
			// first only case
			fragColor = source0*firstModifyColor;
            fragDepth = textureLookup2D(depthTex0_, fragCoord).a;
        }
	}
    else if (any(notEqual(source1.rgb, backgroundColor_.rgb)))  {
		// second only case
		fragColor = source1*secondModifyColor;
        fragDepth = textureLookup2D(depthTex1_, fragCoord).a;
    }	
	else {
		// none case
		fragColor = source1;
        fragDepth = textureLookup2D(depthTex1_, fragCoord);
    }
	gl_FragColor = fragColor;
    gl_FragDepth = fragDepth;
#endif
}
