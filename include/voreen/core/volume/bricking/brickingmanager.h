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

#ifndef VRN_BRICKINGMANAGER_H
#define VRN_BRICKINGMANAGER_H


#include "voreen/core/io/brickedvolumereader.h"
#include "voreen/core/io/ioprogress.h"

#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/volume/volumehandle.h"

#include "voreen/core/volume/bricking/boxbrickingregion.h"
#include "voreen/core/volume/bricking/brickedvolume.h"
#include "voreen/core/volume/bricking/brickedvolumegl.h"
#include "voreen/core/volume/bricking/brickinginformation.h"
#include "voreen/core/volume/bricking/brickingregionmanager.h"
#include "voreen/core/volume/bricking/balancedbrickresolutioncalculator.h"
#include "voreen/core/volume/bricking/cameralodselector.h"
#include "voreen/core/volume/bricking/errorlodselector.h"
#include "voreen/core/volume/bricking/largevolumemanager.h"
#include "voreen/core/volume/bricking/maximumbrickresolutioncalculator.h"
#include "voreen/core/volume/bricking/packingbrickassigner.h"
#include "voreen/core/volume/bricking/rammanager.h"
#include "voreen/core/volume/bricking/volumebrickcreator.h"

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/voreenpainter.h"

#include <math.h>
#include <time.h>

#include "tgt/camera.h"
#include "tgt/gpucapabilities.h"

namespace voreen {

    /**
    * The BrickingManager is the central organizer for bricking. The different
    * parts of bricking are controlled from here. Every bricked volume loaded is managed
    * by a BrickingManager. 
    */
	template<class T>
	class BrickingManager : public LargeVolumeManager {
	public:

        /**
        * @param volumeHandle   The VolumeHandle that will contain the BrickedVolume once the
        *                       BrickingManager has created the packed texture and index texture.
        * @param brickedVolumeReader    The BrickedVolumeReader that is used to read the volume 
        *                               data of single bricks from the bv file. In contrast to
        *                               "normal" VolumeReaders, each BrickedVolume needs its
        *                               own unique instance of a BrickedVolumeReader.
        * @param brickingInformation    The struct containing all the information necessary for
        *                               bricking.
        * @param progess       The IOProgress used to update the loading bar in the GUI.
        */
		BrickingManager(VolumeHandle* volumeHandle, BrickedVolumeReader* brickedVolumeReader,
						BrickingInformation& brickingInformation,IOProgress* progress = 0);

		/**
		* Deletes everything.
		*/
		~BrickingManager();

        /**
        * Changes the class responsible for calculating the available brick resolutions (aka LODs)
        * when camera position is used for LOD assignment. 
        */
		void changeBrickResolutionCalculator(std::string mode);

        /**
        * Changes the class responsible for assigning the LODs to the bricks. If a 
        * CameraLodSelector is used, the distance from the bricks to the camera position 
        * will be used to prioritize the bricks. The the available resolutions calculated by the
        * BrickResolutionCalculator will be assigned (for example, if 500 bricks can get LOD 0 and the
        * remaining 2800 can get LOD 1, the 500 bricks closest to the camera will get LOD 0). 
        * If an ErrorLodSelector is used, the root mean square error between brick resolutions 
        * will be used to assign LODs. In that case, the results of the BrickResolutionCalculator
        * are irrelevant.
        */ 
        void changeBrickLodSelector(std::string selector);

        /**
        * Sets whether or not bricks should be updated after coarseness mode has ended. 
        * This only has effect if the camera position is used to assign LODs. Setting this 
        * to "true" causes all bricks to calculate their distance to the camera again,
        * and depending on the result, their LODs might change. If set to false, the bricks
        * will never be updated, no matter how much the camera position is changed. 
        */
		void setUpdateBricks(bool b);

        /**
        * The standard processMessage function.
        */
		virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

	protected:

        /**
        * Starts the whole bricking process by calculating the dimensions of the packed
        * texture and index texture, creating the VolumeBricks and PackingBricks,
        * filling them with data and so on. After all that is done, a bricked volume is
        * created, that can be used in voreen networks like any other volume. 
        */
        void createBrickedVolume();

