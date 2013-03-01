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

#include "mod_spherharmshading.frag"
#line 1

// variable for storing compositing results
vec4 result = vec4(0.0);

// declare entry and exit parameters
uniform sampler2D entryPoints_;            // ray entry points
uniform sampler2D entryPointsDepth_;       // ray entry points depth
uniform TextureParameters entryParameters_;

uniform sampler2D exitPoints_;             // ray exit points
uniform sampler2D exitPointsDepth_;        // ray exit points depth
uniform TextureParameters exitParameters_;

uniform float samplingStepSize_;

// declare volume
uniform VolumeParameters volumeStruct_;
uniform sampler3D volume_;    // volume data set

uniform TransFuncParameters transferFunc_;
uniform TF_SAMPLER_TYPE transferFuncTex_;

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
    raySetup(first, last, samplingStepSize_, rayDirection, tIncr, tEnd);

    float tDepth = -1.0f;
    bool finished = false;
    WHILE(!finished) {
        vec3 samplePos = first + t * rayDirection;
        vec4 voxel = getVoxel(volume_, volumeStruct_, samplePos);
        //vec4 voxel = getVoxel(shcoeffsI0_, volumeStruct_, samplePos);

        // apply classification (if BRDFs are not enabled, this uses normal TF classification)
        vec4 color = applyTF(transferFunc_, transferFuncTex_, voxel.a);

        // if opacity greater zero, apply compositing
        if (color.a > 0.0) {
            //calculate gradients
            if(t == 0.0)
                voxel.xyz = fixClipBorderGradient(samplePos, rayDirection, entryPoints_, entryParameters_);
            else
                voxel.xyz = CALC_GRADIENT(volume_, volumeStruct_, samplePos);

            // apply shading
            color.rgb = shShading(voxel.xyz, samplePos, -rayDirection, color.rgb);

            result = compositeDVR(result, color, t, samplingStepSize_, tDepth);
        }

        finished = earlyRayTermination(result.a, EARLY_RAY_TERMINATION_OPACITY);
        t += tIncr;
        finished = finished || (t > tEnd);
    } END_WHILE
    gl_FragDepth = getDepthValue(tDepth, tEnd, entryPointsDepth_, entryParameters_, exitPointsDepth_, exitParameters_);

    // FIXME no image without manually setting gl_FragDepth < 1.0
    gl_FragDepth = 0.5;
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
         //fragCoords are lying inside the bounding box
        rayTraversal(frontPos, backPos);

    //result = vec4(0.0, 1.0, 0.0, 1.0);
    FragData0 = result;
}

