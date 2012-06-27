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
{
    duration_ = 60.f * 15.f;
    TransFunc* func = property_->get();
    if (func)
        func = func->clone();
    else {
        func = new TransFunc1DKeys();
        property_->set(func->clone());
    }
    timeline_ = new TransFuncPropertyTimelineState(new PropertyKeyValue<TransFunc*>(func,0));
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
    TransFuncPropertyTimelineState* tl = dynamic_cast<TransFuncPropertyTimelineState*>(timeline_);
    TransFunc* func = const_cast<TransFunc*>(tl->getPropertyAt(time));
    return func;
}

template <>
void TemplatePropertyTimeline<TransFunc*>::renderAt(float time) {
    if (!activeOnRendering_)
        return;

    property_->set(getPropertyAt(time));
}

////////////////// special case Camera*
template <>
TemplatePropertyTimeline<Camera>::TemplatePropertyTimeline(TemplateProperty<Camera>* prop)
    : property_(prop)
    , activeOnRendering_(true)
    , timelineChanged_(false)
{
    duration_ = 60.f * 15.f;

    tgt::Camera cam = property_->get();

//    Camera* node0 = new Camera(cam->getPosition(), cam->getFocus(), cam->getUpVector(), cam->getStrafe());
//    node0->setNodeIdentifier("Node 0");
//    node0->setDirection(node0->getStrafe());
    Camera node0 = Camera(cam);
    timeline_ = new CameraPropertyTimelineState(new PropertyKeyValue<Camera>(node0, 0.f));
}

template <>
void TemplatePropertyTimeline<Camera>::resetTimeline() {
    timelineChanged_ = true;
    lastChanges_.push_back(timeline_);
    undoObserver_->animationChanged(this);

    tgt::Camera cam = property_->get();

/*    Camera* node0 = new Camera(cam->getPosition(), cam->getFocus(), cam->getUpVector(), cam->getStrafe());
    node0->setNodeIdentifier("Node 0");
    node0->setDirection(node0->getStrafe()); */
    Camera node0 = Camera(cam);
    timeline_ = new CameraPropertyTimelineState(new PropertyKeyValue<Camera>(node0,0));

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it) {
        (*it)->timelineChanged();
    }
}

template <>
const PropertyKeyValue<Camera>* TemplatePropertyTimeline<Camera>::newKeyValue(float time) {
    time = floor(time * 10000.f) / 10000.f;

    if (time > duration_) {
        timelineChanged_ = false;
        return 0;
    }
    lastChanges_.push_back(timeline_->clone());
    undoObserver_->animationChanged(this);

    const PropertyKeyValue<Camera>* kv = timeline_->newKeyValue(time);

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it) {
        (*it)->timelineChanged();
    }

return kv;
}

template <>
Camera TemplatePropertyTimeline<Camera>::privateGetPropertyAt(float time) {
    CameraPropertyTimelineState* tl = dynamic_cast<CameraPropertyTimelineState*>(timeline_);
    tgtAssert(tl, "No CameraPropertyTimelineState");
    //Camera cam = const_cast<Camera>(tl->getPropertyAt(time));
    Camera cam = tl->getPropertyAt(time);
    return cam;
}

template <>
void TemplatePropertyTimeline<Camera>::renderAt(float time) {
    if (!activeOnRendering_)
        return;

    Camera camera = getPropertyAt(time);
    static_cast<CameraProperty*>(property_)->set(camera);
}

template <>
bool TemplatePropertyTimeline<Camera>::changeValueOfKeyValue(Camera value, const PropertyKeyValue<Camera>* keyvalue) {
    tgtAssert(property_, "No property");
    timelineChanged_ = true;
    std::string errorMsg;
    if (!property_->isValidValue(value, errorMsg))
        return false;
    bool temp = timeline_->changeValueOfKeyValue(value, keyvalue);

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();

    return temp;
}

//template <>
//bool TemplatePropertyTimeline<Camera*>::propertyIsLinked() const {
//    tgtAssert(property_, "No property");
//    if (property_->getLinks().size() > 0)
//        return true;
//    else
//        return false;
//}

template <>
const TemplateProperty<Camera>* TemplatePropertyTimeline<Camera>::getCorrespondingProperty() const {
    return 0;
}

template <>
std::string TemplatePropertyTimeline<Camera>::getPropertyName() const {
    tgtAssert(property_, "No property");
    return property_->getGuiName();
}

