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

#if defined(SH_BLEEDING) || defined(SH_BLEED_SUB)
varying out vec4 FragData4;
varying out vec4 FragData5;
varying out vec4 FragData6;
varying out vec4 FragData7;
#endif

// declare transfer-func volume, which contains all transferfunc-values for the original volume; also, it's mipmaps
// contain compressed tf-volumes generated in the SHCoeffTrans class by using a special minification filter (see
// documentation there)
//uniform VolumeParameters tfVolumeParameters_;
uniform sampler3D tfVolume_;

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
//uniform float curSlice_;

// we also need to know how many samples were precomputed, so we can use a for-loop to look up each sample in lightSamples_
// and lightCoeffs_
uniform int sampleNumSqrt_;

// for future use:  the number of coefficients we want to project; at the moment, its a constant 16
//uniform int numCoeffs_;

// if we use SH_BLEEDING, we have to do two rendering passes, since we have to render into 16 targets simultaneously, but current
// graphic-boards only support 8... when this changes in the future (the OpenGL standard already uses 16 targets), there will be
// a significant speedup in coefficient generation
#if defined(SH_BLEEDING) || defined(SH_BLEED_SUB)
uniform int pass_;
#endif

#ifdef CONSIDER_NORMALS
uniform sampler3D normVolume_;
#endif

#ifdef SH_SUBSURFACE
//const float eta = 1.3;
const float eta = 1.5;
//const float R0 = pow(1.0 - eta, 2.0) / pow(1.0 + eta, 2.0);
#endif

// finally, we need a global renormalization constant, which is 4*pi / number of samples. This stems from Monte-Carlo-Integration
// and the fact that the surface of a unit sphere (which we use in spherical harmonics) is 4*pi (which is the maximal result we can
// get from integration), but individual transfer-values that are to be compressed must not exceed the range 0..1.
float normConst_;

//vec3 calcNormal(in vec3 pos);

