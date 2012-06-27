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

#ifndef VRN_VOLUMEMANAGER_H
#define VRN_VOLUMEMANAGER_H

#include "voreen/core/volume/volume.h"

namespace voreen {

class VolumeHandle;
class BrickedVolumeReader;

/**
* This class is just a wrapper for the BrickingManager.
* The BrickingManager is templated and therefore can't be used
* as an attribute in a class easily. So we do things just like
* VolumeAtomic and Volume do. BrickingManager derives from 
* LargeVolumeManager, and so the LVM can be used as an attribute. 
*/
class LargeVolumeManager : public MessageReceiver{

public:

    /**
    * @param volumeHandle   the VolumeHandle that will contain the BrickedVolume that will be created
    *                       by the BrickingManager (child class of LargeVolumeManager).
    * @param brickedReder   The BrickedVolumeReader that is used to read the VolumeBricks' volume.
    */
	LargeVolumeManager(VolumeHandle* volumeHandle, BrickedVolumeReader* brickedReader);
	
	~LargeVolumeManager();

	virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

    /**
    * Changes the class responsible for calculating the available brick resolutions (aka LODs)
    * when camera position is used for LOD assignment. 
    */
    virtual void changeBrickResolutionCalculator(std::string);

    /**
    * Changes the class responsible for assigning the LODs to the bricks.
    */
    virtual void changeBrickLodSelector(std::string);

    /**
    * Sets whether or not bricks should be updated after coarseness mode has ended. 
    * This only has effect if the camera position is used to assign LODs. Setting this 
    * to "true" causes all bricks to calculate their distance to the camera again,
    * and depending on the result, their LODs might change. If set to false, the bricks
    * will never be updated, no matter how much the camera position is changed. 
    */
	virtual void setUpdateBricks(bool b);

    /**
    * Adds a BoxBrickingRegion to the RegionManager. The BoxBrickingRegion will then have
    * impact on the LOD assignment to bricks, if camera position is used to assign them.
    */
    virtual void addBoxBrickingRegion(int prio, tgt::vec3 clipLLF, tgt::vec3 clipURB);

protected:

private:

};



} //namespace voreen

#endif

