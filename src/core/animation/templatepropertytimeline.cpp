/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#include "voreen/core/animation/templatepropertytimeline.h"
#include "voreen/core/animation/animation.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/transfunc/transfunc2dprimitives.h"
#include "voreen/core/properties/cameraproperty.h"
#include "tgt/camera.h"

using tgt::Camera;

namespace voreen {

////////////////// special case TransFunc*
template <>
TemplatePropertyTimeline<TransFunc*>::TemplatePropertyTimeline(TemplateProperty<TransFunc*>* prop)
    : property_(prop)
    , activeOnRendering_(true)
    , timelineChanged_(false)
    , undoObserver_(0)
{
    duration_ = 60.f * 15.f;
    TransFunc* func = property_->get();
    if (func)
        func = func->clone();
    else {
        LWARNINGC("", "warn 1");
        func = new TransFunc1DKeys();
        property_->set(func->clone());
    }
    timeline_ = new TransFuncPropertyTimelineState(new PropertyKeyValue<TransFunc*>(func,0));
}

template <>
TemplatePropertyTimeline<TransFunc*>::TemplatePropertyTimeline()
    : property_(0)
    , activeOnRendering_(true)
    , timelineChanged_(false)
    , undoObserver_(0)
{
    duration_ = 60.f * 15.f;
    timeline_ = new TransFuncPropertyTimelineState();
}

template <>
void TemplatePropertyTimeline<TransFunc*>::setProperty(Property* p) {
    TemplateProperty<TransFunc*>* tp = dynamic_cast<TemplateProperty<TransFunc*>*>(p);
    if(tp) {
        property_ = tp;
        if(timeline_->getKeyValues().size() == 0) {
            delete timeline_;
            timeline_ = 0;

            TransFunc* func = property_->get();
            if (func)
                func = func->clone();
            else {
                LWARNINGC("", "warn 2");
                func = new TransFunc1DKeys();
                property_->set(func->clone());
            }
            timeline_ = new TransFuncPropertyTimelineState(new PropertyKeyValue<TransFunc*>(func,0));
        }
    }
    else {
        LERRORC("voreen.TemplatePropertyTimeline", "Property type mismatch!");
    }
}

template <>
void TemplatePropertyTimeline<TransFunc*>::resetTimeline() {
    timelineChanged_ = true;
    lastChanges_.push_back(timeline_);
    undoObserver_->animationChanged(this);

    TransFunc* func = property_->get();
    if (func)
        func = func->clone();
    else {
        LWARNINGC("", "warn 3");
        func = new TransFunc1DKeys();
        property_->set(func->clone());
    }
    timeline_ = new TransFuncPropertyTimelineState(new PropertyKeyValue<TransFunc*>(func,0));

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it) {
        (*it)->timelineChanged();
    }
}

template <>
TransFunc* TemplatePropertyTimeline<TransFunc*>::privateGetPropertyAt(float time) {
    TemplatePropertyTimelineState<TransFunc*>* tl = dynamic_cast<TemplatePropertyTimelineState<TransFunc*>* >(timeline_);
    if (tl) {
        TransFunc* func = const_cast<TransFunc*>(tl->getPropertyAt(time));
        return func;
    }
    else {
        LERRORC("TemplatePropertyTimeline<TransFunc*>", "no timeline state");
        return 0;
    }
}

template <>
void TemplatePropertyTimeline<TransFunc*>::renderAt(float time) {
    if (!activeOnRendering_)
        return;

    property_->set(getPropertyAt(time));
}

template <>
void TemplatePropertyTimeline<TransFunc*>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time*10000)/10000;
    TransFunc* value = property_->get();
    TransFunc* animatedValue = getPropertyAt(time);

    TransFunc1DKeys* f1 = dynamic_cast<TransFunc1DKeys*>(value);
    TransFunc1DKeys* f2 = dynamic_cast<TransFunc1DKeys*>(animatedValue);

    TransFunc2DPrimitives* f3 = dynamic_cast<TransFunc2DPrimitives*>(value);
    TransFunc2DPrimitives* f4 = dynamic_cast<TransFunc2DPrimitives*>(animatedValue);

    if (f1 && f2) {
        if ((*f1) != (*f2)) {
            const PropertyKeyValue<TransFunc*>* kv = timeline_->newKeyValue(time);
            if (!kv) {
                kv = new PropertyKeyValue<TransFunc*>(value,time);
            }
            this->changeValueOfKeyValue(value,kv);
            return;
        }
        else {
            if (forceKeyValue){
                newKeyValue(time);
            }
        }
        return;
    }

    if (f3 && f4) {
        if ((*f3) != (*f4)) {
            const PropertyKeyValue<TransFunc*>* kv = timeline_->newKeyValue(time);
            if (!kv) {
                kv = new PropertyKeyValue<TransFunc*>(value,time);
            }
            this->changeValueOfKeyValue(value,kv);
            return;
        }
        else {
            if (forceKeyValue){
                newKeyValue(time);
            }
        }
        return;
    }
    const PropertyKeyValue<TransFunc*>* kv = timeline_->newKeyValue(time);
    if (!kv) {
        kv = new PropertyKeyValue<TransFunc*>(value,time);
    }
    this->changeValueOfKeyValue(value,kv);
}

} // namespace voreen
