/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#line 1

varying out vec4 FragData0;
varying out vec4 FragData1;
varying out vec4 FragData2;
varying out vec4 FragData3;

#if defined(SH_BLEEDING) || defined(SH_BLEED_SUB) || defined(SH_SUBSURFACE)
varying out vec4 FragData4;
varying out vec4 FragData5;
varying out vec4 FragData6;
#endif

// the volSize_ parameter contains the size of the actual volumes that we render into, which can be smaller than
// the original size of the volume (see origVolSize_)
uniform ivec3 volSize_;
uniform ivec3 origVolSize_;

uniform float scale_;

// the following two textures contain the randomized samples towards which we send rays from the voxels, and the respective
// sh-function-results for the theta and phi values of each sample
uniform sampler2D lightSamples_;
uniform sampler2D lightCoeffs_;

uniform int sampleNumSqrt_;

// since we render slice-wise into the 3d-volumes, we need to know which slice we are currently rendering into, in order to
// calculate the correct texture-coordinate for the lookup in the tf-volume
uniform int curSlice_;

#ifdef SH_SUBSURFACE
uniform sampler3D shcoeffsI0_;
uniform sampler3D shcoeffsI1_;
uniform sampler3D shcoeffsI2_;
#endif

#ifdef SH_BLEED_SUB
uniform sampler3D shcoeffsR0_;
uniform sampler3D shcoeffsR1_;
uniform sampler3D shcoeffsR2_;
uniform sampler3D shcoeffsG0_;
uniform sampler3D shcoeffsG1_;
uniform sampler3D shcoeffsG2_;
uniform sampler3D shcoeffsB0_;
uniform sampler3D shcoeffsB1_;
uniform sampler3D shcoeffsB2_;
#endif

//uniform sampler3D shcoeffsI3_;
//uniform sampler3D shcoeffsR1_;
//uniform sampler3D tfVolume_;
//uniform sampler3D volume_;

float normConst_;

#ifndef M_PIf
#define M_PIf 3.14159265
#endif

//marble
#define sigma_srR 2.19
#define sigma_srG 2.62
#define sigma_srB 3.00
#define sigma_aR 0.0021
#define sigma_aG 0.0041
#define sigma_aB 0.0071
#define fres_drR 0.83
#define fres_drG 0.79
#define fres_drB 0.75

//skin 1
//#define sigma_srR 0.74
//#define sigma_srG 0.88
//#define sigma_srB 1.01
//#define sigma_aR 0.032
//#define sigma_aG 0.17
//#define sigma_aB 0.48
//#define fres_drR 0.44
//#define fres_drG 0.22
//#define fres_drB 0.13

 ////skin 2
//#define sigma_srR 1.09
//#define sigma_srG 1.59
//#define sigma_srB 1.79
//#define sigma_aR 0.013
//#define sigma_aG 0.07
//#define sigma_aB 0.145
//#define fres_drR 0.63
//#define fres_drG 0.44
//#define fres_drB 0.34

// chicken1
//#define sigma_srR 0.15
//#define sigma_srG 0.21
//#define sigma_srB 0.38
//#define sigma_aR 0.015
//#define sigma_aG 0.077
//#define sigma_aB 0.19
//#define fres_drR 0.31
//#define fres_drG 0.15
//#define fres_drB 0.10

//// chicken2
//#define sigma_srR 0.19
//#define sigma_srG 0.25
//#define sigma_srB 0.32
//#define sigma_aR 0.018
//#define sigma_aG 0.088
//#define sigma_aB 0.20
//#define fres_drR 0.32
//#define fres_drG 0.16
//#define fres_drB 0.10

// wholemilk
//#define sigma_srR 2.55
//#define sigma_srG 3.21
//#define sigma_srB 3.77
//#define sigma_aR 0.0011
//#define sigma_aG 0.0024
//#define sigma_aB 0.014
//#define fres_drR 0.91
//#define fres_drG 0.88
//#define fres_drB 0.76

// ketchup
//#define sigma_srR 0.18
//#define sigma_srG 0.07
//#define sigma_srB 0.03
//#define sigma_aR 0.061
//#define sigma_aG 0.97
//#define sigma_aB 1.45
//#define fres_drR  0.16
//#define fres_drG 0.01
//#define fres_drB 0.0

//const float R0 = pow(1.0 - eta, 2.0) / pow(1.0 + eta, 2.0);
//const vec3 eta = vec3(1.5);
//const vec3 fres_dr = -1.44/(eta*eta) + 0.710/eta + 0.668 + 0.0636*eta;

const vec3 fres_dr = vec3(fres_drR, fres_drG, fres_drB);
//const vec3 fres_dr = vec3(1.0 - fres_drR, 1.0 - fres_drG, 1.0 - fres_drB);
const vec3 A = (1.0 + fres_dr) / (1.0 - fres_dr);

