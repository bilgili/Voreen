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

#include "voreen/core/animation/templatepropertytimelinestate.h"
#include "voreen/core/animation/animation.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"
#include "voreen/core/animation/interpolation/camerainterpolationfunctions.h"
#include "tgt/camera.h"

using tgt::Camera;

namespace voreen {

template <class T>
TemplatePropertyTimelineState<T>::TemplatePropertyTimelineState() {
}

template <class T>
TemplatePropertyTimelineState<T>::~TemplatePropertyTimelineState() {
    // delete all saved keyvalues and the interpolationfunctions between them
    typename std::map<float,PropertyKeyValue<T>*>::iterator it;
    for (it = values_.begin(); it != values_.end(); ++it) {
        if ((*it).second->getFollowingInterpolationFunction())
            delete ((*it).second->getFollowingInterpolationFunction());

        delete ((*it).second);
    }
    values_.clear();
}

template <class T>
TemplatePropertyTimelineState<T>::TemplatePropertyTimelineState(PropertyKeyValue<T>* kv) {
    // creates the class with one given standard-keyvalue
    values_.insert(std::pair<float,PropertyKeyValue<T>*>(kv->getTime(),kv));
}

template <class T>
const std::map<float,PropertyKeyValue<T>*>& TemplatePropertyTimelineState<T>::getKeyValues() const {
    return values_;
}

template <class T>
const PropertyKeyValue<T>* TemplatePropertyTimelineState<T>::newKeyValue(float time) {
    time = floor(time * 10000.f) / 10000.f;
    // if there already exists a keyvalue at the given point of time -> do nothing
    if (values_.find(time) != values_.end())
        return 0;

    // calculates the value of the property at the given time to set this value into the new keyvalue
    T value = getPropertyAt(time);

    // create the new keyvalue
    PropertyKeyValue<T>* kv = new PropertyKeyValue<T>(value,time);

    // insert the new keyvalue into the map
    values_.insert(std::pair<float,PropertyKeyValue<T>*>(time,kv));

    //// adapt the interpolationfunctions between the keyvalues
    typename std::map<float,PropertyKeyValue<T>*>::iterator it;
    it = values_.find(time);

    // if new value is the first value:
    if (it == values_.begin()) {
        // if new value is the only value:
        it++;
        if (it != values_.end()) {
            InterpolationFunction<T>* func = new InterpolationFunction<T>();
            (*it).second->setForegoingInterpolationFunction(func);
            it--;
            (*it).second->setFollowingInterpolationFunction(func);
        }
    }
    else {
        it++;
        // if the new value is the last one
        if (it == values_.end()) {
            it--;
            InterpolationFunction<T>* func = new InterpolationFunction<T>();
            (*it).second->setForegoingInterpolationFunction(func);
            it--;
            (*it).second->setFollowingInterpolationFunction(func);
            it++;
        }
        else {
            // if the value is somewhere in the middle of the timeline
            InterpolationFunction<T>* func1 = new InterpolationFunction<T>();
            InterpolationFunction<T>* func2 = new InterpolationFunction<T>();
            (*it).second->setForegoingInterpolationFunction(func2);
            it--;
            (*it).second->setFollowingInterpolationFunction(func2);
            (*it).second->setForegoingInterpolationFunction(func1);
            it--;
            delete (*it).second->getFollowingInterpolationFunction();
            (*it).second->setFollowingInterpolationFunction(func1);
            it++;
        }
    }
    // returns the pointer to the new keyvalue
    return kv;
}

template <class T>
bool TemplatePropertyTimelineState<T>::changeValueOfKeyValue(T value, const PropertyKeyValue<T>* keyvalue) {
    const float time = keyvalue->getTime();
    typename std::map<float,PropertyKeyValue<T>*>::iterator it;
    it = values_.find(time);
    if (it != values_.end()) {
        (*it).second->setValue(value);
        return true;
    }
    return false;
}

template <class T>
bool TemplatePropertyTimelineState<T>::changeSmoothnessOfKeyValue(bool smooth, const PropertyKeyValue<T>* keyvalue) {
    const float time = keyvalue->getTime();
    typename std::map<float,PropertyKeyValue<T>*>::iterator it;
    it = values_.find(time);
    if (it != values_.end()) {
        (*it).second->setSmooth(smooth);
        return true;
    }
    return false;
}

template <class T>
ChangeTimeOfKeyValueReturn TemplatePropertyTimelineState<T>::changeTimeOfKeyValue(float time, const PropertyKeyValue<T>* keyvalue) {
    time = floor(time * 10000.f) / 10000.f;
    const float oldTime = keyvalue->getTime();
    typename std::map<float,PropertyKeyValue<T>*>::iterator it;
    it = values_.find(oldTime);

    // if the given keyvalue is not in the timeline
    if (it == values_.end())
        return KV_NOT_FOUND;

    // if the times are equal -> nothing to do
    if (time == oldTime)
        return KV_EQUAL_TO_OLD;

    // set the new time of the keyvalue
    PropertyKeyValue<T>* keyvalueToChange = (*it).second;
    keyvalueToChange->setTime(time);

    // adapt all the interpolationfunctions between the keyvalues
    if (time > oldTime) {
        it++;
        if ((it == values_.end()) || ((*it).first > time )) {
            values_.erase(oldTime);
            values_.insert(std::pair<float,PropertyKeyValue<T>*>(time,keyvalueToChange));
            return KV_TIME_CHANGED;
        }
        it--;
    }
    if (time < oldTime) {
        if ((it == values_.begin()) || ((*(--it)).first<time)) {
            values_.erase(oldTime);
            values_.insert(std::pair<float,PropertyKeyValue<T>*>(time,keyvalueToChange));
            return KV_TIME_CHANGED;
        }
        else {
            it++;
        }
    }
    // keyvalue has to be insertet at another point
        // delete keyvalue from the old point:
            // if value is the first one
    if (it == values_.begin()) {
        (*it).second->setFollowingInterpolationFunction(0);
        it++;
        delete ((*it).second->getForegoingInterpolationFunction());
        (*it).second->setForegoingInterpolationFunction(0);
        values_.erase(oldTime);
    }
    else {
            // if value is the last one
        it++;
        if (it == values_.end()) {
            it--;
            (*it).second->setForegoingInterpolationFunction(0);
            it--;
            delete ((*it).second->getFollowingInterpolationFunction());
            (*it).second->setFollowingInterpolationFunction(0);
            values_.erase(oldTime);
        }
        else {
            // if value is in the middle
            InterpolationFunction<T>* func = new InterpolationFunction<T>();
            (*it).second->setForegoingInterpolationFunction(func);
            it--;
            delete (*it).second->getFollowingInterpolationFunction();
            delete (*it).second->getForegoingInterpolationFunction();
            (*it).second->setFollowingInterpolationFunction(0);
            (*it).second->setForegoingInterpolationFunction(0);
            it--;
            (*it).second->setFollowingInterpolationFunction(func);
            values_.erase(oldTime);
        }
    }
        // insert keyvalue at the new point:
            // if there already exists a keyvalue at the new time
    if (values_.find(time) != values_.end()) {
        it = values_.find(time);
        (*it).second->setValue(keyvalueToChange->getValue());
        delete keyvalueToChange;
        return KV_EXISTS_AT_NEW_TIME;
    }

    values_.insert(std::pair<float,PropertyKeyValue<T>*>(time,keyvalueToChange));

    it = values_.find(time);

    // if new value is the first value:
    if (it == values_.begin()) {
        // only do something if there are multiple values
        it++;
        if (it != values_.end()) {
            InterpolationFunction<T>* func = new InterpolationFunction<T>();
            (*it).second->setForegoingInterpolationFunction(func);
            it--;
            (*it).second->setFollowingInterpolationFunction(func);
        }
    }
    else {
        it++;
        // if the new value is the last one
        if (it == values_.end()) {
            it--;
            InterpolationFunction<T>* func = new InterpolationFunction<T>();
            (*it).second->setForegoingInterpolationFunction(func);
            it--;
            (*it).second->setFollowingInterpolationFunction(func);
        }
        else {
            // if new value is in the middle
            InterpolationFunction<T>* func1 = new InterpolationFunction<T>();
            InterpolationFunction<T>* func2 = new InterpolationFunction<T>();
            (*it).second->setForegoingInterpolationFunction(func2);
            it--;
            (*it).second->setFollowingInterpolationFunction(func2);
            (*it).second->setForegoingInterpolationFunction(func1);
            it--;
            delete (*it).second->getFollowingInterpolationFunction();
            (*it).second->setFollowingInterpolationFunction(func1);
        }
    }
    return KV_MOVED_TO_DIFFERENT_INTERVAL;
}

template <class T>
DeleteKeyValueReturn TemplatePropertyTimelineState<T>::deleteKeyValue(const PropertyKeyValue<T>* keyvalue){
    const float time = keyvalue->getTime();
    typename std::map<float,PropertyKeyValue<T>*>::iterator it;
    it = values_.find(time);

    // if wrong parameter do nothing
    if (it == values_.end()) {
        return KV_NOT_THERE;
    }

    // if keyvalue ist the only value do nothing
    if (it == values_.begin()) {
        it++;
        if (it == values_.end())
            return KV_IS_THE_ONLY_ONE;
        it--;
    }

    // if value is the first one
    if (it == values_.begin()) {
        it++;
        delete ((*it).second->getForegoingInterpolationFunction());
        (*it).second->setForegoingInterpolationFunction(0);
        values_.erase(time);
        return KV_DELETED;
    }
    // if value is the last one
    it++;
    if (it == values_.end()){
        it--;
        it--;
        delete ((*it).second->getFollowingInterpolationFunction());
        (*it).second->setFollowingInterpolationFunction(0);
        values_.erase(time);
        return KV_DELETED;
    }
    // if value is in the middle
    InterpolationFunction<T>* func = new InterpolationFunction<T>();
    (*it).second->setForegoingInterpolationFunction(func);
    it--;
    delete (*it).second->getFollowingInterpolationFunction();
    delete (*it).second->getForegoingInterpolationFunction();
    it--;
    (*it).second->setFollowingInterpolationFunction(func);
    values_.erase(time);
    return KV_DELETED;
}

template <class T>
void TemplatePropertyTimelineState<T>::setInterpolationFunctionBefore(InterpolationFunction<T>* func,PropertyKeyValue<T>* keyvalue) {
    typename std::map<float,PropertyKeyValue<T>*>::iterator it;
    it = values_.find(keyvalue->getTime());
    if (it == values_.end())
        return;

    if (it == values_.begin())
        // there cannot be an interpolationfunction before the first keyvalue
        return;

    (*it).second->setForegoingInterpolationFunction(func);
    it--;
    (*it).second->setFollowingInterpolationFunction(func);
}

template <class T>
void TemplatePropertyTimelineState<T>::setInterpolationFunctionAfter(InterpolationFunction<T>* func,PropertyKeyValue<T>* keyvalue) {
    typename std::map<float,PropertyKeyValue<T>*>::iterator it;
    it = values_.find(keyvalue->getTime());
    if (it == values_.end())
        return;
    it++;
    if (it == values_.end())
        // there cannot be an interpolationfunction after the last keyvalue
        return;

    (*it).second->setForegoingInterpolationFunction(func);
    it--;
    (*it).second->setFollowingInterpolationFunction(func);
}

template <class T>
const T TemplatePropertyTimelineState<T>::getPropertyAt(float time) {
    typename std::map<float,PropertyKeyValue<T>*>::iterator it;
    it = values_.find(time);

    // if the given point of time is exactly on a keyvalue
    if (it != values_.end())
        return ((*it).second->getValue());

    // if the given point of time is earlier than the first keyalue
    it = values_.upper_bound(time);
    if (it == values_.begin())
        return ((*it).second->getValue());

    // if the given point of time is later than the last keyalue
    if (it == values_.end()) {
        it--;
        return ((*it).second->getValue());
    }
    // if the given point of time lies between two keyvalues -> call the corresponding interpolationfunction
    typename std::map<float,PropertyKeyValue<T>*>::iterator it2;
    it2 = it;
    it--;

    const InterpolationFunction<T>* func = (*it).second->getFollowingInterpolationFunction();
    const MultiPointInterpolationFunction<T>* multifunc = dynamic_cast<const MultiPointInterpolationFunction<T>*>(func);
    if (multifunc) {
        // call a function with multiple points
        // create vector of the used keyvalues
        std::vector<PropertyKeyValue<T>*> keys;
        // search for the first value in the multi-point interval
        while ((it!=values_.begin()) && (it->second->isSmoothed()))
            it--;

        do {
            keys.push_back((*it).second->clone());
            it++;
        }
        while ((it != values_.end()) && it->second->isSmoothed());

        if (it != values_.end())
            keys.push_back((*it).second->clone());

        // interpolate value
        T returnvalue = multifunc->interpolate(keys,time);

        // delete all copied keys
        typename std::vector<PropertyKeyValue<T>*>::const_iterator delIt;
        for (delIt = keys.begin(); delIt != keys.end(); ++delIt)
            delete (*delIt);
        keys.clear();

        // return
        return returnvalue;
    }
    else {
        return func->interpolate(
            it->second->getValue(),
            it2->second->getValue(),
            (time- (it->first)) / ((it2->first) - (it->first)));
    }
}

template <class T>
TemplatePropertyTimelineState<T>* TemplatePropertyTimelineState<T>::clone() const {
    TemplatePropertyTimelineState<T>* timeline = new TemplatePropertyTimelineState<T>();

    // copy all keyvalues
    typename std::map<float,PropertyKeyValue<T>*>::const_iterator it;
    for (it = values_.begin(); it != values_.end(); ++it)
        timeline->values_.insert(std::pair<float,PropertyKeyValue<T>*>(it->first, it->second->clone()));

    // copy all interpolationfunctions between the keyvalues
    typename std::map<float,PropertyKeyValue<T>*>::const_iterator it2;
    it2 = timeline->values_.begin();
    for (it = values_.begin(); it != values_.end();) {
        const InterpolationFunction<T>* func;
        InterpolationFunction<T>* func2;
        func = (*it).second->getFollowingInterpolationFunction();
        if (!func) {
            it2++;
            it++;
            continue;
        }
        func2 = func->clone();
        (*it2).second->setFollowingInterpolationFunction(func2);
        it2++;
        it++;
        (*it2).second->setForegoingInterpolationFunction(func2);
    }

    return timeline;
}

template <class T>
void TemplatePropertyTimelineState<T>::setDuration(float duration) {
    std::map<float,PropertyKeyValue<T>*> newValues;

    typename std::map<float,PropertyKeyValue<T>*>::const_iterator it = values_.begin();

    // if first keyvalue is too late -> set it to time=0
    if ((*it).first > duration) {
        (*it).second->setTime(0);
        this->changeTimeOfKeyValue(0,(*it).second);
    }

    it = values_.begin();

    // copy all keyvalues with time<=duration
    while((it != values_.end()) && (it->first <= duration)) {
        newValues.insert(std::pair<float,PropertyKeyValue<T>*>(it->first, it->second->clone()));
        it++;
    }

    // copy all interpolationfunctions
    typename std::map<float,PropertyKeyValue<T>*>::const_iterator it2;
    it2 = values_.begin();
    for (it = newValues.begin(); it != newValues.end();) {
        const InterpolationFunction<T>* func;
        InterpolationFunction<T>* func2;

        func = it2->second->getFollowingInterpolationFunction();
        if (!func) {
            it2++;
            it++;
            continue;
        }
        func2 = func->clone();
        it++;
        it2++;
        if (it != newValues.end()) {
            it--;
            it->second->setFollowingInterpolationFunction(func2);
            it++;
            it->second->setForegoingInterpolationFunction(func2);
        }
        else {
            delete func2;
            it--;
            it->second->setFollowingInterpolationFunction(0);
            it++;
        }
    }

    // delete all old keyvalues
    for (it = values_.begin(); it != values_.end(); ++it) {
        if (it->second->getFollowingInterpolationFunction())
            delete (it->second->getFollowingInterpolationFunction());

        delete (it->second);
    }
    values_.clear();

    values_ = newValues;
}

template <class T>
void TemplatePropertyTimelineState<T>::serialize(XmlSerializer& s) const {
    s.serialize("values", values_);
}

template <class T>
void TemplatePropertyTimelineState<T>::deserialize(XmlDeserializer& s) {
    s.deserialize("values", values_);
}

TransFuncPropertyTimelineState::TransFuncPropertyTimelineState(PropertyKeyValue<TransFunc*>* kv) {
    values_.insert(std::pair<float,PropertyKeyValue<TransFunc*>*>(kv->getTime(),kv));
}

TransFuncPropertyTimelineState::TransFuncPropertyTimelineState() {}

TransFuncPropertyTimelineState::~TransFuncPropertyTimelineState() {
    std::map<float,PropertyKeyValue<TransFunc*>*>::iterator it;
    for (it = values_.begin(); it != values_.end(); ++it) {
        if (it->second->getFollowingInterpolationFunction())
            delete (it->second->getFollowingInterpolationFunction());

        delete (it->second->getValue());
        delete (it->second);
    }
    values_.clear();
}

/////////////////// Special implementation for TransFunc*-Property

const PropertyKeyValue<TransFunc*>* TransFuncPropertyTimelineState::newKeyValue(float time) {
    time = floor(time * 10000.f) / 10000.f;
    if (values_.find(time) != values_.end())
        return 0;

    TransFunc* value = const_cast<TransFunc*>(getPropertyAt(time));

    PropertyKeyValue<TransFunc*>* kv = new PropertyKeyValue<TransFunc*>(value,time);

    values_.insert(std::pair<float,PropertyKeyValue<TransFunc*>*>(time,kv));

    std::map<float,PropertyKeyValue<TransFunc*>*>::iterator it;
    it = values_.find(time);

    // if new value is the first value:
    if (it == values_.begin()) {
        // only do something if there are multiple values
        it++;
        if (it != values_.end()) {
            InterpolationFunction<TransFunc*>* func = new InterpolationFunction<TransFunc*>();
            it->second->setForegoingInterpolationFunction(func);
            it--;
            it->second->setFollowingInterpolationFunction(func);
        }
    }
    else {
        it++;
        // if the new value is the last one
        if (it == values_.end()) {
            it--;
            InterpolationFunction<TransFunc*>* func = new InterpolationFunction<TransFunc*>();
            it->second->setForegoingInterpolationFunction(func);
            it--;
            it->second->setFollowingInterpolationFunction(func);
            it++;
        }
        else {
            InterpolationFunction<TransFunc*>* func1 = new InterpolationFunction<TransFunc*>();
            InterpolationFunction<TransFunc*>* func2 = new InterpolationFunction<TransFunc*>();
            it->second->setForegoingInterpolationFunction(func2);
            it--;
            it->second->setFollowingInterpolationFunction(func2);
            it->second->setForegoingInterpolationFunction(func1);
            it--;
            delete (*it).second->getFollowingInterpolationFunction();
            it->second->setFollowingInterpolationFunction(func1);
            it++;
        }
    }

    return kv;
}

bool TransFuncPropertyTimelineState::changeValueOfKeyValue(TransFunc* value, const PropertyKeyValue<TransFunc*>* keyvalue) {
    const float time = keyvalue->getTime();
    std::map<float,PropertyKeyValue<TransFunc*>*>::iterator it;
    it = values_.find(time);
    if (it != values_.end()) {
        if (value != it->second->getValue()) {
            delete (it->second->getValue());
            it->second->setValue(value->clone());
        }
        return true;
    }
    return false;
}

DeleteKeyValueReturn TransFuncPropertyTimelineState::deleteKeyValue(const PropertyKeyValue<TransFunc*>* keyvalue) {
    const float time = keyvalue->getTime();
    std::map<float,PropertyKeyValue<TransFunc*>*>::iterator it;
    it = values_.find(time);

    // if wrong parameter do nothing
    if (it == values_.end())
        return KV_NOT_THERE;

    // if keyvalue ist the only value do nothing
    if (it == values_.begin()) {
        it++;
        if (it == values_.end())
            return KV_IS_THE_ONLY_ONE;
        it--;
    }

    // if value is the first one
    if (it == values_.begin()) {
        it++;
        delete (it->second->getForegoingInterpolationFunction());
        it->second->setForegoingInterpolationFunction(0);
        it--;
        delete (it->second->getValue());
        values_.erase(time);
        return KV_DELETED;
    }
    // if value is the last one
    it++;
    if (it == values_.end()) {
        it--;
        it--;
        delete (it->second->getFollowingInterpolationFunction());
        it->second->setFollowingInterpolationFunction(0);
        it++;
        delete (it->second->getValue());
        values_.erase(time);
        return KV_DELETED;
    }
    // if value is in the middle
    InterpolationFunction<TransFunc*>* func = new InterpolationFunction<TransFunc*>();
    it->second->setForegoingInterpolationFunction(func);
    it--;
    delete it->second->getFollowingInterpolationFunction();
    delete it->second->getValue();
    delete it->second->getForegoingInterpolationFunction();
    it--;
    it->second->setFollowingInterpolationFunction(func);
    values_.erase(time);
    return KV_DELETED;
}

const TransFunc* TransFuncPropertyTimelineState::getPropertyAt(float time) {
    std::map<float,PropertyKeyValue<TransFunc*>*>::iterator it;
    it = values_.find(time);
    if (it != values_.end()) {
        if (it->second->getValue()) {
            return ((*it).second->getValue()->clone());
        }
        else {
            LERRORC("TransFuncPropertyTimelineState", "Keyvalue contains no value");
            return 0;
        }
    }

    it = values_.upper_bound(time);
    if (it == values_.begin())
        return (it->second->getValue()->clone());

    if (it == values_.end()) {
        it--;
        return (it->second->getValue()->clone());
    }
    std::map<float,PropertyKeyValue<TransFunc*>*>::iterator it2;
    it2 = it;
    it--;

    const InterpolationFunction<TransFunc*>* func = (*it).second->getFollowingInterpolationFunction();
    const MultiPointInterpolationFunction<TransFunc*>* multifunc = dynamic_cast<const MultiPointInterpolationFunction<TransFunc*>*>(func);
    if (multifunc) {
        // call a function with multiple points
        // create vector of the used keyvalues
        std::vector<PropertyKeyValue<TransFunc*>*> keys;
        // search for the first value in the multi-point interval
        while ((it!=values_.begin()) && (it->second->isSmoothed()))
            it--;

        do {
            keys.push_back((*it).second->clone());
            it++;
        } while ((it != values_.end()) && (it->second->isSmoothed()));

        if (it != values_.end())
            keys.push_back(it->second->clone());

        // interpolate value
        TransFunc* returnvalue = multifunc->interpolate(keys,time);

        // delete all copied keys
        std::vector<PropertyKeyValue<TransFunc*>*>::const_iterator delIt;
        for (delIt = keys.begin(); delIt != keys.end(); ++delIt)
            delete (*delIt);

        keys.clear();

        // return
        return returnvalue;
    }
    else {
        return func->interpolate(
            it->second->getValue(),
            it2->second->getValue(),
            (time - (it->first))/((it2->first) - (it->first)));
    }
}
TemplatePropertyTimelineState<TransFunc*>* TransFuncPropertyTimelineState::clone() {
    TransFuncPropertyTimelineState* timeline = new TransFuncPropertyTimelineState();

    std::map<float,PropertyKeyValue<TransFunc*>*>::const_iterator it;
    for (it = values_.begin(); it != values_.end(); ++it) {
        timeline->values_.insert(std::pair<float,PropertyKeyValue<TransFunc*>*>(it->first, it->second->clone()));
    }

    std::map<float,PropertyKeyValue<TransFunc*>*>::const_iterator it2;
    it2 = timeline->values_.begin();
    for (it = values_.begin(); it != values_.end(); ) {
        const InterpolationFunction<TransFunc*>* func;
        InterpolationFunction<TransFunc*>* func2;
        func = it->second->getFollowingInterpolationFunction();
        if (!func) {
            it2++;
            it++;
            continue;
        }
        func2 = func->clone();
        it2->second->setFollowingInterpolationFunction(func2);
        it2++;
        it++;
        it2->second->setForegoingInterpolationFunction(func2);
    }

    return timeline;
}

void TransFuncPropertyTimelineState::serialize(XmlSerializer& s) const {
    s.serialize("values", values_);
}

void TransFuncPropertyTimelineState::deserialize(XmlDeserializer& s) {
    s.deserialize("values", values_);
}


/////////////////// Special implementation for Camera*-Property

CameraPropertyTimelineState::CameraPropertyTimelineState(PropertyKeyValue<Camera>* kv) {
    values_.insert(std::pair<float,PropertyKeyValue<Camera>*>(kv->getTime(),kv));
}

CameraPropertyTimelineState::CameraPropertyTimelineState(std::map<float,PropertyKeyValue<Camera>*> values) {
    values_ = values;
}

CameraPropertyTimelineState::CameraPropertyTimelineState() {}

CameraPropertyTimelineState::~CameraPropertyTimelineState() {
    std::map<float,PropertyKeyValue<Camera>*>::iterator it;
    for (it = values_.begin(); it != values_.end(); ++it) {
        if (it->second->getFollowingInterpolationFunction())
            delete (it->second->getFollowingInterpolationFunction());

        delete (it->second);
    }
    values_.clear();
}

const PropertyKeyValue<Camera>* CameraPropertyTimelineState::newKeyValue(float time) {
    time = floor(time * 10000.f) / 10000.f;
    if (values_.find(time) != values_.end())
        return 0;

    //Camera value = const_cast<Camera>(getPropertyAt(time));
    Camera value = getPropertyAt(time);

/*    std::stringstream out;
    out << nodeCounter_++;
    std::string name("Node ");
    name.append(out.str());
    value->setNodeIdentifier(name);
    value->setDirection(value->getStrafe()); */

    PropertyKeyValue<Camera>* kv = new PropertyKeyValue<Camera>(value,time);

    values_.insert(std::pair<float,PropertyKeyValue<Camera>*>(time,kv));

    std::map<float,PropertyKeyValue<Camera>*>::iterator it;
    it = values_.find(time);
    //tgt::vec3 ownPosition = value.getPosition();
    // if new value is the first value:
    if (it == values_.begin()) {
        // only do something if there are multiple values
        it++;
        if (it != values_.end()) {
            InterpolationFunction<Camera>* func = new CameraSphericalLinearInterpolationFunction();
            it->second->setForegoingInterpolationFunction(func);
            //tgt::vec3 followingPosition = (*it).second->getValue().getPosition();
            it--;
            it->second->setFollowingInterpolationFunction(func);
            // (*it).second->getValue()->setDirection(followingPosition - ownPosition);
        }
    }
    else {
        it++;
        // if the new value is the last one
        if (it == values_.end()) {
            it--;
            InterpolationFunction<Camera>* func = new CameraSphericalLinearInterpolationFunction();
            it->second->setForegoingInterpolationFunction(func);
            it--;
            it->second->setFollowingInterpolationFunction(func);
            tgt::vec3 foregoingPosition = it->second->getValue().getPosition();
            it++;
            if (values_.size() >= 3) {
                // set direction of predecessor
                //tgt::vec3 followingPosition = it->second->getValue().getPosition();
                it--;
                it--;
                foregoingPosition = it->second->getValue().getPosition();
                it++;
//              (*it).second->getValue()->setDirection(0.5f*(followingPosition - foregoingPosition));
            }
        }
        else {
            // if the new value is in between
            InterpolationFunction<Camera>* func1 = new CameraSphericalLinearInterpolationFunction();
            InterpolationFunction<Camera>* func2 = new CameraSphericalLinearInterpolationFunction();

            it->second->setForegoingInterpolationFunction(func2);
            //tgt::vec3 followingPosition = it->second->getValue().getPosition();
            it--;
            it->second->setFollowingInterpolationFunction(func2);
            it->second->setForegoingInterpolationFunction(func1);
            it--;
            delete it->second->getFollowingInterpolationFunction();
            it->second->setFollowingInterpolationFunction(func1);
            //tgt::vec3 foregoingPosition = it->second->getValue().getPosition();
            it++;
        }
    }

    return kv;
}

bool CameraPropertyTimelineState::changeValueOfKeyValue(Camera value, const PropertyKeyValue<Camera>* keyvalue) {
    const float time = keyvalue->getTime();
    std::map<float,PropertyKeyValue<Camera>*>::iterator it;
    it = values_.find(time);

    if (it != values_.end()) {
        //TODO
        //if (value != it->second->getValue()) {
            it->second->setValue(value);
        //}
        return true;
    }
    return false;
}

DeleteKeyValueReturn CameraPropertyTimelineState::deleteKeyValue(const PropertyKeyValue<tgt::Camera>* keyvalue){
    const float time = keyvalue->getTime();
    std::map<float,PropertyKeyValue<tgt::Camera>*>::iterator it;
    it = values_.find(time);

    // if wrong parameter do nothing
    if (it == values_.end())
        return KV_NOT_THERE;

    // if keyvalue ist the only value do nothing
    if (it == values_.begin()) {
        it++;
        if (it == values_.end())
            return KV_IS_THE_ONLY_ONE;
        it--;
    }

    // if value is the first one
    if ( it == values_.begin() ) {
        it++;
        delete (it->second->getForegoingInterpolationFunction());
        it->second->setForegoingInterpolationFunction(0);
        it--;
        values_.erase(time);
        return KV_DELETED;
    }
    // if value is the last one
    it++;
    if (it == values_.end()) {
        it--;
        it--;
        delete (it->second->getFollowingInterpolationFunction());
        it->second->setFollowingInterpolationFunction(0);
        it++;
        values_.erase(time);
        return KV_DELETED;
    }
    // if value is in the middle
    InterpolationFunction<tgt::Camera>* func = new InterpolationFunction<tgt::Camera>();
    it->second->setForegoingInterpolationFunction(func);
    it--;
    delete it->second->getFollowingInterpolationFunction();
    delete it->second->getForegoingInterpolationFunction();
    it--;
    it->second->setFollowingInterpolationFunction(func);
    values_.erase(time);
    return KV_DELETED;
}

const tgt::Camera CameraPropertyTimelineState::getPropertyAt(float time){
    tgtAssert(!values_.empty(), "No key values");

    std::map<float,PropertyKeyValue<tgt::Camera>*>::iterator it;
    it = values_.find(time);
    if (it!=values_.end())
        return getPropertyAt(time + 0.001f);//((*it).second->getValue()->clone());

    it = values_.upper_bound(time);
    if (it == values_.begin())
        return (it->second->getValue());

    if (it == values_.end()) {
        it--;
        return (it->second->getValue());
    }
    std::map<float,PropertyKeyValue<tgt::Camera>*>::iterator it2;
    it2 = it;
    it--;

    const InterpolationFunction<tgt::Camera>* func = it->second->getFollowingInterpolationFunction();
    const MultiPointInterpolationFunction<tgt::Camera>* multifunc = dynamic_cast<const MultiPointInterpolationFunction<tgt::Camera>*>(func);
    if (multifunc) {
        // call a function with multiple points
        // create vector of the used keyvalues
        std::vector<PropertyKeyValue<tgt::Camera>*> keys;
        // search for the first value in the multi-point interval
        while ((it!=values_.begin()) && (it->second->isSmoothed()))
            it--;

        do {
            keys.push_back((*it).second->clone());
            it++;
        } while ((it != values_.end()) && (it->second->isSmoothed()));

        if (it != values_.end())
            keys.push_back(it->second->clone());

        // interpolate value
        tgt::Camera returnvalue = multifunc->interpolate(keys,time);

        // delete all copied keys
        std::vector<PropertyKeyValue<tgt::Camera>*>::const_iterator delIt;
        for (delIt = keys.begin(); delIt != keys.end(); ++delIt)
            delete (*delIt);

        keys.clear();

        // return
        return returnvalue;
    }
    else {
        return func->interpolate(
            it->second->getValue(),
            it2->second->getValue(),
            (time-(it->first))/(it2->first-(it->first)));
    }
}

TemplatePropertyTimelineState<tgt::Camera>* CameraPropertyTimelineState::clone(){
    CameraPropertyTimelineState* timeline = new CameraPropertyTimelineState();
    std::map<float,PropertyKeyValue<tgt::Camera>*>::const_iterator it;
    for (it = values_.begin(); it != values_.end(); ++it)
        timeline->values_.insert(std::pair<float,PropertyKeyValue<tgt::Camera>*>(it->first, it->second->clone()));

    std::map<float,PropertyKeyValue<tgt::Camera>*>::const_iterator it2;
    it2 = timeline->values_.begin();
    for (it = values_.begin(); it != values_.end();) {
        const InterpolationFunction<tgt::Camera>* func;
        InterpolationFunction<tgt::Camera>* func2;
        func = it->second->getFollowingInterpolationFunction();
        if (!func) {
            it2++;
            it++;
            continue;
        }
        func2 = func->clone();
        it2->second->setFollowingInterpolationFunction(func2);
        it2++;
        it++;
        it2->second->setForegoingInterpolationFunction(func2);
    }

    return timeline;
}

void CameraPropertyTimelineState::serialize(XmlSerializer& s) const {
    s.serialize("values", values_);
}

void CameraPropertyTimelineState::deserialize(XmlDeserializer& s) {
    s.deserialize("values", values_);
}

template class TemplatePropertyTimelineState<float>;
template class TemplatePropertyTimelineState<int>;
template class TemplatePropertyTimelineState<bool>;
template class TemplatePropertyTimelineState<tgt::ivec2>;
template class TemplatePropertyTimelineState<tgt::ivec3>;
template class TemplatePropertyTimelineState<tgt::ivec4>;
template class TemplatePropertyTimelineState<tgt::vec2>;
template class TemplatePropertyTimelineState<tgt::vec3>;
template class TemplatePropertyTimelineState<tgt::vec4>;
template class TemplatePropertyTimelineState<tgt::mat2>;
template class TemplatePropertyTimelineState<tgt::mat3>;
template class TemplatePropertyTimelineState<tgt::mat4>;
template class TemplatePropertyTimelineState<tgt::Camera>;
template class TemplatePropertyTimelineState<std::string>;
template class TemplatePropertyTimelineState<ShaderSource>;
template class TemplatePropertyTimelineState<TransFunc*>;

} // namespace voreen
