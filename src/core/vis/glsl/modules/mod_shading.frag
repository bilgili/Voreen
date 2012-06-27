/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

/**
 * This module contains all functions which can be used for shading
 * the current voxel within a raycaster.
 * The functions below, which implement a full shading model as for
 * instance Phong or Toon shading, are referenced by RC_APPLY_SHADING
 * which is used in the raycaster fragment shaders.
 */

// using the define is necessary as ATi GLSL compilers have difficulties with
// assigments like lightParams = gl_LightSource[0]
#define lightParams gl_LightSource[0]

// front material
gl_MaterialParameters matParams = gl_FrontMaterial;

// uniforms needed for shading
uniform vec3 cameraPosition_;   // in world coordinates
uniform vec3 lightPosition_;    // in world coordinates    

/**
 * Returns attenuation based on the currently set opengl values.
 * Incorporates constant, linear and quadratic attenuation.
 *
 * @param d Distance to the light source.
 */
float getAttenuation(in float d) {
	return 1.0 / (lightParams.constantAttenuation +
				  lightParams.linearAttenuation * d +
				  lightParams.quadraticAttenuation * d * d);
}


/**
 * Returns the ambient term, considering the currently set opengl lighting
 * parameters. When USE_OPENGL_MATERIAL is set, opengls current ambient
 * color is used, otherwise the color ka.
 *
 * @param ka The ambient color to be used. Usually this is fetched from the
 * transfer function.
 */
vec3 getAmbientTerm(in vec3 ka) {
	#ifdef USE_OPENGL_MATERIAL
		ka = matParams.ambient.rgb;
	#endif
	return ka * lightParams.ambient.rgb;
}


/**
 * Returns the diffuse term, considering the currently set opengl lighting
 * parameters. When USE_OPENGL_MATERIAL is set, opengls current diffuse
 * color is used, otherwise the color kd.
 *
 * @param kd The diffuse color to be used. Usually this is fetched from the
 * transfer function.
 * @param N The surface normal used for lambert shading.
 * @param L The normalized light vector used for lambert shading.
 */
vec3 getDiffuseTerm(in vec3 kd, in vec3 N, in vec3 L) {
	float NdotL = max(dot(N, L), 0.0);
	#ifdef USE_OPENGL_MATERIAL
		kd = matParams.diffuse.rgb;
	#endif
	return kd * lightParams.diffuse.rgb * NdotL;
}


/**
 * This function implements the soft lighting technique described by
 * Josip Basic in the technote 'A cheap soft lighting for real-time 3D
 * environments. When USE_OPENGL_MATERIAL is set, opengls current diffuse
 * color is used, otherwise the color kd.
 *
 * @param kd The diffuse color to be used. Usually this is fetched from the
 * transfer function.
 * @param N The surface normal used for lambert shading.
 * @param L The normalized light vector used for lambert shading.
 */
vec3 getLerpDiffuseTerm(in vec3 kd, in vec3 N, in vec3 L) {
	float alpha = 0.5;
	vec3 NV = mix(N, L, alpha);
	float NVdotL = max(dot(NV, L), 0.0);
	#ifdef USE_OPENGL_MATERIAL
		kd = matParams.diffuse.rgb;
	#endif
	return kd * lightParams.diffuse.rgb * NVdotL;
}


/**
 * Returns the specular term, considering the currently set opengl lighting
 * parameters. When USE_OPENGL_MATERIAL is set, opengls current specular
 * color and shininess is used, otherwise the color ks and the shininess coefficient alpha.
 *
 * @param ks The specular material color to be used.
 * @param N The surface normal used.
 * @param L The normalized light vector used.
 * @param V The viewing vector used.
 * @param alpha The shininess coefficient used.
 */
vec3 getSpecularTerm(in vec3 ks, in vec3 N, in vec3 L, in vec3 V, in float alpha) {
	vec3 H = normalize(V + L);
	#ifdef USE_OPENGL_MATERIAL
		ks = matParams.specular.rgb;
		alpha = matParams.shininess;
	#endif
    float NdotH = pow(max(dot(N, H), 0.0), alpha);
	return ks * lightParams.specular.rgb * NdotH;
}


