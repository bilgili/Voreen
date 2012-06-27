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

#include "modules/vrn_shaderincludes.frag"

// declare volume
uniform sampler3D volume_;                      // volume data set
uniform VOLUME_PARAMETERS volumeParameters_;    // texture lookup parameters for volume_

uniform SAMPLER2D_TYPE previousRes_;
uniform TEXTURE_PARAMETERS previousResParams_;

vec4 renderSlice() {
    vec3 texCoord = gl_TexCoord[0].xyz;

    // texture coordinate adaption for non-uniform volumes
    texCoord -= vec3(0.5);
    texCoord *= volumeParameters_.volumeCubeSizeRCP_*2.0;
    texCoord += vec3(0.5);

    if (texCoord.x >= 0.0 && texCoord.x <= 1.0 &&
        texCoord.y >= 0.0 && texCoord.y <= 1.0 &&
        texCoord.z >= 0.0 && texCoord.z <= 1.0) {
        float intensity = getVoxel(volume_, volumeParameters_, texCoord).a;
        vec4 color = applyTF(transferFunc_, intensity);
        color.rgb *= color.a;
        return color;
    } else
        return vec4(0.0);
}

/***
 * The main method.
 ***/
void main() {

    vec2 p = gl_FragCoord.xy * screenDimRCP_;

    vec4 currentRes = renderSlice();
    gl_FragColor = currentRes;
}
