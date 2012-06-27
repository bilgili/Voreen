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

#ifndef VRN_TEMPLATEPROPERTYTIMELINE_H
#define VRN_TEMPLATEPROPERTYTIMELINE_H

#include <list>
#include <deque>
#include <map>
#include "voreen/core/properties/templateproperty.h"
#include "voreen/core/animation/propertykeyvalue.h"
#include "voreen/core/animation/interpolationfunction.h"
#include "voreen/core/animation/propertytimeline.h"
#include "voreen/core/animation/undoableanimation.h"
#include "voreen/core/animation/templatepropertytimelinestate.h"
#include "voreen/core/animation/timelineobserver.h"
#include "voreen/core/animation/animation.h"

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

class Animation;

/**
 * This class organizes all animationsettings of one property of the type T.
 */
template <class T>
class TemplatePropertyTimeline : public PropertyTimeline, public Observable<TimelineObserver> {
public:
    /**
     * Constructor of the class.
     * Parameter is the property object which should be animated.
     */
    TemplatePropertyTimeline(TemplateProperty<T>* prop);

    /**
     * Destructor.
     */
    ~TemplatePropertyTimeline();

    /// returns true if the timeline is empty
    bool isEmpty();

    /**
     * Returns a sorted vector of all keyvalues of the timeline.
     */
    virtual const std::map<float,PropertyKeyValue<T>*> getKeyValues() const;

    /**
     * Returns a pointer to the corresponding property for the gui.
     * With this pointer it is possible to determine the subtype of the property.
     */
    virtual const TemplateProperty<T>* getCorrespondingProperty() const;

    /**
     * Adds a new keyvalue at a certain time:
     * If there already exists a keyvalue at the given time, this function does nothing.
     *
     * The interplationfunction at the time of the new keyvalue will be deleted and replaced by two default
     * interpolations before and after the new keyvalue.
     * Returns the pointer to the new keyvalue.
     * If there already existed a keyvalue at the given time a nullpointer is returned.
     * If the given time is after duration time a nullpointer is returned.
     */
    virtual const PropertyKeyValue<T>* newKeyValue(float time);

    /**
     * Changes the value of a keyvalue:
     * returns true, if the value was changed,
     * returns false, if the given keyvalue couldn't be found in the timeline.
     */
    virtual bool changeValueOfKeyValue(T value, const PropertyKeyValue<T>* keyvalue);

    /**
     * Changes the smooth-variable of a keyvalue:
     * returns true, if the value was changed,
     * returns false, if the given keyvalue couldn't be found in the timeline.
     */
    virtual bool changeSmoothnessOfKeyValue(bool smooth, const PropertyKeyValue<T>* keyvalue);

    /**
     * Changes the time of a keyvalue:
     * If there is already a keyvalue at the given time, this function deletes the given keyvalue
     * and sets the value of the already existing keyvalue to the deleted value.
     * If the order of the keyvalues changes by setting the new time, the function reacts as if
     * the given keyvalue was deleted and a new one was added at the given time with the same value as the deleted one.
     * Returns true if the given time is valid and the change is done.
     *
     * returns KV_NOT_FOUND                   if the given keyvalue couldn't be found in the timeline
     * returns KV_EQUAL_TO_OLD                if the new time is equal to the old one
     * returns KV_TIME_CHANGED                if the new time is in the same interval between the same other keyvalues
     * returns KV_EXISTS_AT_NEW_TIME          if there already exists a new keyvalue at the new time
     * returns KV_MOVED_TO_DIFFERENT_INTERVAL if the new time is in another interval between two other keyvalues.
     */
    virtual ChangeTimeOfKeyValueReturn changeTimeOfKeyValue(float time, const PropertyKeyValue<T>* keyvalue);

    /**
     * Deletes the given keyvalue:
     * The two interpolationfunctions before and after the deleted keyvalue will be deleted too and replaced by one default interpolation.
     * returns KV_NOT_FOUND       if the given keyvalue couldn't be found in the timeline
     * returns KV_IS_THE_ONLY_ONE if teh given keyvalue is the only value in the timeline and mustn#t be deleted
     * returns KV_DELETED         if the keyvalue was succesful deleted
     */
    virtual DeleteKeyValueReturn deleteKeyValue(const PropertyKeyValue<T>* keyvalue);

