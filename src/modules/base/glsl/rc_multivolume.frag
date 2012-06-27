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

#include "modules/vrn_shaderincludes.frag"

// variables for storing compositing results
vec4 result = vec4(0.0);
vec4 result1 = vec4(0.0);
vec4 result2 = vec4(0.0);


// declare entry and exit parameters
uniform SAMPLER2D_TYPE entryPoints_;            // ray entry points
uniform SAMPLER2D_TYPE entryPointsDepth_;       // ray entry points depth
uniform TEXTURE_PARAMETERS entryParameters_;
uniform SAMPLER2D_TYPE exitPoints_;             // ray exit points
uniform SAMPLER2D_TYPE exitPointsDepth_;        // ray exit points depth
uniform TEXTURE_PARAMETERS exitParameters_;
// declare volume
uniform sampler3D volume1_;                      // volume data set
uniform VOLUME_PARAMETERS volumeParameters1_;    // texture lookup parameters for volume1_
uniform TF_SAMPLER_TYPE_1 transferFunc_;

#ifdef VOLUME_2_ACTIVE
uniform sampler3D volume2_;                      // volume data set
uniform VOLUME_PARAMETERS volumeParameters2_;    // texture lookup parameters for volume2_
uniform TF_SAMPLER_TYPE_2 transferFunc2_;
#endif

#ifdef VOLUME_3_ACTIVE
uniform sampler3D volume3_;                      // volume data set
uniform VOLUME_PARAMETERS volumeParameters3_;    // texture lookup parameters for volume3_
uniform TF_SAMPLER_TYPE_3 transferFunc3_;
#endif

#ifdef VOLUME_4_ACTIVE
uniform sampler3D volume4_;                      // volume data set
uniform VOLUME_PARAMETERS volumeParameters4_;    // texture lookup parameters for volume4_
uniform TF_SAMPLER_TYPE_4 transferFunc4_;
#endif


bool inUnitCube(vec3 sample) {
    /*return (all(lessThanEqual(sample, vec3(1.0))) && (all(greaterThanEqual(sample, vec3(0.0)))));

    if(any(greaterThan(sample, vec3(1.0))))
        return false;
    if(any(lessThan(sample, vec3(0.0))))
        return false;

    return !(any(greaterThan(sample, vec3(1.0))) || any(lessThan(sample, vec3(0.0)))); */

    if(any(greaterThan(sample, vec3(1.0))) || any(lessThan(sample, vec3(0.0))))
        return false;
    return true;
}

vec3 worldToTex(vec3 worldSamplePos, VOLUME_PARAMETERS volumeParameters) {
    //From world space to object space:
    vec3 samplePos = (volumeParameters.volumeTransformationINV_ * vec4(worldSamplePos, 1.0)).xyz;

    //From object space to texture space:
    samplePos = (samplePos + (volumeParameters.volumeCubeSize_ * 0.5)) * volumeParameters.volumeCubeSizeRCP_;
    return samplePos;
}

/***
 * Performs the ray traversal
 * returns the final fragment color.
 ***/
