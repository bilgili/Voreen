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

#ifndef VRN_RAMMANAGER_H
#define VRN_RAMMANAGER_H


#include "voreen/core/volume/bricking/brickinginformation.h"

namespace voreen {
	class BrickedVolumeReader;

	/**
	* Manages the RAM while bricking large datasets that wouldn't fit into
	* the RAM otherwise. The bricks of the volume are accessed through this
	* class, and if a brick isn't resident in the RAM, it is loaded from
	* the harddrive. If there isn't enough RAM available for that, another
	* (currently not needed) brick is deleted from RAM. 
	*/
	template<class T>
	class RamManager {
	public:
		RamManager(BrickingInformation& brickingInformation, BrickedVolumeReader* brickedVolumeReader,
            size_t ramSize);

		bool readBrickFromDisk(VolumeBrick<T>* volBrick, size_t lod);

		BrickedVolumeReader* getBrickedVolumeReader();

		void setBrickedVolumeReader(BrickedVolumeReader* brickedVolumeReader);
	
	protected:

		/**
		* Calculates how many bytes will be needed to store the brick in RAM.
		*/
		int getNumBytes(size_t lod);

		/**
		* Deletes bricks from RAM until numBytes have been freed. 
		*/
		bool freeMem(size_t numBytes);

		/**
		* Increases usedRamInByte_ by numBytes. Exception handling will be added.
		*/
		bool increaseUsedRam(size_t numBytes);

		BrickingInformation& brickingInformation_;
		
		int ramSizeInMegaByte_;
		unsigned long ramSizeInByte_;				//This is chosen deliberately as unsigned long
													//to take future RAM sizes into account
		unsigned long usedRamInByte_;

		BrickedVolumeReader* brickedReader_;		//The reader used to read bricks from the disk.

		/**
		* Whenever this threshold is reached (that much ram is allocated)
		* a message is printed in the console. Just for debugging, will be
		* removed later. (s_rade02)
		*/
		//unsigned long ramThresholdInByte_;			

		/**
		* This list logs which lods of which bricks are resident in the RAM
		* at the moment. Every time a lod of a brick is read into the RAM,
		* a pair is inserted at the end of this list, consisting of the brick
		* and the lod. Whenever there isn't enough RAM available, the first
		* element of this list gets deleted, and of course the associated brick and
		* its lod aswell. 
		*/
		std::list<std::pair<VolumeBrick<T>*, size_t> > volumesInRam_;

	private:


	}; //class


	template<class T>
	RamManager<T>::RamManager(BrickingInformation& brickingInformation, BrickedVolumeReader* brickedVolumeReader,
							 size_t ramSize) 
		: brickingInformation_(brickingInformation),
          ramSizeInMegaByte_(ramSize),
          usedRamInByte_(0),
          brickedReader_(brickedVolumeReader)
	{
			ramSizeInByte_ = ramSizeInMegaByte_ * 1024 * 1024;
			//ramThresholdInByte_ = 1048576;
	}

	template<class T>
	BrickedVolumeReader* RamManager<T>::getBrickedVolumeReader() {
		return brickedReader_;
	}

	template<class T>
	void RamManager<T>::setBrickedVolumeReader(BrickedVolumeReader* brickedVolumeReader) {
		brickedReader_ = brickedVolumeReader;
	}


	template<class T>
	bool RamManager<T>::increaseUsedRam(size_t numBytes) {
		bool success = true;
		
		if (usedRamInByte_ + numBytes > ramSizeInByte_) {
			success = freeMem(numBytes);
		}

		if (!success) {
			return false;
		}
		usedRamInByte_ = usedRamInByte_ + numBytes;
		/*if (usedRamInByte_ > ramThresholdInByte_) {
			std::cout << "Now using: " << usedRamInByte_ << "("<<usedRamInByte_/(1024*1024)<<" MB) Ram"<<std::endl;
			ramThresholdInByte_ = usedRamInByte_ + 1048576;
		}*/
		return true;
	}

	template<class T>
	int RamManager<T>::getNumBytes(size_t lod) {
		int numVoxels = brickingInformation_.numVoxelsInBrick / 
				static_cast<int> (pow(8.0f,(float)lod));
		return numVoxels * brickingInformation_.originalVolumeBytesAllocated;
	}

	template<class T>
	bool RamManager<T>::readBrickFromDisk(voreen::VolumeBrick<T> *volBrick, size_t lod) {
		size_t bytes = getNumBytes(lod);

		bool memAllocated = increaseUsedRam(bytes); 
		
		if (!memAllocated) {
			return false;
		}

		tgt::ivec3 dimensions = tgt::ivec3(brickingInformation_.brickSize) / static_cast<int>(pow(2.0f,(float)lod));
		
		int numVoxels = dimensions.x*dimensions.y*dimensions.z;
		int numBytes = numVoxels*sizeof(T);
		
		T* newVolume = new T[numVoxels];
		
		brickedReader_->readBrick(volBrick, (char*)newVolume,numBytes, lod);

		volBrick->addLodVolume((char*)newVolume,lod);

        if (!volBrick->getAllVoxelsEqual() ) {
		    volumesInRam_.push_back(std::pair<VolumeBrick<T>*,size_t> (volBrick,lod) );
        }
		
		return true;

	}

	template<class T>
	bool RamManager<T>::freeMem(size_t numBytes) {
		size_t bytesFreed = 0;
		
		while (bytesFreed < numBytes) {

			if (volumesInRam_.size() == 0) {
				return false; //This means something has gone wrong, we can't free any more RAM
			}
			std::pair<VolumeBrick<T>* , size_t> element = volumesInRam_.front();

			//Delete the volume. This is done in the deleteLodVolume function of
			//the VolumeBrick, so that the brick knows that that lod isn't there anymore
			element.first->deleteLodVolume(element.second);

			//Remove the entry from the list, because it's no longer in RAM.
			volumesInRam_.pop_front();

			int newBytesFreed = brickingInformation_.numVoxelsInBrick / 
				static_cast<int> ( pow(8.0f,(float)element.second) );

			newBytesFreed = newBytesFreed * brickingInformation_.originalVolumeBytesAllocated;
			
			bytesFreed += newBytesFreed;
		}
		usedRamInByte_ = usedRamInByte_ - bytesFreed;
		return true;
	}


} //namespace




#endif