    /**
     * Sets the interpolationfunction between the given keyvalue and its successor.
     */
    virtual void setInterpolationFunctionAfter(InterpolationFunction<T>* func, PropertyKeyValue<T>* keyvalue);
    /**
     * Sets the interpolationfunction between the given keyvalue and its predecessor.
     */
    virtual void setInterpolationFunctionBefore(InterpolationFunction<T>* func, PropertyKeyValue<T>* keyvalue);

    /**
     * This function deletes all keyvalues of the timeline and takes the current setting of the processornetwork
     * as the standard value at time 0.
     */
    virtual void resetTimeline();

    /**
     * This function returns true if the corresponding property is linked with another one.
     */
    virtual bool propertyIsLinked() const;

    /**
     * Sets the current setting as a keyvalue at the given time if it differs from the animated value.
     */
    virtual void setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

    /**
     * Returns a copy of the current timelinestate.
     */
    virtual TemplatePropertyTimelineState<T>* getCurrentTimelineState() const ;

    /**
     * Sets the current propertytimelinestate by cloning the given timelinestate.
     */
    virtual void setCurrentTimelineState(TemplatePropertyTimelineState<T>* timelinestate);

    /**
     * This function registers the given animation class as an utils/observer.
     * Everytime there is a change in the timeline the observer will be contacted.
     */
    virtual void registerUndoObserver(Animation* observer);

    /**
     * Returns the name of the animated property.
     */
    virtual std::string getPropertyName() const ;

    /**
     * Sets the interactionmode of the corresponding property.
     */
    virtual void setInteractionMode(bool interactionmode,void* source);

    /**
     * Calculates the animated value to a certain point of time.
     */
    const T getPropertyAt(float time);

    /**
     * Creates a new undo-state.
     */
    virtual void setNewUndoState();

    /**
     *  Sets a bool if this property should be animated on rendering.
     */
    virtual void setActiveOnRendering(bool activeOnRendering);

    /**
     * Returns if the property is animated on rendering.
     */
    virtual bool getActiveOnRendering() const;

    /**
     * Returns true if the timeline was changed.
     */
    virtual bool isChanged() const ;

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    virtual Property* getProperty() const;

protected:
    friend class PropertyTimelineFactory;
    friend class XmlDeserializer;
    friend class TimelineObserver;
    friend class Animation;

    /**
     * Default constructor.
     */
    TemplatePropertyTimeline();

    /**
     * This function calculates the propertyvalue at the given time and changes the corresponding property object to that value.
     */
    virtual void renderAt(float time);

    /**
     * Called from animation-class.
     * All keyvalues after the durationtime will be deleted.
     */
    virtual void setDuration(float duration);

    /**
     * Pointer to the corresponding property.
     */
    TemplateProperty<T>* property_;

    bool activeOnRendering_;

    bool timelineChanged_;

    /**
     * Calculates the animated value to a certain point of time.
     */
    T privateGetPropertyAt(float time);

    /**
     * This function provides an undo-functionality for this timeline.
     * It should only be called by the undoobserver which manages all undos and redos of the whole animation.
     * If the undo-function is called another timelinestate is set as current, this means that all pointers to the keyvalues of this timeline get invalid.
     */
    virtual void undo();

    /**
     * This function provides a redo-functionality for this timeline.
     * It should only be called by the undoObserver which manages all undos and redos of the whole animation.
     * If the undo-function is called another timelinestate is set as current, this means that all pointers to the keyvalues of this timeline get invalid.
     */
    virtual void redo();

    /*
     * This function deletes all redo-states (stack lastundos_).
     */
    virtual void clearRedoStates();

    /**
     * This function deletes all states (stack lastundos_ and lastchanges_)
     */
    virtual void clearAllStates();

    /**
     * This function removes the oldest undo-state.
     * If the number of possible undo-steps is decreased, this function adapts the related stack lastchanges_.
     */
    virtual void removeOldestUndoState();

    /**
     * Pointer to the current timelinestate.
     */
    TemplatePropertyTimelineState<T>* timeline_;

    /**
     * Deques of all undo and redo timelinestates.
     */
    std::deque<TemplatePropertyTimelineState<T>*> lastChanges_;
    std::deque<TemplatePropertyTimelineState<T>*> lastUndos_;

