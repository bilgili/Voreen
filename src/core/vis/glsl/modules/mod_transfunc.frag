// un-commenting the line below will break 2D-TFs!
// use TF->getShaderDefines(); in <raycaster>->generateHeader() to get the correct define!
// #define TF_INTENSITY
#if defined(TF_INTENSITY)
    uniform sampler1D transferFunc_;                        // transfer function

    vec4 applyTF(float intensity) {
        #ifdef BITDEPTH_12
            return texture1D(transferFunc_, intensity * 16.0);
        #else
            return texture1D(transferFunc_, intensity);
        #endif
    }

    vec4 applyTF(vec4 intensity) {
        #ifdef BITDEPTH_12
            return texture1D(transferFunc_, intensity.a * 16.0);
        #else
            return texture1D(transferFunc_, intensity.a);
        #endif
    }
#elif defined(TF_INTENSITY_GRADIENT)
    uniform sampler2D transferFunc_;                        // transfer function

    vec4 applyTF(float intensity) {
        #ifdef BITDEPTH_12
            return texture2D(transferFunc_, vec2(intensity * 16.0, intensity * 16.0));
        #else
            return texture2D(transferFunc_, vec2(intensity,intensity));
        #endif
    }

    vec4 applyTF(float intensity, float gradientMagnitude) {
        #ifdef BITDEPTH_12
            return texture2D(transferFunc_, vec2(intensity * 16.0, gradientMagnitude));
        #else
            return texture2D(transferFunc_, vec2(intensity, gradientMagnitude));
        #endif
    }

    vec4 applyTF(vec4 intensityGradient) {
	
	#ifdef BITDEPTH_12
            return texture2D(transferFunc_, vec2(intensityGradient.a*16.0, length(intensityGradient.rgb)) );
        #else
            return texture2D(transferFunc_, vec2(intensityGradient.a, length(intensityGradient.rgb))); 
	#endif
    }
#endif


#if defined(MOD_TF_SIMPLE)

#else
vec4 triPreClassFetch(vec3 sample, sampler3D volumeTex, VOLUME_PARAMETERS volumeTexParameters,
                      vec3 volumeDimensions, sampler1D classTex, float fetchFactor)
{
	// perform trilinear interpolation with pre-classification
	vec3 volDimMinusOne = volumeDimensions;//-ivec3(1);
	vec3 min = floor(sample*volDimMinusOne)/volDimMinusOne;
	vec3 max = ceil(sample*volDimMinusOne)/volDimMinusOne;
	vec3 pos0 = vec3(min.x, min.y, min.z);
	vec3 pos1 = vec3(max.x, min.y, min.z);
	vec3 pos2 = vec3(max.x, max.y, min.z);
	vec3 pos3 = vec3(min.x, max.y, min.z);
	vec3 pos4 = vec3(min.x, min.y, max.z);
	vec3 pos5 = vec3(max.x, min.y, max.z);
	vec3 pos6 = vec3(max.x, max.y, max.z);
	vec3 pos7 = vec3(min.x, max.y, max.z);
	vec4 normal0 = textureLookup3D(volumeTex, volumeTexParameters, pos0);
	vec4 normal1 = textureLookup3D(volumeTex, volumeTexParameters, pos1);
	vec4 normal2 = textureLookup3D(volumeTex, volumeTexParameters, pos2);
	vec4 normal3 = textureLookup3D(volumeTex, volumeTexParameters, pos3);
	vec4 normal4 = textureLookup3D(volumeTex, volumeTexParameters, pos4);
	vec4 normal5 = textureLookup3D(volumeTex, volumeTexParameters, pos5);
	vec4 normal6 = textureLookup3D(volumeTex, volumeTexParameters, pos6);
	vec4 normal7 = textureLookup3D(volumeTex, volumeTexParameters, pos7);

	vec4 voxColor0 = texture1D(classTex, normal0.a*fetchFactor);
	vec4 voxColor1 = texture1D(classTex, normal1.a*fetchFactor);
	vec4 voxColor2 = texture1D(classTex, normal2.a*fetchFactor);
	vec4 voxColor3 = texture1D(classTex, normal3.a*fetchFactor);
	vec4 voxColor4 = texture1D(classTex, normal4.a*fetchFactor);
	vec4 voxColor5 = texture1D(classTex, normal5.a*fetchFactor);
	vec4 voxColor6 = texture1D(classTex, normal6.a*fetchFactor);
	vec4 voxColor7 = texture1D(classTex, normal7.a*fetchFactor);

	vec4 lerpX0Bottom = mix(voxColor0, voxColor1, 1.0-fract(volDimMinusOne.x*sample.x));
	vec4 lerpX0Top    = mix(voxColor3, voxColor2, fract(volDimMinusOne.x*sample.x));

	vec4 lerpX1Bottom = mix(voxColor4, voxColor5, fract(volDimMinusOne.x*sample.x));
	vec4 lerpX1Top    = mix(voxColor7, voxColor6, fract(volDimMinusOne.x*sample.x));

	vec4 lerpY0 = mix(lerpX0Bottom, lerpX0Top, fract(volDimMinusOne.y*sample.y));
	vec4 lerpY1 = mix(lerpX1Bottom, lerpX1Top, fract(volDimMinusOne.y*sample.y));


	vec4 voxColor;// = mix(lerpY0, lerpY1, fract(volDimMinusOne.z*sample.z));
	voxColor = lerpX0Bottom;
	return voxColor;
}