		/**
		* Depending on how much texture memory is available, this function
        * calculates the optimal texture dimensions to store the volume data. For example,
        * if 240MB texture memory could be used, this function would calculate the dimensions
        * of a texture holding 240MB of data. This also depends on the voxel type of course.
		*/
		tgt::ivec3 calculateOptimalTextureDims();

		/**
		* Creates the indexVolume, which is nothing else than creating a volume with 
		* the given dimensions. The indexVolume is later filled with values by 
		* updateIndexVolume(..)
		*/
		void createIndexVolume(tgt::ivec3 dimensions);

		/**
		* Updates the index volume by inserting the packBricks location at the 
		* VolumeBricks position
		*/
		void updateIndexVolume(VolumeBrick<T>* volBrick, PackingBrick<T>* packBrick);

		/**
		* Creates the PackingBricks.
        * @param numbricks The number of bricks to create.
        * @param packedVolume The volume into which the PackingBricks will write once they
        * are filled. 
		*/
		void createPackingBricks(tgt::ivec3 numbricks, VolumeAtomic<T>* packedVolume);

		/**
		* Gets the determined level of detail for every VolumeBrick and puts that volume 
        * into an empty PackingBrick.
		*/
		void fillPackingBricks();

		/**
		* Writes to volume data from all PackingBricks in bricksWithData_ to the 
		* packed volume.
		*/
		void writeVolumeDataToPackedVolume();

		/**
		* If some bricks have been assigned new LODs, this function updates the packed
        * volume with the new data.
		*/
		void updatePackedVolume();

		/**
		* Updates the texture of the packed volume with the new data in the updated
		* packed volume. Called by updatePackedVolume()
		*/
		void updatePackedVolumeTexture();

		/**
		* Collects information about the original volume and puts it into
		* brickingInformation_.
		*/ 
		void getBrickingInformation();

        /**
        * If some part of the bricking "pipeline" has changed, like the ResolutionCalculator,
        * or the LODSelector, this function updates the necessary aspects of the bricking
        * process. 
        */
		void updateBricking();

        /**
        * Adds a BoxBrickingRegion to the RegionManager. The BoxBrickingRegion will then have
        * impact on the LOD assignment to bricks, if camera position is used to assign them.
        */
        void addBoxBrickingRegion(int prio, tgt::vec3 clipLLF, tgt::vec3 clipURB);

		/**
		* The PackingBricks subdivide a volume and receive their data from the VolumeBricks later on.
		* They basically only exist to make the packing of the volume data into the packed volume 
        * efficient.
		*/
		std::list<Brick*> packingBricks_;			

		/**
		* These PackingBricks have been given data and a position in the packed volume. 
		* Calling write() for all these generates the packed volume. 
		*/
		std::vector<PackingBrick<T>*> bricksWithData_;	

		/**
		* The level of detail selector used to assign the bricks their
		* level of detail.
		*/
		BrickLodSelector* brickLodSelector_;

		/**
		* The BrickResolutionCalculator used to calculate which resolutions (LODs) are available.
        * This information is then used by the CameraLODSelector for example.
		*/
		BrickResolutionCalculator* brickResolutionCalculator_;

		/**
		* The reader that reads the brick's volume data from the bv file.
		*/
		BrickedVolumeReader* brickedVolumeReader_;

        /**
        * The class which creates the VolumeBricks. The code of that class
        * could also be in the BrickingManager class, but I think this way
        * it is cleaner. (s_rade02)
        */
		VolumeBrickCreator<T>* volumeBrickCreator_;

        /**
        * The class that assigns the VolumeBricks to PackingBricks. The code
        * could also be in the BrickingManager class, but this way it is 
        * cleaner I think, and there might be other ways of assigning the 
        * VolumeBricks later, and this way that is easier to change. 
        */
		PackingBrickAssigner<T>* packingBrickAssigner_;

        /**
        * The RamManager used for controlling the reading of volume data for VolumeBricks from disk.
        * The RamManager makes certain that a threshold of RAM isn't crossed when filling RAM 
        * with voluem data.
        */
        RamManager<T>* ramManager_;

        /**
        * The struct holding all the information neccessary for bricking. This
        * struct is nearly always passed by reference to the different classes
        * involved in bricking in order to keep the amount of parameters needed
        * low. 
        */
		BrickingInformation& brickingInformation_;

