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
* Converts a position from block coordinates (e.g. 7.3 € [0, 8]) to a position
* in normal coordinates (e.g. 0.9125 € [0,1] ).
*/
vec3 convertToNormalCoordinates(vec3 blockCoords) {
    vec3 normalCoords;
    normalCoords.x = blockCoords.x / numbricksX_;
    normalCoords.y = blockCoords.y / numbricksY_;
    normalCoords.z = blockCoords.z / numbricksZ_;

    return normalCoords;
}

/**
* Converts a position from normal coordinates (e.g. 0.9125 € [0,1] )
* to block coordinates (e.g. 7.3 € [0, 8]).
*/
vec3 convertToBlockCoordinates(vec3 normalCoords) {
    vec3 blockCoords;
    blockCoords.x = normalCoords.x*numbricksX_;
    blockCoords.y = normalCoords.y*numbricksY_;
    blockCoords.z = normalCoords.z*numbricksZ_;

    return blockCoords;
}

/**
* Calculates the position of the lower-left-front brick to the given
* block coordinates. For example, the LLF brick to (4.3, 3.9, 2.1)
* is (3.0, 3.0, 1.0).
*/
vec3 calculateLowerLeftFrontBlock(vec3 blockCoords) {

    vec3 temp = vec3(blockCoords - 0.5);
    vec3 temp2 = vec3(numbricksX_, numbricksY_, numbricksZ_);

    vec3 result = floor( clamp( temp, vec3(0.0), temp2) );
    return result;
}

/**
* Creates a "local" coordinate system by putting the block coordinates
* into relation to the lower-left-front block's location. This way coordinates
* relative to the center of the eight neighboring bricks are obtained. (For example
* <0.3, 0.1, -0.5> € [-1, 1]^3, where 1.0 means one brick, not the maximum of a normal
* texture). These coordinates are needed for the interblock interpolation to weigh
* the edges between bricks.
*/
vec3 calculateLocalCoordinates( vec3 blockCoords, vec3 lowerLeftFrontBlock) {
    vec3 coords;
    coords.x = blockCoords.x - lowerLeftFrontBlock.x - 1.0;
    coords.y = blockCoords.y - lowerLeftFrontBlock.y - 1.0;
    coords.z = blockCoords.z - lowerLeftFrontBlock.z - 1.0;

    return coords;
}

/**
* Calculates in which brick a given sample resides.
*/
vec3 getBrick(vec3 sample) {
    vec3 brick;

    brick.x = temp5 * floor( sample.x * numbricksX_);
    brick.y = temp6 * floor( sample.y * numbricksY_);
    brick.z = temp7 * floor( sample.z * numbricksZ_);

    vec3 boundary = vec3(boundaryX_, boundaryY_, boundaryZ_);
    return clamp(brick, vec3(0.0), boundary);


}

/**
* Calculates the coordinates for the lookup in the index volume.
* Ideally this would not be neccessary and we could simply make the
* lookup at the brick, but if you have a 2D Texture with
* 4 texels, texel 1 would be at (0.25,0.25) and not (0,0) (which
* would be the brick). That's why we have to make this
* adjustment.
*/
vec3 getIndexVolumePosition(vec3 brick) {
    vec3 result;
    result.x = brick.x + temp1; //temp1 = 1.0f / (2.0f * numbricksX_);
    result.y = brick.y + temp2; //temp2 = 1.0f / (2.0f * numbricksY_);
    result.z = brick.z + temp3; //temp3 = 1.0f / (2.0f * numbricksZ_);

    return result;
}

/**
* Gets the boundary distance of a given block, meaning the distance
* of the texels inside a brick to the boundary/edge of that brick. This
* distance is needed to move the sample position to the sample boundary
* when using intrablock interpolation, also for taking samples when using
* interblock interpolation. As that distance only depends on the scalefactor
* of the brick, only the scalefactor is passed as a parameter.
*/
float getBoundaryDistanceOfBlock(float scalefactor) {
    float distance = temp4 * scalefactor; //temp4 = 1.0f / (2.0f * bricksize);
    return distance;
}

/**
* Checks if the given position in normal coordinates ( [0,1] ) is inside the
* volume or not. IMPORTANT: As this function is used to test whether the
* lower-left-front corner of a brick is inside a volume, 1.0 actually does
* NOT lie inside the volume, as a brick starting at 1.0 would not be inside
* the volume.
*
* If you ask yourself why this test is done so strangely, if-else statements
* are VERY expensive and slow the interpolation down significantly. That's why
* the glsl shader function "stop(..)" is used to perform the actual test.
* Multiplication of the results yields 0 if the coordinates are not inside the
* volume.
*/
float isInsideVolume(vec3 volumeCoordinates) {

    vec3 smaller = step(vec3(0.0), volumeCoordinates);
    vec3 bigger = vec3(1.0) - step( vec3(1.0), volumeCoordinates);

    return smaller.x* smaller.y* smaller.z* bigger.x* bigger.y* bigger.z;
}

/**
* Clamps the position to the inside of the bounding box around the samples in a brick.
* The distance between the brick boundary and the bounding box around the samples
* (the sample boundary) is passed as a parameter. This is the only difference to the
* "clampPositionToSampleBoundary" function. Both functions do exactly the same and could
* be merged to one function, however, sometimes one can save the calculation of the
* boundary distance (because it might be already known) and call this function here directly,
* thus saving rendering time. If you know a better way to name the two functions, go right
* ahead :)
*/
vec3 clampPositionToSampleBoundary2(vec3 brick, vec3 position, float boundaryDistance) {

    vec3 temp = vec3(temp5, temp6, temp7);  //temp5 = 1.0f / numbricksX_ etc
    vec3 distanceInBlock;
    vec3 lowerBoundary;
    vec3 upperBoundary;

    /*We need to clamp the position to [brick+boundaryDistance, brick+1.0 - boundaryDistance],
    but the boundaryDistance is not yet converted to global coordinates. A value
    of e.g. 0.5 means the distance is half a block. The following calculation converts
    this value to normal coordinates.*/
    distanceInBlock = boundaryDistance * temp;

    //We have the right distance, now setup the boundaries for clamping.
    lowerBoundary = brick + distanceInBlock;
    upperBoundary = brick + temp - distanceInBlock;

    return clamp(position, lowerBoundary, upperBoundary);
}

/**
* Clamps the position to the inside of the bounding box around the samples in a brick.
* The distance between the brick boundary and the bounding box around the samples
* (the sample boundary) can be calculated by the scalefactor. This is the only difference to the
* "clampPositionToSampleBoundary2" function. Both functions do exactly the same and could
* be merged to one function, however, sometimes one can save the calculation of the
* boundary distance (because it might be already known) and call the other function directly,
* thus saving rendering time. If you know a better way to name the two functions, go right
* ahead :)
*/
vec3 clampPositionToSampleBoundary(vec3 blockStart, vec3 pos, float scalefactor) {
    float distance = temp4 * scalefactor;

    return clampPositionToSampleBoundary2(blockStart, pos, distance);
}


/**
* Checks if the position is inside the sample boundary.
*/
bool insideSampleBoundary(vec3 brick,vec3 pos, float scalefactor) {

    float distance = temp4 * scalefactor;

    vec3 result;
    result = clampPositionToSampleBoundary2(brick, pos, distance);

    return ( (result.x == pos.x) && (result.y == pos.y) && (result.z == pos.z) );

}
