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

uniform SAMPLER2D_TYPE colorTex_;
uniform SAMPLER2D_TYPE depthTex_;
uniform TEXTURE_PARAMETERS texParams_;
uniform SAMPLER2D_TYPE colorTexMask_;
uniform TEXTURE_PARAMETERS texParamsMask_;

uniform vec4 maskColor_;

void main() {
    vec2 p = gl_FragCoord.xy * screenDimRCP_;
    vec4 color = textureLookup2Dnormalized(colorTex_, texParams_, p);
    float depth = textureLookup2Dnormalized(depthTex_, texParams_, p).z;
    vec4 maskValue = textureLookup2Dnormalized(colorTexMask_, texParamsMask_, p);
    if (length(maskValue - maskColor_) < 0.01) {
        gl_FragColor = vec4(0.0);
        gl_FragDepth = 1.0;
    }
    else {
        gl_FragColor = color;
        gl_FragDepth = depth;
    }
}
