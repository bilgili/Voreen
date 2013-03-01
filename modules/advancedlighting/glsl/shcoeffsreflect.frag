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
//varying out vec4 FragData3;

// declare transfer-func volume, which contains all transferfunc-values for the original volume; also, it's mipmaps
// contain compressed tf-volumes generated in the SHCoeffTrans class by using a special minification filter (see
// documentation there)
uniform sampler3D tfVolume_;

uniform sampler3D shcoeffsI0_;
uniform sampler3D shcoeffsI1_;
uniform sampler3D shcoeffsI2_;
uniform sampler3D shcoeffsI3_;

#ifdef CONSIDER_NORMALS
uniform sampler3D normVolume_;
#endif

// the volSize_ parameter contains the size of the actual volumes that we render into, which can be smaller than
// the original size of the volume (see origVolSize_)
uniform ivec3 volSize_;

// the origVolSize_ parameter contains the size of the volume of the transferfunc-volume (which is also the size of
// the original Volume)
uniform ivec3 origVolSize_;

// the following two textures contain the randomized samples towards which we send rays from the voxels, and the respective
// sh-coefficients for the theta and phi values of each sample
uniform sampler2D lightSamples_;
uniform sampler2D lightCoeffs_;

// since we render slice-wise into the 3d-volumes, we need to know which slice we are currently rendering into, in order to
// calculate the correct texture-coordinate for the lookup in the tf-volume
uniform int curSlice_;

// we also need to know how many samples were precomputed, so we can use a for-loop to look up each sample in lightSamples_
// and lightCoeffs_
uniform int sampleNumSqrt_;

//// if we use SH_BLEEDING, we have to do two rendering passes, since we have to render into 16 targets simultaneously, but current
//// graphic-boards only support 8... when this changes in the future (the OpenGL standard already uses 16 targets), there will be
//// a significant speedup in coefficient generation
//#ifdef SH_BLEEDING
//uniform int pass_;
//#endif

// finally, we need a global renormalization constant, which is 4*pi / number of samples. This stems from Monte-Carlo-Integration
// and the fact that the surface of a unit sphere (which we use in spherical harmonics) is 4*pi (which is the maximal result we can
// get from integration), but individual transfer-values that are to be compressed must not exceed the range 0..1.  Note that in
// the shcalc-class, the possibility of using only a certain area on the unit-sphere for sampling, which would lower the value 4*pi
// to only the extent of that area, but we assume here that all of the sphere is being used for sampling in regards to the transfer-
// coefficients.
float normConst_;

#ifndef M_PIf
#define M_PIf 3.14159265
#endif

const vec4 shId = vec4(3.54491, 0.0200977, -0.00693648, 0.0250663);

float calcGradLength(in vec3 pos) {
    vec3 s1, s2, delta;
    delta = vec3(1.0) / vec3(origVolSize_);

    //s1.x = texture3D(volume_, pos-vec3(delta.x, 0.0, 0.0)).a;
    //s2.x = texture3D(volume_, pos+vec3(delta.x, 0.0, 0.0)).a;
    //s1.y = texture3D(volume_, pos-vec3(0.0, delta.y, 0.0)).a;
    //s2.y = texture3D(volume_, pos+vec3(0.0, delta.y, 0.0)).a;
    //s1.z = texture3D(volume_, pos-vec3(0.0, 0.0, delta.z)).a;
    //s2.z = texture3D(volume_, pos+vec3(0.0, 0.0, delta.z)).a;

    s1.x = texture3D(tfVolume_, pos-vec3(delta.x, 0.0, 0.0)).a;
    s2.x = texture3D(tfVolume_, pos+vec3(delta.x, 0.0, 0.0)).a;
    s1.y = texture3D(tfVolume_, pos-vec3(0.0, delta.y, 0.0)).a;
    s2.y = texture3D(tfVolume_, pos+vec3(0.0, delta.y, 0.0)).a;
    s1.z = texture3D(tfVolume_, pos-vec3(0.0, 0.0, delta.z)).a;
    s2.z = texture3D(tfVolume_, pos+vec3(0.0, 0.0, delta.z)).a;

    //return length(s1-s2);
    return distance(s1, s2);
}