        /**
        * Is the current rendering using coarseness? This is needed to determine
        * when to update the packed volume, i.e. when to recalculate brick 
        * resolutions. TODO: This should be done on Mouse Release for example,
        * as not every network has coarseness. 
        */
		bool coarsenessOn_;

		/**
		* Should bricks change their resolution when camera position used to determine
        * their LOD changes?
		*/
		bool updateBricks_;

        /**
        * The VolumeHandle this BrickingManager is part of. The BrickedVolume created
        * by the BrickingManager is put into that VolumeHandle.
        */
		VolumeHandle* volumeHandle_;

        /**
        * The packed volume consisting of all the volume data of all VolumeBricks at
        * different resolutions.
        */
		VolumeAtomic<T>* packedVolume_;

        /**
        * The volume storing the information where a VolumeBricks data can be found
        * in the packed volume.
        */
		Volume4xUInt16* indexVolume_;

        /**
        * The volume used to store the dimensions of the original volume. This is needed
        * because both the packed and index volume have different dimensions than the original
        * volume, but the dimensions of the original volume are needed to create the Entry-Exit
        * points. That's why this volume stores those dimensions, but never allocates memory.
        * It's basically an empty container of the correct size. 
        */
        VolumeAtomic<uint8_t>* eepVolume_;

        /**
        * A Volume holding three different volumes. The eep volume, the packed volume
        * and the index volume. This BrickedVolume will then be used in the network
        * instead of all three volumes seperately. This way old networks don't have to be
        * changed.
        */
		BrickedVolume* brickedVolume_;

        /**
        * The IOPRogress used to update the loading bar in the GUI.
        */
        IOProgress* ioProgress_;

        static const std::string loggerCat_;

	private:


	}; //class 	
	

    template<class T>
    const std::string BrickingManager<T>::loggerCat_("voreen.core.volume.bricking.BrickingManager");

	template<class T>
	BrickingManager<T>::BrickingManager(VolumeHandle* volumeHandle, BrickedVolumeReader* brickedVolumeReader,
										BrickingInformation& brickingInformation,IOProgress* ioProgress)

        : LargeVolumeManager(volumeHandle, brickedVolumeReader),
          brickedVolumeReader_(brickedVolumeReader),
          brickingInformation_(brickingInformation),
          volumeHandle_(volumeHandle),
          ioProgress_(ioProgress)
	{

		packedVolume_ = 0;
		indexVolume_ = 0;
        eepVolume_ = 0;
		brickLodSelector_ = 0;
		brickResolutionCalculator_ = 0;
		volumeBrickCreator_ = 0;
		packingBrickAssigner_ = 0;
        updateBricks_ = false;
		coarsenessOn_ = false; 

        //Fill the brickingInformation_ struct with information necessary for bricking. 
		getBrickingInformation();

        //Now create the bricked volume. 
		createBrickedVolume();
	}

	template<class T>
	void BrickingManager<T>::getBrickingInformation() {

        //This ugly code line calculates how many different resolution levels there are, for example,
        //if the bricksize is 32, there are a 6 different resolution levels (32 16 8 4 2 1). 
        brickingInformation_.totalNumberOfResolutions = static_cast<int> ( ( log( 
            (float)brickingInformation_.brickSize) / log(2.0) ) + 1);

		brickingInformation_.numVoxelsInBrick = brickingInformation_.brickSize * 
            brickingInformation_.brickSize * brickingInformation_.brickSize;

		brickingInformation_.totalNumberOfBricksNeeded = brickingInformation_.numBricks.x * 
														brickingInformation_.numBricks.y * 
														brickingInformation_.numBricks.z; 

		brickingInformation_.originalVolumeVoxelSizeInByte = sizeof(T);

		uint64_t temp = (uint64_t)brickingInformation_.originalVolumeVoxelSizeInByte *
						brickingInformation_.originalVolumeNumVoxels;

		temp = temp / ( (uint64_t)1024 * (uint64_t)1024);

		brickingInformation_.originalVolumeSizeMB = static_cast<int>(temp); 

        LINFO("original volume size: " << brickingInformation_.originalVolumeDimensions << " ("
              << brickingInformation_.originalVolumeSizeMB << " MB)");
        
		tgt::ivec3 bricksize = tgt::ivec3(brickingInformation_.brickSize);
		
		for (int i=0; i<brickingInformation_.totalNumberOfResolutions; i++) {
			brickingInformation_.lodToDimensionsMap.insert(std::pair<int,tgt::ivec3>(i, bricksize));
			bricksize=bricksize / 2;
		}

		brickingInformation_.camera=0;
        brickingInformation_.regionManager = 0;

        if (maxGpuMemory_ == 0) {
            brickingInformation_.gpuAvailableMemory = estimateMaxGpuMemory();
            LINFO("GPU memory available for bricking (estimated): "
                  << brickingInformation_.gpuAvailableMemory << " MB");
        } else {
            brickingInformation_.gpuAvailableMemory = maxGpuMemory_;
            LINFO("GPU memory available for bricking (user-specified): "
                  << brickingInformation_.gpuAvailableMemory << " MB");
        }

        LINFO("brick size: " << brickingInformation_.brickSize);
	}

