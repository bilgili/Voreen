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

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;
uniform TEXTURE_PARAMETERS textureParameters_;

uniform bool blurDepth_;
uniform vec4 blurChannels;
uniform vec4 nblurChannels;

uniform vec2 dir_;
uniform float sigma_;
int halfKernelDim_;
float[25] gaussKernel_;


// computes the Gauss kernel and returns its norm factor
float computeGaussKernel() {
    // compute kernel
    for (int i=0; i<=halfKernelDim_; i++)
        gaussKernel_[i] = exp(-float(i*i)/(2.0*sigma_*sigma_));
    // compute norm
    float norm = 0.0;
    for (int i=1; i<=halfKernelDim_; i++)
        norm += gaussKernel_[i];
    // so far we have just computed norm for one half
    norm = 2.0 * norm + gaussKernel_[0];
    return norm;
}

/***
 * Performs a Gaussian filter on the color buffer. And returns the
 * filtered value.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @norm - the normalization factor of the kernel
 * @dir - since separability is exploited, the blur direction needs to be specified
 ***/
vec4 applyColorGaussFilter(in vec2 fragCoord, in float norm, in vec2 dir) {
    vec4 result = vec4(0.0);
    for (int i=-halfKernelDim_; i<=halfKernelDim_; i++) {
        int absi = i;
        if (absi < 0) absi = -absi;
        vec4 curColor = textureLookup2Dscreen(shadeTex_, textureParameters_,
                               fragCoord+(dir*i));
        result +=  curColor * gaussKernel_[absi];
    }
    result /= norm;
    return result;
}

/***
 * Performs a Gaussian filter on the depth buffer. And returns the
 * filtered value.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @norm - the normalization factor of the kernel
 * @dir - since separability is exploited, the blur direction needs to be specified
 ***/
float applyDepthGaussFilter(in vec2 fragCoord, in float norm, in vec2 dir) {
    float result = 0.0;
    for (int i=-halfKernelDim_; i<=halfKernelDim_; i++) {
        int absi = i;
        if (absi < 0) absi = -absi;
        float curColor = textureLookup2Dscreen(depthTex_, textureParameters_,
                               fragCoord+(dir*i)).z;
        result += curColor * gaussKernel_[absi];
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
    gl_FragColor = blurChannels*applyColorGaussFilter(fragCoord, norm, dir_) + nblurChannels*curColor;
    float depth;
    if (blurDepth_) depth = applyDepthGaussFilter(fragCoord, norm, dir_);
    else depth = textureLookup2Dscreen(depthTex_, textureParameters_, fragCoord).z;
    gl_FragDepth = depth;
 }
