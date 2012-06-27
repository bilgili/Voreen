
#include "modules/mod_sampler2d.frag"
#include "modules/mod_sampler3d.frag"

uniform SAMPLER2D_TYPE entryParams_;	            // ray entry points
uniform SAMPLER2D_TYPE entryParamsDepth_;           // ray entry points depth
uniform SAMPLER2D_TYPE exitParams_;	                // ray exit points
uniform SAMPLER2D_TYPE exitParamsDepth_;	        // ray exit points depth

uniform sampler3D volume_;                          // volume dataset
uniform VOLUME_PARAMETERS volumeParameters_;        
uniform sampler3D segmentation_;                    // segmented dataset
uniform VOLUME_PARAMETERS segmentationParameters_;

uniform float volumeScaleFactor_;                   // the main volume's intensities are scaled by this factor
                                                    // (necessary for 12-bit datasets)

uniform sampler1D transferFunc_; // transfer function
uniform float lowerThreshold_;
uniform float upperThreshold_;

uniform float penetrationDepth_;
uniform float visibilityThreshold_;

varying float a_;
varying float b_;

#include "modules/mod_depth.frag"

float calcDepth(float z) {
    return (a_ + (b_/z));
}

vec4 fillIDBuffer(inout vec4 first, in vec4 last) {
    vec3 direction = last.rgb - first.rgb;
    float tend = length(direction);
    direction = normalize(direction);

    float t = 0.0;
    float stepIncr = 0.005;
    vec4 result = vec4(0.0);

    bool finished = false;

    // 2 nested loops allow for more than 255 iterations,
    // should not be slower than while (t < tend)
    for (int loop0=0; !finished && loop0<255; loop0++) {
        for (int loop1=0; !finished && loop1<255; loop1++) {

            gl_FragDepth = 1.0;
            
            vec3 p = first.rgb + t * direction;
            float intensity = textureLookup3D(volume_, volumeParameters_, p).a;
            intensity *= volumeScaleFactor_;

            //if (intensity != 0.0 && intensity >= lowerThreshold_ && intensity < upperThreshold_) {
            if ( intensity >= lowerThreshold_ && intensity <= upperThreshold_ && texture1D(transferFunc_, intensity).a > 0.0 ) {
                  
                // interpolate and write depth value
                gl_FragDepth = calculateDepthValue(t/tend);
                
                // write first hit position to rgb channels
                result.rgb = p.rgb;
                
			    float seg = textureLookup3D(segmentation_, segmentationParameters_, p).a;
			   
			    // if not hit any segment, penetrate volume until a segment is hit
			    // or penetration-depth is reached
			    float t_add = 0.0;
			    while ((seg == 0.0) && (t_add < penetrationDepth_)){
			        t_add += 2.0*stepIncr;
			        p = first.rgb + (t+t_add)*direction;
			        seg = textureLookup3D(segmentation_, segmentationParameters_, p).a;
			    }
			   
				if (seg > 0.0)
			        result.a = seg;
			    else
			        result.a = 1.0;
                               
               
                                
                finished = true;
            }
            t += stepIncr;
            if (t > tend) finished = true;
        }
    }
    return result;
}

void main() {

    vec2 p = gl_FragCoord.xy;
    vec4 frontPos = textureLookup2D(entryParams_, p);
    vec4 backPos = textureLookup2D(exitParams_, p);
    
    vec4 result=vec4(0.0);
    if ( !((length(backPos.rgb)==0.0) || (length(frontPos.rgb)==0.0) || (backPos.rgb == frontPos.rgb)) ){
   		result = fillIDBuffer(frontPos, backPos);
	} else {
		gl_FragDepth = 1.0;
    }
	
    gl_FragColor = result;
    
}