template <>
void TemplatePropertyTimeline<Camera>::setInteractionMode(bool interactionmode, void* source) {
    tgtAssert(property_, "No property");
    property_->toggleInteractionMode(interactionmode, source);
}

template <>
DeleteKeyValueReturn TemplatePropertyTimeline<Camera>::deleteKeyValue(const PropertyKeyValue<Camera>* keyvalue) {
    timelineChanged_ = true;
    lastChanges_.push_back(timeline_->clone());
    undoObserver_->animationChanged(this);

    DeleteKeyValueReturn temp = timeline_->deleteKeyValue(keyvalue);

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();

    return temp;
}

template <>
void TemplatePropertyTimeline<Camera>::serialize(XmlSerializer& s) const {
    tgtAssert(property_, "No property");
    s.serialize("activeOnRendering", activeOnRendering_);
    s.serialize("propertyOwner", property_->getOwner());
    s.serialize("propertyId", property_->getID());
    s.serialize("duration", duration_);
    s.serialize("timeline", timeline_);
}

template <>
void TemplatePropertyTimeline<Camera>::deserialize(XmlDeserializer& s) {
    s.deserialize("activeOnRendering", activeOnRendering_);
    PropertyOwner* propertyOwner = 0;
    s.deserialize("propertyOwner", propertyOwner);
    std::string propertyId;
    s.deserialize("propertyId", propertyId);
    if (propertyOwner)
        property_ = dynamic_cast<CameraProperty*>(propertyOwner->getProperty(propertyId));
    else
        LERRORC("TemplatePropertyTimeline<Camera>", "deserialize(): no property owner");
    s.deserialize("duration", duration_);
    s.deserialize("timeline", timeline_);
}

template <>
void TemplatePropertyTimeline<Camera>::undo() {
    lastUndos_.push_back(timeline_);
    timeline_ = new CameraPropertyTimelineState(lastChanges_.back()->getKeyValues());
    lastChanges_.pop_back();

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();
}

template <>
void TemplatePropertyTimeline<Camera>::redo() {
    lastChanges_.push_back(timeline_);
    timeline_ = new CameraPropertyTimelineState(lastUndos_.back()->getKeyValues());
    lastUndos_.pop_back();

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();
}


////////////////// special case ShaderSource
template <>
void TemplatePropertyTimeline<ShaderSource>::renderAt(float time) {
    if (!activeOnRendering_)
        return;

    // changing the shadersource only if it was changed otherwise the shader has to be rebuild everytime
    ShaderSource sh = property_->get();
    ShaderSource newsh = getPropertyAt(time);
    newsh.originalFragmentFilename_ = sh.originalFragmentFilename_;
    newsh.originalVertexFilename_ = sh.originalVertexFilename_;
    newsh.originalGeometryFilename_ = sh.originalGeometryFilename_;

    if (sh != newsh)
        property_->set(newsh);
}

