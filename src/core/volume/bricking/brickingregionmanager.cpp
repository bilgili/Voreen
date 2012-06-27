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

#include "voreen/core/volume/bricking/brickingregionmanager.h"

namespace voreen {

    BrickingRegionManager::BrickingRegionManager(BrickingInformation& brickingInformation)
        : brickingInformation_(brickingInformation) {

        std::vector<Brick*> volumeBricks = brickingInformation_.volumeBricks;

        for (size_t i=0; i<volumeBricks.size(); i++) {
            bricksWithoutRegion_.push_back(volumeBricks.at(i));
        }

        brickingRegions_ = std::vector<BrickingRegion*>();
    }

    void BrickingRegionManager::addRegion(BrickingRegion* region) {

        int priority = region->getPriority();

        removeBricksFromVector(region);

        if (brickingRegions_.size() == 0) {
            brickingRegions_.push_back(region);
        } else {
            for (size_t i=0; i<brickingRegions_.size(); i++) {
                if (priority > brickingRegions_.at(i)->getPriority()) {
                    brickingRegions_.insert(brickingRegions_.begin() + i, region);
                    break;
                }
            }
        }
    }

    void BrickingRegionManager::deleteRegion(BrickingRegion* region) {
        for (size_t i=0; i<brickingRegions_.size(); i++) {
            if (brickingRegions_.at(i) == region) {
                delete brickingRegions_.at(i);
                brickingRegions_.erase(brickingRegions_.begin() + i);
                break;
            }
        }
    }

    void BrickingRegionManager::deleteRegions(int priority) {

        size_t i=0;
        while (i < brickingRegions_.size() ) {
            if (brickingRegions_.at(i)->getPriority() == priority) {
                delete brickingRegions_.at(i);
                brickingRegions_.erase(brickingRegions_.begin() + i);
                i--;
            }
            i++;
        }
    }

    std::vector<BrickingRegion*> BrickingRegionManager::getBrickingRegions() {
        return brickingRegions_;
    }

    BrickingRegionManager::~BrickingRegionManager() {
        for (size_t i=0; i < brickingRegions_.size(); i++) {
            delete brickingRegions_.at(i);
        }
    }

    void BrickingRegionManager::removeBricksFromVector(BrickingRegion* region) {
        std::vector<Brick*> bricks = region->getBricks();

        for (size_t i=0; i<bricks.size(); i++) {
            Brick* currentBrick = bricks.at(i);

            std::vector<Brick*>::iterator finder = find(bricksWithoutRegion_.begin(),
                bricksWithoutRegion_.end(), currentBrick);

            if (finder != bricksWithoutRegion_.end() ) {
                bricksWithoutRegion_.erase(finder);
            }
        }
    }

    std::vector<Brick*> BrickingRegionManager::getBricksWithoutRegion() {
        return bricksWithoutRegion_;
    }



}

