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

/*
    pre-integration stuff
*/
#ifdef USE_PRE_INTEGRATION
#version 110

#line 7

uniform float sliceDistance_;
varying vec3 sb_;

#endif // USE_PRE_INTEGRATION

/*
    other vars
*/

varying vec3 v_;

void main() {
    gl_FrontColor = gl_Color;
    v_ = (gl_ModelViewMatrix * gl_Vertex).xyz;
    gl_TexCoord[0] = gl_TextureMatrix[VOL_TEX] * gl_MultiTexCoord0;

    /*
        compute sb_ for pre-integration
    */
#ifdef USE_PRE_INTEGRATION
    vec4 vPos = vec4(0, 0, 0, 1);
    vPos = gl_ModelViewMatrixInverse * vPos;
    vec4 vDir = vec4(0, 0, -1, 1);
    vDir = normalize(gl_ModelViewMatrixInverse * vDir);

    vec4 eyeToVert = normalize(gl_Vertex - vPos);
    vec4 sb = gl_Vertex - eyeToVert * (sliceDistance_ / dot(vDir, eyeToVert));
    sb_ = (gl_TextureMatrix[VOL_TEX] * sb).xyz;
#endif // USE_PRE_INTEGRATION

    /*
        set out vertex
    */
    gl_Position = ftransform();
}