const vec3 sigma_a = vec3(sigma_aR, sigma_aG, sigma_aB);
//const vec3 sigma_a = vec3(sigma_aR);
const vec3 sigma_sr = vec3(sigma_srR, sigma_srG, sigma_srB);
//const vec3 sigma_sr = vec3(sigma_srR);
const vec3 sigma_tr = sigma_a + sigma_sr;

const vec3 alpha_r = (1.0 / (4.0*M_PIf*M_PIf))*(sigma_sr / sigma_tr);
const vec3 sigma_e = sqrt(3.0*sigma_a*sigma_tr);
const vec3 l_u = 1.0 / sigma_tr;
const vec3 z_v = l_u*(1.0+(4.0/3.0)*A);

void accumulateSHCoeffs(in vec3 pos, inout mat4 result0, inout mat4 result1) {

    float millimeterPerVoxel = 2.0 * scale_ / float(max(origVolSize_.x, max(origVolSize_.y, origVolSize_.z)));

    //int rad = 16;
    //int rad = 12;
    const int rad = 8;
    //const int rad = 4;
    //int rad = 2;
    //int rad = 1;

    float level;

    for(int j1 = -rad; j1 <= rad; j1++) {
        for(int j2 = -rad; j2 <= rad; j2++) {
            for(int j3 = -rad; j3 <= rad; j3++) {

                vec3 offset = vec3(j1, j2, j3);
                level = length(offset) - 1.0;
                //level = 0.0;

                offset /= vec3(volSize_);
                //offset = normalize(offset);
                //offset /= float(max(volSize_.x, max(volSize_.y, volSize_.z)));

                //offset *= 2.0;
                offset *= pow(2.0, level);
                //offset *= pow(2.0, level+1.0) - 1.0;

                vec3 curPos = pos + offset;
                #ifdef SH_SUBSURFACE
                vec4 firstCoeffs = texture3DLod(shcoeffsI0_, curPos, level);
                if(firstCoeffs.x == 0.5 || length(offset) == 0.0)
                    continue;
                #else
                vec4 firstCoeffsR = texture3DLod(shcoeffsR0_, curPos, level);
                vec4 firstCoeffsG = texture3DLod(shcoeffsG0_, curPos, level);
                vec4 firstCoeffsB = texture3DLod(shcoeffsB0_, curPos, level);
                if((firstCoeffsR.x == 0.5 && firstCoeffsG.x == 0.5 && firstCoeffsB.x == 0.5) || length(offset) == 0.0)
                    continue;
                #endif


                #ifdef SH_SUBSURFACE
                mat3x4 neighbourCoeffs = (2.0*mat3x4(firstCoeffs, texture3DLod(shcoeffsI1_, curPos, level), texture3DLod(shcoeffsI2_, curPos, level)) - 1.0);
                #else
                mat3x4 neighbourCoeffsR = (2.0*mat3x4(firstCoeffsR, texture3DLod(shcoeffsR1_, curPos, level), texture3DLod(shcoeffsR2_, curPos, level)) - 1.0);
                mat3x4 neighbourCoeffsG = (2.0*mat3x4(firstCoeffsG, texture3DLod(shcoeffsG1_, curPos, level), texture3DLod(shcoeffsG2_, curPos, level)) - 1.0);
                mat3x4 neighbourCoeffsB = (2.0*mat3x4(firstCoeffsB, texture3DLod(shcoeffsB1_, curPos, level), texture3DLod(shcoeffsB2_, curPos, level)) - 1.0);
                #endif
                //mat4 res = matrixCompMult(neighbourCoeffs, dirSHValues);
                //vec4 tmp = res[0] + res[1] + res[2] + res[3];
                //float irrad = tmp.x + tmp.y + tmp.z + tmp.w;
                //float irrad = dot(neighbourCoeffs, dirSHValues);

                vec3 tmp = vec3(millimeterPerVoxel*millimeterPerVoxel*length(offset)*length(offset));
                vec3 d_r = sqrt(tmp + l_u*l_u);
                vec3 d_v = sqrt(tmp + z_v*z_v);

                vec3 R_d = alpha_r*(l_u*(sigma_e + 1.0/d_r)*exp(-sigma_e*d_r)/(d_r*d_r) +
                                    //z_v*(sigma_e + 1.0/d_v)*exp(-sigma_e*d_v)/(d_v*d_v))*(1.0 - fres_dr);
                                    z_v*(sigma_e + 1.0/d_v)*exp(-sigma_e*d_v)/(d_v*d_v))*fres_dr;
                                    //z_v*(sigma_e + 1.0/d_v)*exp(-sigma_e*d_v)/(d_v*d_v));

                R_d /= millimeterPerVoxel;
                //R_d /= 3.0;
                //level += 1.0;
                //R_d *= float(level*level*level);
                //R_d *= level*level;
                //R_d *= level;
                //R_d *= pow(millimeterPerVoxel, 3.0);
                //R_d /= millimeterPerVoxel*millimeterPerVoxel*millimeterPerVoxel;
                //R_d /= millimeterPerVoxel*millimeterPerVoxel;
                //R_d = normalize(R_d);
                //R_d *= (1.0 / (pow(2.0*float(rad)+1.0, 3.0) - 1.0));

                #ifdef SH_SUBSURFACE
                result0[0]     += neighbourCoeffs[0]     * R_d.r;
                result0[1]     += neighbourCoeffs[1]     * R_d.r;
                result0[2].x   += neighbourCoeffs[2].x   * R_d.r;

                result0[2].yzw += neighbourCoeffs[0].xyz * R_d.g;
                result0[3].x   += neighbourCoeffs[0].w   * R_d.g;
                result0[3].yzw += neighbourCoeffs[1].xyz * R_d.g;
                result1[0].x   += neighbourCoeffs[1].w   * R_d.g;
                result1[0].y   += neighbourCoeffs[2].x   * R_d.g;

                result1[0].zw  += neighbourCoeffs[0].xy  * R_d.b;
                result1[1].xy  += neighbourCoeffs[0].zw  * R_d.b;
                result1[1].zw  += neighbourCoeffs[1].xy  * R_d.b;
                result1[2].xy  += neighbourCoeffs[1].zw  * R_d.b;
                result1[2].z   += neighbourCoeffs[2].x   * R_d.b;
                #else
                result0[0]     += neighbourCoeffsR[0]     * R_d.r;
                result0[1]     += neighbourCoeffsR[1]     * R_d.r;
                result0[2].x   += neighbourCoeffsR[2].x   * R_d.r;

                result0[2].yzw += neighbourCoeffsG[0].xyz * R_d.g;
                result0[3].x   += neighbourCoeffsG[0].w   * R_d.g;
                result0[3].yzw += neighbourCoeffsG[1].xyz * R_d.g;
                result1[0].x   += neighbourCoeffsG[1].w   * R_d.g;
                result1[0].y   += neighbourCoeffsG[2].x   * R_d.g;

                result1[0].zw  += neighbourCoeffsB[0].xy  * R_d.b;
                result1[1].xy  += neighbourCoeffsB[0].zw  * R_d.b;
                result1[1].zw  += neighbourCoeffsB[1].xy  * R_d.b;
                result1[2].xy  += neighbourCoeffsB[1].zw  * R_d.b;
                result1[2].z   += neighbourCoeffsB[2].x   * R_d.b;
                #endif
            }
        }
    }

    // finally, the accumulated coefficients are renormalized, and we are done for this voxel!
    result0 = 0.5*result0 + 0.5;
    result1 = 0.5*result1 + 0.5;
}