// this implementation offers three possibilities for spherical-harmonics-use, concerning the handling of color-bleeding:  we can either
// have no bleeding at all (so all we get for each voxel are 16 coefficients, which we can fit into a mat4 called result), we can have
// constant bleeding, where the bleeding-color is simply saved as a color into a seperate volume, in addition to the sh-coefficients, or
// we can use separate sh-coefficients for each color in addition to the transfer-sh-coefficients, which results in slight changes of the
// bleeding color with the rotation of the light-function - this is, of course, the most expensive option, since two passes are needed.
// The parameters that are passed are the position of the current voxel within the volume, and one or two arguments in which our results are
// saved and returned.
void monteCarloInt(in vec3 pos, inout mat4 result) {

    // I am not exactly sure how long the initial step size should be, some experimentation is needed
    //vec3 shadRayDelta = sqrt(3.0) / (vec3(origVolSize_));
    vec3 shadRayDelta = 1.0 / (vec3(origVolSize_));

    // TODO this is configured for 4 bands of coeffs. change this to a variable
    // In the main loop, we visit each randomized sample to calculate how much light arrives from that sample to the
    // current voxel (and optionally, what light-color the ray accumulates on its way).
    for(int i = 0; i < sampleNumSqrt_; i++) {
        for(int j = 0; j < sampleNumSqrt_; j++) {
            // calculate the tex-coord of the current sample
            vec2 coord = vec2(i, j) / float(sampleNumSqrt_);
            vec4 sample = texture(lightSamples_, coord);

            // look up the vector that points towards the current sample
            vec3 dir = normalize(2.0*sample.xyz - 1.0);

            //float curInt = dot(2.0*texture(lightCoeffs_, 0.5*coord) - 1.0, shId[0]);
            //if(curInt < 0.0)
                //continue;

            #ifdef CONSIDER_NORMALS
            float dp = dot(texture3D(normVolume_, pos).xyz, dir);
            if(dp <= 0.0)
                continue;
            #endif

            vec3 fac;
            fac = vec3(1.0);

            // the initial increment we use to step through the volume along the ray
            vec3 rayInc  = fac * shadRayDelta * dir;

            // we start one step from the current position to avoid artifacts (we don't want the voxel to bleed on itself)
            //#ifndef CONSIDER_NORMALS
            //vec3 curPos = pos;
            //#else
            vec3 curPos = pos + rayInc;
            //#endif

            // in each step along the ray, we look up the color of the ray-position in the tf-volume
            vec4 rayColor;

            // the level-variable is used to count how many steps we have taken, and to pick the right mip-map level of the
            // tf-volume for the color-lookup
            float level = 0.0;

            // start at an intensity of 1.0, which is reduced depending on the transfer-function-values that the ray has to
            // pass through
            float shadFac = 1.0;

            bool reachedLightsource = false;

            float intensity = 0.0;
            vec4 raySample;
            int counter = 0;

            // start the ray from the voxel to the current sample
            while(true) {
                // if the ray exceeds the bounds of the volume, we are done with it and can add the ray information to the
                // total info we have to collect about this voxel
                if(any(greaterThan(curPos, vec3(1.0))) || any(lessThan(curPos, vec3(0.0)))) {
                    reachedLightsource = true;
                    break;
                }

                counter++;
                raySample = texture3DLod(tfVolume_, curPos, level);

                // reflect!
                if(raySample.a == 1.0) {
                    if(counter <= 1) {
                        reachedLightsource = true;
                        break;
                    }

                    #ifdef SH_ERI
                    vec3 tmpInc = rayInc;
                    curPos -= rayInc;
                    //float forward = -1.0;
                    float forward = 1.0;
                    while(true) {
                        tmpInc /= vec3(2.0);
                        //vec3 tmpPos = shadPos + forward*shadInc;
                        curPos += forward*tmpInc;
                        rayColor = texture3D(tfVolume_, curPos);

                        //shadPos = tmpPos;
                        if(length(tmpInc) < 1.0 / float(volSize_.x))
                            break;

                        if(rayColor.a > 0.95)
                            forward = -1.0;
                        else
                            forward = 1.0;
                    }
                    #endif

                    //mat4 neighbourCoeffs = (2.0*mat4(texture3D(shcoeffsI0_, curPos), texture3D(shcoeffsI1_, curPos),
                                                     //texture3D(shcoeffsI2_, curPos), texture3D(shcoeffsI3_, curPos)) - 1.0);
                    //neighbourCoeffs = matrixCompMult(neighbourCoeffs, 0.33*shId);
                    //neighbourCoeffs
                    //vec4 tmp = neighbourCoeffs[0] + neighbourCoeffs[1] + neighbourCoeffs[2] + neighbourCoeffs[3];
                    //intensity = 0.7*(tmp.x + tmp.y + tmp.z + tmp.w);
                    //intensity = (2.0*texture3D(shcoeffsI0_, curPos).x - 1.0)*(1.0 / sqrt(4.0*M_PIf));
                    break;
                }

                #ifdef SH_ERI
                rayInc *= 2.0;
                level += 1.0;
                #endif
                curPos += rayInc;
            }

            // now we have to look up the 16 geometrical sh-coefficients for this sample.  They depend only on the theta and phi angles.
            // They are saved in a RGBA-2D-Texture, divided into 4 quadrants, so we can access 16 values in one texture.
            if(!reachedLightsource) {

                //vec4 coeffs0 = texture(lightCoeffs_, 0.5*coord                 );
                //vec4 coeffs1 = texture(lightCoeffs_, 0.5*coord + vec2(0.5, 0.0));
                //vec4 coeffs2 = texture(lightCoeffs_, 0.5*coord + vec2(0.0, 0.5));
                //vec4 coeffs3 = texture(lightCoeffs_, 0.5*coord +      0.5      );

                //// remap values into the -1...1 range
                //mat4 coeffs = 2.0*mat4(coeffs0, coeffs1, coeffs2, coeffs3) - 1.0;

                //#ifdef CONSIDER_NORMALS
                //shadFac *= dp;
                //#endif

                // finally, we add the geometrical coefficients for this sample, weighted by the remaining intensity that a ray that
                // passes from this sample to the current voxel still possesses.  The result parameter will, at the end of the two
                // for-loops, contain the result of the Monte-Carlo integration; in this case, the amount of light that can arrive at
                // the voxel, given the current transfer-function.
                //result[0] += coeffs[0] * raySample.r * intensity;
                //result[1] += coeffs[0] * raySample.g * intensity;
                //result[2] += coeffs[0] * raySample.b * intensity;
                //result[0] += coeffs[0] * raySample.r;
                //result[1] += coeffs[0] * raySample.g;
                //result[2] += coeffs[0] * raySample.b;
                vec4 neighbourCoeffs = 2.0*texture3D(shcoeffsI0_, curPos) - 1.0;
                //raySample /= pow(distance(curPos, pos) * float(origVolSize_.x), 2.0);
                //raySample *= 2.0*calcGradLength(curPos);

                #ifdef CONSIDER_NORMALS
                raySample *= dp;
                #endif

                result[0] += neighbourCoeffs * raySample.r;
                result[1] += neighbourCoeffs * raySample.g;
                result[2] += neighbourCoeffs * raySample.b;
            }
        }
    }  // end of for-loops

    // finally, the accumulated coefficients are renormalized, and we are done for this voxel!
    result *= normConst_;
    result = 0.5*result + 0.5;
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

    //// idea: discard 0-transparency voxels right at the beginning
    if(texture3D(shcoeffsI0_, pos).r == 0.5) {
        FragData0 = vec4(0.5);
        FragData1 = vec4(0.5);
        FragData2 = vec4(0.5);
        //FragData3 = vec4(0.5);
        return;
    }

    // our result coefficient-matrix, initialized to 0
    mat4 result = mat4(0.0);

    monteCarloInt(pos, result);

    // write the results to the result volumes
    FragData0 = result[0];
    FragData1 = result[1];
    FragData2 = result[2];
    //FragData3 = result[3];
}

