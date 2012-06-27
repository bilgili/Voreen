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

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

class Animation;

/**
 * This class organizes all animationsettings of one property of the type T.
 */
template <class T> class TemplatePropertyTimeline : public PropertyTimeline, public Observable<TimelineObserver> {
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
    bool empty();

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

} // namespace voreen

#endif
