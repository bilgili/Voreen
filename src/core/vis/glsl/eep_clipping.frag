
#include "modules/mod_sampler2d.frag"
#include "modules/mod_sampler3d.frag"           // contains struct VOLUME_PARAMETERS 

uniform SAMPLER2D_TYPE entryTex_;	     
uniform SAMPLER2D_TYPE exitTex_; 

uniform VOLUME_PARAMETERS volumeParameters_;    // additional information about the volume the eep are generated for

/**
 * Calculates missing entry points (clipped by near plane) by back-projecting the
 * fragment to the proxy-geometry's object coordinates.
 **/
void main() {
    
    vec4 backgroundColor_ = vec4(0.0,0.0,0.0,0.0);
	vec2 fragCoord = gl_FragCoord.xy;
	
	vec4 exitCol = textureLookup2D(exitTex_, fragCoord);

	if (any(notEqual(exitCol.rgba, backgroundColor_.rgba)))  {
	    
	    vec4 entryCol = textureLookup2D(entryTex_, fragCoord);
	    
	    if (any(notEqual(entryCol.rgba, backgroundColor_.rgba)))  {
    		discard;
	    } 
	    else {
	        
	        // I don't know why, but NDC z-value -20.00 seems to be more correct than -1.0
	        // However, this only applies to coarseness mode, without coarseness depth=-1.0 is fine
	        
	        // viewport coordinates -> NDC
	        vec4 pos = vec4( (gl_FragCoord.xy * screenDimRCP_)*2.0-vec2(1.0), -20.0, 1.0);
	        // NDC -> object coordinates
	        pos = gl_ModelViewMatrixInverse * gl_ProjectionMatrixInverse * pos;
	        pos /= pos.w;
	        
	        // object coordinates -> texture coordinates
	        vec3 eep = (pos.xyz * volumeParameters_.volumeCubeSizeRCP_) + 0.5;
            	        
	        // write new entry point
	        gl_FragColor = vec4(eep, 1.0);
	        gl_FragDepth = 0.0;
	        
	    }
	} 
	
	else {
	    discard;
	}
}
