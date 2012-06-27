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

#ifndef VRN_BRICKINGREGION_H
#define VRN_BRICKINGREGION_H

#include "voreen/core/volume/bricking/brickinginformation.h"

namespace voreen {

    /**
    * This class defines a region inside a bricked volume and returns all bricks
    * inside it. The region also has a priority. Regions are used to determine which
    * bricks get which resolution in bricking. Bricks inside a region with a high priority
    * get a higher resolution. The kind of the region is defined by the subclasses.
    */
    class BrickingRegion {
    public:

        /**
         * @param brickingInformation    Struct containing all the information about
         *                               bricking, including all the VolumeBricks.
         */
        BrickingRegion(BrickingInformation& brickingInformation);
        
        virtual ~BrickingRegion() {}
        
        /**
         * Determines which VolumeBricks lie inside the defined region and
         * stores them in the bricksInTheRegion_ vector.
         */
        virtual void calculateBricksInRegion();

        /**
         * Gets the VolumeBricks inside the region.
         */
        std::vector<Brick*> getBricks();

        /**
         * Sets the priority of this region.
         */
        void setPriority(int priority);

        /**
        * Gets the priority of this region.
        */
        int getPriority();

    protected:

        /**
         * All the information about bricking.
         */
        BrickingInformation& brickingInformation_;

        /**
         * Vector containing all the bricks inside the defined region.
         */
        std::vector<Brick*> bricksInTheRegion_;

        /**
         * The priority of this region.
         */
        int priority_;

    private:

    };

    /**
     * This simplest region of them all. You manually supply a vector of bricks that are to be
     * considered inside the region. 
     */
    class SimpleBrickingRegion : public BrickingRegion {
    public:

        SimpleBrickingRegion(BrickingInformation& brickingInformation, std::vector<Brick*> bricks) 
            :BrickingRegion(brickingInformation) {
            bricksInTheRegion_ = bricks;
        }

    protected:

    private:

    }; //class

}

#endif