	template<class T>
	BrickingManager<T>::~BrickingManager() {

        if (brickingInformation_.regionManager != 0) {
            delete brickingInformation_.regionManager;
            brickingInformation_.regionManager = 0;
        }

		std::vector<Brick*> bricks = brickingInformation_.volumeBricks;
		for (size_t i=0; i < bricks.size(); i++) {
			delete bricks.at(i);
		}
        bricks.clear();

        std::list<Brick*> packBricks = brickingInformation_.packingBricks;
        while (packBricks.size() > 0) {
            Brick* currentBrick = packBricks.front();
            packBricks.pop_front();
            delete currentBrick;
        }

        bricks = brickingInformation_.packingBricksWithData;
		for (size_t i=0; i < bricks.size(); i++) {
			delete bricks.at(i);
		}
        bricks.clear();

        delete brickResolutionCalculator_;
        delete volumeBrickCreator_;
        delete packingBrickAssigner_;
        delete brickLodSelector_;
        delete ramManager_;
        delete brickedVolumeReader_;
        
        //TODO: Delete volumehandle too?
	}


	template<class T>
	tgt::ivec3 BrickingManager<T>::calculateOptimalTextureDims() {
	
        uint64_t gpuMemorySizeInByte = brickingInformation_.gpuAvailableMemory * 1024 * 1024;

        uint64_t originalVolumeVoxelSizeInByte =
            (uint64_t)brickingInformation_.originalVolumeVoxelSizeInByte *
            brickingInformation_.originalVolumeNumVoxels;
        
        if (originalVolumeVoxelSizeInByte < (uint64_t)gpuMemorySizeInByte) {
            LINFO("original volume is smaller than memory available for bricking");
            gpuMemorySizeInByte = originalVolumeVoxelSizeInByte;
        }
        LINFO("using " << gpuMemorySizeInByte / (1024*1024) << " MB for bricking");
        
        int voxelSize = brickingInformation_.originalVolumeBytesAllocated;
        int brickSize = brickingInformation_.brickSize;
        int maxDim = GpuCaps.getMax3DTextureSize();

        if ((uint64_t)maxDim*maxDim*maxDim*brickingInformation_.originalVolumeVoxelSizeInByte < gpuMemorySizeInByte) {
            gpuMemorySizeInByte = (uint64_t)maxDim*maxDim*maxDim*brickingInformation_.originalVolumeVoxelSizeInByte;
            LINFO("maximum texture dimension " << maxDim << " reduces usable memory to "
                  << gpuMemorySizeInByte / (1024*1024) << " MB"); 
        }

        int xDim,yDim,zDim;
		xDim = yDim = zDim = brickSize;
        	
		uint64_t memoryUsed = xDim*yDim*zDim * voxelSize;

        //Increase the texture's x-dimension if possible, otherwise try y, then z.
		while (memoryUsed < gpuMemorySizeInByte) {
            if (xDim + brickSize <= maxDim) {
				xDim = xDim+brickSize;
			} else if (yDim + brickSize <= maxDim) {
				yDim = yDim + brickSize;
			} else if (zDim + brickSize <= maxDim) {
				zDim = zDim + brickSize;
            } else {
                std::stringstream s;
                s << "Max 3d texture sizes doesn't allow creation of a texture holding " << gpuMemorySizeInByte << " bytes.";
                LINFO(s.str() );
                break;
            }
		    
            memoryUsed = xDim * yDim * zDim * voxelSize;
        }

        //Increasing the y or z-dimension in the while loop above potentially doubled the
        //texture size, so we have to reduce the y or x-dimension to cope for that. 
        int tempXDim, tempYDim, tempZDim;
        while (memoryUsed >= gpuMemorySizeInByte) {
			tempXDim=xDim;
			tempYDim=yDim;
			tempZDim=zDim;
			bool miniumReached=true;

			if (xDim > brickSize) {
				tempXDim = xDim - brickSize;
				miniumReached=false;
			} else if (yDim > brickSize) {
				tempYDim = yDim - brickSize;
				miniumReached=false;
			} else if (zDim > brickSize) {
				tempZDim = zDim - brickSize;
				miniumReached=false;
			}
			if (miniumReached) {
				break;
			}

			memoryUsed=tempXDim * tempYDim * tempZDim * voxelSize;

			if (memoryUsed >= gpuMemorySizeInByte) {
				xDim = tempXDim;
				yDim = tempYDim;
				zDim = tempZDim;
			}
		}

        return tgt::ivec3(xDim,yDim,zDim);
	}

