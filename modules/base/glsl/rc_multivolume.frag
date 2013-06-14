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

#include "modules/mod_sampler2d.frag"
#include "modules/mod_sampler3d.frag"
#include "modules/mod_transfunc.frag"

#include "modules/mod_raysetup.frag"

#include "modules/mod_depth.frag"
#include "modules/mod_compositing.frag"
#include "modules/mod_gradients.frag"
#include "modules/mod_shading.frag"

// variables for storing compositing results
vec4 result = vec4(0.0);
vec4 result1 = vec4(0.0);
vec4 result2 = vec4(0.0);

uniform float samplingStepSize_;

// declare entry and exit parameters
uniform sampler2D entryPoints_;            // ray entry points
uniform sampler2D entryPointsDepth_;       // ray entry points depth
uniform TextureParameters entryParameters_;

uniform sampler2D exitPoints_;             // ray exit points
uniform sampler2D exitPointsDepth_;        // ray exit points depth
uniform TextureParameters exitParameters_;

// declare volumes
#ifdef VOLUME_1_ACTIVE
uniform VolumeParameters volumeStruct1_;
uniform sampler3D volume1_;    // volume dataset 1
uniform TransFuncParameters transferFunc1_;
uniform TF_SAMPLER_TYPE_1 transferFuncTex1_;
#endif

#ifdef VOLUME_2_ACTIVE
uniform VolumeParameters volumeStruct2_;
uniform sampler3D volume2_;    // volume dataset 2
uniform TransFuncParameters transferFunc2_;
uniform TF_SAMPLER_TYPE_2 transferFuncTex2_;
#endif

#ifdef VOLUME_3_ACTIVE
uniform VolumeParameters volumeStruct3_;
uniform sampler3D volume3_;    // volume dataset 3
uniform TransFuncParameters transferFunc3_;
uniform TF_SAMPLER_TYPE_3 transferFuncTex3_;
#endif

#ifdef VOLUME_4_ACTIVE
uniform VolumeParameters volumeStruct4_;
uniform sampler3D volume4_;    // volume dataset 4
uniform TransFuncParameters transferFunc4_;
uniform TF_SAMPLER_TYPE_4 transferFuncTex4_;
#endif

/***
 * Performs the ray traversal
 * returns the final fragment color.
 ***/
