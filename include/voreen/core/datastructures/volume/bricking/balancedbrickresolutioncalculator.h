/*-**********************************************************************
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

#ifndef VRN_BALANCEDBRICKRESOLUTIONCALCULATOR_H
#define VRN_BALANCEDBRICKRESOLUTIONCALCULATOR_H

#include "voreen/core/datastructures/volume/bricking/brickresolutioncalculator.h"

namespace voreen {

    /**
    * This class calculates balanced resolutions for the bricks.
    * That means that one of two resolutions next to each other are chosen
    * for every brick, so that there are only small differences between the
    * resolutions. So all blocks have either resolution 4 or 5 for example.
    * As a result, depending on the graphics hardware, there might be
    * no blocks of highest resolution at all.
    */
    class BalancedBrickResolutionCalculator : public BrickResolutionCalculator {

    public:
        BalancedBrickResolutionCalculator(BrickingInformation& brickingInformation);

        virtual void calculateBrickResolutions();

    protected:

    private:



    }; //end of class



} //namespace

#endif
