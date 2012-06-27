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

#ifndef VRN_VOLUMEBRICKCREATOR_H
#define VRN_VOLUMEBRICKCREATOR_H

#include "voreen/core/volume/bricking/brickinginformation.h"
#include "voreen/core/volume/bricking/brick.h"
#include "voreen/core/volume/bricking/rammanager.h"

namespace voreen {

    /**
    * Creates VolumeBricks. The VolumeBrickCreator is initialised with the 
    * parameters of the original volume for which VolumeBricks should be
    * created, like dimensions, spacing etc. You have to use createNextBrick() 
    * until all bricks are created. The reason for not creating all bricks at 
    * once is that really large volumes need very many VolumeBricks, and creating
    * all of them at once might need too much memory. VolumeBricks holding volumedata
    * in which every voxel is the same, are for example directly deleted after writing
    * their data to the packed texture, to avoid this problem. 
    * 
    */
	template<class T>
	class VolumeBrickCreator {

	public:

        /**
        * All parameters are required. 
        */
		VolumeBrickCreator(tgt::ivec3 numberOfBricksToCreate, int brickSize, 
            tgt::ivec3 originalVolumeDimensions, tgt::vec3 originalVolumeSpacing, 
            tgt::vec3 originalVolumeLLF, tgt::vec3 originalVolumeURB, 
            RamManager<T>* ramManager);


        /**
        * Creates the next VolumeBrick. VolumeBricks are first created
        * in x-direction, then y, then z. When all bricks are created,
        * 0 is returned. 
        */
		VolumeBrick<T>* createNextBrick();

	protected:
		tgt::ivec3 numberOfBricksToCreate_; //The number of bricks to create in each dim.
		int brickSize_;                     //The size of the bricks to create
		tgt::ivec3 originalVolumeDimensions_;
		tgt::vec3 originalVolumeSpacing_; 
		tgt::vec3 originalVolumeLLF_;
		tgt::vec3 originalVolumeURB_;
		tgt::vec3 originalVolumeAbsoluteDimensions_; //URB-LLF
		tgt::vec3 absolutePosition_;        //The position in camera coordinates
		RamManager<T>* ramManager_;         //The RamManager given to the created bricks
		BrickedVolumeReader* brickedVolumeReader_;  //Automatically extracted from the RamManager

		tgt::ivec3 numberOfCreatedBricks_;  //Counts the bricks that are created
		bool allBricksCreated_;             //Are all bricks created? 
		int xpos_;                          //Helper variables to determine the bricks position. 
		int ypos_;
		int zpos_;
	private:


	}; //class

    /*
    * Nothing special here, just initialising the variables with the parameters.
    */
	template<class T>
	VolumeBrickCreator<T>::VolumeBrickCreator(tgt::ivec3 numberOfBricksToCreate, 
        int brickSize, tgt::ivec3 originalVolumeDimensions, tgt::vec3 originalVolumeSpacing, 
        tgt::vec3 originalVolumeLLF, tgt::vec3 originalVolumeURB, RamManager<T>* ramManager) {

		    numberOfBricksToCreate_ = numberOfBricksToCreate;
		    brickSize_ = brickSize;
		    originalVolumeDimensions_ = originalVolumeDimensions;
		    originalVolumeSpacing_ = originalVolumeSpacing;
		    originalVolumeLLF_ = originalVolumeLLF;
		    originalVolumeURB_ = originalVolumeURB;
		    originalVolumeAbsoluteDimensions_ = abs( originalVolumeURB_ - originalVolumeLLF_ );
		    absolutePosition_ = tgt::vec3(0.0);
		    ramManager_ = ramManager; 
		    numberOfCreatedBricks_ = tgt::ivec3(0);
		    allBricksCreated_ = false;
		    xpos_=0;
		    ypos_=0;
		    zpos_=0;
		    brickedVolumeReader_ = ramManager_->getBrickedVolumeReader();
	}


	template<class T>
	VolumeBrick<T>* VolumeBrickCreator<T>::createNextBrick() {
		
        if (allBricksCreated_) {
			return 0;
		}
			
        //Calculate the position of the brick that is to be created. Meaning
        //something like <512,320,256> in a originally 768^3 volume. 
		xpos_=numberOfCreatedBricks_.x * brickSize_;
		ypos_=numberOfCreatedBricks_.y * brickSize_;
		zpos_=numberOfCreatedBricks_.z * brickSize_;

        //Calculate the bricks position in world coordinates. 
		absolutePosition_.x = originalVolumeLLF_.x + ( ( (float)xpos_ / originalVolumeDimensions_.x ) 
            * originalVolumeAbsoluteDimensions_.x* originalVolumeSpacing_.x);

		absolutePosition_.y = originalVolumeLLF_.y + ( ( (float)ypos_ / originalVolumeDimensions_.y ) 
            * originalVolumeAbsoluteDimensions_.y* originalVolumeSpacing_.y);

		absolutePosition_.z = originalVolumeLLF_.z + ( ( (float)zpos_ / originalVolumeDimensions_.z ) 
            * originalVolumeAbsoluteDimensions_.z* originalVolumeSpacing_.z);

        //Create the brick
		VolumeBrick<T>* newBrick = new VolumeBrick<T>(tgt::ivec3(xpos_,ypos_,zpos_),
            absolutePosition_,tgt::ivec3(brickSize_));

        //Read the position from the info file where the bricks volumedata can be found in
        //the data file. Also checks if all voxels are the same in this brick.
		brickedVolumeReader_->readBrickPosition(newBrick);

        //Set the RamManager for this brick
		newBrick->setRamManager(ramManager_);

        //Check if all bricks are created
		numberOfCreatedBricks_.x++;
		if (numberOfCreatedBricks_.x >= numberOfBricksToCreate_.x) {
			numberOfCreatedBricks_.y++;
			numberOfCreatedBricks_.x = 0;
			if (numberOfCreatedBricks_.y >= numberOfBricksToCreate_.y) {
				numberOfCreatedBricks_.z++;
				numberOfCreatedBricks_.y = 0;
				if (numberOfCreatedBricks_.z >= numberOfBricksToCreate_.z) {
					allBricksCreated_ = true;
				}
			}
		}
		return newBrick;
	}

} //namespace

#endif
