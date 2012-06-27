
#include "modules/mod_sampler2d.frag"
#include "modules/mod_normdepth.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;

uniform float lambda_;

/***
 * Performs a low-pass filter on the depth buffer. And returns the
 * difference between the lowpass filtered and the original depth
 * value.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @kernelSize - should be odd and defines the size of the filter kernel (kernelSize x kernelSize)
 ***/
float calcDeltaD(in vec2 fragCoord, in int kernelSize) {
    float centerDepth = normDepth(textureLookup2D(depthTex_, fragCoord.xy).z);
	
	float regionDiff = 0.0;
	int halfKernel = int(floor(float(kernelSize)/2.0));
	for (int x=-halfKernel;x<halfKernel;x++) {
		for (int y=-halfKernel;y<halfKernel;y++) {
            float curDepth = normDepth(textureLookup2D(depthTex_, vec2(fragCoord.x+float(x), fragCoord.y+float(y)) ).z);
			if (curDepth < centerDepth) {
				regionDiff += centerDepth-curDepth;
			}
		}
	}
	if (centerDepth >= 0.9) regionDiff = 0.0;
	return (regionDiff/(pow(float(kernelSize),2.0)-1.0));
}

/***
 * The main method.
 ***/
void main() {

    vec4 fragCoord = gl_FragCoord;
    vec4 shadeCol = textureLookup2D(shadeTex_, fragCoord.xy );

	float deltaD = calcDeltaD(fragCoord.xy, 15);
	// apply depth darkening
	gl_FragColor = shadeCol*vec4(vec3(1.0-deltaD*lambda_),1.0);
    gl_FragDepth = textureLookup2D(depthTex_, fragCoord.xy).z;
}