vec4 edgemix(vec4 x, vec4 y, float a, float intensityx, float intensityy) {
#ifdef BITDEPTH_12
	if (x.a > 0.0 && intensityx*16.0 >= lowerThreshold_ && intensityx*16.0 < upperThreshold_) {
        if (y.a > 0.0 && intensityy*16.0 >= lowerThreshold_ && intensityy*16.0 < upperThreshold_) {
#else
	if (x.a > 0.0 && intensityx >= lowerThreshold_ && intensityx < upperThreshold_) {
        if (y.a > 0.0 && intensityy >= lowerThreshold_ && intensityy < upperThreshold_) {
#endif
            //if (a>0.5) {
            //    return vec4(y.rgb,(y.a-x.a)*a+x.a);
            //}else{
            //    return vec4(x.rgb,(y.a-x.a)*a+x.a);
            //}
            return mix(x,y,a);
        }else{
            //return x;
            return vec4(x.rgb,(1.0-a)*x.a);
        }
    }else{
#ifdef BITDEPTH_12
        if (y.a > 0.0 && intensityy*16.0 >= lowerThreshold_ && intensityy*16.0 < upperThreshold_) {
#else
        if (y.a > 0.0 && intensityy >= lowerThreshold_ && intensityy < upperThreshold_) {
#endif
            //return y;
            return vec4(y.rgb,a*y.a);
        }else{
            return vec4(0.0);
        }
    }
}

float intensitymix(float intensity1, float intensity2) {
#ifdef BITDEPTH_12
	if (intensity1*16.0 >= lowerThreshold_ && intensity1*16.0 < upperThreshold_) {
#else
	if (intensity1 >= lowerThreshold_ && intensity1 < upperThreshold_) {
#endif
        return intensity1;
    }else{
        return intensity2;
    }
}

vec4 mixPreClassFetch(vec3 sample, sampler3D volumeTex, VOLUME_PARAMETERS volumeTexParameters)
{
    vec3 pos = sample * volumeTexParameters.datasetDimensions_;
    vec3 erg1 = floor(pos);
    vec3 erg2 = pos + vec3(1.0);
    vec3 mixf = pos - erg1;
    erg1 *= volumeTexParameters.datasetDimensionsRCP_;
    erg2 *= volumeTexParameters.datasetDimensionsRCP_;

	float intensity000 = textureLookup3D(volumeTex, volumeTexParameters, erg1).a;
	float intensity001 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg1.y,erg2.z)).a;
	float intensity010 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg2.y,erg1.z)).a;
	float intensity011 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg2.y,erg2.z)).a;
	float intensity100 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg1.y,erg1.z)).a;
	float intensity101 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg1.y,erg2.z)).a;
	float intensity110 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg2.y,erg1.z)).a;
	float intensity111 = textureLookup3D(volumeTex, volumeTexParameters, erg2).a;

	vec4 color000 = applyTF(intensity000);
	vec4 color001 = applyTF(intensity001);
	vec4 color010 = applyTF(intensity010);
	vec4 color011 = applyTF(intensity011);
	vec4 color100 = applyTF(intensity100);
	vec4 color101 = applyTF(intensity101);
	vec4 color110 = applyTF(intensity110);
	vec4 color111 = applyTF(intensity111);

    vec4 color00 = edgemix(color000, color001, mixf.z, intensity000, intensity001);
    vec4 color01 = edgemix(color010, color011, mixf.z, intensity010, intensity011);
    vec4 color10 = edgemix(color100, color101, mixf.z, intensity100, intensity101);
    vec4 color11 = edgemix(color110, color111, mixf.z, intensity110, intensity111);

    vec4 color0 = edgemix(color00, color01, mixf.y, intensitymix(intensity000,intensity001), intensitymix(intensity010,intensity011));
    vec4 color1 = edgemix(color10, color11, mixf.y, intensitymix(intensity100,intensity101), intensitymix(intensity110,intensity111));

    vec4 color = edgemix(color0, color1, mixf.x, lowerThreshold_, lowerThreshold_);
    return color;
}