// standard implementation
//template <class T>
//TemplatePropertyTimeline<T>::TemplatePropertyTimeline(TemplateProperty<T>* prop)
//    : property_(prop)
//    , activeOnRendering_(true)
//    , timelineChanged_(false)
//{
//    duration_ = 60.f * 15.f;
//
//    timeline_ = new TemplatePropertyTimelineState<T>(new PropertyKeyValue<T>(property_->get(),0));
//}
//
//template <class T>
//TemplatePropertyTimeline<T>::TemplatePropertyTimeline() {}
//
//template <class T>
//TemplatePropertyTimeline<T>::~TemplatePropertyTimeline() {
//    delete timeline_;
//
//    TemplatePropertyTimelineState<T>* temp;
//    while (lastChanges_.size()) {
//        temp = lastChanges_.back();
//        lastChanges_.pop_back();
//        delete temp;
//    }
//    while (lastUndos_.size()) {
//        temp = lastUndos_.back();
//        lastUndos_.pop_back();
//        delete temp;
//    }
//}
//
//template <class T>
//bool TemplatePropertyTimeline<T>::isEmpty() {
//    if (timeline_->getKeyValues().size() > 1)
//        return false;
//    else
//        return true;
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::resetTimeline() {
//    timelineChanged_ = true;
//    lastChanges_.push_back(timeline_);
//    undoObserver_->animationChanged(this);
//
//    timeline_ = new TemplatePropertyTimelineState<T>(new PropertyKeyValue<T>(property_->get(),0));
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//}
//
//template <class T>
//const T TemplatePropertyTimeline<T>::getPropertyAt(float time) {
//    return privateGetPropertyAt(time);
//}
//
//template <class T>
//T TemplatePropertyTimeline<T>::privateGetPropertyAt(float time) {
//    return timeline_->getPropertyAt(time);
//}
//
//template <class T>
//const std::map<float,PropertyKeyValue<T>*> TemplatePropertyTimeline<T>::getKeyValues() const{
//    return timeline_->getKeyValues();
//}
//
//template <class T>
//const PropertyKeyValue<T>* TemplatePropertyTimeline<T>::newKeyValue(float time) {
//    time = floor(time * 10000.f) / 10000.f;
//
//    if (time > duration_) {
//        timelineChanged_ = false;
//        return 0;
//    }
//
//    lastChanges_.push_back(timeline_->clone());
//    undoObserver_->animationChanged(this);
//
//    const PropertyKeyValue<T>* kv = timeline_->newKeyValue(time);
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//
//    return kv;
//}
//
//template <class T>
//bool TemplatePropertyTimeline<T>::changeValueOfKeyValue(T value, const PropertyKeyValue<T>* keyvalue) {
//    timelineChanged_ = true;
//    std::string errorMsg;
//    if (!(property_->isValidValue(value, errorMsg)))
//        return false;
//    bool temp = timeline_->changeValueOfKeyValue(value, keyvalue);
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//
//    return temp;
//}
//
//template <class T>
//bool TemplatePropertyTimeline<T>::changeSmoothnessOfKeyValue(bool smooth, const PropertyKeyValue<T>* keyvalue) {
//    timelineChanged_ = true;
//    bool temp = timeline_->changeSmoothnessOfKeyValue(smooth, keyvalue);
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//
//    return temp;
//}
//
//template <class T>
//ChangeTimeOfKeyValueReturn TemplatePropertyTimeline<T>::changeTimeOfKeyValue(float time, const PropertyKeyValue<T>* keyvalue) {
//    if (time > duration_) {
//        timelineChanged_ = false;
//        return KV_TIME_AFTER_DURATION;
//    }
//
//    time = floor(time * 10000.f) / 10000.f;
//    timelineChanged_ = true;
//
//    ChangeTimeOfKeyValueReturn temp = timeline_->changeTimeOfKeyValue(time,keyvalue);
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//
//    return temp;
//}
//
//template <class T>
//DeleteKeyValueReturn TemplatePropertyTimeline<T>::deleteKeyValue(const PropertyKeyValue<T>* keyvalue) {
//    timelineChanged_ = true;
//    lastChanges_.push_back(timeline_->clone());
//    undoObserver_->animationChanged(this);
//
//    DeleteKeyValueReturn temp = timeline_->deleteKeyValue(keyvalue);
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//
//    return temp;
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::setInterpolationFunctionBefore(InterpolationFunction<T>* func,PropertyKeyValue<T>* keyvalue) {
//    timelineChanged_ = true;
//    lastChanges_.push_back(timeline_->clone());
//    undoObserver_->animationChanged(this);
//
//    timeline_->setInterpolationFunctionBefore(func,keyvalue);
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::setInterpolationFunctionAfter(InterpolationFunction<T>* func,PropertyKeyValue<T>* keyvalue) {
//    timelineChanged_ = true;
//    lastChanges_.push_back(timeline_->clone());
//    undoObserver_->animationChanged(this);
//
//    timeline_->setInterpolationFunctionAfter(func,keyvalue);
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::renderAt(float time) {
//    if (!activeOnRendering_)
//        return;
//
//    property_->set(getPropertyAt(time));
//}
//
//template <class T>
//bool TemplatePropertyTimeline<T>::propertyIsLinked() const {
//    if (property_->getLinks().size() > 0)
//        return true;
//    else
//        return false;
//}
//
//template <class T>
//const TemplateProperty<T>* TemplatePropertyTimeline<T>::getCorrespondingProperty() const {
//    return property_;
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::registerUndoObserver(Animation* observer) {
//    undoObserver_ = observer;
//}
//
//template <class T>
//std::string TemplatePropertyTimeline<T>::getPropertyName() const {
//    return property_->getGuiName();
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::undo() {
//    lastUndos_.push_back(timeline_);
//    timeline_ = lastChanges_.back();
//    lastChanges_.pop_back();
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::redo() {
//    lastChanges_.push_back(timeline_);
//    timeline_ = lastUndos_.back();
//    lastUndos_.pop_back();
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::clearRedoStates() {
//    TemplatePropertyTimelineState<T>* temp;
//    while (lastUndos_.size() > 0) {
//        temp = lastUndos_.back();
//        lastUndos_.pop_back();
//        delete temp;
//    }
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::clearAllStates() {
//    TemplatePropertyTimelineState<T>* temp;
//    while (lastUndos_.size() > 0) {
//        temp = lastUndos_.back();
//        lastUndos_.pop_back();
//        delete temp;
//    }
//    while (lastChanges_.size() > 0) {
//        temp = lastChanges_.back();
//        lastChanges_.pop_back();
//        delete temp;
//    }
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::removeOldestUndoState() {
//    TemplatePropertyTimelineState<T>* temp;
//    temp = lastChanges_.front();
//    lastChanges_.pop_front();
//    delete temp;
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::setInteractionMode(bool interactionmode,void* source) {
//    property_->toggleInteractionMode(interactionmode,source);
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::setNewUndoState() {
//    lastChanges_.push_back(timeline_->clone());
//    undoObserver_->animationChanged(this);
//}
//
//template <class T>
//bool TemplatePropertyTimeline<T>::getActiveOnRendering() const {
//    return activeOnRendering_;
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::setActiveOnRendering(bool activeOnRendering) {
//    activeOnRendering_ = activeOnRendering;
//}
//
//template <class T>
//bool TemplatePropertyTimeline<T>::isChanged() const {
//    return timelineChanged_;
//}
//
//template <class T>
//TemplatePropertyTimelineState<T>* TemplatePropertyTimeline<T>::getCurrentTimelineState() const {
//    return timeline_->clone();
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::setCurrentTimelineState(TemplatePropertyTimelineState<T>* timelinestate) {
//    timelineChanged_ = true;
//    lastChanges_.push_back(timeline_->clone());
//    undoObserver_->animationChanged(this);
//
//    delete timeline_;
//    timeline_ = timelinestate->clone();
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//}
//
//template <class T>
//void TemplatePropertyTimeline<T>::setDuration(float duration) {
//    duration_ = duration;
//
//    timeline_->setDuration(duration);
//
//    const std::vector<TimelineObserver*> timelineObservers = getObservers();
//    std::vector<TimelineObserver*>::const_iterator it;
//    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
//        (*it)->timelineChanged();
//}

/////////////////// special implementation of the function 'setCurrentSettingAsKeyvalue' for all possible templates
template <>
void TemplatePropertyTimeline<float>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time * 10000.f) / 10000.f;
    float value = property_->get();
    float animatedValue = getPropertyAt(time);
    if (value != animatedValue) {
        const PropertyKeyValue<float>* kv = timeline_->newKeyValue(time);
        if (!kv)
            kv = new PropertyKeyValue<float>(value,time);
        changeValueOfKeyValue(value,kv);
    }
    else if (forceKeyValue)
        newKeyValue(time);
}

template <>
void TemplatePropertyTimeline<int>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time * 10000.f) / 10000.f;
    int value = property_->get();
    int animatedValue = getPropertyAt(time);
    if (value != animatedValue) {
        const PropertyKeyValue<int>* kv = timeline_->newKeyValue(time);
        if (!kv)
            kv = new PropertyKeyValue<int>(value,time);
        changeValueOfKeyValue(value,kv);
    }
    else if (forceKeyValue)
        newKeyValue(time);
}

template <>
void TemplatePropertyTimeline<bool>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time * 10000.f) / 10000.f;
    bool value = property_->get();
    bool animatedValue = getPropertyAt(time);
    if (value != animatedValue) {
        const PropertyKeyValue<bool>* kv = timeline_->newKeyValue(time);
        if (!kv)
            kv = new PropertyKeyValue<bool>(value,time);
        changeValueOfKeyValue(value,kv);
    }
    else if (forceKeyValue)
        newKeyValue(time);
}

template <>
void TemplatePropertyTimeline<tgt::ivec2>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time * 10000.f) / 10000.f;
    tgt::ivec2 value = property_->get();
    tgt::ivec2 animatedValue = getPropertyAt(time);
    if ((value.elem[0] != animatedValue.elem[0]) || (value.elem[1] != animatedValue.elem[1])) {
        const PropertyKeyValue<tgt::ivec2>* kv = timeline_->newKeyValue(time);
        if (!kv)
            kv = new PropertyKeyValue<tgt::ivec2>(value,time);
        changeValueOfKeyValue(value,kv);
    }
    else if (forceKeyValue)
        newKeyValue(time);
}

template <>
void TemplatePropertyTimeline<tgt::vec2>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time*10000)/10000;
    tgt::vec2 value = property_->get();
    tgt::vec2 animatedValue = getPropertyAt(time);
    if ((value.elem[0] != animatedValue.elem[0]) || (value.elem[1] != animatedValue.elem[1])) {
        const PropertyKeyValue<tgt::vec2>* kv = timeline_->newKeyValue(time);
        if (!kv) {
            kv = new PropertyKeyValue<tgt::vec2>(value,time);
        }
        this->changeValueOfKeyValue(value,kv);
    }
    else {
        if (forceKeyValue){
            newKeyValue(time);
        }
    }
}

template <>
void TemplatePropertyTimeline<tgt::ivec3>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time*10000)/10000;
    tgt::ivec3 value = property_->get();
    tgt::ivec3 animatedValue = getPropertyAt(time);
    if ((value.elem[0] != animatedValue.elem[0]) || (value.elem[1] != animatedValue.elem[1]) || (value.elem[2] != animatedValue.elem[2])) {
        const PropertyKeyValue<tgt::ivec3>* kv = timeline_->newKeyValue(time);
        if (!kv) {
            kv = new PropertyKeyValue<tgt::ivec3>(value,time);
        }
        this->changeValueOfKeyValue(value,kv);
    }
    else {
        if (forceKeyValue){
            newKeyValue(time);
        }
    }
}

template <>
void TemplatePropertyTimeline<tgt::vec3>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time*10000)/10000;
    tgt::vec3 value = property_->get();
    tgt::vec3 animatedValue = getPropertyAt(time);
    if ((value.elem[0] != animatedValue.elem[0]) || (value.elem[1] != animatedValue.elem[1]) || (value.elem[2] != animatedValue.elem[2])) {
        const PropertyKeyValue<tgt::vec3>* kv = timeline_->newKeyValue(time);
        if (!kv) {
            kv = new PropertyKeyValue<tgt::vec3>(value,time);
        }
        this->changeValueOfKeyValue(value,kv);
    }
    else {
        if (forceKeyValue){
            newKeyValue(time);
        }
    }
}

