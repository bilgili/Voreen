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

#ifndef VRN_BRICKRESOLUTIONCALCULATOR_H
#define VRN_BRICKRESOLUTIONCALCULATOR_H

#include "voreen/core/datastructures/volume/bricking/brickinginformation.h"

namespace voreen {

    /**
    * This class is the superclass to all classes that implement
    * the calculation of the brick resolutions used in the bricking of
    * large volumes. Each class implementing that calculation receives
    * all the information it needs by the BrickingInformation struct,
    * including information about the available gpu memory and so on. The
    * calculated resolutions are stored in the same BrickingInformation
    * struct.
    */
    class BrickResolutionCalculator {
    public:
        BrickResolutionCalculator(BrickingInformation& brickingInformation);

        virtual ~BrickResolutionCalculator() {}

        /**
         * Calculates the available brick resolutions a BrickLodSelector can assign
         * to the VolumeBricks. The available resolutions are stored in the
         * brickingInformation_ struct.
         */
        virtual void calculateBrickResolutions() = 0;

    protected:
        BrickingInformation& brickingInformation_;

    private:

    }; //end of class



} //namespace

#endif

