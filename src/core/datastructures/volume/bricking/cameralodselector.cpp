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

#include "tgt/camera.h"

#include "voreen/core/datastructures/volume/bricking/cameralodselector.h"
#include "voreen/core/datastructures/volume/bricking/brickingregionmanager.h"
#include "voreen/core/datastructures/volume/bricking/boxbrickingregion.h"
#include <time.h>

namespace voreen {

    CameraLodSelector::CameraLodSelector(BrickingInformation &brickingInformation)
        : BrickLodSelector(brickingInformation)
    {
    }

    void CameraLodSelector::selectLods() {
        bricksWithoutLod_.clear();
        bricksWithoutLod_ = std::vector<Brick*>(brickingInformation_.volumeBricks);

        int currentLevelOfDetail = 0;               // The lod we are currently assigning to bricks.
        int maxNumberOfCurrentLODBlocks = 0;        // The number of bricks that can be given that lod.
        int numberOfBlocksOfCurrentLODAssigned = 0; // The number of bricks that have that lod assigned.

        //In brickResolutions there is the information about how many bricks can be given which lod.
        for (size_t i=0; i<brickingInformation_.brickResolutions.size(); i++) {
            if (brickingInformation_.brickResolutions.at(i) != 0) {
                currentLevelOfDetail = i;
                maxNumberOfCurrentLODBlocks = brickingInformation_.brickResolutions.at(i);
                break;
            }
        }

        //Get all the regions that are defined. Regions with the higher priorities are assigned
        //the higher available levels of detail.
        std::vector<BrickingRegion*> regions = brickingInformation_.regionManager->getBrickingRegions();

        //Get the bricks that are not inside any region and put them inside a pseudo region.
        std::vector<Brick*> bricksWithoutRegion = brickingInformation_.regionManager->getBricksWithoutRegion();
        SimpleBrickingRegion* bricksWithoutRegionRegion = new SimpleBrickingRegion(brickingInformation_,
            bricksWithoutRegion);

        bricksWithoutRegionRegion->setPriority(-99999);
        regions.push_back(bricksWithoutRegionRegion);

        tgt::vec3 cameraPos;
        if (!brickingInformation_.camera)
            cameraPos = tgt::vec3(0.0,0.0,3.75);
        else
            cameraPos = brickingInformation_.camera->getPosition();

        //Regions at the beginning of the vector have a higher priority, so assign the higher LODs to
        //them.
        for (size_t i=0; i<regions.size(); i++) {
            distanceSet_.clear();
            distanceMap_.clear();

            std::vector<Brick*> bricks = regions.at(i)->getBricks();

            for (size_t j=0; j<bricks.size(); j++) {

                if (checkAssignPriority(bricks.at(j) ) == true ) {
                    tgt::vec3 brickPos = bricks.at(j)->getLlf();
                    float dist = tgt::lengthSq(brickPos - cameraPos);

                    //Insert the distance into the distance set. We use a set because no
                    //distance can occur more then once that way.
                    distanceSet_.insert(dist);

                    //Insert the distance together with the volume brick number into the
                    //distance multimap. That way we now the volumebrick numbers for every
                    //distance.
                    distanceMap_.insert(std::pair<float,int>(dist,j));
                }
            }

            //Typical iterator declaration for the multimap.
            typedef std::multimap<float,int>::const_iterator Iterator;

            //Now the iterator for the set.
            std::set<float>::iterator distanceIterator = distanceSet_.begin();


            //Go through the entire set, starting with the smallest distance.
            while (distanceIterator != distanceSet_.end() ) {
                float currentDistance;

                //Get the smallest distance.
                currentDistance = *(distanceIterator);

                //This looks a bit..bad, but as it is a multimap, we have to deal with that. We get a pair
                //of iterators, the first one at the beginning of all entrys with the current distance, the second
                //one at the end.
                const std::pair<Iterator,Iterator> blocks = distanceMap_.equal_range(currentDistance);

                //Start at the first iterator, stop at the second, thereby going through all entrys with
                //the current distance as first value. The iterators point to a pair, the first value the distance,
                //the second one the number of the volume brick that has that distance.
                for (Iterator it = blocks.first; it!= blocks.second; ++it) {

                    //Check if the volume brick isn't empty.
                    if (!bricks.at(it->second)->getAllVoxelsEqual()) {

                        //The brick isn't empty, so we assign the correct lod.
                        bricks.at(it->second)->setCurrentLevelOfDetail(currentLevelOfDetail);
                        numberOfBlocksOfCurrentLODAssigned++;    //Keep track of how many bricks have that lod.

                        //Check if we have assigned this lod to as many bricks as we were allowed to.
                        if (numberOfBlocksOfCurrentLODAssigned >= maxNumberOfCurrentLODBlocks) {
                            //If we have, start assigning the next lod.
                            currentLevelOfDetail++;

                            //Search the next lod we can assign to bricks, meaning the value in the vector is not 0
                            for ( ; (size_t)currentLevelOfDetail < brickingInformation_.brickResolutions.size(); currentLevelOfDetail++){
                                if (brickingInformation_.brickResolutions.at(currentLevelOfDetail) != 0) {
                                    maxNumberOfCurrentLODBlocks = brickingInformation_.brickResolutions.at(currentLevelOfDetail);
                                    numberOfBlocksOfCurrentLODAssigned=0;
                                    break;
                                }
                            }
                        }
                    } else {
                        //The volume brick has an "empty" volume, meaning all voxels have the same value, and
                        //therefore we can assign the lowest lod possible.
                        size_t lowestLod = brickingInformation_.brickResolutions.size()-1;
                        bricks.at(it->second)->setCurrentLevelOfDetail(lowestLod);
                    }
                }

                //We have a lod assigned to all bricks with the current distance, so go to the
                //next distance.
                ++distanceIterator;

            } //distance iterator

        } //region loop

    } //function

    bool CameraLodSelector::checkAssignPriority(Brick* brick) {
        std::vector<Brick*>::iterator finder =
            std::find(bricksWithoutLod_.begin(),bricksWithoutLod_.end(),brick);

        if (finder != bricksWithoutLod_.end()) {
            bricksWithoutLod_.erase(finder);
            return true;
        } else {
            return false;
        }

    }

} //namespace
