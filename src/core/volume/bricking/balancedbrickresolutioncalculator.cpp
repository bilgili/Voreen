/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2007 Visualization and Computer Graphics Group, *
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
 * in the file "license.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * information about commercial licencing please contact the authors. *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/volume/bricking/balancedbrickresolutioncalculator.h"
#include "voreen/core/volume/bricking/brickingregionmanager.h"

namespace voreen {

	BalancedBrickResolutionCalculator::BalancedBrickResolutionCalculator(
        BrickingInformation &brickingInformation) 
		: BrickResolutionCalculator(brickingInformation) 
	{
	}

	void BalancedBrickResolutionCalculator::calculateBrickResolutions() {
		std::vector<int> result;

		//Get the number of bricks we actually have to calculate lods for. Empty
		//bricks always get the lowest possible lod and therefor mustn't be considered
		//in the calculation.
		int numberOfBricks = brickingInformation_.totalNumberOfBricksNeeded 
							- brickingInformation_.numberOfBricksWithEmptyVolumes;

		//The size in byte that one brick has. This depends on the number of bytes a voxel
		//has (like char or short or tgt::vec3) and the number of voxels in a brick obviously. 
		int brickSizeInByte = brickingInformation_.originalVolumeVoxelSizeInByte * 
							  brickingInformation_.numVoxelsInBrick;
		
		//We now start calculating which resolutions can be used. For that we take the size 
		//of the entire volume and check which lods would fit. However empty bricks will get lowest
		//resolution anyway, we can be certain of that, and can therefore  substract those.
		float volumeSize = (float)brickingInformation_.originalVolumeSizeMB;

		float temp = static_cast<float>(
			(brickSizeInByte - 1 ) * (brickingInformation_.numberOfBricksWithEmptyVolumes) );
		
        temp = temp / (1024.f * 1024.f);
		volumeSize = volumeSize - temp;

		//We are looking for two resolutions levels right next to each other, this one is the lower one.
		int lowerResolution = 0;
		bool resolutionFound = false;

        long double availableMemInByte = brickingInformation_.packedVolumeDimensions.x * 
                                    brickingInformation_.packedVolumeDimensions.y *
                                    brickingInformation_.packedVolumeDimensions.z *
                                    brickingInformation_.originalVolumeBytesAllocated -
                                    ((brickingInformation_.numberOfBricksWithEmptyVolumes)* 
                                    brickingInformation_.originalVolumeBytesAllocated);

        double availableMemInMegaByte = availableMemInByte / (1024.0 * 1024.0);

		//Go through all available resolutions and check if the bricked volume would fit
		//into GPU memory if we only used bricks of that resolution. We start at resolution
		//level 1.
		for (int i=1; i<=brickingInformation_.totalNumberOfResolutions; i++) {
			
            //Every decrease in resoltion decreases volume size by 1.
			volumeSize = static_cast<float>(volumeSize / 8.0);	
			
            if (volumeSize < availableMemInMegaByte) {
				//If the volume now fits into the gpu memory, we can take this resolution as the lower one. 
				lowerResolution = i;
				resolutionFound = true;
				break;
			}
		}

		if (resolutionFound == false) {
			//Do someting here, that means the dataset can't be shrinked enough to fit on the gpu.
			//Should never happen though.
		} else {
			
            //The higher resolution is one level above the lower one obviously. 
			int higherResolution = lowerResolution - 1;

			//The sizes of bricks of our lower and higher resolution.
			int lowResolutionBrickSize = static_cast<int> (brickSizeInByte / pow(8.0,lowerResolution));
			int highResolutionBrickSize = static_cast<int> (lowResolutionBrickSize * 8.0);

			int numberOfHighBricks,numberOfLowBricks;

            numberOfHighBricks=0;
            numberOfLowBricks=numberOfBricks;

            bool done = false;

            while(!done) {

                if ( ( (numberOfHighBricks+1) * highResolutionBrickSize) + 
                    ( (numberOfLowBricks-1) * lowResolutionBrickSize) < availableMemInByte) {

                        if (numberOfHighBricks+1 > numberOfBricks || numberOfLowBricks-1 < 0) {
                            //There are no more bricks to give a higher/lower resolution, so 
                            //we're done
                            done=true;
                        } else {
                            numberOfHighBricks++;
                            numberOfLowBricks--;
                        }
                } else {
                    //Increasing the number of high bricks would consume too much memory, so
                    //we're done.
                    done=true;
                }
            }

            //Write the calculated results in the result vector. The number at position 0 means
			//x bricks can have level of detail 0, the number at position 1 means y bricks can have
			//level of detail 1 and so on. 
			for (int i=0; i<brickingInformation_.totalNumberOfResolutions; i++) {
				if (i == higherResolution)
					result.push_back(numberOfHighBricks);
				else if (i == lowerResolution)
					result.push_back(numberOfLowBricks);
				else
					result.push_back(0);
			}
		}

		brickingInformation_.brickResolutions = result;
	}

} //namespace