/**
 * Calculates phong shading by considering the currently set opengl lighting
 * parameters. The parameters ka, kd and ks are not used when USE_OPENGL_MATERIAL is defined,
 * instead the currently set opengl materials are taken into account.
 * The front material's shininess parameter is used in the calculation of the specular term.
 * Attenuation is applied, if the symbol PHONG_APPLY_ATTENUATION is defined.
 *
 * @param gradient The gradient given in volume object space (does not need to be normalized).
 * @param vposTex The voxel position given in volume texture space.
 * @param volumeParams the parameters of the volume to be shaded
 * @param ka The ambient material color to be used.
 * @param kd The diffuse material color to be used.
 * @param ks The specular material color to be used.
 */
vec3 phongShading(in vec3 gradient, in vec3 vposTex, in VOLUME_PARAMETERS volumeParams, in vec3 ka, in vec3 kd, in vec3 ks) {
    // transform voxel position to the volume's object space
    vec3 vpos = (vposTex-0.5)*volumeParams.volumeCubeSize_;
    vec3 N = normalize(gradient);
    vec3 L = lightPosition_ - vpos;
	vec3 V = normalize(cameraPosition_ - vpos);

	// get light source distance for attenuation and normalize light vector
    float d = length(L);
	L /= d;

	vec3 shadedColor = vec3(0.0);
    shadedColor += getAmbientTerm(ka);
    shadedColor += getDiffuseTerm(kd, N, L);
    shadedColor += getSpecularTerm(ks, N, L, V, matParams.shininess);
    #ifdef PHONG_APPLY_ATTENUATION
        shadedColor *= getAttenuation(d);
    #endif
    return shadedColor;
}


/**
 * Calculates phong shading without considering the ambient term.
 * The parameter kd is not considered when USE_OPENGL_MATERIAL is defined,
 * instead the currently set opengl diffuse material is taken into account.
 * The front material's shininess parameter is used in the calculation of the specular term.
 * Attenuation is applied, if the symbol PHONG_APPLY_ATTENUATION is defined.
 *
 * @param gradient The gradient given in volume object space (does not need to be normalized).
 * @param vposTex The voxel position given in volume texture space.
 * @param volumeParams the parameters of the volume to be shaded
 * @param kd The diffuse material color to be used.
 * @param ks The specular material color to be used.
 */
vec3 phongShadingDS(in vec3 gradient, in vec3 vposTex, in VOLUME_PARAMETERS volumeParams, in vec3 kd, in vec3 ks) {
    // transform voxel position to the volume's object space
    vec3 vpos = (vposTex-0.5)*volumeParams.volumeCubeSize_;
    vec3 N = normalize(gradient);
    vec3 L = lightPosition_ - vpos;
	vec3 V = normalize(cameraPosition_ - vpos);

	// get light source distance for attenuation and normalize light vector
    float d = length(L);
    L /= d;

	vec3 shadedColor = vec3(0.0);
	shadedColor += getDiffuseTerm(kd, N, L);
	shadedColor += getSpecularTerm(ks, N, L, V, 1.0);
    #ifdef PHONG_APPLY_ATTENUATION
        shadedColor *= getAttenuation(d);
    #endif
    return shadedColor;
}


/**
 * Calculates phong shading with only considering the specular term.
 * The parameter kd is not considered when USE_OPENGL_MATERIAL is defined,
 * instead the currently set opengl diffuse material is taken into account.
 * The front material's shininess parameter is used in the calculation of the specular term.
 * Attenuation is applied, if the symbol PHONG_APPLY_ATTENUATION is defined.
 *
 * @param gradient The gradient given in volume object space (does not need to be normalized).
 * @param vposTex The voxel position given in volume texture space.
 * @param volumeParams the parameters of the volume to be shaded
 * @param ks The specular material color to be used.
 */