void rayTraversal(in vec3 first, in vec3 last, float entryDepth, float exitDepth) {
    // calculate the required ray parameters
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    vec3 rayDirection;
    raySetup(first, last, samplingStepSize_, rayDirection, tIncr, tEnd);

#ifdef VOLUME_1_ACTIVE
    vec3 vol1first = worldToTex(first, volumeStruct1_);
    vec3 vol1dir = worldToTex(last, volumeStruct1_) - vol1first;
    float lastIntensity1 = 0.0f; //used for pre-integrated transfer-functions
#endif
#ifdef VOLUME_2_ACTIVE
    vec3 vol2first = worldToTex(first, volumeStruct2_);
    vec3 vol2dir = worldToTex(last, volumeStruct2_) - vol2first;
    float lastIntensity2 = 0.0f; //used for pre-integrated transfer-functions
#endif
#ifdef VOLUME_3_ACTIVE
    vec3 vol3first = worldToTex(first, volumeStruct3_);
    vec3 vol3dir = worldToTex(last, volumeStruct3_) - vol3first;
    float lastIntensity3 = 0.0f; //used for pre-integrated transfer-functions
#endif
#ifdef VOLUME_4_ACTIVE
    vec3 vol4first = worldToTex(first, volumeStruct4_);
    vec3 vol4dir = worldToTex(last, volumeStruct4_) - vol4first;
    float lastIntensity4 = 0.0f; //used for pre-integrated transfer-functions
#endif

    float realT;
    float tDepth = -1.0f;
    bool finished = false;
    WHILE(!finished) {
        realT = t / tEnd;

        vec3 worldSamplePos = first + t * rayDirection;

#ifdef VOLUME_1_ACTIVE
        vec3 samplePos1 = vol1first + realT * vol1dir;

        vec4 voxel1;
        if(inUnitCube(samplePos1))
            voxel1 = getVoxel(volume1_, volumeStruct1_, samplePos1);
        else
            voxel1 = vec4(0.0);

#ifdef CLASSIFICATION_REQUIRES_GRADIENT
        // calculate gradients
        voxel1.xyz = CALC_GRADIENT(volume1_, volumeStruct1_, samplePos1);
#endif

        // apply classification
        vec4 color = RC_APPLY_CLASSIFICATION(transferFunc1_, transferFuncTex1_, voxel1, lastIntensity1);

        // if opacity greater zero, apply compositing
        if (color.a > 0.0) {
#ifndef CLASSIFICATION_REQUIRES_GRADIENT
            voxel1.xyz = CALC_GRADIENT(volume1_, volumeStruct1_, samplePos1);
#endif

            // apply shading
            color.rgb = APPLY_SHADING_1(voxel1.xyz, texToPhysical(samplePos1, volumeStruct1_), volumeStruct1_.lightPositionPhysical_, volumeStruct1_.cameraPositionPhysical_, color.rgb, color.rgb, vec3(1.0,1.0,1.0));

            result = RC_APPLY_COMPOSITING_1(result, color, worldSamplePos, voxel1.xyz, t, samplingStepSize_, tDepth);
            result1 = RC_APPLY_COMPOSITING_2(result1, color, worldSamplePos, voxel1.xyz, t, samplingStepSize_, tDepth);
            result2 = RC_APPLY_COMPOSITING_3(result2, color, worldSamplePos, voxel1.xyz, t, samplingStepSize_, tDepth);
        }
        lastIntensity1 = voxel1.a;
#endif

#ifdef VOLUME_2_ACTIVE
        //second sample:
        vec3 samplePos2 = vol2first + realT * vol2dir;

        vec4 voxel2;
        if(inUnitCube(samplePos2))
            voxel2 = getVoxel(volume2_, volumeStruct2_, samplePos2);
        else
            voxel2 = vec4(0.0);

#ifdef CLASSIFICATION_REQUIRES_GRADIENT
        // calculate gradients
        voxel2.xyz = CALC_GRADIENT(volume2_, volumeStruct2_, samplePos2);
#endif

        // apply classification
        vec4 color2 = RC_APPLY_CLASSIFICATION2(transferFunc2_, transferFuncTex2_, voxel2, lastIntensity2);

        // if opacity greater zero, apply compositing
        if (color2.a > 0.0) {
#ifndef CLASSIFICATION_REQUIRES_GRADIENT
            voxel2.xyz = CALC_GRADIENT(volume2_, volumeStruct2_, samplePos2);
#endif

            // apply shading
            color2.rgb = APPLY_SHADING_2(voxel2.xyz, texToPhysical(samplePos2, volumeStruct2_), volumeStruct2_.lightPositionPhysical_, volumeStruct2_.cameraPositionPhysical_, color2.rgb, color2.rgb, vec3(1.0,1.0,1.0));

            result = RC_APPLY_COMPOSITING_1(result, color2, worldSamplePos, voxel2.xyz, t, samplingStepSize_, tDepth);
            result1 = RC_APPLY_COMPOSITING_2(result1, color2, worldSamplePos, voxel2.xyz, t, samplingStepSize_, tDepth);
            result2 = RC_APPLY_COMPOSITING_3(result2, color2, worldSamplePos, voxel2.xyz, t, samplingStepSize_, tDepth);
        }
        lastIntensity2 = voxel2.a;
#endif

#ifdef VOLUME_3_ACTIVE
        //second sample:
        vec3 samplePos3 = vol3first + realT * vol3dir;

        vec4 voxel3;
        if(inUnitCube(samplePos3))
            voxel3 = getVoxel(volume3_, volumeStruct3_, samplePos3);
        else
            voxel3 = vec4(0.0);

#ifdef CLASSIFICATION_REQUIRES_GRADIENT
        // calculate gradients
        voxel3.xyz = CALC_GRADIENT(volume3_, volumeStruct3_, samplePos3);
#endif

        // apply classification
        vec4 color3 = RC_APPLY_CLASSIFICATION3(transferFunc3_, transferFuncTex3_, voxel3, lastIntensity3);

        // if opacity greater zero, apply compositing
        if (color3.a > 0.0) {
#ifndef CLASSIFICATION_REQUIRES_GRADIENT
            voxel3.xyz = CALC_GRADIENT(volume3_, volumeStruct3_, samplePos3);
#endif

            // apply shading
            color3.rgb = APPLY_SHADING_3(voxel3.xyz, texToPhysical(samplePos3, volumeStruct3_), volumeStruct3_.lightPositionPhysical_, volumeStruct3_.cameraPositionPhysical_, color3.rgb, color3.rgb, vec3(1.0,1.0,1.0));

            result = RC_APPLY_COMPOSITING_1(result, color3, worldSamplePos, voxel3.xyz, t, samplingStepSize_, tDepth);
            result1 = RC_APPLY_COMPOSITING_2(result1, color3, worldSamplePos, voxel3.xyz, t, samplingStepSize_, tDepth);
            result2 = RC_APPLY_COMPOSITING_3(result2, color3, worldSamplePos, voxel3.xyz, t, samplingStepSize_, tDepth);
        }
        lastIntensity3 = voxel3.a;
#endif

#ifdef VOLUME_4_ACTIVE
        //second sample:
        vec3 samplePos4 = vol4first + realT * vol4dir;

        vec4 voxel4;
        if(inUnitCube(samplePos4))
            voxel4 = getVoxel(volume4_, volumeStruct4_, samplePos4);
        else
            voxel4 = vec4(0.0);

#ifdef CLASSIFICATION_REQUIRES_GRADIENT
        // calculate gradients
        voxel4.xyz = CALC_GRADIENT(volume4_, volumeStruct4_, samplePos4);
#endif

        // apply classification
        vec4 color4 = RC_APPLY_CLASSIFICATION4(transferFunc4_, transferFuncTex4_, voxel4, lastIntensity4);

        // if opacity greater zero, apply compositing
        if (color4.a > 0.0) {
#ifndef CLASSIFICATION_REQUIRES_GRADIENT
            voxel4.xyz = CALC_GRADIENT(volume4_, volumeStruct4_, samplePos4);
#endif

            // apply shading
            color4.rgb = APPLY_SHADING_4(voxel4.xyz, texToPhysical(samplePos4, volumeStruct4_), volumeStruct4_.lightPositionPhysical_, volumeStruct4_.cameraPositionPhysical_, color4.rgb, color4.rgb, vec3(1.0,1.0,1.0));

            result = RC_APPLY_COMPOSITING_1(result, color4, worldSamplePos, voxel4.xyz, t, samplingStepSize_, tDepth);
            result1 = RC_APPLY_COMPOSITING_2(result1, color4, worldSamplePos, voxel4.xyz, t, samplingStepSize_, tDepth);
            result2 = RC_APPLY_COMPOSITING_3(result2, color4, worldSamplePos, voxel4.xyz, t, samplingStepSize_, tDepth);
        }
        lastIntensity4 = voxel4.a;
#endif
        finished = earlyRayTermination(result.a, EARLY_RAY_TERMINATION_OPACITY);
        t += tIncr;
        finished = finished || (t > tEnd);
    } END_WHILE
    gl_FragDepth = getDepthValue(tDepth, tEnd, entryDepth, exitDepth);
}

/***
 * The main method.
 ***/
void main() {
    // fetch entry/exit points
    vec2 p = gl_FragCoord.xy * screenDimRCP_;
    vec3 frontPos = textureLookup2Dnormalized(entryPoints_, entryParameters_, p).rgb;
    vec3 backPos = textureLookup2Dnormalized(exitPoints_, exitParameters_, p).rgb;
    float entryDepth = textureLookup2Dnormalized(entryPointsDepth_, entryParameters_, p).z;
    float exitDepth = textureLookup2Dnormalized(exitPointsDepth_, exitParameters_, p).z;

    // determine whether the ray has to be casted
    if (frontPos == backPos)
        // background needs no raycasting
        discard;
    else {
        // fragCoords are lying inside the bounding box
        rayTraversal(frontPos, backPos, entryDepth, exitDepth);
    }

    #ifdef OP0
        FragData0 = result;
    #endif
    #ifdef OP1
        FragData1 = result1;
    #endif
    #ifdef OP2
        FragData2 = result2;
    #endif
}