	template<class T>
	void BrickingManager<T>::createPackingBricks(tgt::ivec3 optimalDimensions, 
        VolumeAtomic<T>* packedVolume) {
	
		int bricksize = brickingInformation_.brickSize;		
		PackingBrick<T>* newBrick;

		for (int xpos=0; xpos < optimalDimensions.x; xpos = xpos+bricksize ) {
			for (int ypos=0; ypos < optimalDimensions.y; ypos = ypos+bricksize ) {
				for (int zpos=0; zpos < optimalDimensions.z; zpos = zpos+bricksize ) {
					
					newBrick = new PackingBrick<T>(tgt::ivec3(xpos, ypos, zpos), tgt::ivec3(bricksize),
												brickingInformation_.packingBricks);

					newBrick->setTargetVolume(packedVolume);

					brickingInformation_.packingBricks.push_back(newBrick);
				}
			}
		}
	}

	template<class T>
	void BrickingManager<T>::fillPackingBricks() {

		std::vector<Brick*> volumeBricks = brickingInformation_.volumeBricks;
		VolumeBrick<T>* currentBrick;

		for (size_t i=0; i< volumeBricks.size(); i++) {
			currentBrick = dynamic_cast<VolumeBrick<T>*>(volumeBricks.at(i));
			packingBrickAssigner_->assignVolumeBrickToPackingBrick(currentBrick);
		}
	}

	template<class T>
	void BrickingManager<T>::writeVolumeDataToPackedVolume() {

		PackingBrick<T>* currentBrick;

		for (size_t i=0;i < brickingInformation_.packingBricksWithData.size(); i++) {
			currentBrick = dynamic_cast<PackingBrick<T>* >(brickingInformation_.packingBricksWithData.at(i));
			currentBrick->setTargetVolume(packedVolume_);
			currentBrick->write();
		}
	}
	
	template<class T>
	void BrickingManager<T>::createIndexVolume(tgt::ivec3 dimensions) {
        if (indexVolume_ != 0) {
            delete indexVolume_;
        }
		indexVolume_ = new Volume4xUInt16(dimensions);
	}

	template<class T>
	void BrickingManager<T>::updateIndexVolume(VolumeBrick<T>* volBrick, PackingBrick<T>* packBrick) {

		int scaleFactor = static_cast<int>( pow(2.f, (int)volBrick->getCurrentLevelOfDetail() ));
		
		tgt::ivec3 indexVolumePosition = volBrick->getPosition() / brickingInformation_.brickSize;
		tgt::ivec4 indexVolumeValue = tgt::ivec4(packBrick->getPosition(),scaleFactor);
		indexVolume_->voxel(indexVolumePosition ) = indexVolumeValue;
	}

	template<class T>
	void BrickingManager<T>::processMessage(Message* msg, const Identifier&) {

		if (msg->id_ == VoreenPainter::cameraChanged_) {
			brickingInformation_.camera = msg->getValue<tgt::Camera*>();
		} 
        else if (msg->id_ == VoreenPainter::switchCoarseness_) {
			coarsenessOn_ = msg->getValue<bool>();
			if (coarsenessOn_ == false) {
				if (updateBricks_) {
                    brickLodSelector_->selectLods();
					updatePackedVolume();
				}
			}
		}
	}

