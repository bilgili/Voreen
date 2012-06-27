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

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;
uniform TEXTURE_PARAMETERS texParams_;

uniform float threshold_;

/***
 * Performs an image based thresholding on the red channel.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @delta - sepcifies the distance to the neighboor texels to be fetched
 ***/
bool threshold(in vec2 fragCoord, in float delta) {
    vec3 N = textureLookup2Dscreen(shadeTex_, texParams_, fragCoord+vec2(0.0,-delta)).rgb;
    vec3 NE = textureLookup2Dscreen(shadeTex_, texParams_, fragCoord+vec2(delta,-delta)).rgb;
    vec3 E = textureLookup2Dscreen(shadeTex_, texParams_, fragCoord+vec2(delta, 0.0)).rgb;
    vec3 SE = textureLookup2Dscreen(shadeTex_, texParams_, fragCoord+vec2(delta, delta)).rgb;
    vec3 S = textureLookup2Dscreen(shadeTex_, texParams_, fragCoord+vec2(0.0, delta)).rgb;
    vec3 SW = textureLookup2Dscreen(shadeTex_, texParams_, fragCoord+vec2(-delta, delta)).rgb;
    vec3 W = textureLookup2Dscreen(shadeTex_, texParams_, fragCoord+vec2(-delta, 0.0)).rgb;
    vec3 NW = textureLookup2Dscreen(shadeTex_, texParams_, fragCoord+vec2(-delta,-delta)).rgb;

    if (length(N)+length(NE)+length(E)+length(SE)+length(S)+length(SW)+length(W)+length(NW) >= threshold_)
        return true;
    else
        return false;
}

/***
 * The main method.
 ***/
void main() {
    vec2 fragCoord = gl_FragCoord.xy;
    vec4 resultColor = vec4(0.0);
    float resultDepth = 1.0;
    if (threshold(fragCoord, 1.0)) {
        resultColor = textureLookup2Dscreen(shadeTex_, texParams_, fragCoord);
        resultDepth = textureLookup2Dscreen(depthTex_, texParams_, fragCoord).z;
    }
    gl_FragColor = resultColor;
    gl_FragDepth = resultDepth;
}
