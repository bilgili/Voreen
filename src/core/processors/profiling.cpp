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

#include "voreen/core/processors/profiling.h"
#include <iomanip>
#include "tgt/tgt_gl.h"
#include "tgt/logmanager.h"

#ifdef _MSC_VER  // high-performance counter
#include <windows.h>
#include <winbase.h>
#endif

namespace voreen {

//const std::string ProfilingBlock::loggerCat_ = "voreen.ProfilingBlock";
const std::string PerformanceSample::loggerCat_ = "voreen.PerformanceSample";

PerformanceSample::PerformanceSample(PerformanceSample* parent, std::string name) : parent_(parent), name_(name), time_(-1.0f) {
}

PerformanceSample* PerformanceSample::addChild(std::string name) {
    children_.push_back(PerformanceSample(this, name));
    return &(children_.back());
}

void PerformanceSample::print(int level) const {
    std::string spaces = "";
    for(int j=0; j<level; j++)
        spaces += ' ';

    float percent = 100.0f;
    if(parent_)
        percent = 100.0f * time_ / parent_->getTime();

    float percentChildren = 100.0f * (getChildrenTime()) / time_;

    if (children_.size() > 0 && parent_)
        LINFO(spaces << name_ << " (" << children_.size() << " children): " << std::setprecision(10) << time_ << " secs (" << percent << "% of parent) (" << percentChildren << "% in children)");
    else if (children_.size() > 0 && !parent_)
        LINFO(spaces << name_ << " (" << children_.size() << " children): " << std::setprecision(10) << time_ << " secs (" << percentChildren << "% in children)");
    else if (children_.size() == 0 && parent_)
        LINFO(spaces << name_ << ": " << std::setprecision(10) << time_ << " secs (" << percent << "% of parent)");
    else
        LINFO(spaces << name_ << ": " << std::setprecision(10) << time_ << " secs");

    for(size_t i=0; i<children_.size(); i++)
        children_[i].print(level+1);
}

float PerformanceSample::getChildrenTime() const {
    float time = 0.0f;
    for(size_t i=0; i<children_.size(); i++)
        time += children_[i].getTime();
    return time;
}

void PerformanceSample::setTime(float time) {
    time_ = time;
}

float PerformanceSample::getTime() const {
    return time_;
}

//----------------------------------------------------------------

PerformanceRecord::PerformanceRecord() : current_(0) {
}

PerformanceRecord::~PerformanceRecord() {
    while(!samples_.empty()) {
        delete samples_.back();
        samples_.pop_back();
    }
}

void PerformanceRecord::startBlock(const ProfilingBlock* const pb) {
    if(current_ == 0)
        current_ = new PerformanceSample(current_, pb->getName());
    else {
        current_ = current_->addChild(pb->getName());
    }
}

void PerformanceRecord::endBlock(const ProfilingBlock* const pb) {
    current_->setTime(pb->getTime());
    if(current_->getParent() == 0)
        samples_.push_back(current_);

    current_ = current_->getParent();
}

const std::vector<PerformanceSample*> PerformanceRecord::getSamples() const {
    return samples_;
}

PerformanceSample* PerformanceRecord::getLastSample() const {
    if(samples_.empty())
        return 0;
    else
        return samples_.back();
}

//----------------------------------------------------------------

ProfilingBlock::ProfilingBlock(std::string name, PerformanceRecord& pr) : name_(name), pr_(pr) {
    //LINFO("Starting Block " << name);
    pr_.startBlock((const ProfilingBlock* const) this);
    //glFinish();

#ifdef _MSC_VER  // use high-performance counter on windows, if available
    if (QueryPerformanceFrequency(&hpfTicksPerSecond_) && QueryPerformanceCounter(&hpfStart_)) {
        useHpf_ = true;
    }
    else { // hpf not available
        useHpf_ = false;
        start_ = clock();
    }
#else
    start_ = clock();
#endif
}

ProfilingBlock::~ProfilingBlock() {
    //glFinish();
#ifdef _MSC_VER
    if (useHpf_)
        QueryPerformanceCounter(&hpfEnd_);
    else
        end_ = clock();
#else
    end_ = clock();
#endif
    //LINFO("Finishing Block " << name_);
    pr_.endBlock((const ProfilingBlock* const)this);
}

float ProfilingBlock::getTime() const {
#ifdef _MSC_VER
    if (useHpf_)
        return ((hpfEnd_.QuadPart-hpfStart_.QuadPart) / static_cast<float>(hpfTicksPerSecond_.QuadPart));
    else
        return ((end_-start_) / static_cast<float>(CLOCKS_PER_SEC));
#else
    return ((end_-start_) / static_cast<float>(CLOCKS_PER_SEC));
#endif
}

std::string ProfilingBlock::getName() const {
    return name_;
}

} // namespace
