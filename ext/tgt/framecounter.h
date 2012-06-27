/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef TGT_FRAMECOUNTER_H
#define TGT_FRAMECOUNTER_H

#include <cmath>
#include <iostream>

#include "tgt/stopwatch.h"

namespace tgt {

/**
 *  This class can be used to calculate the framerate using the Stopwatch class.
 *  The rate variable specifies the number of frames that are averaged for the
 *  calculation; a higher rate means more significant numbers but it naturally
 *  also lessens the number of updates.
 */
class FrameCounter {
protected:
    Stopwatch stopwatch_;
    Stopwatch speedtimer_;
    float averagefps_;
    int rate_;
    int speedrate_;
    int counter_;
    int speedcounter_;
    float speedfactor_;

public:

    /// The Constructor; the standard update rate is 50.
    FrameCounter(int rate = 50, int speedrate = 0)
        : stopwatch_(Stopwatch()),
          speedtimer_(Stopwatch()),
          averagefps_(0.f),
          rate_(rate),
          speedrate_(speedrate),
          counter_(0),
          speedcounter_(0),
          speedfactor_(0.3f
    ) {
    }

    /// This should be called every frame.
    void update();

    /// This functions is called to get the current average framerate
    float getFPS();
};

}

#endif //TGT_FRAMECOUNTER_H_
