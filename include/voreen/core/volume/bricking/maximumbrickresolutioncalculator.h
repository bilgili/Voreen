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

#ifndef VRN_MAXIMUMBRICKRESOLUTIONCALCULATOR_H
#define VRN_MAXIMUMBRICKRESOLUTIONCALCULATOR_H

#include "voreen/core/volume/bricking/brickresolutioncalculator.h"

namespace voreen {

    /**
    * This class calculates maximum resolutions for the bricks.
    * That means that it is tried to provide as many bricks as
    * possible with maximum resolution. As a result of this, every
    * other brick will have MINUMUM resolution, losing nearly all
    * of its information.
    */
    class MaximumBrickResolutionCalculator : public BrickResolutionCalculator {

    public:
        MaximumBrickResolutionCalculator(BrickingInformation& brickingInformation);

        /**
        * Calculates brick resolutions in a way that produces a maximum amount
        * of highest resolution bricks while reducing all other bricks to lowest
        * resolution. The calculated resolutions are stored in the brickingInformation_
        * struct.
        */
        virtual void calculateBrickResolutions();

    protected:

    private:



    }; //end of class



} //namespace




#endif
