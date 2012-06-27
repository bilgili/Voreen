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

#include "modules/mod_sampler2d.frag"
#include "modules/mod_sampler3d.frag"        // contains struct VOLUME_PARAMETERS

uniform SAMPLER2D_TYPE entryTex_;
uniform SAMPLER2D_TYPE entryTexDepth_;
uniform SAMPLER2D_TYPE exitTex_;

uniform VOLUME_PARAMETERS volumeParameters_; // additional information about the volume the eep are generated for

/**
 * Calculates missing entry points (clipped by near plane) by back-projecting the
 * fragment to the proxy-geometry's object coordinates.
 **/
void main() {

    vec2 fragCoord = gl_FragCoord.xy;
    vec4 exitCol = textureLookup2D(exitTex_, fragCoord);
    vec4 entryCol = textureLookup2D(entryTex_, fragCoord);

    if (exitCol != vec4(0.0)) {

        if (entryCol != vec4(0.0)) {
            // entry point is okay
            gl_FragColor = entryCol;
            gl_FragDepth = textureLookup2D(entryTexDepth_, fragCoord).z;
        }
        else {
            // entry point is 0 because of near plane clipping: fill holes

            // I don't know why, but NDC z-value -20.00 seems to be more correct than -1.0
            // However, this only applies to coarseness mode, without coarseness depth=-1.0 is fine

            // viewport coordinates -> NDC
            vec4 pos = vec4((gl_FragCoord.xy * screenDimRCP_) * 2.0 - vec2(1.0), -20.0, 1.0);

            // NDC -> object coordinates
            pos = gl_ModelViewMatrixInverse * gl_ProjectionMatrixInverse * pos;
            pos /= pos.w;

            // object coordinates -> texture coordinates
            vec3 eep = (pos.xyz * volumeParameters_.volumeCubeSizeRCP_) + 0.5;
            // write new entry point
            gl_FragColor = vec4(eep, 1.0);
            gl_FragDepth = 0.0;
        }
    }
    else {
        // exit point is 0
        discard;
    }
}
