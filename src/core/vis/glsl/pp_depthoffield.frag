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
#include "modules/mod_normdepth.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;

uniform float depthThreshold_;

/***
 * Performs an image based blurring in all four channels.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @delta - sepcifies the distance to the neighboor texels to be fetched
 ***/
vec4 blur(in vec2 fragCoord, in float delta) {
    vec4 center = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y) ).rgba;
    vec4 west = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y) ).rgba;
    vec4 northwest = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y-delta) ).rgba;
    vec4 north = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y-delta) ).rgba;
    vec4 northeast = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y-delta) ).rgba;
    vec4 east = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y) ).rgba;
    vec4 southeast = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y+delta) ).rgba;
    vec4 south = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y+delta) ).rgba;
    vec4 southwest = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y+delta) ).rgba;

    return (3.0*center + west + northwest + north + northeast + east + southeast + south + southwest)/11.0;
}

/***
 * The main method.
 ***/
void main() {

    vec4 fragCoord = gl_FragCoord;

    // read and normalize depth value
    float depth = textureLookup2D(depthTex_, fragCoord.xy).z;
    float depthNorm = normDepth(depth);

    vec4 result = textureLookup2D(shadeTex_, fragCoord.xy);
    if (depthNorm > depthThreshold_) result = vec4(blur(fragCoord.xy, depthNorm*5.0).rgb, 1.0);

    gl_FragColor = result;
    gl_FragDepth = depth;
}
