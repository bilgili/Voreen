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

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;
uniform TEXTURE_PARAMETERS textureParameters_;

uniform vec4 blurChannels;
uniform vec4 nblurChannels;

uniform float sigma_;
int halfKernelDim_;
float[25] gaussKernel_;


// computes the Gauss kernel and returns its norm factor
float computeGaussKernel() {
    // compute kernel
    for (int x=0; x<=halfKernelDim_; x++)
        gaussKernel_[x] = exp(-float(x*x)/(2.0*sigma_*sigma_));
    // compute norm
    float norm = 0.0;
    for (int x=0; x<=halfKernelDim_; x++) {
        for (int y=1; y<=halfKernelDim_; y++)
            norm += gaussKernel_[x]*gaussKernel_[y];
    }
    // so far we have just computed norm for one quarter
    norm = 4.0 * norm + gaussKernel_[0]*gaussKernel_[0];
    return norm;
}

/***
 * Performs a Gaussian filter on the color buffer. And returns the
 * filtered value.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @norm - the normalization factor of the kernel
 ***/
vec4 applyColorGaussFilter(in vec2 fragCoord, in float norm) {
    vec4 result = vec4(0.0);
    for (int x=-halfKernelDim_; x<=halfKernelDim_; x++) {
        int absx = x;
        if (absx < 0) absx = -absx;
        for (int y=-halfKernelDim_; y<=halfKernelDim_; y++) {
            int absy = y;
            if (absy < 0) absy = -absy;
            vec4 curColor = textureLookup2Dscreen(shadeTex_, textureParameters_,
                                            vec2(fragCoord.x+float(x), fragCoord.y+float(y)));
            result +=  curColor * gaussKernel_[absx]*gaussKernel_[absy];
        }
    }
    result /= norm;
    return result;
}

/***
 * The main method.
 ***/
void main() {
    vec2 fragCoord = gl_FragCoord.xy;
    vec4 curColor = textureLookup2Dscreen(shadeTex_, textureParameters_, fragCoord);

    // initialize Gaussian filter
    halfKernelDim_ = int(2.5 * sigma_);
    float norm = computeGaussKernel();
    // apply Gaussian only to selected channels
    gl_FragColor = blurChannels*applyColorGaussFilter(fragCoord, norm) + nblurChannels*curColor;
    gl_FragDepth = textureLookup2Dscreen(depthTex_, textureParameters_, fragCoord).z;
 }
