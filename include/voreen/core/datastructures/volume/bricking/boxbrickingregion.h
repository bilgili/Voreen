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

#ifndef VRN_BOXBRICKINGREGION_H
#define VRN_BOXBRICKINGREGION_H

#include "voreen/core/datastructures/volume/bricking/brickingregion.h"

namespace voreen {

    /**
    * This class defines a box inside the volume. Every brick inside that
    * box will be inside the region.
    */
    class BoxBrickingRegion : public BrickingRegion{
    public:

        BoxBrickingRegion(BrickingInformation& brickingInformation);

        /**
        * Define the box by specifying the lower-left-front and upper-right-back corner
        * of the block.
        */
        void setBox(tgt::vec3 boxLLF,tgt::vec3 boxURB);

        /**
        * Calculate all bricks inside the specified box.
        */
        virtual void calculateBricksInRegion();

    protected:
        /**
        * Lower-left-front corner of the box.
        */
        tgt::vec3 boxLLF_;

        /**
        * Upper-right-back corner of the box.
        */
        tgt::vec3 boxURB_;

    private:

    };

}

#endif
