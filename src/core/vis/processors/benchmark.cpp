/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/benchmark.h"

namespace voreen {

#ifdef __unix__
#include <time.h>
#endif

const std::string ProcessBenchmark::loggerCat_ = "voreen.ProcessBenchmark";

ProcessBenchmark::ProcessBenchmark(const std::string& processorName)
    : frameCount_(-1)
{
    if (!processorName.empty())
        addProcessorName(processorName);
}

void ProcessBenchmark::addProcessorName(const std::string& name) {
    for (size_t i=0; i < processorNames_.size(); i++) {
        if (processorNames_[i] == name)
            return;
    }
    processorNames_.push_back(name);
}

void ProcessBenchmark::beforeProcess(Processor* p) {
    for (size_t i=0; i < processorNames_.size(); i++) {
        if (processorNames_[i] == p->getName()) {
            startTicks_ = getTicks();
            glFinish(); // sync all OpenGL operations
            return;
        }
    }
}

void ProcessBenchmark::afterProcess(Processor* p) {
    for (size_t i=0; i < processorNames_.size(); i++) {
        if (processorNames_[i] == p->getName()) {
            glFinish(); // sync all OpenGL operations
            uint64_t end = getTicks();

            if (frameCount_ >= 0) {
                frameCount_++;
                totalTime_ += end - startTicks_;
            } else {
                float fps = 1.f / ((end - startTicks_) / 1000000.f);
                LINFO(p->getName() << ": " << fps << "fps (" << end - startTicks_ << " microseconds)");
            }
            return;
        }
    }

}

uint64_t ProcessBenchmark::getTicks() {
    uint64_t ticks;

#ifdef __unix__
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    ticks = now.tv_sec * 1000000 + now.tv_nsec / 1000;
#else
    ticks = 0;
#endif
    return ticks;
}

/*
void ProcessBenchmark::processMessage(Message* msg, const std::string& dest) {
    if (msg->id_ == "processBenchmark.startTiming") {
        if (frameCount_ > 0) {
            LERROR("already timing, stop this first");
        } else {
            totalTime_ = 0;
            frameCount_ = 0;
            LINFO("starting timing");
        }
    }
    else if (msg->id_ == "processBenchmark.stopTiming") {
        //TODO: support timing multiple processors at the same time
        if (frameCount_ > 0) {
            float avgtime = totalTime_ / static_cast<float>(frameCount_);
            float fps = 1.f / (avgtime / 1000000.f);
            LINFO("FPS: " << fps << "\t(" << totalTime_ / 1000000.f << "s),"
                  << " timed " << frameCount_ << " frames");
            frameCount_ = -1;
        } else {
            LERROR("got no frames for timing");
        }
    }
    else if (msg->id_ == "processBenchmark.addProcessorName") {
        addProcessorName(msg->getValue<std::string>());
    }
}*/

} // namespace