/***
 * The main method.
 ***/
void main() {

    // see comment at declaration
    normConst_ = 4.0*M_PIf / float(sampleNumSqrt_*sampleNumSqrt_);

    // the 3d-texture-coordinate is composed of the screen-coordinate for x and y, and the slice-number for z.  To remap it
    // to 0..1 range, we divide this by the size of the volume we render into.
    vec3 pos = vec3(vec2(gl_FragCoord.xy), float(curSlice_)) / vec3(volSize_);
    pos.z += 0.5 / float(volSize_.z);

    #ifdef SH_SUBSURFACE
    if(texture3D(shcoeffsI0_, pos).r == 0.5) {
    #else
    if(texture3D(shcoeffsR0_, pos).r == 0.5 && texture3D(shcoeffsG0_, pos).r == 0.5 && texture3D(shcoeffsB0_, pos).r == 0.5) {
    #endif
        FragData0 = vec4(0.5);
        FragData1 = vec4(0.5);
        FragData2 = vec4(0.5);
        FragData3 = vec4(0.5);
        FragData4 = vec4(0.5);
        FragData5 = vec4(0.5);
        FragData6 = vec4(0.5);
    }
    else {
        // our result coefficient-matrix, initialized to 0
        mat4 result0 = mat4(0.0);
        mat4 result1 = mat4(0.0);

        accumulateSHCoeffs(pos, result0, result1);

        // write the results to the result volumes
        FragData0 = result0[0];
        FragData1 = result0[1];
        FragData2 = result0[2];
        FragData3 = result0[3];
        FragData4 = result1[0];
        FragData5 = result1[1];
        FragData6 = vec4(result1[2].xyz, 0.0);
    }
}

