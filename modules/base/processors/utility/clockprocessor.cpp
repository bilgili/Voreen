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

#include "clockprocessor.h"
#include "voreen/core/voreenapplication.h"
#include "tgt/event/timeevent.h"

namespace voreen {

ClockProcessor::ClockProcessor()
    : Processor(),
    counter_(0),
    highResCounter_(0.0f),
    period_(1),
    timerIsActive_(false),
    timer_(0),
    eventHandler_(),
    enableTimerProp_("enableTimerProp", "Enable Timer", false),
    intervalProp_("intervalProp", "Interval (ms)", 1000, 1, 10 * 1000),
    counterStyleProp_("counterStyle", "Counter Style"),
    periodProp_("period", "Period (counts)", 2, 1, 1000),
    tickCounterProp_("tickCounterProp", "Counter", 0, 0, 1000),
    useHighResCounterProp_("useHighResCounterProp", "Use High Resolution Counter", false),
    resolutionProp_("resolutionProp", "Resolution (1/counts)", 0.5f, 0.0f, 1.0f),
    highResCounterProp_("highResCounterProp", "High Resolution Counter", 0.0f, 0.0f, 1000.f),
    resetProp_("resetProp", "Reset Counters")
{

    eventHandler_.addListenerToBack(this);
    timer_ = VoreenApplication::app()->createTimer(&eventHandler_);

    enableTimerProp_.onChange(CallMemberAction<ClockProcessor>(this, &ClockProcessor::toggleTimer));
    intervalProp_.onChange(CallMemberAction<ClockProcessor>(this, &ClockProcessor::onIntervalChange));
    periodProp_.onChange(CallMemberAction<ClockProcessor>(this, &ClockProcessor::onPeriodChange));
    resetProp_.onChange(CallMemberAction<ClockProcessor>(this, &ClockProcessor::resetCounter));
    useHighResCounterProp_.onChange(CallMemberAction<ClockProcessor>(this,
        &ClockProcessor::onUseHighResCounterChange));

    tickCounterProp_.setWidgetsEnabled(false);
    highResCounterProp_.setWidgetsEnabled(false);

    counterStyleProp_.addOption("linear", "Linear", ClockProcessor::COUNTER_LINEAR);
    counterStyleProp_.addOption("cyclic", "Cyclic", ClockProcessor::COUNTER_CYCLIC);
    counterStyleProp_.addOption("reversing", "Reversing", ClockProcessor::COUNTER_REVERSING);
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

void ClockProcessor::process() {}

void ClockProcessor::initialize() throw (tgt::Exception) {
    Processor::initialize();

    // Startup the timer if desired
    if (enableTimerProp_.get()) {
        if (timer_) {
            startTimer();
        }
        else {
            LWARNING("Timer object could not be created. Disabling processor.");
            enableTimerProp_.set(false);
            enableTimerProp_.setWidgetsEnabled(false);
            timerIsActive_ = false;
        }
    }
}

void ClockProcessor::timerEvent(tgt::TimeEvent* te) {
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

    if (te)
        te->accept();
}

bool ClockProcessor::isTimerActive() const {
    return timerIsActive_;
}

void ClockProcessor::startTimer() {

    if (!timer_) {
        LWARNING("No timer.");
        return;
    }

    if (!timerIsActive_) {
        timerIsActive_ = true;
        timer_->start(intervalProp_.get());
    }
}

void ClockProcessor::stopTimer() {
    if (timer_) {
        timer_->stop();
    }
    else {
        LWARNING("No timer.");
    }
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

void ClockProcessor::resetCounter() {
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
    }
    else {
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