void rayTraversal(in vec3 first, in vec3 last) {
    // calculate the required ray parameters
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    vec3 rayDirection;
    raySetup(first, last, rayDirection, tIncr, tEnd);

    vec3 vol1first = worldToTex(first, volumeParameters1_);
    vec3 vol1dir = worldToTex(last, volumeParameters1_) - vol1first;
#ifdef VOLUME_2_ACTIVE
    vec3 vol2first = worldToTex(first, volumeParameters2_);
    vec3 vol2dir = worldToTex(last, volumeParameters2_) - vol2first;
#endif
#ifdef VOLUME_3_ACTIVE
    vec3 vol3first = worldToTex(first, volumeParameters3_);
    vec3 vol3dir = worldToTex(last, volumeParameters3_) - vol3first;
#endif
#ifdef VOLUME_4_ACTIVE
    vec3 vol4first = worldToTex(first, volumeParameters4_);
    vec3 vol4dir = worldToTex(last, volumeParameters4_) - vol4first;
#endif

    float realT;
    RC_BEGIN_LOOP {
        realT = t / tEnd;

        vec3 worldSamplePos = first + t * rayDirection;

        //--------------VOLUME 1---------------------
        //vec3 samplePos1 = worldToTex(worldSamplePos, volumeParameters1_);
        vec3 samplePos1 = vol1first + realT * vol1dir;

        vec4 voxel1;
        if(inUnitCube(samplePos1))
            voxel1 = getVoxel(volume1_, volumeParameters1_, samplePos1);
        else
            voxel1 = vec4(0.0);

        // calculate gradients
        //TODO: adapt t, rayDirection, entryPoints_, entryParameters_ ?
        voxel1.xyz = RC_CALC_GRADIENTS(voxel1.xyz, samplePos1, volume1_, volumeParameters1_, t, rayDirection, entryPoints_, entryParameters_);

        // apply classification
        vec4 color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel1);

        // apply shading
        color.rgb = RC_APPLY_SHADING_1(voxel1.xyz, samplePos1, volumeParameters1_, color.rgb, color.rgb, vec3(1.0,1.0,1.0));

        // if opacity greater zero, apply compositing
        if (color.a > 0.0) {
            RC_BEGIN_COMPOSITING
            //TODO: adapt t, tDepth ?
            result = RC_APPLY_COMPOSITING(result, color, worldSamplePos, voxel1.xyz, t, tDepth);
            result1 = RC_APPLY_COMPOSITING_1(result1, color, worldSamplePos, voxel1.xyz, t, tDepth);
            result2 = RC_APPLY_COMPOSITING_2(result2, color, worldSamplePos, voxel1.xyz, t, tDepth);
            RC_END_COMPOSITING
        }

#ifdef VOLUME_2_ACTIVE
        //second sample:
        //vec3 samplePos2 = worldToTex(worldSamplePos, volumeParameters2_);
        vec3 samplePos2 = vol2first + realT * vol2dir;

        vec4 voxel2;
        if(inUnitCube(samplePos2))
            voxel2 = getVoxel(volume2_, volumeParameters2_, samplePos2);
        else
            voxel2 = vec4(0.0);

        // calculate gradients
        voxel2.xyz = RC_CALC_GRADIENTS(voxel2.xyz, samplePos2, volume2_, volumeParameters2_, t, rayDirection, entryPoints_, entryParameters_);

        // apply classification
        vec4 color2 = RC_APPLY_CLASSIFICATION(transferFunc2_, voxel2);

        // apply shading
        color2.rgb = RC_APPLY_SHADING_2(voxel2.xyz, samplePos2, volumeParameters2_, color2.rgb, color2.rgb, vec3(1.0,1.0,1.0));

        // if opacity greater zero, apply compositing
        if (color2.a > 0.0) {
            RC_BEGIN_COMPOSITING
            result = RC_APPLY_COMPOSITING(result, color2, worldSamplePos, voxel2.xyz, t, tDepth);
            result1 = RC_APPLY_COMPOSITING_1(result1, color2, worldSamplePos, voxel2.xyz, t, tDepth);
            result2 = RC_APPLY_COMPOSITING_2(result2, color2, worldSamplePos, voxel2.xyz, t, tDepth);
            RC_END_COMPOSITING
        }
#endif

#ifdef VOLUME_3_ACTIVE
        //second sample:
        //vec3 samplePos3 = worldToTex(worldSamplePos, volumeParameters3_);
        vec3 samplePos3 = vol3first + realT * vol3dir;

        vec4 voxel3;
        if(inUnitCube(samplePos3))
            voxel3 = getVoxel(volume3_, volumeParameters3_, samplePos3);
        else
            voxel3 = vec4(0.0);

        // calculate gradients
        voxel3.xyz = RC_CALC_GRADIENTS(voxel3.xyz, samplePos3, volume3_, volumeParameters3_, t, rayDirection, entryPoints_, entryParameters_);

        // apply classification
        vec4 color3 = RC_APPLY_CLASSIFICATION(transferFunc3_, voxel3);

        // apply shading
        color3.rgb = RC_APPLY_SHADING_3(voxel3.xyz, samplePos3, volumeParameters3_, color3.rgb, color3.rgb, vec3(1.0,1.0,1.0));

        // if opacity greater zero, apply compositing
        if (color3.a > 0.0) {
            RC_BEGIN_COMPOSITING
            result = RC_APPLY_COMPOSITING(result, color3, worldSamplePos, voxel3.xyz, t, tDepth);
            result1 = RC_APPLY_COMPOSITING_1(result1, color3, worldSamplePos, voxel3.xyz, t, tDepth);
            result2 = RC_APPLY_COMPOSITING_2(result2, color3, worldSamplePos, voxel3.xyz, t, tDepth);
            RC_END_COMPOSITING
        }
#endif

#ifdef VOLUME_4_ACTIVE
        //second sample:
        //vec3 samplePos4 = worldToTex(worldSamplePos, volumeParameters4_);
        vec3 samplePos4 = vol4first + realT * vol4dir;

        vec4 voxel4;
        if(inUnitCube(samplePos4))
            voxel4 = getVoxel(volume4_, volumeParameters4_, samplePos4);
        else
            voxel4 = vec4(0.0);

        // calculate gradients
        voxel4.xyz = RC_CALC_GRADIENTS(voxel4.xyz, samplePos4, volume4_, volumeParameters4_, t, rayDirection, entryPoints_, entryParameters_);

        // apply classification
        vec4 color4 = RC_APPLY_CLASSIFICATION(transferFunc4_, voxel4);

        // apply shading
        color4.rgb = RC_APPLY_SHADING_4(voxel4.xyz, samplePos4, volumeParameters4_, color4.rgb, color4.rgb, vec3(1.0,1.0,1.0));

        // if opacity greater zero, apply compositing
        if (color4.a > 0.0) {
            RC_BEGIN_COMPOSITING
            result = RC_APPLY_COMPOSITING(result, color4, worldSamplePos, voxel4.xyz, t, tDepth);
            result1 = RC_APPLY_COMPOSITING_1(result1, color4, worldSamplePos, voxel4.xyz, t, tDepth);
            result2 = RC_APPLY_COMPOSITING_2(result2, color4, worldSamplePos, voxel4.xyz, t, tDepth);
            RC_END_COMPOSITING
        }
#endif
    } RC_END_LOOP(result);
}

/***
 * The main method.
 ***/
void main() {

    vec3 frontPos = textureLookup2D(entryPoints_, entryParameters_, gl_FragCoord.xy).rgb;
    vec3 backPos = textureLookup2D(exitPoints_, exitParameters_, gl_FragCoord.xy).rgb;

    // determine whether the ray has to be casted
    if (frontPos == backPos)
        // background needs no raycasting
        discard;
    else {
        // fragCoords are lying inside the bounding box
        rayTraversal(frontPos, backPos);
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