template <>
void TemplatePropertyTimeline<tgt::ivec4>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time*10000)/10000;
    tgt::ivec4 value = property_->get();
    tgt::ivec4 animatedValue = getPropertyAt(time);
    if ((value.elem[0] != animatedValue.elem[0]) || (value.elem[1] != animatedValue.elem[1]) || (value.elem[2] != animatedValue.elem[2]) || (value.elem[3] != animatedValue.elem[3])) {
        const PropertyKeyValue<tgt::ivec4>* kv = timeline_->newKeyValue(time);
        if (!kv) {
            kv = new PropertyKeyValue<tgt::ivec4>(value,time);
        }
        this->changeValueOfKeyValue(value,kv);
    }
    else {
        if (forceKeyValue){
            newKeyValue(time);
        }
    }
}

template <>
void TemplatePropertyTimeline<tgt::vec4>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time*10000)/10000;
    tgt::vec4 value = property_->get();
    tgt::vec4 animatedValue = getPropertyAt(time);
    if ((value.elem[0] != animatedValue.elem[0]) || (value.elem[1] != animatedValue.elem[1]) || (value.elem[2] != animatedValue.elem[2]) || (value.elem[3] != animatedValue.elem[3])) {
        const PropertyKeyValue<tgt::vec4>* kv = timeline_->newKeyValue(time);
        if (!kv) {
            kv = new PropertyKeyValue<tgt::vec4>(value,time);
        }
        this->changeValueOfKeyValue(value,kv);
    }
    else {
        if (forceKeyValue){
            newKeyValue(time);
        }
    }
}