vec4 jitterPreClassFetch(vec3 sample, sampler3D volumeTex, VOLUME_PARAMETERS volumeTexParameters,
                         sampler2D noiseTex)
{
    float intensity = textureLookup3D(volumeTex, volumeTexParameters, sample).a;

    vec3 jsample = sample +
    (texture2D(noiseTex,vec2(intensity*0.23423+sample.x*3.141+sample.y,-intensity*0.0967+sample.z*3.141+sample.x)).rgb
        - vec3(0.5)) * volumeTexParameters.datasetDimensionsRCP_ /4.0;
    float jintensity = textureLookup3D(volumeTex, volumeTexParameters, jsample).a;

#ifdef BITDEPTH_12
	if (!(jintensity*16.0 >= lowerThreshold_ && jintensity*16.0 < upperThreshold_)) {
#else
	if (!(jintensity >= lowerThreshold_ && jintensity < upperThreshold_)) {
#endif
        jintensity = intensity;
    }
    vec4 color = applyTF(jintensity);

    vec3 pos = sample * volumeTexParameters.datasetDimensions_;
    vec3 erg1 = floor(pos);
    vec3 erg2 = pos + vec3(1.0);
    vec3 mixf = pos - erg1;
    erg1 *= volumeTexParameters.datasetDimensionsRCP_;
    erg2 *= volumeTexParameters.datasetDimensionsRCP_;

	float alpha000 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, erg1).a).a;
	float alpha001 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg1.y,erg2.z)).a).a;
	float alpha010 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg2.y,erg1.z)).a).a;
	float alpha011 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg2.y,erg2.z)).a).a;
	float alpha100 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg1.y,erg1.z)).a).a;
	float alpha101 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg1.y,erg2.z)).a).a;
	float alpha110 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg2.y,erg1.z)).a).a;
	float alpha111 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, erg2).a).a;

    float alpha00 = (alpha001-alpha000)*mixf.z+alpha000;
    float alpha01 = (alpha011-alpha010)*mixf.z+alpha010;
    float alpha10 = (alpha101-alpha100)*mixf.z+alpha100;
    float alpha11 = (alpha111-alpha110)*mixf.z+alpha110;

    float alpha0 = (alpha01-alpha00)*mixf.y+alpha00;
    float alpha1 = (alpha11-alpha10)*mixf.y+alpha10;

    color.a *= (alpha1-alpha0)*mixf.x+alpha0;

    return color;
}

vec4 intensityPreClassFetch(vec3 sample, sampler3D volumeTex, VOLUME_PARAMETERS volumeTexParameters,
                            float factor)
{
    float intensity = textureLookup3D(volumeTex, volumeTexParameters, floor((sample * volumeTexParameters.datasetDimensions_)+vec3(0.5))*volumeTexParameters.datasetDimensionsRCP_).a;
    vec3 pos = sample * volumeTexParameters.datasetDimensions_;
    vec3 erg1 = floor(pos);
    vec3 erg2 = pos + vec3(1.0);
    vec3 mixf = pos - erg1;
    erg1 *= volumeTexParameters.datasetDimensionsRCP_;
    erg2 *= volumeTexParameters.datasetDimensionsRCP_;

	float intensity000 = textureLookup3D(volumeTex, volumeTexParameters, erg1).a;
	float intensity001 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg1.y,erg2.z)).a;
	float intensity010 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg2.y,erg1.z)).a;
	float intensity011 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg2.y,erg2.z)).a;
	float intensity100 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg1.y,erg1.z)).a;
	float intensity101 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg1.y,erg2.z)).a;
	float intensity110 = textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg2.y,erg1.z)).a;
	float intensity111 = textureLookup3D(volumeTex, volumeTexParameters, erg2).a;

    float intensity00 = (intensity001-intensity000)*mixf.z+intensity000;
    float intensity01 = (intensity011-intensity010)*mixf.z+intensity010;
    float intensity10 = (intensity101-intensity100)*mixf.z+intensity100;
    float intensity11 = (intensity111-intensity110)*mixf.z+intensity110;

    float intensity0 = (intensity01-intensity00)*mixf.y+intensity00;
    float intensity1 = (intensity11-intensity10)*mixf.y+intensity10;

    float dif = abs(intensity - ((intensity1-intensity0)*mixf.x+intensity0));

    vec4 color = applyTF(intensity);
    color.a *= (1.0-min(1.0,dif/factor));
    return color;
}