vec3 phongShadingS(in vec3 gradient, in vec3 vposTex, in VOLUME_PARAMETERS volumeParams, in vec3 ks) {
    // transform voxel position to the volume's object space
    vec3 vpos = (vposTex-0.5)*volumeParams.volumeCubeSize_;
    vec3 N = normalize(gradient);
    vec3 L = normalize(lightPosition_ - vpos);
	vec3 V = normalize(cameraPosition_ - vpos);

	// get light source distance for attenuation and normalize light vector
    float d = length(L);
    L /= d;
        
	vec3 shadedColor = vec3(0.0);
	shadedColor += getSpecularTerm(ks, N, L, V, matParams.shininess);
    #ifdef PHONG_APPLY_ATTENUATION
        shadedColor *= getAttenuation(d);
    #endif
    return shadedColor;
}


/**
 * Calculates phong shading without considering the specular term.
 * The parameter kd is not considered when USE_OPENGL_MATERIAL is defined,
 * instead the currently set opengl diffuse material is taken into account.
 * Attenuation is applied, if the symbol PHONG_APPLY_ATTENUATION is defined.
 *
 * @param gradient The gradient given in volume object space (does not need to be normalized).
 * @param vposTex The voxel position given in volume texture space.
 * @param volumeParams the parameters of the volume to be shaded
 * @param kd The diffuse material color to be used.
 * @param ka The ambient material color to be used.
 */
vec3 phongShadingDA(in vec3 gradient, in vec3 vposTex, in VOLUME_PARAMETERS volumeParams, in vec3 kd, in vec3 ka) {
    // transform voxel position to the volume's object space
    vec3 vpos = (vposTex-0.5)*volumeParams.volumeCubeSize_;
    vec3 N = normalize(gradient);
    vec3 L = lightPosition_ - vpos;

	// get light source distance for attenuation and normalize light vector
    float d = length(L);
    L /= d;

	vec3 shadedColor = vec3(0.0);
    shadedColor += getAmbientTerm(ka);
	shadedColor += getDiffuseTerm(kd, N, L);
	#ifdef PHONG_APPLY_ATTENUATION
        shadedColor *= getAttenuation(d);
    #endif
    return shadedColor;
}


/**
 * Calculates lambertian shading.
 * The parameter kd is not considered when USE_OPENGL_MATERIAL is defined,
 * instead the currently set opengl diffuse material is taken into account.
 * Attenuation is applied, if the symbol PHONG_APPLY_ATTENUATION is defined.
 *
 * @param gradient The gradient given in volume object space (does not need to be normalized).
 * @param vposTex The voxel position given in volume texture space.
 * @param volumeParams the parameters of the volume to be shaded
 * @param kd The diffuse material color to be used.
 */
vec3 phongShadingD(in vec3 gradient, in vec3 vposTex, in VOLUME_PARAMETERS volumeParams, in vec3 kd) {
    // transform voxel position to the volume's object space
    vec3 vpos = (vposTex-0.5)*volumeParams.volumeCubeSize_;
    vec3 N = normalize(gradient);
    vec3 L = lightPosition_ - vpos;

	// get light source distance for attenuation and normalize light vector
    float d = length(L);
    L /= d;

	vec3 shadedColor = vec3(0.0);
	shadedColor += getDiffuseTerm(kd, N, L);
	#ifdef PHONG_APPLY_ATTENUATION
        shadedColor *= getAttenuation(d);
    #endif
    return shadedColor;
}


/**
 * Calculates a quantized toon shading.
 * The parameter kd is not considered when USE_OPENGL_MATERIAL is defined,
 * instead the currently set opengl diffuse material is taken into account.
 *
 * @param gradient The gradient given in volume object space (does not need to be normalized).
 * @param vposTex The voxel position given in volume texture space.
 * @param volumeParams the parameters of the volume to be shaded
 * @param numShades The number of different shadings.
 */
vec3 toonShading(in vec3 gradient, in vec3 vposTex, in VOLUME_PARAMETERS volumeParams, in vec3 kd, in int numShades) {
    // transform voxel position to the volume's object space
    vec3 vpos = (vposTex-0.5)*volumeParams.volumeCubeSize_;
    vec3 N = normalize(gradient);
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