template <>
void TemplatePropertyTimeline<tgt::mat2>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time*10000)/10000;
    tgt::mat2 value = property_->get();
    tgt::mat2 animatedValue = getPropertyAt(time);
    bool valueChanged = false;

    for (int i = 0; i < value.size; i++) {
        if (value.elem[i] != animatedValue.elem[i]) {
            valueChanged = true;
        }
    }
    if (valueChanged) {
        const PropertyKeyValue<tgt::mat2>* kv = timeline_->newKeyValue(time);
        if (!kv) {
            kv = new PropertyKeyValue<tgt::mat2>(value,time);
        }
        this->changeValueOfKeyValue(value,kv);
    }
    else {
        if (forceKeyValue){
            newKeyValue(time);
        }
    }
}

template <>
void TemplatePropertyTimeline<tgt::mat3>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time*10000)/10000;
    tgt::mat3 value = property_->get();
    tgt::mat3 animatedValue = getPropertyAt(time);
    bool valueChanged = false;

    for (int i = 0; i < value.size; i++) {
        if (value.elem[i] != animatedValue.elem[i]) {
            valueChanged = true;
        }
    }
    if (valueChanged) {
        const PropertyKeyValue<tgt::mat3>* kv = timeline_->newKeyValue(time);
        if (!kv) {
            kv = new PropertyKeyValue<tgt::mat3>(value,time);
        }
        this->changeValueOfKeyValue(value,kv);
    }
    else {
        if (forceKeyValue){
            newKeyValue(time);
        }
    }
}

