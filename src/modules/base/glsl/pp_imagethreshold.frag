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
#include "modules/mod_filtering.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;
uniform TEXTURE_PARAMETERS texParams_;

uniform float lowerThreshold_;
uniform float upperThreshold_;
uniform vec4 lowerMaskColor_;
uniform vec4 upperMaskColor_;

void main() {
    vec2 p = gl_FragCoord.xy * screenDimRCP_;
    vec4 fragColor = textureLookup2Dnormalized(shadeTex_, texParams_, p);
    float gray = rgbToGrayScale(fragColor).x;

    if (gray <= lowerThreshold_)
        fragColor = lowerMaskColor_;
    else if (gray > upperThreshold_)
        fragColor = upperMaskColor_;

    gl_FragColor = fragColor;
    gl_FragDepth = textureLookup2Dnormalized(depthTex_, texParams_, p).z;
}
