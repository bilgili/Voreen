#include "modules/mod_shadowing.frag"

/**
 * Returns attenuation based on the currently set opengl values.
 * Incorporates constant, linear and quadratic attenuation.
 * If PHONG_APPLY_ATTENUATION is undefined 1.0 is returned.
 *
 * @param d Distance to the light source. 
 */
float getAttenuation(in float d) {
	#if defined(PHONG_APPLY_ATTENUATION)
		return 1.0 / (lightParams.constantAttenuation +
					  lightParams.linearAttenuation * d +
					  lightParams.quadraticAttenuation * d * d);
	#else
		return 1.0;
	#endif
}

/**
 * Returns the ambient term, considering the currently set opengl lighting
 * parameters. When USE_OPENGL_MATERIAL is set, opengls current ambient
 * color is used, otherwise the color ka.
 * If PHONG_ADD_AMBIENT is undefined vec3(0.0) is returned.
 *
 * @param ka The ambient color to be used. Usually this is fetched from the
 * transfer function.
 */
vec3 getAmbientTerm(in vec3 ka) {
	#if defined(PHONG_ADD_AMBIENT)
		#ifdef USE_OPENGL_MATERIAL
			ka = matParams.ambient.rgb;
		#endif
		return ka * lightParams.ambient.rgb;       
	#else
		return vec3(0.0);
	#endif
}

/**
 * Returns the diffuse term, considering the currently set opengl lighting
 * parameters. When USE_OPENGL_MATERIAL is set, opengls current diffuse
 * color is used, otherwise the color kd.
 * if PHONG_ADD_DIFFUSE is undefined vec3(0.0) is returned.
 *
 * @param kd The diffuse color to be used. Usually this is fetched from the
 * transfer function.
 * @param N The surface normal used for lambert shading.
 * @param L The normalized light vector used for lambert shading.
 */
vec3 getDiffuseTerm(in vec3 kd, in vec3 N, in vec3 L) {
	#if defined(PHONG_ADD_DIFFUSE)
		float NdotL = max(dot(N, L), 0.0);
		#ifdef USE_OPENGL_MATERIAL
			kd = matParams.diffuse.rgb;
		#endif
		return kd * lightParams.diffuse.rgb * NdotL;
	#else
		return vec3(0.0);
	#endif
}

/**
 * This function implements the soft lighting technique described by
 * Josip Basic in the technote 'A cheap soft lighting for real-time 3D
 * environments. When USE_OPENGL_MATERIAL is set, opengls current diffuse
 * color is used, otherwise the color kd. If PHONG_ADD_DIFFUSE is
 * undefined vec3(0.0) is returned.
 *
 * @param kd The diffuse color to be used. Usually this is fetched from the
 * transfer function.
 * @param N The surface normal used for lambert shading.
 * @param L The normalized light vector used for lambert shading.
 */
vec3 getLerpLambert(in vec3 kd, in vec3 N, in vec3 L) {
	#if defined(PHONG_ADD_DIFFUSE)
		float alpha = 0.5;
		vec3 Nv = alpha*N + (1.0-alpha)*L;//lerp(N, L, alpha);
		float NdotL = max(dot(Nv, L), 0.0);
		#ifdef USE_OPENGL_MATERIAL
			kd = matParams.diffuse.rgb;
		#endif
		return kd * lightParams.diffuse.rgb * NdotL;
	#else
		return vec3(0.0);
	#endif
}

/**
 * Returns the specular term, considering the currently set opengl lighting
 * and material parameters.
 * If PHONG_ADD_SPECULAR is undefined vec3(0.0) is returned.
 *
 * @param vpos The current voxel's position in object coordinates
 * @param N The surface normal used.
 * @param L The normalized light vector used.
 */
vec3 getSpecularTerm(in vec3 vpos, in vec3 N, in vec3 L) {
	#if defined(PHONG_ADD_SPECULAR)
    	vec3 V = normalize(cameraPosition_ - vpos);
		vec3 H = normalize(V + L);       

        float NdotH = pow(max(dot(N, H), 0.0), matParams.shininess);
		return matParams.specular.rgb * lightParams.specular.rgb * NdotH;
	#else
		return vec3(0.0);
	#endif
}

/**
 * Calculates phong shading without considering the ambient term.
 * Diffuse reflection is only considered when PHONG_ADD_DIFFUSE is defined.
 * Specular reflection is only considered when PHONG_ADD_SPECULAR is defined.
 * The parameter kd is not considered when USE_OPENGL_MATERIAL is defined,
 * instead the currently set opengl diffuse material is taken into account.
 *
 * @param normal The normal given in volume object space (does not need to be normalized).
 * @param vposTex The voxel position given in volume texture space.
 * @param volumeParams the parameters of the volume to be shaded
 * @param kd The diffuse material color to be used.
 */
vec3 phongShadingNoKa(in vec4 normal, in vec3 vposTex, in VOLUME_PARAMETERS volumeParams, in vec3 kd) {
    
    // transform voxel position to the volume's object space
    vec3 vpos = (vposTex-0.5)*volumeParams.volumeCubeSize_;
    vec3 N = normalize(normal.xyz);
    vec3 L = lightPosition_ - vpos;
    
    float d = length(L);
    L /= d; // normalize light vector (looks complicated, but easier for the optimizer)
    vec3 shadedColor = vec3(0.0);

	shadedColor += getDiffuseTerm(kd, N, L);
	shadedColor += getSpecularTerm(vpos, N, L);

    shadedColor *= getAttenuation(d);

    return shadedColor;
}

