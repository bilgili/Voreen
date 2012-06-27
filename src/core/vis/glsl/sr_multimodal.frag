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

uniform sampler3D volumeCT_;
uniform sampler3D volumePET_;
uniform sampler1D transferFuncCT_;
uniform sampler1D transferFuncPET_;

vec4 applyTFCT(float intensity) {
    #ifdef BITDEPTH_12
        return texture1D(transferFuncCT_, intensity * 16.0);
    #else
        return texture1D(transferFuncCT_, intensity);
    #endif
}

void main() {
    // fetch intensity
    vec4 intensityCT = texture3D(volumeCT_, gl_TexCoord[0].xyz);
    vec4 intensityPET = texture3D(volumePET_, gl_TexCoord[0].xyz);

    vec4 mat = applyTFCT(intensityCT.a);

    // FIXME: hack until transfer function is accessible
    if (intensityPET.a > 0.57)
        mat.r += intensityPET.a;

    gl_FragColor = mat;
}