	template<class T>
	void BrickingManager<T>::updatePackedVolume() {

		//change the resolution of the determined blocks and upload them to the 3D texture
		updatePackedVolumeTexture();

		//regenerate the index 3D texture (we could also do a lot of updates, just like with
		//the packed 3D texture, but the index texture is always quite small, this should never
		//take long)
        BrickedVolumeGL* brickedVolumeGL = dynamic_cast<BrickedVolumeGL*>(volumeHandle_->getVolumeGL());
        
        if (brickedVolumeGL) {
		    VolumeGL* indexVolume = brickedVolumeGL->getIndexVolumeGL();

            if (indexVolume) {
		        delete indexVolume;
		        indexVolume=new VolumeGL(indexVolume_);
		        brickedVolumeGL->setIndexVolumeGL(indexVolume);
            }
        }

	}

	template<class T>
	void BrickingManager<T>::updatePackedVolumeTexture() {

		//Get the texture we need to update
        BrickedVolumeGL* brickedVolumeGL = dynamic_cast<BrickedVolumeGL*>(volumeHandle_->getVolumeGL());
		if (!brickedVolumeGL) {
			return;
		}

		const VolumeTexture* packedTexture = brickedVolumeGL->getPackedVolumeGL()->getTexture();

		//a map holding all VolumeBricks that need a new LOD
		std::map<int, std::vector<Brick* > > brickMap;
		//an iterator for that map
		std::map<int, std::vector<Brick* > >::iterator brickMapIterator;
		//a map holding all PackingBricks holding space for a volume of the old LOD that are now free
		std::map<int, std::vector<Brick* > > packBrickMap;
		//an iterator for that map 
		std::map<int, std::vector<Brick* > >::iterator packBrickMapIterator;

		int oldLod, newLod;

		for (size_t i=0; i<brickingInformation_.volumeBricks.size(); i++) {
			
            VolumeBrick<T>* currentBrick = dynamic_cast<VolumeBrick<T>* >(
                brickingInformation_.volumeBricks.at(i));

			if (currentBrick->getLevelOfDetailChanged() ) {
				//If the brick's LOD needs to be changed, get its old and new LOD
				oldLod = currentBrick->getOldLevelOfDetail();
				newLod = currentBrick->getCurrentLevelOfDetail();

				//Get the VolumeBrick's old PackingBrick and put it into the PackingBrick map, because that
				//PackingBrick is now free and can be used again.
				packBrickMapIterator = packBrickMap.find(oldLod);
				if (packBrickMapIterator != packBrickMap.end() ) {
					packBrickMap[oldLod].push_back(currentBrick->getPackingBrick() );
				} else {
					std::vector<Brick* > newPackVector;
					newPackVector.push_back(currentBrick->getPackingBrick() );
					packBrickMap.insert( std::pair<int,std::vector<Brick* > >(oldLod,newPackVector) );
				}

				//Put the VolumeBrick into the VolumeBrick map which holds all VolumeBricks that need
				//a new PackingBrick of a different size assigned. 
				brickMapIterator = brickMap.find(newLod);
				if (brickMapIterator != brickMap.end() ) {
					brickMap[newLod].push_back(currentBrick);
				} else {
					std::vector<Brick* > newVector;
					newVector.push_back(currentBrick);
					brickMap.insert( std::pair<int,std::vector<Brick* > >(newLod,newVector) );
				}

			}
		}

		//Bind the texture to be updated 
		packedTexture->bind();

		//Go through the map of all VolumeBricks needing a new PackingBrick and assign them
		//the PackingBricks of the PackingBrick map (because all those bricks are free)
		brickMapIterator = brickMap.begin();

		while (brickMapIterator != brickMap.end() ) {
			int newLod = brickMapIterator->first;
			//Get all the VolumeBricks needing a new PackingBrick, and all the PackingBricks 
			//that can be used for that. 
			std::vector<Brick* > changedVolumeBricks = brickMap[newLod];
			std::vector<Brick* > freePackBricks = packBrickMap[newLod];

            if (changedVolumeBricks.size() > 0 && freePackBricks.size() > 0) {
			//now assign those packingbricks to the volumebricks
			    for (size_t i=0; i< changedVolumeBricks.size() ; i++) {
				    VolumeBrick<T>* volBrick = dynamic_cast<VolumeBrick<T>* > (changedVolumeBricks.at(i));
				    PackingBrick<T>* packBrick = dynamic_cast<PackingBrick<T>* > (freePackBricks.at(i));

				    //Assign the VolumeBrick's correct volume (that means of the correct LOD) to the
				    //PackingBrick, and tell the VolumeBrick that it has a new PackingBrick.
				    tgt::ivec3 dims = brickingInformation_.lodToDimensionsMap[newLod];
 				    packBrick->setSourceVolume( (T*)volBrick->getLodVolume(newLod), dims );
				    volBrick->setPackingBrick(packBrick);
    				
				    //Update the index texture because the VolumeBrick now has a new place in the packed
				    //volume. 
				    updateIndexVolume(volBrick,packBrick);

				    //At last update the packed texture with the PackingBrick's new content.
				    packBrick->updateTexture(packedTexture);
			    }
            }

			brickMapIterator++;
		}
	}

