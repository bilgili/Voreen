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
#include "modules/mod_normdepth.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;

uniform float lambda_;

/***
 * Performs a low-pass filter on the depth buffer. And returns the
 * difference between the lowpass filtered and the original depth
 * value.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @kernelSize - should be odd and defines the size of the filter kernel (kernelSize x kernelSize)
 ***/
float calcDeltaD(in vec2 fragCoord, in int kernelSize) {
    float centerDepth = normDepth(textureLookup2D(depthTex_, fragCoord.xy).z);

    float regionDiff = 0.0;
    int halfKernel = int(floor(float(kernelSize)/2.0));
    for (int x=-halfKernel;x<halfKernel;x++) {
        for (int y=-halfKernel;y<halfKernel;y++) {
            float curDepth = normDepth(textureLookup2D(depthTex_, vec2(fragCoord.x+float(x), fragCoord.y+float(y)) ).z);
            if (curDepth < centerDepth) {
                regionDiff += centerDepth-curDepth;
            }
        }
    }
    if (centerDepth >= 0.9) regionDiff = 0.0;
    return (regionDiff/(pow(float(kernelSize),2.0)-1.0));
}

/***
 * The main method.
 ***/
void main() {

    vec4 fragCoord = gl_FragCoord;
    vec4 shadeCol = textureLookup2D(shadeTex_, fragCoord.xy );

    float deltaD = calcDeltaD(fragCoord.xy, 15);
    // apply depth darkening
    gl_FragColor = shadeCol*vec4(vec3(1.0-deltaD*lambda_),1.0);
    gl_FragDepth = textureLookup2D(depthTex_, fragCoord.xy).z;
}
