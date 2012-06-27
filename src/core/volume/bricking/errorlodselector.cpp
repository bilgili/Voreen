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

#include "voreen/core/volume/bricking/errorlodselector.h"

namespace voreen {

	ErrorLodSelector::ErrorLodSelector(BrickingInformation &brickingInformation) 
		: BrickLodSelector(brickingInformation) 
	{

        voxelSizeInByte_ = brickingInformation_.originalVolumeVoxelSizeInByte;

        availableMemoryInByte_  = brickingInformation_.packedVolumeDimensions.x * 
                                    brickingInformation_.packedVolumeDimensions.y *
                                    brickingInformation_.packedVolumeDimensions.z *
                                    brickingInformation_.originalVolumeBytesAllocated -
                                    (brickingInformation_.numberOfBricksWithEmptyVolumes* 
                                    brickingInformation_.originalVolumeBytesAllocated);

        usedMemoryInByte_ = brickingInformation_.totalNumberOfBricksNeeded * voxelSizeInByte_;
	}

    void ErrorLodSelector::selectLods() {

        initializeErrorSet();
        bool finished = false;

        std::set<ErrorStruct, std::greater<ErrorStruct> >::iterator errorIterator;
        
        while (!finished) {
            if (errorSet_.begin() == errorSet_.end() ){
                finished = true;
                break;
            }

            errorIterator = errorSet_.begin();

            ErrorStruct currentErrorStruct;
            currentErrorStruct.brick= errorIterator->brick;
            currentErrorStruct.improvementPerByte = errorIterator->improvementPerByte;
            currentErrorStruct.memRequiredForImprovement = errorIterator->memRequiredForImprovement;
            currentErrorStruct.numVoxels = errorIterator->numVoxels;
            currentErrorStruct.nextLod = errorIterator->nextLod;
            errorSet_.erase(errorSet_.begin());

            if (currentErrorStruct.memRequiredForImprovement + usedMemoryInByte_ > availableMemoryInByte_) {

            } else {

                usedMemoryInByte_ = usedMemoryInByte_ + currentErrorStruct.memRequiredForImprovement;

                int newLod = currentErrorStruct.nextLod;
                int currentLod = currentErrorStruct.brick->getCurrentLevelOfDetail();

                currentErrorStruct.numVoxels = static_cast<int>(
                    currentErrorStruct.numVoxels * pow(8.0,currentLod-newLod));

                currentErrorStruct.brick->setCurrentLevelOfDetail(newLod);

                calculateNextImprovement(currentErrorStruct);
            }

        }
	}

    void ErrorLodSelector::initializeErrorSet() {
        errorSet_.clear();

        int lowestLod = brickingInformation_.totalNumberOfResolutions-1;

        for (size_t i=0; i<brickingInformation_.volumeBricks.size(); i++) {
            Brick* currentBrick = brickingInformation_.volumeBricks.at(i);
            currentBrick->setCurrentLevelOfDetail(lowestLod);    

            ErrorStruct errorStruct;
            errorStruct.brick = currentBrick;
            errorStruct.numVoxels = 1;
            
            calculateNextImprovement(errorStruct);
        }
    }

   void ErrorLodSelector::calculateNextImprovement(ErrorStruct errorStruct) {
        int currentLod = errorStruct.brick->getCurrentLevelOfDetail();
        float currentError = errorStruct.brick->getError(currentLod);

        int newLod = -1;
        float betterError = 0;
        float tempError = -1;
        float bestImprovement = 0.0;

        //Calculate which improvement (improvement to which LOD) produces the highest
        //error reduction per byte. 
        for (int j=currentLod-1; j>=0; j--) {
            tempError = errorStruct.brick->getError(j);

            if (tempError < currentError) {
                int memRequiredForImprovement = static_cast<int> (
                    ((errorStruct.numVoxels * pow(8.0, currentLod - j) ) - 
                    errorStruct.numVoxels)* voxelSizeInByte_);
                
                float improvementPerByte = (currentError-tempError) / memRequiredForImprovement;

                if (improvementPerByte > bestImprovement) {
                    newLod = j;
                    betterError = tempError;
                    bestImprovement = improvementPerByte;
                }
            }
        }

        if (newLod != -1) {

            errorStruct.memRequiredForImprovement = static_cast<int>(
                ((errorStruct.numVoxels * pow(8.0,currentLod-newLod) )-
                errorStruct.numVoxels)* voxelSizeInByte_);

            errorStruct.improvementPerByte = (currentError - betterError) /
                errorStruct.memRequiredForImprovement;
           
            errorStruct.nextLod = newLod;
            
            errorSet_.insert(errorStruct);
        }
    }

} //namespace