    template<class T>
    void BrickingManager<T>::changeBrickResolutionCalculator(std::string mode) {

		if (mode == "maximum") {
            delete brickResolutionCalculator_;
            delete brickLodSelector_;
			brickResolutionCalculator_ = new MaximumBrickResolutionCalculator(brickingInformation_);
            brickLodSelector_ = new CameraLodSelector(brickingInformation_);
		} else if (mode == "balanced") {
            delete brickResolutionCalculator_;
            delete brickLodSelector_;
			brickResolutionCalculator_ = new BalancedBrickResolutionCalculator(brickingInformation_);
            brickLodSelector_ = new CameraLodSelector(brickingInformation_);
		} 
		updateBricking();
    }

    template<class T>
    void BrickingManager<T>::changeBrickLodSelector(std::string selector) {
		if (selector == "Camera-based") {
            delete brickLodSelector_;
            brickLodSelector_ = new CameraLodSelector(brickingInformation_);
		} else if (selector == "Error-based") {
            delete brickLodSelector_;
            brickLodSelector_ = new ErrorLodSelector(brickingInformation_);
            setUpdateBricks(false);
		} 
		updateBricking();
    }

	template<class T>
    void BrickingManager<T>::setUpdateBricks(bool b) {
		updateBricks_=b;
    }