    /**
     * Pointer to the animationclass as undoutils/observer.
     */
    Animation* undoObserver_;
};

template <>
TemplatePropertyTimeline<TransFunc*>::TemplatePropertyTimeline(TemplateProperty<TransFunc*>* prop);

template <>
void TemplatePropertyTimeline<TransFunc*>::resetTimeline();

template <>
TransFunc* TemplatePropertyTimeline<TransFunc*>::privateGetPropertyAt(float time);

template <>
void TemplatePropertyTimeline<TransFunc*>::renderAt(float time);

template <>
TemplatePropertyTimeline<tgt::Camera>::TemplatePropertyTimeline(TemplateProperty<tgt::Camera>* prop);

template <>
void TemplatePropertyTimeline<tgt::Camera>::resetTimeline();

template <>
const PropertyKeyValue<tgt::Camera>* TemplatePropertyTimeline<tgt::Camera>::newKeyValue(float time);

template <>
tgt::Camera TemplatePropertyTimeline<tgt::Camera>::privateGetPropertyAt(float time);

template <>
void TemplatePropertyTimeline<tgt::Camera>::renderAt(float time);

template <>
bool TemplatePropertyTimeline<tgt::Camera>::changeValueOfKeyValue(tgt::Camera value, const PropertyKeyValue<tgt::Camera>* keyvalue);

template <>
const TemplateProperty<tgt::Camera>* TemplatePropertyTimeline<tgt::Camera>::getCorrespondingProperty() const;

template <>
std::string TemplatePropertyTimeline<tgt::Camera>::getPropertyName() const;

template <>
void TemplatePropertyTimeline<tgt::Camera>::setInteractionMode(bool interactionmode, void* source);

template <>
DeleteKeyValueReturn TemplatePropertyTimeline<tgt::Camera>::deleteKeyValue(const PropertyKeyValue<tgt::Camera>* keyvalue);

template <>
void TemplatePropertyTimeline<tgt::Camera>::serialize(XmlSerializer& s) const;

template <>
void TemplatePropertyTimeline<tgt::Camera>::deserialize(XmlDeserializer& s);

template <>
void TemplatePropertyTimeline<tgt::Camera>::undo();

template <>
void TemplatePropertyTimeline<tgt::Camera>::redo();

template <>
void TemplatePropertyTimeline<ShaderSource>::renderAt(float time);

template <class T>
TemplatePropertyTimeline<T>::TemplatePropertyTimeline(TemplateProperty<T>* prop)
: property_(prop)
, activeOnRendering_(true)
, timelineChanged_(false)
{
    duration_ = 60.f * 15.f;

    timeline_ = new TemplatePropertyTimelineState<T>(new PropertyKeyValue<T>(property_->get(),0));
}

template <class T>
TemplatePropertyTimeline<T>::TemplatePropertyTimeline() {}

template <class T>
TemplatePropertyTimeline<T>::~TemplatePropertyTimeline() {
    delete timeline_;

    TemplatePropertyTimelineState<T>* temp;
    while (lastChanges_.size()) {
        temp = lastChanges_.back();
        lastChanges_.pop_back();
        delete temp;
    }
    while (lastUndos_.size()) {
        temp = lastUndos_.back();
        lastUndos_.pop_back();
        delete temp;
    }
}

template <class T>
bool TemplatePropertyTimeline<T>::isEmpty() {
    if (timeline_->getKeyValues().size() > 1)
        return false;
    else
        return true;
}

template <class T>
void TemplatePropertyTimeline<T>::resetTimeline() {
    timelineChanged_ = true;
    lastChanges_.push_back(timeline_);
    undoObserver_->animationChanged(this);

    timeline_ = new TemplatePropertyTimelineState<T>(new PropertyKeyValue<T>(property_->get(),0));

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();
}

template <class T>
const T TemplatePropertyTimeline<T>::getPropertyAt(float time) {
    return privateGetPropertyAt(time);
}

template <class T>
T TemplatePropertyTimeline<T>::privateGetPropertyAt(float time) {
    return timeline_->getPropertyAt(time);
}

