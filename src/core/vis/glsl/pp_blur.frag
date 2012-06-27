/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

uniform float delta_;
uniform vec4 blurChannels;
uniform vec4 nblurChannels;

/***
 * Performs an image based blurring in the specified channels.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @blurChannels - specifies the channels to be blurred
 * @nblurChannels - specifies the channels not to be blurred
 * @delta - specifies the distance to the neighboor texels to be fetched
 ***/
vec4 blur(in vec2 fragCoord, in vec4 blurChannels, in vec4 nblurChannels, in float delta) {
    vec4 center = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y) ).rgba;
    vec4 west = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y) ).rgba;
    vec4 northwest = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y-delta) ).rgba;
    vec4 north = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y-delta) ).rgba;
    vec4 northeast = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y-delta) ).rgba;
    vec4 east = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y) ).rgba;
    vec4 southeast = textureLookup2D(shadeTex_, vec2(fragCoord.x+delta, fragCoord.y+delta) ).rgba;
    vec4 south = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y+delta) ).rgba;
    vec4 southwest = textureLookup2D(shadeTex_, vec2(fragCoord.x-delta, fragCoord.y+delta) ).rgba;
    
    return nblurChannels*center + blurChannels*(3.0*center + west + northwest + north + northeast + east + southeast + south + southwest)/11.0;
}


/***
 * The main method.
 ***/
void main() {
    vec4 fragCoord = gl_FragCoord;
    gl_FragColor = blur(fragCoord.xy, blurChannels, nblurChannels, delta_);
    
    gl_FragDepth = textureLookup2D(depthTex_, fragCoord.xy ).z;
    
 }
