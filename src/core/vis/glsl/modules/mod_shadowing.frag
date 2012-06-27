#ifdef USE_SHADOWS_3DTEXTURE
uniform sampler3D shadowVolume_;
#endif

#ifdef USE_SHADOWS_SHADOWRAY
uniform float shadowThreshold_;
uniform vec3 volSizePG_;
#endif
#ifdef USE_SHADOWS_SHADOWRAY_ALPHA
uniform vec3 volSizePG_;
#endif

#ifdef USE_SHADOWS_SHADOWMAP
uniform SAMPLER2D_TYPE shadowMap1_;
uniform mat4 viewMatrixShadow_;
uniform mat4 projectionMatrixShadow_;
uniform vec3 volumeTextureSize_;
uniform float volumeTextureSizeMax_;
uniform vec3 volSizePG_;
#endif

#ifdef USE_SHADOWS_SHADOWMAPDEEP
uniform SAMPLER2D_TYPE shadowMap1_;
uniform SAMPLER2D_TYPE shadowMap2_;
uniform SAMPLER2D_TYPE shadowMap3_;
uniform SAMPLER2D_TYPE shadowMap4_;
uniform mat4 viewMatrixShadow_;
uniform mat4 projectionMatrixShadow_;
uniform vec3 volumeTextureSize_;
uniform float volumeTextureSizeMax_;
uniform vec3 volSizePG_;
#endif

#ifdef USE_SHADOWS_DEEPSHADOWSOFT
uniform SAMPLER2D_TYPE shadowMap1_;
uniform SAMPLER2D_TYPE shadowMap2_;
uniform SAMPLER2D_TYPE shadowMap3_;
uniform SAMPLER2D_TYPE shadowMap4_;
uniform mat4 viewMatrixShadow_;
uniform mat4 projectionMatrixShadow_;
uniform vec3 volumeTextureSize_;
uniform float volumeTextureSizeMax_;
uniform vec3 volSizePG_;
uniform vec2 shadowTexSize_;
uniform float lightSize_;
#endif

#ifdef USE_SHADOWS_DEEPSHADOWSOFT_STATIC
uniform SAMPLER2D_TYPE shadowMap1_;
uniform SAMPLER2D_TYPE shadowMap2_;
uniform SAMPLER2D_TYPE shadowMap3_;
uniform SAMPLER2D_TYPE shadowMap4_;
uniform mat4 viewMatrixShadow_;
uniform mat4 projectionMatrixShadow_;
uniform vec3 volumeTextureSize_;
uniform float volumeTextureSizeMax_;
uniform vec3 volSizePG_;
uniform vec2 shadowTexSize_;
uniform float lightSize_;

#define STATIC_FILTER_SIZE 5.0
#endif



// Really very strange workaround for certain NVIDIA drivers on Linux. Will show colorful
// garbage as if program length exceeded without this fix.
#ifdef VRN_GL_NVIDIA_VERSION
#if (VRN_GL_NVIDIA_VERSION == 9755)
    #ifdef VRN_WORKAROUND_NV_LINUX_9755
        #define IOTA 1e-45
    #else
        #define IOTA 0.0
    #endif // VRN_WORKAROUND_NV_LINUX_9755
#else
    #define IOTA 0.0
#endif //(VRN_GL_NVIDIA_VERSION == 9755)

#else
    #define IOTA 0.0
#endif //VRN_GL_NVIDIA_VERSION

