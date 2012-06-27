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

uniform SAMPLER2D_TYPE mirrorTex_;

uniform int pass_;
uniform float coarseMirror_;
uniform vec2 size_;

/***
 * The main method.
 ***/
void main() {
    if (pass_ == 0) {
        vec4 fragCoord = gl_FragCoord;
        gl_FragColor = textureLookup2D(mirrorTex_, vec2(gl_TexCoord[0])/(coarseMirror_*size_)*screenDim_).rgba;
        gl_FragDepth = fragCoord.z;
    }
    else {
        vec4 fragCoord = gl_FragCoord;
        gl_FragColor = textureLookup2D(mirrorTex_, fragCoord.xy/size_*screenDim_).rgba;
        gl_FragDepth = fragCoord.z;
    }
}