template <class T>
const std::map<float,PropertyKeyValue<T>*> TemplatePropertyTimeline<T>::getKeyValues() const{
    return timeline_->getKeyValues();
}

template <class T>
const PropertyKeyValue<T>* TemplatePropertyTimeline<T>::newKeyValue(float time) {
    time = floor(time * 10000.f) / 10000.f;

    if (time > duration_) {
        timelineChanged_ = false;
        return 0;
    }

    lastChanges_.push_back(timeline_->clone());
    undoObserver_->animationChanged(this);

    const PropertyKeyValue<T>* kv = timeline_->newKeyValue(time);

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();

    return kv;
}

template <class T>
bool TemplatePropertyTimeline<T>::changeValueOfKeyValue(T value, const PropertyKeyValue<T>* keyvalue) {
    timelineChanged_ = true;
    std::string errorMsg;
    if (!(property_->isValidValue(value, errorMsg)))
        return false;
    bool temp = timeline_->changeValueOfKeyValue(value, keyvalue);

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();

    return temp;
}

template <class T>
bool TemplatePropertyTimeline<T>::changeSmoothnessOfKeyValue(bool smooth, const PropertyKeyValue<T>* keyvalue) {
    timelineChanged_ = true;
    bool temp = timeline_->changeSmoothnessOfKeyValue(smooth, keyvalue);

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();

    return temp;
}

template <class T>
ChangeTimeOfKeyValueReturn TemplatePropertyTimeline<T>::changeTimeOfKeyValue(float time, const PropertyKeyValue<T>* keyvalue) {
    if (time > duration_) {
        timelineChanged_ = false;
        return KV_TIME_AFTER_DURATION;
    }

    time = floor(time * 10000.f) / 10000.f;
    timelineChanged_ = true;

    ChangeTimeOfKeyValueReturn temp = timeline_->changeTimeOfKeyValue(time,keyvalue);

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();

    return temp;
}

