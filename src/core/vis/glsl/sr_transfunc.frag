/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

// needs to be include or mod_transfunc.frag will cause errors due to
// methods planePreClassFetch() and triPreClassFetch() used for
// ambient occlusion raycaster... (quite ugly imho) (df)
//
#define MOD_SAMPLER3D 1
#include "modules/mod_sampler3d.frag"
#line 3

#define MOD_TF_SIMPLE 1
#include "modules/mod_transfunc.frag"
#line 7

uniform sampler3D volumeDataset_;

// conditional uniforms and varyings
#ifdef USE_PHONG_LIGHTING
    varying vec3 v_;
    uniform vec3 datasetDimensions_;
#endif // USE_PHONG_LIGHTING

#ifdef USE_SAMPLING_RATE
    uniform float samplingRate_;
#endif // USE_SAMPLING_RATE


void main() {
    // fetch intensity
    vec4 intensity = texture3D(volumeDataset_, gl_TexCoord[0].xyz);

/*
    // calculate normal
    //
#ifdef USE_PHONG_LIGHTING
    vec3 n;
//#endif // USE_PHONG_LIGHTING

//#ifdef USE_PHONG_LIGHTING
#ifdef USE_CALC_GRADIENTS
    // calculate gradient on the fly
    //
    vec3 offset = vec3(1.0, 1.0, 1.0);
    offset /= datasetDimensions_;

    // six lookups for the volume
    vec3 sample1, sample2;
    sample2.x = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3(-offset.x, 0.0, 0.0)).a;
    sample2.y = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3(0.0, -offset.y, 0.0)).a;
    sample2.z = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3(0.0, 0.0, -offset.z)).a;
    sample1.x = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3( offset.x, 0.0, 0.0)).a;
    sample1.y = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3(0.0,  offset.y, 0.0)).a;
    sample1.z = texture3D(volumeDataset_, gl_TexCoord[0].xyz + vec3(0.0, 0.0,  offset.z)).a;

    // six lookups for the transfer function
    vec3 alpha1, alpha2;
    alpha1.x = applyTF(sample1.x).a;
    alpha1.y = applyTF(sample1.y).a;
    alpha1.z = applyTF(sample1.z).a;
    alpha2.x = applyTF(sample2.x).a;
    alpha2.y = applyTF(sample2.y).a;
    alpha2.z = applyTF(sample2.z).a;

    n = normalize(alpha2 - alpha1);
    intensity.xyz = n;
    n = normalize(gl_NormalMatrix * n);
#else // USE_CALC_GRADIENTS
    n = normalize(gl_NormalMatrix * normalize(intensity.xyz));
    intensity.xyz -= 0.5;   //is this done by the matrix above? the un-normalized normal is needed for the 2d TF
#endif // USE_CALC_GRADIENTS
#endif // USE_PHONG_LIGHTING
*/

    // get value via the transfer function
    vec4 mat = applyTF(transferFunc_, intensity.a);

    // keep sampling rate in mind
    //
#ifdef USE_SAMPLING_RATE
    mat.a *= samplingRate_;
#endif // USE_SAMPLING_RATE

/*
    // calculate phong lighting
    //
#ifdef USE_PHONG_LIGHTING
    vec3 l = normalize(gl_LightSource[0].position.xyz - v_);
    vec3 r = 2.0 * dot(n, l) * (n - l);
//     vec3 h = normalize(l + v);


    vec4 ambi = gl_LightSource[0].ambient  * mat * 0.1;
    vec4 diff = gl_LightSource[0].diffuse  * mat * max( dot(n, l), 0.0 );
    vec4 spec = gl_LightSource[0].specular * mat * pow( max(dot(r, v_), 0.0), 2.0);

    float d = length(v_);
    float factor = 1.0 /
        ( gl_LightSource[0].constantAttenuation
        + gl_LightSource[0].linearAttenuation * d
        + gl_LightSource[0].quadraticAttenuation * d*d);

    //vec4 fragColor = min( vec4(1.0, 1.0, 1.0, 1.0), gl_Material ambi + factor * (spec + diff) ) * gl_Color;
    vec4 fragColor = min( vec4(1.0, 1.0, 1.0, 1.0), ambi + factor * (spec + diff) ) * gl_Color;
    fragColor.a = mat.a * gl_Color.a;
#else // USE_PHONG_LIGHTING
*/
    vec4 fragColor = mat;
//#endif // USE_PHONG_LIGHTING

    gl_FragColor = fragColor;
}

