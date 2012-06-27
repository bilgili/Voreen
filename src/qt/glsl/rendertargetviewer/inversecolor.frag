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

#include "../modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE backTex_;
uniform SAMPLER2D_TYPE frontTex_;
uniform TEXTURE_PARAMETERS texParameters_;
uniform float threshold_;

/**
 * Adds inverse of back texture to front texture modulated by red component. Used for rendering
 * text that stays visible independent of the background.
 */
void main() {
    vec2 coord = gl_TexCoord[0].xy;
    vec4 back = textureLookup2Dnormalized(backTex_, texParameters_, coord);
    vec4 front = textureLookup2Dnormalized(frontTex_, texParameters_, coord);

    vec4 backInv = vec4(1.0 - back.rgb, 1.0);
    backInv.r += (backInv.r >= 0.5 ? 1.0 : -1.0) * threshold_;
    backInv.g += (backInv.g >= 0.5 ? 1.0 : -1.0) * threshold_;
    backInv.b += (backInv.b >= 0.5 ? 1.0 : -1.0) * threshold_;

    gl_FragColor = mix(back, backInv, front.r); // do not use alpha component, as FTGL does not
                                                // generate correct transparency
}