template <class T>
DeleteKeyValueReturn TemplatePropertyTimeline<T>::deleteKeyValue(const PropertyKeyValue<T>* keyvalue) {
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

template <class T>
void TemplatePropertyTimeline<T>::setInterpolationFunctionBefore(InterpolationFunction<T>* func,PropertyKeyValue<T>* keyvalue) {
    timelineChanged_ = true;
    lastChanges_.push_back(timeline_->clone());
    undoObserver_->animationChanged(this);

    timeline_->setInterpolationFunctionBefore(func,keyvalue);

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();
}

template <class T>
void TemplatePropertyTimeline<T>::setInterpolationFunctionAfter(InterpolationFunction<T>* func,PropertyKeyValue<T>* keyvalue) {
    timelineChanged_ = true;
    lastChanges_.push_back(timeline_->clone());
    undoObserver_->animationChanged(this);

    timeline_->setInterpolationFunctionAfter(func,keyvalue);

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();
}

template <class T>
void TemplatePropertyTimeline<T>::renderAt(float time) {
    if (!activeOnRendering_)
        return;

    property_->set(getPropertyAt(time));
}

template <class T>
bool TemplatePropertyTimeline<T>::propertyIsLinked() const {
    if (property_->getLinks().size() > 0)
        return true;
    else
        return false;
}

template <class T>
const TemplateProperty<T>* TemplatePropertyTimeline<T>::getCorrespondingProperty() const {
    return property_;
}

template <class T>
void TemplatePropertyTimeline<T>::registerUndoObserver(Animation* observer) {
    undoObserver_ = observer;
}

template <class T>
std::string TemplatePropertyTimeline<T>::getPropertyName() const {
    return property_->getGuiName();
}

template <class T>
void TemplatePropertyTimeline<T>::undo() {
    lastUndos_.push_back(timeline_);
    timeline_ = lastChanges_.back();
    lastChanges_.pop_back();

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();
}

template <class T>
void TemplatePropertyTimeline<T>::redo() {
    lastChanges_.push_back(timeline_);
    timeline_ = lastUndos_.back();
    lastUndos_.pop_back();

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();
}

template <class T>
void TemplatePropertyTimeline<T>::clearRedoStates() {
    TemplatePropertyTimelineState<T>* temp;
    while (lastUndos_.size() > 0) {
        temp = lastUndos_.back();
        lastUndos_.pop_back();
        delete temp;
    }
}

template <class T>
void TemplatePropertyTimeline<T>::clearAllStates() {
    TemplatePropertyTimelineState<T>* temp;
    while (lastUndos_.size() > 0) {
        temp = lastUndos_.back();
        lastUndos_.pop_back();
        delete temp;
    }
    while (lastChanges_.size() > 0) {
        temp = lastChanges_.back();
        lastChanges_.pop_back();
        delete temp;
    }
}

template <class T>
void TemplatePropertyTimeline<T>::removeOldestUndoState() {
    TemplatePropertyTimelineState<T>* temp;
    temp = lastChanges_.front();
    lastChanges_.pop_front();
    delete temp;
}

template <class T>
void TemplatePropertyTimeline<T>::setInteractionMode(bool interactionmode,void* source) {
    property_->toggleInteractionMode(interactionmode,source);
}

template <class T>
void TemplatePropertyTimeline<T>::setNewUndoState() {
    lastChanges_.push_back(timeline_->clone());
    undoObserver_->animationChanged(this);
}

template <class T>
bool TemplatePropertyTimeline<T>::getActiveOnRendering() const {
    return activeOnRendering_;
}

template <class T>
void TemplatePropertyTimeline<T>::setActiveOnRendering(bool activeOnRendering) {
    activeOnRendering_ = activeOnRendering;
}

template <class T>
bool TemplatePropertyTimeline<T>::isChanged() const {
    return timelineChanged_;
}

template <class T>
TemplatePropertyTimelineState<T>* TemplatePropertyTimeline<T>::getCurrentTimelineState() const {
    return timeline_->clone();
}

template <class T>
void TemplatePropertyTimeline<T>::setCurrentTimelineState(TemplatePropertyTimelineState<T>* timelinestate) {
    timelineChanged_ = true;
    lastChanges_.push_back(timeline_->clone());
    undoObserver_->animationChanged(this);

    delete timeline_;
    timeline_ = timelinestate->clone();

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();
}

template <class T>
void TemplatePropertyTimeline<T>::setDuration(float duration) {
    duration_ = duration;

    timeline_->setDuration(duration);

    const std::vector<TimelineObserver*> timelineObservers = getObservers();
    std::vector<TimelineObserver*>::const_iterator it;
    for (it = timelineObservers.begin(); it != timelineObservers.end(); ++it)
        (*it)->timelineChanged();
}

template <>
void TemplatePropertyTimeline<float>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<int>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<bool>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<tgt::ivec2>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<tgt::vec2>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<tgt::ivec3>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<tgt::vec3>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<tgt::ivec4>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<tgt::vec4>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<tgt::mat2>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<tgt::mat3>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<tgt::mat4>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<std::string>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<ShaderSource>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<tgt::Camera>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <>
void TemplatePropertyTimeline<TransFunc*>::setCurrentSettingAsKeyvalue(float time, bool forceKeyValue);

template <class T>
void TemplatePropertyTimeline<T>::serialize(XmlSerializer& s) const {
    s.serialize("activeOnRendering", activeOnRendering_);
    s.serialize("propertyOwner", property_->getOwner());
    s.serialize("propertyId", property_->getID());
    s.serialize("duration", duration_);
    s.serialize("timeline", timeline_);
    s.serialize("tlchanged", timelineChanged_);
}

template <class T>
void TemplatePropertyTimeline<T>::deserialize(XmlDeserializer& s) {
    s.deserialize("activeOnRendering", activeOnRendering_);
    PropertyOwner* propertyOwner = 0;
    s.deserialize("propertyOwner", propertyOwner);
    std::string propertyId;
    s.deserialize("propertyId", propertyId);
    property_ = dynamic_cast<TemplateProperty<T>*>(propertyOwner->getProperty(propertyId));
    if (!property_)
        LWARNINGC("TemplatePropertyTimeline", "Property defined in animation timeline does not exist: "
        << propertyOwner->getName() << "::" << propertyId);

    s.deserialize("duration", duration_);
    s.deserialize("timeline", timeline_);
    s.deserialize("tlchanged", timelineChanged_);
}

template <class T>
Property* TemplatePropertyTimeline<T>::getProperty() const {
    return property_;
}

} // namespace voreen

#endif
