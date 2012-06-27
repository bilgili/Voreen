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

/**
* This file contains the functions necessary to do lookups in the packed texture
* or the index texture. For more information see Kraus and Ertl's paper
* "Adaptive Texture Maps".
*/

/**
* Performs a lookup in the index texture at the position indicated by "lookupPos".
* What this does is retrieving a brick's position in the packed texture from the index
* texture.
*/
vec4 indexVolumeLookup(vec3 lookupPos) {

    //Simply perform the lookup.
    vec4 indexVolumeSample = textureLookup3D(indexVolume_, indexVolumeParameters_,lookupPos);

    // The result has to be converted to [0,1]. That's because when you create the index
    // texture, 256 might be the maximum position in the x direction and should therefor equal
    // 1 in the shader. But the index texture is of the type uint16_t, and therefor has a maximum
    // of 65535, which results in 256 not equaling 1 at all. The following equations take care of that.
    indexVolumeSample.x = indexVolumeSample.x* indexVolumeFactorX_;
    indexVolumeSample.y = indexVolumeSample.y* indexVolumeFactorY_;
    indexVolumeSample.z = indexVolumeSample.z* indexVolumeFactorZ_;

    // This is the scalefactor. Writing , for example, "16.0" to the index texture results in a value
    // far below 1.0 in the shader. In order to retrieve the original value, the scalefactor has to
    // be multiplied by 65535.0 (As the index texture is of the type uint16_t)
    indexVolumeSample.a =indexVolumeSample.a * 65535.0;

    return indexVolumeSample;
}

/**
* Lookup the sample in the packed texture. To do this, one needs the brick's position
* in the packed texture, indicated by the index volume sample.
*/
vec4 normalPackedVolumeLookup(vec3 sample, vec3 brick, vec4 indexVolumeSample) {
    vec3 offset;
    float scaleFactor = 1.0 / indexVolumeSample.a;

    // First, check how far the sample is away from the startpoint of the brick (the llf corner).
    // Then, multiply that distance by the scaleFactor because the brick might have become smaller.
    // After that, multiply by the offsetFactor, which takes care of the fact that the packed texture
    // almost certainly has different dimensions than the original volume, therefore, for example,
    // "0.2" does not mean the same distance in the two textures.
    offset.x = (sample.x-brick.x) * scaleFactor * offsetFactorX_;
    offset.y = (sample.y-brick.y) * scaleFactor * offsetFactorY_;
    offset.z = (sample.z-brick.z) * scaleFactor * offsetFactorZ_;

    vec3 packedVolumeSample;

    //Add that offset to the startpoint of the brick in the packed texture.
    packedVolumeSample.x = indexVolumeSample.x + offset.x;
    packedVolumeSample.y = indexVolumeSample.y + offset.y;
    packedVolumeSample.z = indexVolumeSample.z + offset.z;

    //Now do the lookup.
    vec4 result = textureLookup3D(packedVolume_, packedVolumeParameters_, packedVolumeSample);

    return result;
}

/**
* Do a lookup in the packed texture, but if the sample position is outside the sample boundary
* (the bounding box encompassing all voxels in the brick), move the sample position to the inside
* of the bounding box (i.e. clamp it). This natutrally introduces an error, but eliminates the
* interpolation error that neighboring bricks would cause in the hardware interpolation. See
* Ljung et al.'s paper for more details.
*/
vec4 clampedPackedVolumeLookup(vec3 sample, vec3 brick, vec4 indexVolumeSample) {

    sample = clampPositionToSampleBoundary(brick, sample, indexVolumeSample.a);

    return normalPackedVolumeLookup(sample, brick, indexVolumeSample);
}

/**
* This is basically the same function as above, with the difference that the boundary distance
* is already known and doesn't need to be calculated. Since optimization is necessary to keep
* interblock interpolation as interactive as possible, the two functions weren't merged into one.
* Saving the calculation of the boundary distance is worth it.
*/
vec4 takeClampedSampleFromBlock(vec3 block, vec3 sample, vec4 indexVolumeSample, float boundaryDistance) {

    vec3 clampedSample = clampPositionToSampleBoundary2(block, sample, boundaryDistance);

    return normalPackedVolumeLookup(clampedSample, block, indexVolumeSample);
}
