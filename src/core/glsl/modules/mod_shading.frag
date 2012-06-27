/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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
 * the current sample within a raycaster.
 * The functions below, which implement a full shading model as for
 * instance Phong or Toon shading, are referenced by RC_APPLY_SHADING
 * which is used in the raycaster fragment shaders.
 */

/*
 * This struct contains all information about reuqired
 * to access a light source.
 */
struct LIGHT_SOURCE {
    vec3 position_;        // light position in world space
    vec3 ambientColor_;    // ambient color (r,g,b)
    vec3 diffuseColor_;    // diffuse color (r,g,b)
    vec3 specularColor_;   // specular color (r,g,b)
    vec3 attenuation_;     // attenuation (constant, linear, quadratic)
};


// uniforms needed for shading
uniform vec3 cameraPosition_;   // in world coordinates
uniform float shininess_;       // material shininess parameter
uniform LIGHT_SOURCE lightSource_;


/**
 * Returns attenuation based on the currently set opengl values.
 * Incorporates constant, linear and quadratic attenuation.
 *
 * @param d Distance to the light source.
 */
float getAttenuation(in float d) {
    float att = 1.0 / (lightSource_.attenuation_.x +
                       lightSource_.attenuation_.y * d +
                       lightSource_.attenuation_.z * d * d);
    return min(att, 1.0);
}


/**
 * Returns the ambient term, considering the user defined lighting
 * parameters.
 *
 * @param ka The ambient color to be used, which is fetched from the
 * transfer function.
 */
vec3 getAmbientTerm(in vec3 ka) {
    return ka * lightSource_.ambientColor_;
}


/**
 * Returns the diffuse term, considering the user defined lighting
 * parameters.
 *
 * @param kd The diffuse color to be used, which is fetched from the
 * transfer function.
 * @param N The surface normal used for lambert shading.
 * @param L The normalized light vector used for lambert shading.
 */
vec3 getDiffuseTerm(in vec3 kd, in vec3 N, in vec3 L) {
    float NdotL = max(dot(N, L), 0.0);
    return kd * lightSource_.diffuseColor_ * NdotL;
}


/**
 * This function implements the soft lighting technique described by
 * Josip Basic in the technote 'A cheap soft lighting for real-time 3D
 * environments.
 *
 * @param kd The diffuse color to be used, which is fetched from the
 * transfer function.
 * @param N The surface normal used for lambert shading.
 * @param L The normalized light vector used for lambert shading.
 */
vec3 getLerpDiffuseTerm(in vec3 kd, in vec3 N, in vec3 L) {
    float alpha = 0.5;
    vec3 NV = mix(N, L, alpha);
    float NVdotL = max(dot(NV, L), 0.0);
    return kd * lightSource_.diffuseColor_ * NVdotL;
}


/**
 * Returns the specular term, considering the user defined lighting
 * parameters.
 *
 * @param ks The specular material color to be used.
 * @param N The surface normal used.
 * @param L The normalized light vector used.
 * @param V The viewing vector used.
 * @param alpha The shininess coefficient used.
 */
vec3 getSpecularTerm(in vec3 ks, in vec3 N, in vec3 L, in vec3 V, in float alpha) {
    vec3 H = normalize(V + L);
    float NdotH = pow(max(dot(N, H), 0.0), alpha);
    return ks * lightSource_.specularColor_ * NdotH;
}

/**
 * Calculates phong shading by considering the currently set OpenGL lighting
 * and material parameters.  The front material's shininess parameter is used
 * in the calculation of the specular term.
 * Attenuation is applied, if the symbol PHONG_APPLY_ATTENUATION is defined.
 *
 * @param pos The position to to be lighted given in world space
 * @param normal The object normal given in world space (does not need to be normalized).
 */
vec3 phongShading(in vec3 pos, in vec3 normal, in vec3 ka, in vec3 kd, in vec3 ks) {

    vec3 N = normalize(normal);
    vec3 L = lightSource_.position_ - pos;
    vec3 V = normalize(cameraPosition_ - pos);

    // get light source distance for attenuation and normalize light vector
    float d = length(L);
    L /= d;

    vec3 shadedColor = vec3(0.0);
    shadedColor += getAmbientTerm(ka);
    shadedColor += getDiffuseTerm(kd, N, L);
    shadedColor += getSpecularTerm(ks, N, L, V, shininess_);
    #ifdef PHONG_APPLY_ATTENUATION
        shadedColor *= getAttenuation(d);
    #endif
    return shadedColor;
}

/**********************************************************************
 *    The following functions require volume access and can therefore *
 *    only be defined, when the texture access has been defined in    *
 *    mod_sampler3d.frag or mod_bricking.frag.                        *
 **********************************************************************/

 #ifdef MOD_SAMPLER3D

/**
 * Calculates phong shading for a volume voxel by considering the user defined lighting
 * parameters.
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
    vec3 L = volumeParams.lightPositionOBJ_ - vpos;              // using light position in volume object space
    vec3 V = normalize(volumeParams.cameraPositionOBJ_ - vpos);  // using camera position in volume object space

    // get light source distance for attenuation and normalize light vector
    float d = length(L);
    L /= d;

    vec3 shadedColor = vec3(0.0);
    shadedColor += getAmbientTerm(ka);
    shadedColor += getDiffuseTerm(kd, N, L);
    shadedColor += getSpecularTerm(ks, N, L, V, shininess_);
    #ifdef PHONG_APPLY_ATTENUATION
        shadedColor *= getAttenuation(d);
    #endif
    return shadedColor;
}


/**
 * Calculates phong shading without considering the ambient term.
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
    vec3 L = volumeParams.lightPositionOBJ_ - vpos;              // using light position in volume object space
    vec3 V = normalize(volumeParams.cameraPositionOBJ_ - vpos);  // using camera position in volume object space

    // get light source distance for attenuation and normalize light vector
    float d = length(L);
    L /= d;

    vec3 shadedColor = vec3(0.0);
    shadedColor += getDiffuseTerm(kd, N, L);
    shadedColor += getSpecularTerm(ks, N, L, V, shininess_);
    #ifdef PHONG_APPLY_ATTENUATION
        shadedColor *= getAttenuation(d);
    #endif
    return shadedColor;
}


/**
 * Calculates phong shading with only considering the specular term.
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
    vec3 L = volumeParams.lightPositionOBJ_ - vpos;              // using light position in volume object space
    vec3 V = normalize(volumeParams.cameraPositionOBJ_ - vpos);  // using camera position in volume object space

    // get light source distance for attenuation and normalize light vector
    float d = length(L);
    L /= d;

    vec3 shadedColor = vec3(0.0);
    shadedColor += getSpecularTerm(ks, N, L, V, shininess_);
    #ifdef PHONG_APPLY_ATTENUATION
        shadedColor *= getAttenuation(d);
    #endif
    return shadedColor;
}


/**
 * Calculates phong shading without considering the specular term.
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
    vec3 L = volumeParams.lightPositionOBJ_ - vpos;    // using light position in volume object space

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
    vec3 L = volumeParams.lightPositionOBJ_ - vpos;    // using light position in volume object space

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
    vec3 L = volumeParams.lightPositionOBJ_ - vpos;    // using light position in volume object space
    float NdotL = max(dot(N,L),0.0);

    kd *= lightSource_.diffuseColor_;

    for (int i=1; i <= numShades; i++) {
        if (NdotL <= float(i) / float(numShades)) {
            NdotL = float(i) / float(numShades);
            break;
        }
    }
    return vec3(kd.rgb * NdotL);
}

#endif // MOD_SAMPLER3D

