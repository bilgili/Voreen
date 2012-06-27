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

#include "voreen/core/datastructures/volume/bricking/standardbrickresolutioncalculator.h"
#include "voreen/core/datastructures/volume/bricking/brickingregionmanager.h"

namespace voreen {

    StandardBrickResolutionCalculator::StandardBrickResolutionCalculator(BrickingInformation &brickingInformation)
        : BrickResolutionCalculator(brickingInformation)
    {
    }

    void StandardBrickResolutionCalculator::calculateBrickResolutions() {
        std::vector<int> result;

        /*int numBricksWithoutRegion = 0;
        if (brickingInformation_.regionManager) {
            if (brickingInformation_.regionManager->getBrickingRegions().size() != 0) {
                numBricksWithoutRegion = brickingInformation_.regionManager->getBricksWithoutRegion().size();
            }
        }*/

        //Get the number of bricks we actually have to calculate lods for. Empty
        //bricks always get the lowest possible lod and therefor mustn't be considered
        //in the calculation.
        int numberOfBricks = brickingInformation_.totalNumberOfBricksNeeded
                            - brickingInformation_.numberOfBricksWithEmptyVolumes;
                            //- numBricksWithoutRegion;

        int lodFor64Voxels = brickingInformation_.totalNumberOfResolutions-3;

        int lod64BrickSizeInByte = brickingInformation_.originalVolumeVoxelSizeInByte * 64;

        int resolution = lodFor64Voxels;
        int resolutionBrickSize = lod64BrickSizeInByte;
        int sixteenth = numberOfBricks/16;

        long double availableMemInByte = brickingInformation_.packedVolumeDimensions.x *
                                    brickingInformation_.packedVolumeDimensions.y *
                                    brickingInformation_.packedVolumeDimensions.z *
                                    brickingInformation_.originalVolumeBytesAllocated -
                                    (brickingInformation_.numberOfBricksWithEmptyVolumes*
                                    brickingInformation_.originalVolumeBytesAllocated);

        for (int i=lodFor64Voxels-1; i >=0; i--) {
            int temp = sixteenth*(resolutionBrickSize*8) + (sixteenth*15)*lod64BrickSizeInByte;
            if (temp <=availableMemInByte/*gpuMemorySizeInByte*/) {
                resolution=i;
                resolutionBrickSize*=8;
            } else {
                break;
            }
        }

       int numberOfHighBricks,numberOfLowBricks;
       numberOfHighBricks=0;
       numberOfLowBricks=numberOfBricks;
       bool done=false;

       while(!done) {

           if ( ( (numberOfHighBricks+1) * resolutionBrickSize) +
               ( (numberOfLowBricks-1) * lod64BrickSizeInByte) < availableMemInByte/*gpuMemorySizeInByte*/) {

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
            if (i == resolution)
                result.push_back(numberOfHighBricks);
            else if (i == lodFor64Voxels)
                result.push_back(numberOfLowBricks);
            else
                result.push_back(0);
        }

        //result.at(result.size()-1) +=numBricksWithoutRegion;

        //We also need to calculate how many voxels the packed volume needs to be able to store.
        //This is a simple calculation, one only has to remember to add 1 voxel for every brick with
        //an empty volume (because those always have lowest possible lod = 1 voxel)
        /*brickingInformation_.numberOfVoxelsNeededForPackedVolume = (numberOfHighBricks * resolutionBrickSize) +
                                (numberOfLowBricks * lod64BrickSizeInByte) +
                                (brickingInformation_.numberOfBricksWithEmptyVolumes * 1) ;*/

        brickingInformation_.brickResolutions = result;
    }



} //namespace

