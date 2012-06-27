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

#include "voreen/core/vis/processors/clockprocessor.h"
#include "voreen/core/application.h"

namespace voreen {

ClockProcessor::ClockProcessor()
    : Processor(),
    counter_(0),
    highResCounter_(0.0f),
    period_(1),
    timerIsActive_(false),
    timer_(0),
    eventHandler_(),
    enableTimerProp_("enableTimerProp", "enable timer", false),
    intervalProp_("intervalProp", "interval (ms)", 1000, 1, 10 * 1000),
    counterStyleProp_("counterStyle", "counter style"),
    periodProp_("period", "period (counts)", 2, 1, 1000),
    tickCounterProp_("tickCounterProp", "counter", 0, 0, 1000),
    useHighResCounterProp_("useHighResCounterProp", "use high resolution counter", false),
    resolutionProp_("resolutionProp", "resolution (1/counts)", 0.5f, 0.0f, 1.0f),
    highResCounterProp_("highResCounterProp", "high resolution counter:", 0.0f, 0.0f, 1000.f),
    resetProp_("resetProp", "reset counters")
{

    eventHandler_.addListenerToBack(this);
    timer_ = VoreenApplication::app()->createTimer(&eventHandler_);

    enableTimerProp_.onChange(CallMemberAction<ClockProcessor>(this, &ClockProcessor::toggleTimer));
    intervalProp_.onChange(CallMemberAction<ClockProcessor>(this, &ClockProcessor::onIntervalChange));
    periodProp_.onChange(CallMemberAction<ClockProcessor>(this, &ClockProcessor::onPeriodChange));
    resetProp_.onChange(CallMemberAction<ClockProcessor>(this, &ClockProcessor::onResetCounter));
    useHighResCounterProp_.onChange(CallMemberAction<ClockProcessor>(this,
        &ClockProcessor::onUseHighResCounterChange));

    tickCounterProp_.setWidgetsEnabled(false);
    highResCounterProp_.setWidgetsEnabled(false);

    counterStyleProp_.addOption("linear", "linear", ClockProcessor::COUNTER_LINEAR);
    counterStyleProp_.addOption("cyclic", "cyclic", ClockProcessor::COUNTER_CYCLIC);
    counterStyleProp_.addOption("reversing", "reversing", ClockProcessor::COUNTER_REVERSING);
    counterStyleProp_.onChange(CallMemberAction<ClockProcessor>(this, &ClockProcessor::onCounterStyleChange));
    counterStyleProp_.set("cyclic");

    addProperty(enableTimerProp_);
    addProperty(intervalProp_);
    addProperty(counterStyleProp_);
    addProperty(periodProp_);
    addProperty(tickCounterProp_);
    addProperty(useHighResCounterProp_);
    addProperty(resolutionProp_);
    addProperty(highResCounterProp_);
    addProperty(resetProp_);

    onCounterStyleChange();
    onPeriodChange();
    onUseHighResCounterChange();
}

ClockProcessor::~ClockProcessor() {
    delete timer_;
}

const std::string ClockProcessor::getProcessorInfo() const {
    return "Allows time-based manipulation of properties by linking them with the ClockProcessor's pseudo property \"counter\".";
}

void ClockProcessor::process() {}

void ClockProcessor::initialize() throw (VoreenException) {
    Processor::initialize();

    // Startup the timer if desired
    //
    if (enableTimerProp_.get())
        startTimer();
}

void ClockProcessor::timerEvent(tgt::TimeEvent* te) {
    if (te == 0)
        return;

    ++counter_;
    if (counter_ < 0)   // in case of overflow of int, reset counter to zero to prevent strange results
        counter_ = 0;

    switch (counterStyleProp_.getValue()) {
        case COUNTER_LINEAR:
            tickCounterProp_.set(counter_);
            break;
        case COUNTER_CYCLIC:
            counter_ = counter_ % period_;
            tickCounterProp_.set(counter_);
            break;
        case COUNTER_REVERSING:
            if (counter_ >= (2 * (period_ - 1)))
                counter_ = 0;

            if (counter_ < period_)
                tickCounterProp_.set(counter_);
            else
                tickCounterProp_.set(((2 * (period_ - 1)) - counter_));
            break;
    }

    if (useHighResCounterProp_.get())
        triggerHighResCounter();
    te->accept();
}

bool ClockProcessor::isTimerActive() const {
    return timerIsActive_;
}

void ClockProcessor::startTimer() {
    if (!timerIsActive_) {
        timerIsActive_ = true;
        timer_->start(intervalProp_.get());
    }
}

void ClockProcessor::stopTimer() {
    timer_->stop();
    timerIsActive_ = false;
}

// protected methods
//

void ClockProcessor::onCounterStyleChange() {
    if (counterStyleProp_.getValue() != COUNTER_LINEAR)
        periodProp_.setVisible(true);
    else
        periodProp_.setVisible(false);
}

void ClockProcessor::onIntervalChange() {
    // If the timer has been active and the interval changes, the timer has to be
    // restarted.
    //
    if (timerIsActive_) {
        stopTimer();
        startTimer();
    }
}

void ClockProcessor::onPeriodChange() {
    period_ = periodProp_.get();
    if (period_ <= 0)    // prevent errors caused by possibly defect properties
        period_ = 1;
}

void ClockProcessor::onResetCounter() {
    counter_ = 0;
    tickCounterProp_.set(counter_);
    onResolutionChange();
}

void ClockProcessor::onResolutionChange() {
    highResCounter_ = 0.0f;
    highResCounterProp_.set(highResCounter_);
}

void ClockProcessor::onUseHighResCounterChange() {
    if (useHighResCounterProp_.get() == true) {
        resolutionProp_.setVisible(true);
        highResCounterProp_.setVisible(true);
    } else {
        resolutionProp_.setVisible(false);
        highResCounterProp_.setVisible(false);
    }
}

void ClockProcessor::toggleTimer() {
    if (enableTimerProp_.get())
        startTimer();
    else
        stopTimer();
}

void ClockProcessor::triggerHighResCounter() {
    highResCounter_ += resolutionProp_.get();

    switch (counterStyleProp_.getValue()) {
        case COUNTER_LINEAR:
            highResCounterProp_.set(highResCounter_);
            break;
        case COUNTER_CYCLIC:
            if (highResCounter_ >= static_cast<float>(period_))
                highResCounter_ = 0.0f;
            highResCounterProp_.set(highResCounter_);
            break;
        case COUNTER_REVERSING:
            {
                float limit = (2.0f * (static_cast<float>(period_) - resolutionProp_.get()));
                if (highResCounter_ >= limit)
                    highResCounter_ = 0.0;

                if (highResCounter_ < static_cast<float>(period_))
                    highResCounterProp_.set(highResCounter_);
                else
                    highResCounterProp_.set((limit - highResCounter_));
            }
            break;
    }
}

}   // namespace
