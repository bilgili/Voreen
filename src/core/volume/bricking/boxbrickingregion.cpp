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


#include "voreen/core/volume/bricking/boxbrickingregion.h"

namespace voreen {

    BoxBrickingRegion::BoxBrickingRegion(BrickingInformation& brickingInformation) :
    BrickingRegion(brickingInformation) {

    }

    void BoxBrickingRegion::setBox(tgt::vec3 boxLLF,tgt::vec3 boxURB) {
        boxLLF_ = boxLLF;
        boxURB_ = boxURB;
    }

    void BoxBrickingRegion::calculateBricksInRegion() {

        std::vector<Brick*> volumeBricks = brickingInformation_.volumeBricks;

        for (size_t i=0; i<volumeBricks.size(); i++) {
            Brick* currentBrick = volumeBricks.at(i);
            tgt::vec3 pos = currentBrick->getLlf();

            if ( !(pos.x < boxLLF_.x || pos.y < boxLLF_.y || pos.z < boxLLF_.z ||
                pos.x >= boxURB_.x || pos.y >= boxURB_.y || pos.z >= boxURB_.z))  {

                    bricksInTheRegion_.push_back(currentBrick);
            }
        }
    }


}