#ifdef USE_SHADOWS_SHADOWRAY
//casts a shadowRay with accumulating alpha values
vec4 shadowRunner(in vec3 curVoxel, in vec3 lightPosition) {
	vec3 direction = lightPosition - curVoxel;
	
	float t=0.0;
	float tend = length(direction);
	direction = normalize(direction);
	bool finished = false;
	
	float shadow = 0.0;
	float shadowRun = 0.0;
	
    float stepIncr = 0.005 * raycastingQualityFactorRCP_;

    //while (!finished) {
	t += 0.01;
	for(int loop=0; !finished && loop < 255; ++loop){
        for(int loop1=0; !finished && loop1 < 255; ++loop1){
		    t += stepIncr;
		    vec3 sample = curVoxel + t * direction;
	        float value = textureLookup3D(volume_, volumeParameters_, sample).a;
#ifdef BITDEPTH_12
		    if (value*16.0 + IOTA >= lowerThreshold_ && value*16.0 < upperThreshold_) {
#else
		    if (value + IOTA >= lowerThreshold_ && value < upperThreshold_) {
#endif
			    vec4 tfcolor = applyTF(value);
			    if(tfcolor.a >= shadowThreshold_){
    			    finished = true;
    				shadow = 1.0;
			    }
    		}
		    if(t>=tend) {
    			finished = true;		
    		}
        }
	}
	//save shadow true/false
	//save distance to first shadow object
	return vec4(t, shadow, shadow ,shadow);	
}
#endif
#ifdef USE_SHADOWS_SHADOWRAY_ALPHA
//casts a shadowRay with accumulating alpha values
vec4 shadowRunner(in vec3 curVoxel, in vec3 lightPosition) {
	vec3 direction = lightPosition - curVoxel;
	
	float t=0.0;
	float tend = length(direction);
	direction = normalize(direction);
	bool finished = false;
	
    float resultAlpha = 0.0;
	
    float stepIncr = 0.01 * raycastingQualityFactorRCP_;

    //while (!finished) {
	t += 0.01;
	for(int loop=0; !finished && loop < 255; ++loop){
        for(int loop1=0; !finished && loop1 < 255; ++loop1){
    		t += stepIncr;
		    vec3 sample = curVoxel + t * direction;
    	    float value = textureLookup3D(volume_, volumeParameters_, sample).a;
#ifdef BITDEPTH_12
		    if (value*16.0 + IOTA >= lowerThreshold_ && value*16.0 < upperThreshold_) {
#else
    		if (value + IOTA >= lowerThreshold_ && value < upperThreshold_) {
#endif                
		        vec4 tfcolor = applyTF(value);
                tfcolor.a *= raycastingQualityFactorRCP_;
                resultAlpha = resultAlpha + (1.0-resultAlpha) * tfcolor.a;            
                if(resultAlpha >= 1.0){
    				finished = true;
                }
    		}
		    if(t>=tend) {
		    	finished = true;		
		    }
        }
	}
	//save shadow true/false
	//save distance to first shadow object
	return vec4(t, resultAlpha, resultAlpha ,resultAlpha);	
}
#endif


#if defined(USE_SHADOWS_SHADOWMAPDEEP) || defined(USE_SHADOWS_DEEPSHADOWSOFT) || defined(USE_SHADOWS_DEEPSHADOWSOFT_STATIC)
#ifdef USE_SHADOWS_DEEPSHADOWSOFT
vec4 getShadowDSMatTexPos(vec2 texPos, float d){
#else
float getShadowDSMatTexPos(vec2 texPos, float d){
#endif
    float shadow = 0.0;
	float lastDepth = 0.0;
    float absShadow = 0.0;	
	int lastShadowOffset = 0;    
	bool finished = false;
	vec4 shadowVec = texture2D(shadowMap1_, texPos.xy);        
	float bias = 0.02;	
	if (abs(d) > abs(shadowVec.r+bias)) {
		absShadow = shadowVec.g;
		lastDepth = shadowVec.r;
		lastShadowOffset = 1;
	}
	else {	
		finished = true;
	}
	if (!finished && (abs(d) > abs(shadowVec.b+bias)) && (lastDepth < shadowVec.b)) {
		absShadow = shadowVec.a;
		lastDepth = shadowVec.b;
		lastShadowOffset = 0;
		shadowVec = texture2D(shadowMap2_, texPos.xy);
	}
	else {
		finished = true;	
	}
	if (!finished && (abs(d) > abs(shadowVec.r+bias)) && (lastDepth < shadowVec.r)) {
		absShadow = shadowVec.g;
		lastDepth = shadowVec.r;
		lastShadowOffset = 1;
	}
	else {
		finished = true;
	}
	if (!finished && (abs(d) > abs(shadowVec.b+bias)) && (lastDepth < shadowVec.b)) {
		absShadow = shadowVec.a;
		lastDepth = shadowVec.b;
		lastShadowOffset = 0;
		shadowVec = texture2D(shadowMap3_, texPos.xy);	
	}
	else {
		finished = true;	
	}
	if (!finished && (abs(d) > abs(shadowVec.r+bias)) && (lastDepth < shadowVec.r)) {
		absShadow = shadowVec.g;
		lastDepth = shadowVec.r;
		lastShadowOffset = 1;
	}
	else {
		finished = true;
	}
	if (!finished && (abs(d) > abs(shadowVec.b+bias)) && (lastDepth < shadowVec.b)) {
		absShadow = shadowVec.a;
		lastDepth = shadowVec.b;
		lastShadowOffset = 0;
		shadowVec = texture2D(shadowMap4_, texPos.xy);	
	}
	else {
		finished = true;	
	}
	if (!finished && (abs(d) > abs(shadowVec.r+bias)) && (lastDepth < shadowVec.r)) {
		absShadow = shadowVec.g;
		lastDepth = shadowVec.r;
		lastShadowOffset = 1;
	}
	else {
		finished = true;
	}
	if (!finished && (abs(d) > abs(shadowVec.b+bias)) && (lastDepth < shadowVec.b)) {
		absShadow = shadowVec.a;
		lastDepth = shadowVec.b;
		lastShadowOffset = 2;
	}
	if (lastShadowOffset == 0 && shadowVec.r != 10000.0){
		float controlDist = (shadowVec.r - lastDepth);
		float curDist = (d - lastDepth);
		float percent = clamp(curDist / controlDist,0.0,1.0);
		absShadow = percent*absShadow + (1.0-percent)*shadowVec.g;		
	}
	else if (lastShadowOffset == 1 && shadowVec.b != 10000.0){
		float controlDist = (shadowVec.b - lastDepth);
		float curDist = (d - lastDepth);
		float percent = clamp(curDist / controlDist,0.0,1.0);
		absShadow = percent*absShadow + (1.0-percent)*shadowVec.a;		
	}
	shadow = clamp(absShadow, 0.0, 1.0);
#ifdef USE_SHADOWS_DEEPSHADOWSOFT
    return vec4(lastDepth, d, shadow, 1.0);
#else
    return shadow;
#endif
}
#endif



//returns correct shadow value 0=no shadow - 1=full shadow
float getShadow(vec3 vpos, float d, vec3 L, float currentAlpha, float colorA, float depth){
    float shadow = 0.0;
#ifdef USE_SHADOWS_3DTEXTURE
	shadow = texture3D(shadowVolume_,vpos).a;
#ifdef USE_SHADOW_LOWTHRESHOLD
    if(shadow >= 0.99) {
		shadow = 1.0;
	}
#else
    if(shadow >= 0.05) {
		shadow = 1.0;
	}
#endif
#endif
#ifdef USE_SHADOWS_SHADOWRAY_ALPHA
	shadow = shadowRunner(vpos,lightPosition_).a;
    if(shadow >= 1.0) {
		shadow = 1.0;
	}
#endif
#ifdef USE_SHADOWS_SHADOWRAY
	shadow = shadowRunner(vpos,lightPosition_).a;
    if(shadow >= 1.0) {
		shadow = 1.0;
	}
#endif
#ifdef USE_SHADOWS_SHADOWMAP
	vec3 worldPosition = vpos;	
	worldPosition = (worldPosition * 2.0 * volSizePG_) - volSizePG_;
	vec4 projected = viewMatrixShadow_ * vec4(worldPosition,1.0);
	projected = projectionMatrixShadow_ * projected;
	projected /= projected.w;
	vec2 texPos;
	texPos.xy = (projected.xy + vec2(1.0))/vec2(2.0);
	vec4 shadowLength = texture2D(shadowMap1_, texPos.xy);
	if (abs(d) > shadowLength.r+0.02) 
		shadow =  1.0;
#endif
#ifdef USE_SHADOWS_SHADOWMAPDEEP
    vec3 worldPosition = vpos;	
	worldPosition = (worldPosition * 2.0 * volSizePG_) - volSizePG_;
	vec4 projected = viewMatrixShadow_ * vec4(worldPosition,1.0);
	projected = projectionMatrixShadow_ * projected;
	projected /= projected.w;
	vec2 texPos;
	texPos.xy = (projected.xy + vec2(1.0))/vec2(2.0);
    shadow = getShadowDSMatTexPos(texPos, d);
#endif
#ifdef USE_SHADOWS_DEEPSHADOWSOFT
    vec3 worldPosition = vpos;	
	worldPosition = (worldPosition * 2.0 * volSizePG_) - volSizePG_;
	vec4 projected = viewMatrixShadow_ * vec4(worldPosition,1.0);
	projected = projectionMatrixShadow_ * projected;
	projected /= projected.w;
	vec2 texPos;
	texPos.xy = (projected.xy + vec2(1.0))/vec2(2.0);
    vec4 shadowed = getShadowDSMatTexPos(texPos, d);
    shadowed.b = currentAlpha + (1.0 - currentAlpha) * colorA * (1.0-shadowed.b);
    shadowed.a = depth;
    gl_FragData[1] = shadowed;
#endif
#ifdef USE_SHADOWS_DEEPSHADOWSOFT_STATIC
    vec3 worldPosition = vpos;	
	worldPosition = (worldPosition * 2.0 * volSizePG_) - volSizePG_;
	vec4 projected = viewMatrixShadow_ * vec4(worldPosition,1.0);
	projected = projectionMatrixShadow_ * projected;
	projected /= projected.w;
	vec2 texPos;
	texPos.xy = (projected.xy + vec2(1.0))/vec2(2.0);
    float shadowed = getShadowDSMatTexPos(texPos, d);
    float shadowSize = STATIC_FILTER_SIZE;
    int shadowFilterSizeX = (2*int(shadowSize)+1);
    int shadowFilterSizeY = (2*int(shadowSize)+1);
    float resVal = 0.0;
    float normalizer = 0.0;
    vec2 texPos_off = vec2(0.0);
    for(int shadowCount1=0; shadowCount1<shadowFilterSizeX; shadowCount1+=1){
        for(int shadowCount2=0; shadowCount2<shadowFilterSizeY; shadowCount2+=1){
            vec2 offset = vec2(-shadowSize + float(shadowCount1),-shadowSize + float(shadowCount2));
            float texDist = sqrt(offset.x*offset.x + offset.y*offset.y);
            if ((texDist < shadowSize)){
                texPos_off.x = texPos.x + offset.x/shadowTexSize_.x;
                texPos_off.y = texPos.y + offset.y/shadowTexSize_.y;
                resVal += getShadowDSMatTexPos(texPos_off.xy, d);
                normalizer++;
            }
        }
    }
    shadow = resVal/normalizer;
#endif
    return shadow;
}


