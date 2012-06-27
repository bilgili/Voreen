#include "modules/mod_sampler2d.frag"
#include "modules/mod_sampler3d.frag"
#line 3

uniform SAMPLER2D_TYPE entryParams_;	                // ray entry points
uniform SAMPLER2D_TYPE entryParamsDepth_;               // ray entry points depth
uniform SAMPLER2D_TYPE exitParams_;	                    // ray exit points
uniform SAMPLER2D_TYPE exitParamsDepth_;                // ray exit points depth

uniform sampler3D volume_;                              // volume dataset
uniform VOLUME_PARAMETERS volumeParameters_;            // texture lookup parameters for volume_

#ifdef USE_SEGMENTATION
    uniform sampler3D segmentation_;                    // segmented dataset
    uniform VOLUME_PARAMETERS segmentationParameters_;  // texture lookup parameters for segmentation_
#endif

uniform float lowerThreshold_;
uniform float upperThreshold_;

uniform float raycastingQualityFactorRCP_;

#include "modules/mod_depth.frag"
#include "modules/mod_transfunc.frag"
#ifdef USE_SEGMENTATION
	#include "modules/mod_segmentation.frag"
#endif


/***
 * Performs direct volume rendering and
 * returns the final fragment color.
 ***/
vec4 directRendering(in vec3 first, in vec3 last) {

    vec4 result = vec4(0.0);
    float depthT = -1.0;
    bool finished = false;

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
            voxel.xyz -= 0.5;
			float intensity = voxel.a;
			#if defined(USE_SEGMENTATION)
				intensity *= applySegmentation(sample);
			#endif

			if (intensity >= lowerThreshold_ && intensity < upperThreshold_) {
				// no shading is applied
				vec4 color = applyTF(voxel);

				// perform compositing
				if (color.a > 0.0) {
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
        gl_FragColor = directRendering(frontPos, backPos);
    }
}