template <>
void TemplatePropertyTimeline<tgt::mat4>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time*10000)/10000;
    tgt::mat4 value = property_->get();
    tgt::mat4 animatedValue = getPropertyAt(time);
    bool valueChanged = false;

    for (int i = 0; i < value.size; i++) {
            if (value.elem[i] != animatedValue.elem[i]) {
                valueChanged = true;
            }
    }
    if (valueChanged) {
        const PropertyKeyValue<tgt::mat4>* kv = timeline_->newKeyValue(time);
        if (!kv) {
            kv = new PropertyKeyValue<tgt::mat4>(value,time);
        }
        this->changeValueOfKeyValue(value,kv);
    }else {
        if (forceKeyValue){
            newKeyValue(time);
        }
    }
}

template <>
void TemplatePropertyTimeline<std::string>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time*10000)/10000;
    std::string value = property_->get();
    std::string animatedValue = getPropertyAt(time);
    if (value.compare(animatedValue)!=0) {
        const PropertyKeyValue<std::string>* kv = timeline_->newKeyValue(time);
        if (!kv) {
            kv = new PropertyKeyValue<std::string>(value,time);
        }
        this->changeValueOfKeyValue(value,kv);
    }
    else {
        if (forceKeyValue){
            newKeyValue(time);
        }
    }
}

template <>
void TemplatePropertyTimeline<ShaderSource>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {
    time = floor(time*10000)/10000;
    ShaderSource value = property_->get();
    ShaderSource animatedValue = getPropertyAt(time);
    if (value!=animatedValue)
    {
        const PropertyKeyValue<ShaderSource>* kv = timeline_->newKeyValue(time);
        if (!kv) {
            kv = new PropertyKeyValue<ShaderSource>(value,time);
        }
        this->changeValueOfKeyValue(value,kv);
    }else {
        if (forceKeyValue){
            newKeyValue(time);
        }
    }
}

template <>
void TemplatePropertyTimeline<Camera>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue) {

    time = floor(time*10000)/10000;

    tgt::Camera cam = property_->get();
/*    Camera* value = new CameraNode(cam->getPosition(), cam->getFocus(), cam->getUpVector(), cam->getStrafe());
    CameraNode* animatedValue = getPropertyAt(time); */
    Camera value = cam;
    Camera animatedValue = getPropertyAt(time);

    if ((value.getPosition() != animatedValue.getPosition())
        ||(value.getFocus() != animatedValue.getFocus())
        ||(value.getUpVector() != animatedValue.getUpVector()))
    {
        const PropertyKeyValue<Camera>* kv = timeline_->newKeyValue(time);
        if (!kv) {
            kv = new PropertyKeyValue<Camera>(value,time);
        }
        this->changeValueOfKeyValue(value,kv);
    }
    else {
        if (forceKeyValue){
            newKeyValue(time);
        }
    }
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

    if ((f1) && (f2)) {
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

    if ((f3) && (f4)) {
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

template class TemplatePropertyTimeline<float>;
template class TemplatePropertyTimeline<int>;
template class TemplatePropertyTimeline<bool>;
template class TemplatePropertyTimeline<tgt::ivec2>;
template class TemplatePropertyTimeline<tgt::ivec3>;
template class TemplatePropertyTimeline<tgt::ivec4>;
template class TemplatePropertyTimeline<tgt::vec2>;
template class TemplatePropertyTimeline<tgt::vec3>;
template class TemplatePropertyTimeline<tgt::vec4>;
template class TemplatePropertyTimeline<tgt::mat2>;
template class TemplatePropertyTimeline<tgt::mat3>;
template class TemplatePropertyTimeline<tgt::mat4>;
template class TemplatePropertyTimeline<tgt::Camera>;
template class TemplatePropertyTimeline<std::string>;
template class TemplatePropertyTimeline<ShaderSource>;
template class TemplatePropertyTimeline<TransFunc*>;

} // namespace voreen
