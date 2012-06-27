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

uniform SAMPLER2D_TYPE shadeTex0_;
uniform TEXTURE_PARAMETERS textureParameters0_;
uniform SAMPLER2D_TYPE shadeTex1_;
uniform SAMPLER2D_TYPE depthTex1_;
uniform TEXTURE_PARAMETERS textureParameters1_;
uniform float threshold_ = 0.2;

void main() {
    vec2 fragCoord = gl_FragCoord.xy;

    // fetch input textures
    vec4 shadeCol0 = textureLookup2Dscreen(shadeTex0_, textureParameters0_, fragCoord);
    vec4 shadeCol1 = textureLookup2Dscreen(shadeTex1_, textureParameters1_, fragCoord);
    float fragDepth = textureLookup2Dscreen(depthTex1_, textureParameters1_, fragCoord).z;

    vec4 back = shadeCol1;
    vec4 front = shadeCol0;

    vec4 backInv = vec4(1.0-back.r, 1.0-back.g, 1.0-back.b, 1.0);
    backInv.r = backInv.r >= 0.5
        ? max(backInv.r, backInv.r + threshold_)
        : min(backInv.r, backInv.r - threshold_);
    backInv.g = backInv.g >= 0.5
        ? max(backInv.g, backInv.g + threshold_)
        : min(backInv.g, backInv.g - threshold_);
    backInv.b = backInv.b >= 0.5
        ? max(backInv.b, backInv.b + threshold_)
        : min(backInv.b, backInv.b - threshold_);
    backInv = mix(back, backInv, front.a);

    gl_FragColor = backInv;
    gl_FragDepth = fragDepth;
}
