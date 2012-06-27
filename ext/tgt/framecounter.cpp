/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "tgt/framecounter.h"

namespace tgt {

void FrameCounter::update() {
    counter_++;
    speedcounter_++;
    if(speedcounter_ > speedrate_) {
        speedtimer_.stop();
        float tmp = float(speedtimer_.getRuntime()) / speedrate_;
        if(tmp > 0.f && tmp < 500.f)
            speedfactor_ = tmp;
        speedcounter_ = 0;
        speedtimer_.reset();
        speedtimer_.start();
    }

    // is it time to calculate the average?
    if(counter_ > rate_) {
        stopwatch_.stop();
        // The average time the program needs to render one frame timer.getRuntime()
        // divided by (rate * 1000); 1000 because of milliseconds to seconds.  We add
        // one to the Runtime so we never get a division by zero.  To calculate the frames
        // per seconds, we simply calculate the inverse of seconds per frame; this yields
        // the following formula.
        averagefps_ = (1000.f * rate_) / (stopwatch_.getRuntime() + 1);
        counter_ = 0;
        stopwatch_.reset();
        stopwatch_.start();
    }
}

float FrameCounter::getFPS() {
    return averagefps_;
}

} // namespace tgt
