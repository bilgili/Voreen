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

uniform SAMPLER2D_TYPE entryPoints_;      // ray entry points
uniform SAMPLER2D_TYPE entryPointsDepth_; // depth values of entry points
uniform SAMPLER2D_TYPE exitPoints_;          // ray exit points
uniform SAMPLER2D_TYPE jitterTexture_;    // 8 bit random values

uniform float stepLength_;                  // raycasting step length

/**
 * Jitter entry points in ray direction
 */
void main() {
    vec2 p = gl_FragCoord.xy;
    vec4 frontPos = textureLookup2D(entryPoints_, p);
    vec4 backPos = textureLookup2D(exitPoints_, p);

    vec4 fragColor;
    float fragDepth;

    if ( (backPos.a == 0.0) || (frontPos.a == 0.0) ) {
        fragColor = frontPos;
        fragDepth = 1.0;
    } else {
        float rayLength = length(backPos.rgb - frontPos.rgb);
        // do not jitter very short rays
        if (rayLength <= stepLength_) {
            fragColor = frontPos;
            fragDepth = textureLookup2D(entryPointsDepth_, p ).z;
        }
        else {
            vec3 dir = (backPos.rgb - frontPos.rgb)/rayLength;
            float jitterValue = textureLookup2D(jitterTexture_, p).x;
            vec3 frontPosNew = frontPos.rgb + (jitterValue*stepLength_)*dir;
            // save jitter value in alpha channel (for inverting jittering later if necessary)
            fragColor = vec4(frontPosNew, 1.0 - jitterValue*stepLength_);
            fragDepth = textureLookup2D(entryPointsDepth_, p ).z;
        }
    }

    gl_FragColor = fragColor;
    gl_FragDepth = fragDepth;

}
