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

#include "modules/mod_sampler2d.frag"
#include "modules/mod_depth.frag"

uniform SAMPLER2D_TYPE entryParams_;
uniform SAMPLER2D_TYPE entryParamsDepth_;
uniform TEXTURE_PARAMETERS entryInfo_;
uniform SAMPLER2D_TYPE exitParams_;
uniform SAMPLER2D_TYPE exitParamsDepth_;
uniform TEXTURE_PARAMETERS exitInfo_;
uniform SAMPLER2D_TYPE geometryTex_;
uniform SAMPLER2D_TYPE geometryTexDepth_;
uniform TEXTURE_PARAMETERS geomInfo_;

uniform bool entry_;
uniform float near_;
uniform float far_;
uniform mat4 viewMatrixInverse_;
uniform mat4 projectionMatrixInverse_;
uniform bool useFloatTarget_;
uniform vec3 volumeSize_;

void main() {
    vec2 p = gl_FragCoord.xy * screenDimRCP_;

    vec4 entryCol = textureLookup2Dnormalized(entryParams_, entryInfo_, p);
    float entryDepth = textureLookup2Dnormalized(entryParamsDepth_, entryInfo_, p).z;

    vec4 exitCol = textureLookup2Dnormalized(exitParams_, exitInfo_, p);
    float exitDepth = textureLookup2Dnormalized(exitParamsDepth_, exitInfo_, p).z;

    float geomDepth = textureLookup2Dnormalized(geometryTexDepth_, geomInfo_, p).z;

    if (entry_) {
        if (geomDepth <= entryDepth) {
            gl_FragData[0] = exitCol;
            gl_FragDepth = geomDepth;
        } else {
            gl_FragData[0] = entryCol;
            gl_FragDepth = entryDepth;
        }
    } else {
        if (geomDepth <= entryDepth) {
            gl_FragData[0] = exitCol;
            gl_FragDepth = exitDepth;
        } else if (geomDepth > entryDepth && geomDepth <= exitDepth) {

            // use NDCs from -1 to 1
            vec4 wGeom = vec4(2.0*p - 1.0, 2.0*geomDepth - 1.0, 1.0);
            vec4 wEntry = vec4(2.0*p - 1.0, 2.0*entryDepth- 1.0, 1.0);
            vec4 wExit = vec4(2.0*p - 1.0, 2.0*exitDepth- 1.0, 1.0);

            // reverse perspective division by w (which is equal to the camera-space z)
            float origZG = 2.0*far_*near_ / ((far_ + near_) - wGeom.z*(far_ - near_));
            float origZE = 2.0*far_*near_ / ((far_ + near_) - wEntry.z*(far_ - near_));
            float origZX = 2.0*far_*near_ / ((far_ + near_) - wExit.z*(far_ - near_));
            wGeom *= origZG;
            wEntry *= origZE;
            wExit *= origZX;

            // unproject and reverse camera-transform
            wGeom = viewMatrixInverse_ * (projectionMatrixInverse_ * wGeom);
            wEntry= viewMatrixInverse_ * (projectionMatrixInverse_ * wEntry);
            wExit = viewMatrixInverse_ * (projectionMatrixInverse_ * wExit);

            // We can now obtain the world coordinate.
            vec4 result = vec4(wEntry.xyz + length(wGeom.xyz - wEntry.xyz) * normalize(wExit.xyz - wEntry.xyz), 1.0);

            // Finally, we have to account for non-cube datasets
            result.xyz *= 2.0 / volumeSize_;

            if(!useFloatTarget_)
                gl_FragData[0] = vec4(0.5*result.xyz + 0.5, 1.0);
            else
                gl_FragData[0] = vec4(result.xyz, 1.0);

            gl_FragDepth = geomDepth;

        } else {
            gl_FragData[0] = exitCol;
            gl_FragDepth = exitDepth;
        }
    }
}
