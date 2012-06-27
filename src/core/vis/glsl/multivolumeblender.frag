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
uniform SAMPLER2D_TYPE layer1_;
uniform SAMPLER2D_TYPE layer2_;
uniform SAMPLER2D_TYPE layer3_;


void main() {

     vec4 fragColor = vec4(0.0);
     vec4 finalColor= vec4(0.0);
    vec2 p = gl_FragCoord.xy;
    vec4 shadeCol0 = textureLookup2D(layer1_, p);
    vec4 shadeCol1 = textureLookup2D(layer2_, p);
    fragColor.rgb = shadeCol1.rgb * shadeCol1.a + shadeCol0.rgb * shadeCol0.a * (1.0 - shadeCol1.a);
    fragColor.a = shadeCol1.a + shadeCol0.a * (1.0 - shadeCol1.a);

    vec4 shadeCol2 = textureLookup2D(layer3_, p);
    finalColor.rgb = shadeCol2.rgb * shadeCol2.a + fragColor.rgb * fragColor.a * (1.0 - shadeCol2.a);
    finalColor.a = shadeCol2.a + fragColor.a * (1.0 - shadeCol2.a);

    gl_FragColor = finalColor;

}
