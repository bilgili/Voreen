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

uniform SAMPLER2D_TYPE volumeShadeTex_;
uniform SAMPLER2D_TYPE volumeDepthTex_;
uniform TEXTURE_PARAMETERS volumeTextureParameters_;

uniform SAMPLER2D_TYPE geometryShadeTex_;
uniform SAMPLER2D_TYPE geometryDepthTex_;
uniform TEXTURE_PARAMETERS geometryTextureParameters_;

void main() {
    vec2 fragCoord = gl_FragCoord.xy * screenDimRCP_;

    vec4 volumeColor = textureLookup2Dnormalized(volumeShadeTex_, volumeTextureParameters_, fragCoord);
    float volumeDepth = textureLookup2Dnormalized(volumeDepthTex_, volumeTextureParameters_, fragCoord).z;

    vec4 geometryColor = textureLookup2Dnormalized(geometryShadeTex_, geometryTextureParameters_, fragCoord);
    float geometryDepth = textureLookup2Dnormalized(geometryDepthTex_, geometryTextureParameters_, fragCoord).z;

    if(volumeDepth > geometryDepth) {
        gl_FragColor.rgb = (volumeColor.rgb * volumeColor.a * (1.0-geometryColor.a)) + geometryColor.rgb;
        gl_FragColor.a = ((1.0 - geometryColor.a) * volumeColor.a) + geometryColor.a;
        gl_FragDepth = geometryDepth;
    }
    else {
        gl_FragColor.rgb = (geometryColor.rgb * geometryColor.a * (1.0-volumeColor.a)) + volumeColor.rgb;
        gl_FragColor.a = ((1.0 - volumeColor.a) * geometryColor.a) + volumeColor.a;
        gl_FragDepth = volumeDepth;
    }
}
