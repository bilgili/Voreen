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

#include "tgt/stopwatch.h"
#include "tgt/assert.h"

namespace tgt {

Stopwatch::Stopwatch() {
    reset();
}

uint64_t Stopwatch::getTicks() {

    #ifdef WIN32
        return GetTickCount();
    #else         
        struct timeval tp;
        int result = gettimeofday(&tp, 0);
        if (!result)
            return static_cast<uint64_t>( (static_cast<uint64_t>(tp.tv_sec) ) * 1000 + (tp.tv_usec / 1000));
        else {
            tgtAssert(false, "The system function gettimeofday returned an error code!");
            return 0;
        }
    #endif
}    

uint64_t Stopwatch::getClockCycles() {
    uint64_t result;

    #ifdef __GNUC__
        asm volatile (
            "rdtsc"         //Read Time-Stamp Counter
            : "=A" (result) //EDX:EAX -> result
            :               // no input
        );
    #else  //__GNUC__
        tgtAssert(false, "This function is not yet implemented for your compiler and/or arch!");
        result = 0; //REMOVED WARNING (we can never come here)
    #endif //__GNUC__

    //TODO msvc and icc version must look sth like this: NOT TESTED!!!
    /*
    _asm {
        push eax;
        push edx;
        rdtsc;
        mov eax, result;
        mov edx, result + 4;
        pop edx;
        pop eax;
    };
    */
    
    //Perhaps pushing and popping is unnecessary when edx:eax is standard for 64 bit return values;
    //but declare function as naked and do not use any return command. NOT TESTED!!!
    /*
    _asm {
        rdtsc;
    };
    */
    
    return result;
}

clock_t Stopwatch::getRuntime() {
    if (!isStopped_)
        return (static_cast<clock_t>(getTicks()) - startTime_) + runTime_;
    else
        return runTime_;
}

void Stopwatch::stop() {
    if (!isStopped_) {
        isStopped_ = true;
        runTime_ += static_cast<clock_t>(getTicks()) - startTime_;
    }
}

void Stopwatch::start() {
    if (isStopped_) {
        startTime_ = static_cast<clock_t>(getTicks());
        isStopped_ = false;
    }
}


void Stopwatch::reset() {
    isStopped_ = true;
    startTime_ = 0;
    runTime_   = 0;
}

}
