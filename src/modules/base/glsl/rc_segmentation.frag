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
#include "modules/mod_segmentation.frag"

// variables for storing compositing results
vec4 result = vec4(0.0);
vec4 result1 = vec4(0.0);
vec4 result2 = vec4(0.0);


// declare entry and exit parameters
uniform SAMPLER2D_TYPE entryPoints_;            // ray entry points
uniform SAMPLER2D_TYPE entryPointsDepth_;       // ray entry points depth
uniform TEXTURE_PARAMETERS entryParameters_;
uniform SAMPLER2D_TYPE exitPoints_;                // ray exit points
uniform SAMPLER2D_TYPE exitPointsDepth_;        // ray exit points depth
uniform TEXTURE_PARAMETERS exitParameters_;
// declare volume
uniform sampler3D volume_;                      // volume data set
uniform VOLUME_PARAMETERS volumeParameters_;    // texture lookup parameters for volume_

uniform sampler3D segmentation_;                    // segmentation volume
uniform VOLUME_PARAMETERS segmentationParameters_;  // texture lookup parameters for segmentation_


#ifdef MOD_APPLY_SEGMENTATION

uniform sampler2D segmentationTransferFunc_;

vec4 applySegmentationClassification(vec3 sample, vec4 voxel, sampler3D segmentation, VOLUME_PARAMETERS segmentationParameters) {

    // Determine segment id and perform transfer function lookup within corresponding segmentation transfer function.
    // The 1D transfer function of a segment is stored in the 2D segmentation tf texture as a 3-row wide stripe which is centered around the row 3*i+1.

    float segmentScaleFactor = 255.0;
    if (segmentationParameters.bitDepth_ == 12)
        segmentScaleFactor = 4095.0;
    else if (segmentationParameters.bitDepth_ == 16)
        segmentScaleFactor = 65535.0;

    float segValue = getVoxel(segmentation, segmentationParameters, sample).a;
    float segment = segValue * segmentScaleFactor;

    return texture(segmentationTransferFunc_, vec2(voxel.a, (segment*3.0+1.0)/float(SEGMENTATION_TRANSFUNC_HEIGHT)) );

}

#endif


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

    RC_BEGIN_LOOP {
        vec3 samplePos = first + t * rayDirection;
        vec4 voxel = getVoxel(volume_, volumeParameters_, samplePos);

        // calculate gradients
        voxel.xyz = RC_CALC_GRADIENTS(voxel.xyz, samplePos, volume_, volumeParameters_, t, rayDirection, entryPoints_, entryParameters_);

        #ifdef MOD_APPLY_SEGMENTATION
            // apply segmentation
            vec4 color = applySegmentationClassification(samplePos, voxel, segmentation_, segmentationParameters_);
        #else
            // apply classification
            vec4 color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel);
        #endif

        // apply standard shading
        color.rgb = RC_APPLY_SHADING(voxel.xyz, samplePos, volumeParameters_, color.rgb, color.rgb, color.rgb);

        // if opacity greater zero, apply compositing
        if (color.a > 0.0) {
            result = RC_APPLY_COMPOSITING(result, color, samplePos, voxel.xyz, t, tDepth);
            result1 = RC_APPLY_COMPOSITING_1(result1, color, samplePos, voxel.xyz, t, tDepth);
            result2 = RC_APPLY_COMPOSITING_2(result2, color, samplePos, voxel.xyz, t, tDepth);
        }

    } RC_END_LOOP(result);
}

/***
 * The main method.
 ***/
void main() {
    vec2 p = gl_FragCoord.xy * screenDimRCP_;
    vec3 frontPos = textureLookup2Dnormalized(entryPoints_, entryParameters_, p).rgb;
    vec3 backPos = textureLookup2Dnormalized(exitPoints_, exitParameters_, p).rgb;

    // determine whether the ray has to be casted
    if (frontPos == backPos)
        // background needs no raycasting
        discard;
    else
        // fragCoords are lying inside the bounding box
        rayTraversal(frontPos, backPos);

    #ifdef OP0
        FragData0 = result;
    #endif
    #ifdef OP1
        fragData1 = result1;
    #endif
    #ifdef OP2
        fragData2 = result2;
    #endif
}
