/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "modules/mod_sampler2d.frag"
#include "modules/mod_sampler3d.frag"
#include "modules/mod_transfunc.frag"

// declare volume
uniform VolumeParameters volumeStruct_;
uniform sampler3D volume_;    // texture lookup parameters for volume_

uniform TransFuncParameters transferFunc_;
uniform TF_SAMPLER_TYPE transferFuncTex_;

uniform sampler2D occlusion_;
uniform TextureParameters occlusionParams_;

uniform float dPlaneIncr_;
uniform float sigma_;
uniform int radius_;
uniform vec3 vecView_;
uniform vec3 lightPos_;

uniform bool secondPass_;
uniform mat4 viewMatrix_;

uniform vec2 blurDirection_;

/***
 * Performs a Gaussian filter on the occlusion buffer and returns the
 * filtered value.
 *
 * @fragCoord - screen coordinates of the current fragment
 ***/
vec4 occlude(in vec2 fragCoord) {
    vec3 texCoord = gl_TexCoord[0].xyz;

    float intensity = getVoxel(volume_, volumeStruct_, texCoord).a;
    float alpha = applyTF(transferFunc_, transferFuncTex_, intensity).a;

    float result = 0.0;
    float maxDim = max(volumeStruct_.datasetDimensions_.x, max(volumeStruct_.datasetDimensions_.y, volumeStruct_.datasetDimensions_.z));

    vec2 lOff = (viewMatrix_ * vec4(lightPos_, 1.0)).xy;

    for (int i=-radius_; i <= radius_; i++) {
        float curOcclusion = textureLookup2Dscreen(occlusion_, occlusionParams_, fragCoord + 0.0001 * i * blurDirection_ + maxDim * lOff * blurDirection_ * dPlaneIncr_).a;
        result += curOcclusion;
    }

    result /= float(2*radius_+1);
    result *= sqrt(exp(-sigma_ * alpha * dPlaneIncr_));
    return vec4(result);
}

vec4 renderSlice(vec2 p) {
    vec3 texCoord = gl_TexCoord[0].xyz;

    if (texCoord.x >= 0.0 && texCoord.x <= 1.0 &&
        texCoord.y >= 0.0 && texCoord.y <= 1.0 &&
        texCoord.z >= 0.0 && texCoord.z <= 1.0) {
        float intensity = getVoxel(volume_, volumeStruct_, texCoord).a;
        vec4 color = applyTF(transferFunc_, transferFuncTex_, intensity);
        color.a = 1.0 - exp(-sigma_*color.a*dPlaneIncr_);
        float occlusion = textureLookup2Dscreen(occlusion_, occlusionParams_, p).a;
        return vec4(color.rgb * color.a * occlusion, color.a);
    } else
        return vec4(0.0);
}

/***
 * The main method.
 ***/
void main() {
    if(secondPass_)
        FragData0 = occlude(gl_FragCoord.xy);
    else
        FragData0 = renderSlice(gl_FragCoord.xy);
}
