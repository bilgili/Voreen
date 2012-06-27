/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

/**
* This is the "master" bricking file where the getVoxel and getVoxelUnnormalized
* functions are declared.
*
* The Entry-Exit-Points for bricking are exactly the same as for a normal
* dataset. The volume itself however is handled differently. The volume is
* subdivided into bricks, and all these bricks are packed together in a
* "packed texture". An "index texture" keeps track of where the bricks are
* inside the packed texture. In order to do a texture lookup one needs to do
* the following:
*
* ->    Check in which brick the current sample is.
*
* ->    Calculate where in the index texture we can read the brick's
*       position in the packed texture.
*
* ->    Read that information from the index texture.
*
* ->    Use that information to calculate the sample's exact position
*       in the packed texture.
*
* ->    Read the sample, using either intrablock or interblock interpolation.
*
* For more information read the papers by Kraus and Ertl (Adaptive Texture Maps) and
* by Ljung et al. (Multiresolution Interblock Interpolation in Direct Volume Rendering)
*/

#include "modules/bricking/mod_basics.frag"
#include "modules/bricking/mod_uniforms.frag"
#include "modules/bricking/mod_global_variables.frag"
#include "modules/bricking/mod_math.frag"
#include "modules/bricking/mod_lookups.frag"
#include "modules/bricking/mod_interpolation.frag"
#include "modules/bricking/mod_adaptive_sampling.frag"

/**
* Lookup the current sample.
*/
vec4 getVoxel(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 sample) {

    //Check in which brick the sample is in.
    vec3 brick = getBrick(sample);

    //Calculate where in the index texture the brick's position in the
    //packed texture can be found.
    vec3 lookupPos = getIndexVolumePosition(brick);

    //Read that information.
    vec4 indexVolumeSample = indexVolumeLookup(lookupPos);

    //The scalefactor is stored globally (which is not..elegant) because
    //that information is needed when adaptive sampling is used.
    currentScaleFactor = indexVolumeSample.a;

    vec4 voxel;

    //Do the actual lookup. Either intrablock or interblock interpolation is used.
    voxel = LOOKUP_VOXEL(sample,brick,indexVolumeSample);

    #ifdef ADAPTIVE_SAMPLING
        numberOfSkippedSamples = getNumberOfSkippedSamples(sample, directionWithStepSize);
    #endif

    return voxel;
}

vec4 getVoxelUnnormalized(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 sample) {

     //Check in which brick the sample is in.
    vec3 brick = getBrick(sample);

    //Calculate where in the index texture the brick's position in the
    //packed texture can be found.
    vec3 lookupPos = getIndexVolumePosition(brick);

    //Read that information.
    vec4 indexVolumeSample = indexVolumeLookup(lookupPos);

    vec4 voxel;

    //Do the actual lookup. Either intrablock or interblock interpolation is used.
    voxel = LOOKUP_VOXEL(sample,brick,indexVolumeSample);

    voxel /= packedVolumeParameters_.bitDepthScale_;

    return voxel;
}

/*
 * Function for volume texture lookup. In addition to the volume and the texture coordinates
 * the corresponding VOLUME_PARAMETERS struct has to be passed .
 * In contrast to textureLookup3D() this function does not normalize the intensity values,
 * in order to deal with 12 bit data sets.
 */
vec4 textureLookup3DUnnormalized(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 texCoords) {
    return getVoxelUnnormalized(volume, volumeParameters, texCoords);
}