/**
 * Calculates phong shading.
 * Ambient reflection is only considered when PHONG_ADD_AMBIENT is defined.
 * Diffuse reflection is only considered when PHONG_ADD_DIFFUSE is defined.
 * Specular reflection is only considered when PHONG_ADD_SPECULAR is defined.
 * The parameters ka and kd are not considered when USE_OPENGL_MATERIAL is defined,
 * instead the currently set opengl materials are taken into account.
 *
 * @param normal The normal given in volume object space (does not need to be normalized).
 * @param vposTex The voxel position given in volume texture space.
 * @param volumeParams the parameters of the volume to be shaded
 * @param kd The diffuse material color to be used.
 * @param ka The ambient material color to be used.
 */
vec3 phongShading(in vec4 normal, in vec3 vposTex, in VOLUME_PARAMETERS volumeParams, in vec3 kd, in vec3 ka) {
    
    // transform voxel position to the volume's object space
    vec3 vpos = (vposTex-0.5)*volumeParams.volumeCubeSize_;
    vec3 N = normalize(normal.xyz);
    vec3 L = lightPosition_ - vpos;
    
    float d = length(L);
    L /= d; // normalize light vector (looks complicated, but easier for the optimizer)
    vec3 shadedColor = vec3(0.0);

    shadedColor += getDiffuseTerm(kd, N, L);
    shadedColor += getSpecularTerm(vpos, N, L);

    shadedColor += getAmbientTerm(ka);
    shadedColor *= getAttenuation(d);
    
    return shadedColor;
}
 
//overview version:
// vec3 phongShading(in vec4 normal, in vec3 vpos, in vec3 kd, in vec3 ka) {
//  
//     vec3 N = normalize(normal.xyz);
//     vec3 L = lightPosition_ - vpos;
//     
//     float d = length(L);
//     L /= d; // normalize light vector (looks complicated, but easier for the optimizer)
//     vec3 shadedColor = vec3(0.0);
// 
// 	return lightParams.diffuse.rgb;
// 	//return vec3(max(dot(N, L), 0.0));
// 
// 	shadedColor += getDiffuseTerm(kd, N, L);
// 	//shadedColor += getSpecularTerm(vpos, N, L);
// 
// 
//     //shadedColor += getAmbientTerm(ka);
// 	//shadedColor *= getAttenuation(d);
// 
// 	//return kd;
// 	
//     return shadedColor;
// }



/**
 * Calculates lambertian shading with attenuation.
 * Works only when PHONG_ADD_DIFFUSE is defined.
 * The parameter kd is not considered when USE_OPENGL_MATERIAL is defined,
 * instead the currently set opengl diffuse material is taken into account.
 *
 * @param normal The normal given in volume object space (does not need to be normalized).
 * @param vposTex The voxel position given in volume texture space.
 * @param volumeParams the parameters of the volume to be shaded
 * @param kd The diffuse material color to be used.
 */
vec3 lambertShading(in vec4 normal, in vec3 vposTex, in VOLUME_PARAMETERS volumeParams, in vec3 kd) {

    // transform voxel position to the volume's object space
    vec3 vpos = (vposTex-0.5)*volumeParams.volumeCubeSize_;
    vec3 N = normalize(normal.xyz);
    vec3 L = lightPosition_ - vpos;
    
    float d = length(L);
    L /= d; // normalize light vector (looks complicated, but easier for the optimizer)
    
	vec3 shadedColor = vec3(0.0);
	shadedColor += getDiffuseTerm(kd, N, L);
	shadedColor *= getAttenuation(d);
	
    return shadedColor;
}

/**
 * Calculates a quantized toon shading.
 * The parameter kd is not considered when USE_OPENGL_MATERIAL is defined,
 * instead the currently set opengl diffuse material is taken into account.
 *
 * @param normal The normal given in volume object space (does not need to be normalized).
 * @param vposTex The voxel position given in volume texture space.
 * @param volumeParams the parameters of the volume to be shaded
 * @param numShades The number of different shadings.
 */
vec3 toonShading(in vec4 normal, in vec3 vposTex, in VOLUME_PARAMETERS volumeParams, in vec3 kd, int numShades) {

    // transform voxel position to the volume's object space
    vec3 vpos = (vposTex-0.5)*volumeParams.volumeCubeSize_;
    vec3 N = normalize(normal.xyz);
    vec3 L = normalize(lightPosition_.xyz-vpos.xyz);
    float NdotL = max(dot(N,L),0.0);
    
	// diffuse term
	#ifdef USE_OPENGL_MATERIAL
		kd = matParams.diffuse.rgb;
	#endif
	kd *= lightParams.diffuse.rgb;

    for (int i=1; i <= numShades; i++) {
        if (NdotL <= float(i) / float(numShades)) {
            NdotL = float(i) / float(numShades);
            break;
        }
    }
    return vec3(kd.rgb * NdotL);
}