// this implementation offers three possibilities for spherical-harmonics-use, concerning the handling of color-bleeding:  we can either
// have no bleeding at all (so all we get for each voxel are 16 coefficients, which we can fit into a mat4 called result), we can have
// constant bleeding, where the bleeding-color is simply saved as a color into a seperate volume, in addition to the sh-coefficients, or
// we can use separate sh-coefficients for each color in addition to the transfer-sh-coefficients, which results in slight changes of the
// bleeding color with the rotation of the light-function - this is, of course, the most expensive option, since two passes are needed.
// The parameters that are passed are the position of the current voxel within the volume, and one or two arguments in which our results are
// saved and returned.
#if defined(SH_SHADOWED) || defined(SH_REFLECT) || defined(SH_SUBSURFACE)
void monteCarloInt(in vec3 pos, inout mat4 result) {
#elif defined(SH_BLEEDING) || defined(SH_BLEED_SUB)
void monteCarloInt(in vec3 pos, inout mat4 result0, inout mat4 result1) {
#endif

    // I am not exactly sure how long the initial step size should be, some experimentation is needed
    float maxDim = float(max(origVolSize_.x, max(origVolSize_.y, origVolSize_.z)));
    vec3 cs = vec3(origVolSize_) / maxDim;
    vec3 shadRayDelta = 1.0 / (vec3(maxDim));

    #ifdef CONSIDER_NORMALS
    vec3 normal = normalize(2.0*texture3D(normVolume_, pos).xyz - 1.0);
    #elif defined(SH_SUBSURFACE)
    vec3 normal = normalize(2.0*texture3DLod(tfVolume_, pos, 0.0).xyz - 1.0);
    #endif

    //vec3 normal = normalize(2.0*texture3D(tfVolume_, pos).xyz-1.0);

    // TODO this is configured for 4 bands of coeffs. change this to a variable
    // In the main loop, we visit each randomized sample to calculate how much light arrives from that sample to the
    // current voxel (and optionally, what light-color the ray accumulates on its way).
    for(int i = 0; i < sampleNumSqrt_; i++) {
        for(int j = 0; j < sampleNumSqrt_; j++) {

            // calculate the tex-coord of the current sample
            vec2 sampleCoord = vec2(i, j) / float(sampleNumSqrt_);
            sampleCoord += 0.5 / float(sampleNumSqrt_);

            vec4 sample = texture(lightSamples_, sampleCoord);
            // look up the vector that points towards the current sample
            vec3 dir = normalize(2.0*sample.xyz - 1.0);
            dir /= cs;

            #if defined(CONSIDER_NORMALS) && !defined(SH_BLEED_SUB)
            float dp = dot(dir, normal);
            if(dp <= 0.0)
                continue;
            #endif

            #if defined(SH_SUBSURFACE) || defined(SH_BLEED_SUB)
            float dp = dot(dir, normal);
            if(dp <= 0.0)
                continue;
            #endif

            // the initial increment we use to step through the volume along the ray
            vec3 shadInc  = shadRayDelta * dir;

            // start at an intensity of 1.0, which is reduced depending on the transfer-function-values that the ray has to
            // pass through
            float shadFac = 1.0;

            // we start one step from the current position to avoid artifacts (we don't want the voxel to bleed on itself)
            vec3 shadPos = pos + 2.0 * shadInc;

            // if we want color-bleeding, we need something to save the accumulated color in
            #ifndef SH_SHADOWED
            vec4 colorFac = vec4(0.0);
            #endif

            // in each step along the ray, we look up the color of the ray-position in the tf-volume
            vec4 rayColor;

            // the level-variable is used to count how many steps we have taken, and to pick the right mip-map level of the
            // tf-volume for the color-lookup
            float level = 0.0;

            bool done = false;

            // start the ray from the voxel to the current sample
            while(!done) {

                // look up the color value at this position of the ray in the tf-volume, with the counter variable as mipmap-level
                rayColor = texture3DLod(tfVolume_, shadPos, level);

                #ifdef SH_ERI
                // double the ray-increment, according to ritschel paper (see SHCoeffTrans class for details)
                shadInc *= 2.0;
                level += 1.0;
                #endif

                shadPos += shadInc;

                // decrease the ray-intensity by the looked-up transparency, increase the ray-position, and increment the mip-map/step counter
                shadFac *= 1.0 - rayColor.a;

                // if the ray exceeds the bounds of the volume, we are done with this ray and can add the ray information to the
                // total info we have to collect about this voxel
                if(any(greaterThan(shadPos, vec3(1.0))) || any(lessThan(shadPos, vec3(0.0))) || shadFac < 0.1)
                    break;

                // if we want color-bleeding, now is the time to interpolate the already accumulated color with the newly looked-up color
                #ifndef SH_SHADOWED
                colorFac.xyz = step(5.0, level)*((1.0-shadFac)*(colorFac.xyz + rayColor.xyz));
                #endif
            }

            // now we have to look up the 16 geometrical sh-coefficients for this sample.  They depend only on the theta and phi angles.
            // They are saved in a RGBA-2D-Texture, divided into 4 quadrants, so we can access 16 values in one texture.
            if(shadFac >= 0.1) {

                vec4 coeffs0 = texture(lightCoeffs_, 0.5*sampleCoord                 );
                vec4 coeffs1 = texture(lightCoeffs_, 0.5*sampleCoord + vec2(0.5, 0.0));
                vec4 coeffs2 = texture(lightCoeffs_, 0.5*sampleCoord + vec2(0.0, 0.5));
                vec4 coeffs3 = texture(lightCoeffs_, 0.5*sampleCoord +      0.5      );

                // remap values into the -1...1 range
                mat4 coeffs = 2.0*mat4(coeffs0, coeffs1, coeffs2, coeffs3) - 1.0;

                // the accumulated color has to be multiplied by shadFac, because less visible sample-directions contribute less to bleeding
                #ifndef SH_SHADOWED
                colorFac *= 10.0;
                #endif

                // finally, we add the geometrical coefficients for this sample, weighted by the remaining intensity that a ray that
                // passes from this sample to the current voxel still possesses.  The result parameter will, at the end of the two
                // for-loops, contain the result of the Monte-Carlo integration; in this case, the amount of light that can arrive at
                // the voxel, given the current transfer-function.
                //#if defined(SH_SUBSURFACE)
                //helper += coeffs[0] * shadFac * dp;
                //#elif defined(CONSIDER_NORMALS)
                #if defined(CONSIDER_NORMALS) || defined(SH_SUBSURFACE)
                shadFac *= dp;
                #endif

                #if !defined(SH_BLEEDING) && !defined(SH_BLEED_SUB)
                result += coeffs * shadFac;
                //result += coeffs * shadFac * shadFac;
                #else
                //colorFac += texture3D(tfVolume_, pos).xyz;
                //colorFac *= 0.5;
                colorFac *= shadFac;

                colorFac.rgb += texture3D(tfVolume_, pos).xyz * shadFac;
                //colorFac *= texture3D(tfVolume_, pos).xyz;

                // in case of sh-bleeding, we accumulate the geometrical coefficients, weighted by intensity, as above, and also weighted
                // by each color channel.
                if(pass_ == 0) {
                    result0 += coeffs * colorFac.r;
                    result1 += coeffs * shadFac;
                } else {
                    result0 += coeffs * colorFac.g;
                    result1 += coeffs * colorFac.b;
                }
                #endif
            }
        }
    }  // end of for-loop

    // finally, the accumulated coefficients are renormalized, and we are done for this voxel!
    #if !defined(SH_BLEEDING) && !defined(SH_BLEED_SUB)
    result *= normConst_;
    result = 0.5*result + 0.5;
    #else
    result0 *= normConst_;
    result0 = 0.5*result0 + 0.5;
    result1 *= normConst_;
    result1 = 0.5*result1 + 0.5;
    #endif
}

/***
 * The main method.
 ***/
void main() {

    // TODO for some reason, defining the following as constants shows no results... (e.g. const pi_ = 3.141...;)
    // see comment at declaration
    normConst_ = 8.0*asin(1.0) / float(sampleNumSqrt_*sampleNumSqrt_);

    // the 3d-texture-coordinate is composed of the screen-coordinate for x and y, and the slice-number for z.  To remap it
    // to 0..1 range, we divide this by the size of the volume we render into.
    vec3 pos = vec3(vec2(gl_FragCoord.xy), float(curSlice_)) / vec3(volSize_);
    mat4 result0 = mat4(0.0);

    // for subsurface scattering and / or sh bleeding, we need an additional matrix
    #if defined(SH_BLEEDING) || defined(SH_BLEED_SUB)
    mat4 result1 = mat4(0.0);
    monteCarloInt(pos, result0, result1);
    #else
    monteCarloInt(pos, result0);
    #endif

    FragData0 = result0[0];
    FragData1 = result0[1];
    FragData2 = result0[2];
    FragData3 = result0[3];

    #if defined(SH_BLEEDING) || defined(SH_BLEED_SUB)
    FragData4 = result1[0];
    FragData5 = result1[1];
    FragData6 = result1[2];
    FragData7 = result1[3];
    #endif
}

