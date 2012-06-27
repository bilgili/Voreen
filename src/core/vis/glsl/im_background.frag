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

uniform SAMPLER2D_TYPE shadeTex0_;
uniform SAMPLER2D_TYPE depthTex0_;
uniform TEXTURE_PARAMETERS textureParameters0_;
uniform SAMPLER2D_TYPE shadeTex1_;
uniform SAMPLER2D_TYPE depthTex1_;
uniform TEXTURE_PARAMETERS textureParameters1_;

void main() {
    vec2 p = gl_FragCoord.xy;
    vec4 shadeCol0 = textureLookup2Dnormalized(shadeTex0_, textureParameters0_, p*screenDimRCP_);
    float depth0 = textureLookup2Dnormalized(depthTex0_, textureParameters0_, p*screenDimRCP_).z;
    vec4 shadeCol1 = textureLookup2Dnormalized(shadeTex1_, textureParameters1_, p*screenDimRCP_);
    float depth1 = textureLookup2Dnormalized(depthTex1_, textureParameters1_, p*screenDimRCP_).z;

    vec4 fragColor;
    fragColor.rgb = shadeCol1.rgb * shadeCol1.a + shadeCol0.rgb * shadeCol0.a * (1.0 - shadeCol1.a);
    fragColor.a = 1.0;
    float fragDepth = min(depth0, depth1);

    gl_FragColor = fragColor;
    gl_FragDepth = fragDepth;
}
