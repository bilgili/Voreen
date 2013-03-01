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

const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

// declare volume
uniform VolumeParameters volumeStruct_;
uniform sampler3D volume_;    // volume data set

uniform TransFuncParameters transferFunc_;
uniform TF_SAMPLER_TYPE transferFuncTex_;

uniform sampler2D lightBuf_;
uniform TextureParameters lightBufParameters_;
uniform mat4 lightMat_;
uniform float dPlaneIncr_;
uniform bool secondPass_;

vec4 renderSlice() {
    vec3 texCoord = gl_TexCoord[0].xyz;

    if (inUnitCube(texCoord)) {
        float intensity = getVoxel(volume_, volumeStruct_, texCoord).a;
        vec4 color = applyTF(transferFunc_, transferFuncTex_, intensity);
        color.a = 1.0 - pow(1.0 - color.a, dPlaneIncr_ * SAMPLING_BASE_INTERVAL_RCP);
        color.rgb *= color.a;

        if(!secondPass_) {
            // calculate the pixel location of this voxel on the previously rendered light texture
            vec4 lightPoint = (gl_ProjectionMatrix * lightMat_ * volumeStruct_.physicalToWorldMatrix_ * vec4(gl_TexCoord[1].xyz, 1.0));
            lightPoint.xyz /= lightPoint.w;
            vec2 p = 0.5*lightPoint.xy + 0.5;
            vec4 lightcol = textureLookup2Dnormalized(lightBuf_, lightBufParameters_, p);
            color.rgb *= lightcol.rgb;
        } else
            color.rgb = vec3(0.0);

        return color;
    } else
        return vec4(0.0);
}

/***
 * The main method.
 ***/
void main() {

    vec4 currentRes = renderSlice();
    FragData0 = currentRes;
}
