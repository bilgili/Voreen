
#include "modules/mod_sampler2d.frag"
#include "modules/mod_sampler3d.frag"

uniform SAMPLER2D_TYPE entryParams_;	                // ray entry points
uniform SAMPLER2D_TYPE entryParamsDepth_;               // ray entry points depth
uniform SAMPLER2D_TYPE exitParams_;	                    // ray exit points
uniform SAMPLER2D_TYPE exitParamsDepth_;                // ray exit points depth

uniform sampler3D volume_;                              // volume dataset
uniform VOLUME_PARAMETERS volumeParameters_;            // texture lookup parameters for volume_

uniform float lowerThreshold_;
uniform float upperThreshold_;

uniform float raycastingQualityFactorRCP_;

uniform bool firstHit;
uniform bool petMode_;
uniform float borderThickness_;

#include "modules/mod_depth.frag"
#include "modules/mod_transferfunc.frag"

//#extension GL_ARB_draw_buffers : enable         

/***
 * Performs direct volume rendering and
 * returns the final fragment color. 
 ***/
vec4 directRendering(in vec3 first, in vec3 last) {
	
    vec4 result = vec4(0.0);
	vec4 hitcolor = vec4(0.0);
    float depthT = -1.0;
    bool finished = false;
	bool hitted = false;
	bool inside = true;
	float bT =  borderThickness_ / 2.0;
		
    
    // calculate ray parameters
    float stepIncr = 0.005 * raycastingQualityFactorRCP_;
    float tend;
    float t = 0.0;
    vec3 direction = last.rgb - first.rgb;
    // if direction is a nullvector the entry- and exitparams are the same
    // so special handling for tend is needed, otherwise we divide by zero
    // furthermore both for-loops will cause only 1 pass overall.
    // The test whether last and first are nullvectors is already done in main-function
    // but however the framerates are higher with this test.
    if (direction == vec3(0.0) && last.rgb != vec3(0.0) && first.rgb != vec3(0.0)) {
        tend = stepIncr/2.0;
    }
    else {
        tend = length(direction);
        direction = normalize(direction);
    }

    // 2 nested loops allow for more than 255 iterations
    // should not be slower than while (t < tend)
    for (int loop0=0; !finished && loop0<255; loop0++) {
        for (int loop1=0; !finished && loop1<255; loop1++) {

            vec3 sample = first.rgb + t * direction;
            vec4 voxel = textureLookup3D(volume_, volumeParameters_, sample);
			float intensity = voxel.a;

			inside = !(sample.r < bT || sample.r > 1.0 -bT || sample.g < bT || sample.g > 1.0 -bT || sample.b < bT || sample.b > 1.0-bT); 
			if (petMode_) inside = true;
			if (intensity >= lowerThreshold_ && intensity < upperThreshold_ && inside) {
				// no shading is applied
				vec4 color = applyTF(voxel);
				if (petMode_) {
					//if (intensity < 0.0006 && intensity > 0.000001) color = vec4(0.6,0.1,0.1,1.0) + color;
					//if (intensity < 0.2 &&    intensity > 0.0006) color  = vec4(0.7,0.5,0.1,1.0) + color;
			
		            //if (intensity <= 0.2 && intensity > 0.000001) color = vec4(0.0,0.0,1.0,1.0);//; + color;
					//if (intensity <= 0.4 && intensity > 0.2) color  = vec4(0.0,1.0,0.0,1.0);// + color;
			    	//if (intensity <= 0.6 && intensity > 0.4) color  = vec4(0.9,1.0,0.0,1.0);// + color;
			    	//if (intensity <= 1.0 && intensity > 0.8) color  = vec4(1.0,0.0,0.0,1.0);// + color;
			    	if (intensity <= 1.0 && intensity > 0.00001) color  = vec4(1.0,0.9,0.0,1.0);// + color;
			
			    }
				
				// perform compositing
				if (color.a > 0.0) {
					// check if first hit
					if (firstHit && hitted == false) {
						// write vec4(sample.rgb,1.0) to render target 3
				      //  gl_FragData[1] = vec4(sample.rgb, 1.0);
					  hitcolor = vec4(sample.rgb,1.0);
					  hitted = true;
					}

					// multiply alpha by raycastingQualityFactorRCP_
					// to accomodate for variable slice spacing
					color.a *= raycastingQualityFactorRCP_;
					result.rgb = result.rgb + (1.0 - result.a) * color.a * color.rgb;
					result.a = result.a + (1.0 -result.a) * color.a;
				}

				// save first hit ray parameter for depth value calculation
				if (depthT < 0.0 && result.a > 0.0)
					depthT = t;

				// early ray termination
				if (result.a >= 1.0) {
					result.a = 1.0;
					finished = true;
				}

			}
            t += stepIncr;
            finished = finished || (t > tend);
        }
    }

    // calculate depth value from ray parameter	
	gl_FragDepth = 1.0;
    if (depthT >= 0.0)
        gl_FragDepth = calculateDepthValue(depthT / tend);

   if (firstHit == true) result = hitcolor;
   
   return result;
}

/***
 * The main method.
 ***/
void main() {
	
    vec3 frontPos = textureLookup2D(entryParams_, gl_FragCoord.xy).rgb;
	vec3 backPos = textureLookup2D(exitParams_, gl_FragCoord.xy).rgb;
   
    //determine whether the ray has to be casted
    if ((frontPos == vec3(0.0)) && (backPos == vec3(0.0))) {
        //background need no raycasting
        discard;
    } else {
        //fragCoords are lying inside the boundingbox
        //firstHit=true;
		gl_FragColor = directRendering(frontPos, backPos);
		//firstHit = true;
		//gl_FragData[1] = directRendering(frontPos,backPos);
    }
}
