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
uniform sampler3D volume_;    // volume data set

uniform TransFuncParameters transferFunc_;
uniform TF_SAMPLER_TYPE transferFuncTex_;

uniform sampler2D occlusion_;
uniform TextureParameters occlusionParams_;

vec4 renderSlice(vec2 p) {
    vec3 texCoord = gl_TexCoord[0].xyz;

    if (texCoord.x >= 0.0 && texCoord.x <= 1.0 &&
        texCoord.y >= 0.0 && texCoord.y <= 1.0 &&
        texCoord.z >= 0.0 && texCoord.z <= 1.0) {
        float intensity = getVoxel(volume_, volumeStruct_, texCoord).a;
        vec4 color = applyTF(transferFunc_, transferFuncTex_, intensity);
        float occlusion = textureLookup2Dnormalized(occlusion_, occlusionParams_, p).a;
        float lambda = 1.0 / (1.0 + occlusion - color.a);
        return vec4(color.rgb * color.a * lambda, color.a);
    } else
        return vec4(0.0);
}

/***
 * The main method.
 ***/
void main() {
    vec2 p = gl_FragCoord.xy * screenDimRCP_;
    FragData0 = renderSlice(p);
}
