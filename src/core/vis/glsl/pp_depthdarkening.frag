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
uniform TEXTURE_PARAMETERS texParams_;

uniform float sigma_;
int halfKernelDim_;
float[25] gaussKernel_;

uniform float lambda_;

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
 * Performs a Gaussian filter on the depth buffer. And returns the
 * filtered value.
 *
 * @fragCoord - screen coordinates of the current fragment
 * @norm - the normalization factor of the kernel
 ***/
float applyDepthGaussFilter(in vec2 fragCoord, in float norm) {
    float result = 0.0;
    for (int x=-halfKernelDim_; x<=halfKernelDim_; x++) {
        int absx = x;
        if (absx < 0) absx = -absx;
        for (int y=-halfKernelDim_; y<=halfKernelDim_; y++) {
            int absy = y;
            if (absy < 0) absy = -absy;
            float curDepth = normDepth(textureLookup2Dscreen(depthTex_, texParams_,
                                                       vec2(fragCoord.x+float(x),
                                                            fragCoord.y+float(y))).z);
            result +=  curDepth * gaussKernel_[absx]*gaussKernel_[absy];
        }
    }
    result /= norm;
    return result;
}

/***
 * The main method.
 ***/
void main() {
    vec4 fragCoord = gl_FragCoord;
    vec4 curColor = textureLookup2Dscreen(shadeTex_, texParams_, fragCoord.xy);
    float curDepth = textureLookup2Dscreen(depthTex_, texParams_, fragCoord.xy).z;

    // initialize and apply Gaussian filter
    halfKernelDim_ = int(2.5 * sigma_);
    float norm = computeGaussKernel();
    float deltaD = applyDepthGaussFilter(fragCoord.xy, norm) - normDepth(curDepth);

    // apply depth darkening
    float deltaDNeg = 0.0;
    if (deltaD < 0.0) deltaDNeg = deltaD;
    curColor.rgb += deltaDNeg * lambda_;

    gl_FragColor = curColor;
    gl_FragDepth = curDepth;
}
