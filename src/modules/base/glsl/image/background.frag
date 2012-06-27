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

uniform SAMPLER2D_TYPE colorTex0_;
uniform SAMPLER2D_TYPE depthTex0_;
uniform TEXTURE_PARAMETERS textureParameters0_;
uniform SAMPLER2D_TYPE colorTex1_;
uniform SAMPLER2D_TYPE depthTex1_;
uniform TEXTURE_PARAMETERS textureParameters1_;

void main() {
    vec2 p = gl_FragCoord.xy;
    vec4 color0 = textureLookup2Dnormalized(colorTex0_, textureParameters0_, p*screenDimRCP_);
    float depth0 = textureLookup2Dnormalized(depthTex0_, textureParameters0_, p*screenDimRCP_).z;
    vec4 color1 = textureLookup2Dnormalized(colorTex1_, textureParameters1_, p*screenDimRCP_);
    float depth1 = textureLookup2Dnormalized(depthTex1_, textureParameters1_, p*screenDimRCP_).z;

    vec4 fragColor;
    fragColor.rgb = color1.rgb * color1.a + color0.rgb * color0.a * (1.0 - color1.a);
    fragColor.a = color1.a + color0.a * (1.0 - color1.a);
    float fragDepth = min(depth0, depth1);

    FragData0 = fragColor;
    gl_FragDepth = fragDepth;
}
