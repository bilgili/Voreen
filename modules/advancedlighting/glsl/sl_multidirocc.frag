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

uniform VolumeParameters volumeStruct_;
uniform sampler3D volume_;    // volume data set

uniform TransFuncParameters transferFunc_;
uniform TF_SAMPLER_TYPE transferFuncTex_;

uniform sampler2D occlusionPrev_;
uniform TextureParameters occlusionParams_;

uniform mat4 lightMatrix_;
uniform sampler2D lightEllipse_;

uniform float dPlaneIncr_;
uniform int radius_;

/***
 * Performs a Gaussian filter on the occlusion buffer. And returns the
 * filtered value.
 *
 * @fragCoord - screen coordinates of the current fragment
 ***/
float occlude(in vec2 fragCoord) {
    vec3 texCoord = gl_TexCoord[0].xyz;

    float intensity = getVoxel(volume_, volumeStruct_, texCoord).a;
    float alpha = applyTF(transferFunc_, transferFuncTex_, intensity).a;

    float result = 0.0;
    float norm = 0.0;

    for (int i=-radius_; i <= radius_; i++) {
        for(int j=-radius_; j <= radius_; j++) {
            float curOcclusion = textureLookup2Dscreen(occlusionPrev_, occlusionParams_, fragCoord+ivec2(i,j)).a;

            vec2 p = (lightMatrix_ * vec4(vec2(0.5) + vec2(i,j) / (2.0 * float(radius_)), 0.0, 1.0)).xy;
            p = clamp(p, 0.0, 1.0);
            float light = texture(lightEllipse_, p).x;

            norm += light;
            result += curOcclusion*light;
        }
    }

    //result /= float((2*radius_+1) * (2*radius_+1));
    result /= norm;
    result += alpha;
    return result;
}

/***
 * The main method.
 ***/
void main() {

    vec3 prevCoord = gl_TexCoord[1].xyz - (gl_ModelViewMatrix * vec4(0.0, 0.0, -1.0, 0.0)).xyz * dPlaneIncr_;
    vec4 prevPoint = (gl_ModelViewProjectionMatrix * volumeStruct_.physicalToWorldMatrix_ * vec4(prevCoord, 1.0));
    prevPoint.xyz /= prevPoint.w;
    prevPoint.xy = 0.5*prevPoint.xy + 0.5;
    float occlusion = occlude(prevPoint.xy*screenDim_);

    //float occlusion = occlude(gl_FragCoord.xy);
    //float occlusion = texture(lightEllipse_, gl_FragCoord.xy * screenDimRCP_).x;

    FragData0 = vec4(occlusion);
}