vec4 alphaPreClassFetch(vec3 sample, sampler3D volumeTex, VOLUME_PARAMETERS volumeTexParameters)
{
    float intensity = textureLookup3D(volumeTex, volumeTexParameters, sample).a;

    vec4 color = applyTF(intensity);

    vec3 pos = sample * volumeTexParameters.datasetDimensions_;
    vec3 erg1 = floor(pos);
    vec3 erg2 = pos + vec3(1.0);
    vec3 mixf = pos - erg1;
    erg1 *= volumeTexParameters.datasetDimensionsRCP_;
    erg2 *= volumeTexParameters.datasetDimensionsRCP_;

	float alpha000 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, erg1).a).a;
	float alpha001 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg1.y,erg2.z)).a).a;
	float alpha010 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg2.y,erg1.z)).a).a;
	float alpha011 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg1.x,erg2.y,erg2.z)).a).a;
	float alpha100 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg1.y,erg1.z)).a).a;
	float alpha101 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg1.y,erg2.z)).a).a;
	float alpha110 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, vec3(erg2.x,erg2.y,erg1.z)).a).a;
	float alpha111 = applyTF(textureLookup3D(volumeTex, volumeTexParameters, erg2).a).a;

    float alpha00 = (alpha001-alpha000)*mixf.z+alpha000;
    float alpha01 = (alpha011-alpha010)*mixf.z+alpha010;
    float alpha10 = (alpha101-alpha100)*mixf.z+alpha100;
    float alpha11 = (alpha111-alpha110)*mixf.z+alpha110;

    float alpha0 = (alpha01-alpha00)*mixf.y+alpha00;
    float alpha1 = (alpha11-alpha10)*mixf.y+alpha10;

    color.a *= (alpha1-alpha0)*mixf.x+alpha0;

    return color;
}

vec4 planePreClassFetch(vec3 sample, sampler3D volumeTex, VOLUME_PARAMETERS volumeTexParameters, vec3 volumeDimensions,
                        sampler1D classTex, float fetchFactor, float weighting, float dist, vec3 normal)
{
	vec3 voxelSize = vec3(1.0) * volumeTexParameters.datasetDimensionsRCP_;
	vec3 n = normalize(normal);

	vec3 u = normalize(vec3(-n.y, n.x, 0));
	vec3 v = normalize(cross(n,u));

	vec3 pos1 = sample + u*voxelSize*dist;
	vec3 pos2 = sample - u*voxelSize*dist;
	vec3 pos3 = sample + v*voxelSize*dist;
	vec3 pos4 = sample - v*voxelSize*dist;

	vec4 normal0 = textureLookup3D(volumeTex, volumeTexParameters, sample);
	vec4 normal1 = textureLookup3D(volumeTex, volumeTexParameters, pos1);
	vec4 normal2 = textureLookup3D(volumeTex, volumeTexParameters, pos2);
	vec4 normal3 = textureLookup3D(volumeTex, volumeTexParameters, pos3);
	vec4 normal4 = textureLookup3D(volumeTex, volumeTexParameters, pos4);

	vec4 voxColor0 = texture1D(classTex, normal0.a*fetchFactor);
	vec4 voxColor1 = texture1D(classTex, normal1.a*fetchFactor);
	vec4 voxColor2 = texture1D(classTex, normal2.a*fetchFactor);
	vec4 voxColor3 = texture1D(classTex, normal3.a*fetchFactor);
	vec4 voxColor4 = texture1D(classTex, normal4.a*fetchFactor);

	return weighting*voxColor0 + ((1.0-weighting)/4.0)*(voxColor1+voxColor2+voxColor3+voxColor4);
}
#endif
