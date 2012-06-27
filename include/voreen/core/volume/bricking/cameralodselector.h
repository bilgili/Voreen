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

#ifndef VRN_CAMERALODSELECTOR_H
#define VRN_CAMERALODSELECTOR_H

#include "voreen/core/volume/bricking/bricklodselector.h"

namespace voreen {

	/**
	* This class assigns the levels of detail based on the distance
	* of the VolumeBricks to the current camera position. The
	* farther away the bricks are, the lower their resolution will
	* be. BrickingRegions are also taken into account. The bricks in the region
    * with the highest priority will be assigned a level of detail first, meaning
    * that region priority is more important than camera distance. The available 
    * resolutions that this class can assign must be determined by a 
    * BrickResolutionCalculator before. 
	*/
	class CameraLodSelector : public BrickLodSelector {
	
	public:
		CameraLodSelector(BrickingInformation& brickingInformation);

        /**
        * Assigns a level of detail to all VolumeBricks in the 
        * brickingInformation_.volumeBricks vector, based on their
        * distance to the camera position and whether or not they are in a 
        * BrickingRegion. First, all bricks inside the region with the
        * highest priority are sorted based on their distance and are assigned
        * a level of detail. This repeats until all regions have been
        * passed, and after that, bricks outside the regions are sorted on their
        * distance to the camera. 
        */ 
		virtual void selectLods();

	protected:

        /**
        * Checks if the brick has already been assigned a level of detail. This
        * can happen if a brick is in more than one region. The region with higher
        * priority takes precedence. 
        */
        bool checkAssignPriority(Brick* brick);

		/**
		* A map holding the distances of every brick to the camera position. The float
        * value is the distance, the int value the position of the brick in the 
        * brickingInformation_.volumeBricks vector. 
		*/
		std::multimap<float, int> distanceMap_;

		/**
		* A set holding the distances of all bricks to the camera position.
		* This set is then sorted and the smallest distances are looked up
		* in the distanceMap_, thereby getting the bricks closest to the camera.
		*/
		std::set<float> distanceSet_;

        /**
        * Vector containing all bricks that haven't been assigned a level of detail yet.
        */
        std::vector<Brick*> bricksWithoutLod_;

	private:

	}; //end of class



} //namespace

#endif
