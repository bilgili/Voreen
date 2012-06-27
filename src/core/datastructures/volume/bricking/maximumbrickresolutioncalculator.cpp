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

#include "voreen/core/datastructures/volume/bricking/maximumbrickresolutioncalculator.h"

namespace voreen {

    MaximumBrickResolutionCalculator::MaximumBrickResolutionCalculator(BrickingInformation &brickingInformation)
        : BrickResolutionCalculator(brickingInformation)
    {
    }

    void MaximumBrickResolutionCalculator::calculateBrickResolutions() {
        std::vector<int> result;

        //Get the number of bricks we actually have to calculate lods for. Empty
        //bricks always get the lowest possible lod and therefor mustn't be considered
        //in the calculation.
        int numberOfBricks = brickingInformation_.totalNumberOfBricksNeeded
                            - brickingInformation_.numberOfBricksWithEmptyVolumes;

        //The size in byte that one brick has. This depends on the number of bytes a voxel
        //has (like char or short or tgt::vec3) and the number of voxels in a brick obviously.
        int highestBrickSizeInByte = brickingInformation_.originalVolumeVoxelSizeInByte *
                                     brickingInformation_.numVoxelsInBrick;

        //A brick of lowest resolution always has only one voxel.
        int lowestBrickSizeInByte = brickingInformation_.originalVolumeVoxelSizeInByte;

        long double availableMemInByte = brickingInformation_.packedVolumeDimensions.x *
                                    brickingInformation_.packedVolumeDimensions.y *
                                    brickingInformation_.packedVolumeDimensions.z *
                                    brickingInformation_.originalVolumeBytesAllocated -
                                    (brickingInformation_.numberOfBricksWithEmptyVolumes *
                                     brickingInformation_.originalVolumeBytesAllocated);

        //The maximum resolution possible. That means if even only block of the
        //highest resolution together with only blocks of minimal resolution fit into
        //memory, that is ok. If not, then the resolution below highest resolution is tested.
        //The first resolution that can fit one block into gpu memory becomes the max resolution.
        int maxResolution=0;
        int maxResolutionBrickSizeInByte=highestBrickSizeInByte;

        bool maxResolutionFound=false;

        int i=0;
        while(!maxResolutionFound) {

            maxResolutionBrickSizeInByte = (int)(maxResolutionBrickSizeInByte / pow(8.0,i));

            if ( (maxResolutionBrickSizeInByte * 1 + lowestBrickSizeInByte * (numberOfBricks-1) ) <
                availableMemInByte) {

                maxResolutionFound=true;
                maxResolution=i;
            }
            if (i < brickingInformation_.totalNumberOfResolutions -1)  {
                i++;
            } else {
                break;
            }
        }

        if (maxResolutionFound) {

            int numberOfHighBricks=0;
            int numberOfLowBricks=numberOfBricks;

            bool done=false;

            while(!done) {

                if ( ( (numberOfHighBricks+1) * highestBrickSizeInByte) +
                    ( (numberOfLowBricks-1) * lowestBrickSizeInByte) < availableMemInByte) {

                        if (numberOfHighBricks+1 > numberOfBricks || numberOfLowBricks-1 < 0) {
                            done=true;
                        } else {
                            numberOfHighBricks++;
                            numberOfLowBricks--;
                        }
                } else {
                    done=true;
                }
            }

            //Write the calculated results in the result vector. The number at position 0 means
            //x bricks can have level of detail 0, the number at position 1 means y bricks can have
            //level of detail 1 and so on.
            for (int i=0; i<brickingInformation_.totalNumberOfResolutions; i++) {
                if (i == maxResolution)
                    result.push_back(numberOfHighBricks);
                else if (i == brickingInformation_.totalNumberOfResolutions-1)
                    result.push_back(numberOfLowBricks);
                else
                    result.push_back(0);
            }

        } else {
            //do error handling
        }

        brickingInformation_.brickResolutions = result;

    }

} //namespace