	template<class T>
	void BrickingManager<T>::createBrickedVolume() {
		
        brickingInformation_.packedVolumeDimensions = calculateOptimalTextureDims();
		
        brickResolutionCalculator_ = new MaximumBrickResolutionCalculator(brickingInformation_);

		brickResolutionCalculator_->calculateBrickResolutions();

		//The type of the eep volume doesn't matter as no memory will be allocated anyway.
        eepVolume_ = new VolumeAtomic<uint8_t>(brickingInformation_.originalVolumeDimensions,
													brickingInformation_.originalVolumeSpacing,
													brickingInformation_.originalVolumeBitsStored ,false);

		eepVolume_->meta().setBrickSize(brickingInformation_.brickSize);
        eepVolume_->meta().setFileName("bricking eep volume");


        tgt::ivec3 dims = brickingInformation_.packedVolumeDimensions;
        LINFO("size for packed volume: " << dims << " ("
              << (((long)dims.x*(long)dims.y*(long)dims.z) / (1024*1024))
              * brickingInformation_.originalVolumeVoxelSizeInByte<< " MB)");

        
        packedVolume_ = new VolumeAtomic<T>(brickingInformation_.packedVolumeDimensions,
											brickingInformation_.originalVolumeSpacing,
											brickingInformation_.originalVolumeBitsStored);
    
        packedVolume_->meta().setFileName("bricking packed volume");

		createIndexVolume(brickingInformation_.numBricks);

		createPackingBricks(brickingInformation_.packedVolumeDimensions,packedVolume_);

		ramManager_ = new RamManager<T>(brickingInformation_, brickedVolumeReader_, maxMemory_);

		packingBrickAssigner_ = new PackingBrickAssigner<T>(brickingInformation_,indexVolume_);

		volumeBrickCreator_= new VolumeBrickCreator<T> (brickingInformation_.numBricks,
            brickingInformation_.brickSize ,brickingInformation_.originalVolumeDimensions,
            brickingInformation_.originalVolumeSpacing, brickingInformation_.originalVolumeLLF,
            brickingInformation_.originalVolumeURB, ramManager_);

        ioProgress_->setNumSteps(static_cast<int>(brickingInformation_.totalNumberOfBricksNeeded*1.5));
        int bricksCreated = 0;

		VolumeBrick<T>* newBrick = volumeBrickCreator_->createNextBrick();
        bricksCreated++;
        if (ioProgress_)
            ioProgress_->set(bricksCreated);

		while (newBrick != 0) {
            bricksCreated++;
            if (ioProgress_) {
                ioProgress_->set(bricksCreated);
            }
			if (newBrick->getAllVoxelsEqual() == true) {
                //If all voxels are equal in the VolumeBrick, assign it a PackingBrick immediately.
                //The LOD of this brick will never ever change anyway, so we don't have to keep
                //track of the brick. 
				newBrick->setCurrentLevelOfDetail(brickingInformation_.totalNumberOfResolutions -1);
                packingBrickAssigner_->assignVolumeBrickToPackingBrick(newBrick, true, packedVolume_);
			} else {
                //The brick contains meaningful data, put it into the vector. 
				brickingInformation_.volumeBricks.push_back(newBrick);
			}
			newBrick = volumeBrickCreator_->createNextBrick();
		}

        //Until now, only VolumeBricks with only voxels of the same value have been assigned a PackingBrick.
        //And those bricks will never change their resolution, so they can permanently keep their place 
        //in the packed volume. By creating a backup of the remaining PackingBricks one can rearrange
        //the other bricks inside the packed volume freely without interfering with the positions of the
        //bricks containing only voxels of the same value. Only PackingBricks not yet assigned end up 
        //in the backup. 
        packingBrickAssigner_->createPackingBrickBackups();

        BrickingRegionManager* regionManager = new BrickingRegionManager(brickingInformation_);
        brickingInformation_.regionManager = regionManager;
		
        brickLodSelector_ = new ErrorLodSelector(brickingInformation_);
		brickLodSelector_->selectLods();
        ioProgress_->set(static_cast<int>(brickingInformation_.totalNumberOfBricksNeeded*1.5));
		
		fillPackingBricks();
		
		writeVolumeDataToPackedVolume();

        // Immediately free all bricks after the packed volume has been written. This is done
        // to prevent storing three copies in RAM (one in RamManager, one in packed volume, one
        // by OpenGL), this way they are only stored twice.
        ramManager_->freeAll();

		brickedVolume_ = new BrickedVolume(indexVolume_,packedVolume_,eepVolume_);
		volumeHandle_->setVolume(brickedVolume_);
	}

	template<class T>
	void BrickingManager<T>::updateBricking() {

		brickResolutionCalculator_->calculateBrickResolutions();

		brickLodSelector_->selectLods();

        packingBrickAssigner_->deletePackingBricks();

        packingBrickAssigner_->createPackingBricksFromBackup();
		
		fillPackingBricks();
		
        BrickedVolumeGL* brickedVolumeGL = dynamic_cast<BrickedVolumeGL*>(volumeHandle_->getVolumeGL());
		if (!brickedVolumeGL) {
			return;
		}

		const VolumeTexture* packedTexture = brickedVolumeGL->getPackedVolumeGL()->getTexture();

        packedTexture->bind();

        PackingBrick<T>* currentBrick;
        for (size_t i=0; i<brickingInformation_.packingBricksWithData.size(); i++) {
            currentBrick = dynamic_cast<PackingBrick<T>*>(brickingInformation_.packingBricksWithData.at(i));
            currentBrick->updateTexture(packedTexture);
        }

        VolumeGL* indexVolumeGL = brickedVolumeGL->getIndexVolumeGL();
		delete indexVolumeGL;
		indexVolumeGL=new VolumeGL(indexVolume_);
		brickedVolumeGL->setIndexVolumeGL(indexVolumeGL);
	}

    template<class T>
    void BrickingManager<T>::addBoxBrickingRegion(int prio, tgt::vec3 clipLLF, tgt::vec3 clipURB) {

        BoxBrickingRegion* boxRegion = new BoxBrickingRegion(brickingInformation_);
        boxRegion->setBox(clipLLF,clipURB);
        boxRegion->calculateBricksInRegion();
        boxRegion->setPriority(prio);
        brickingInformation_.regionManager->addRegion(boxRegion);
    }

} //namespace voreen

#endif
