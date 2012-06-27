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

#ifndef VRN_TEMPLATEPROPERTYTIMELINESTATE_H
#define VRN_TEMPLATEPROPERTYTIMELINESTATE_H

#include <vector>
#include <map>
#include "voreen/core/properties/templateproperty.h"
#include "voreen/core/animation/propertykeyvalue.h"
#include "voreen/core/animation/interpolationfunction.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/io/serialization/serialization.h"

#include "tgt/camera.h"

namespace voreen {

class Animation;
class ShaderSource;

//FIXME: move into class!
enum ChangeTimeOfKeyValueReturn {
    KV_NOT_FOUND = 0,
    KV_EQUAL_TO_OLD = 1,
    KV_TIME_CHANGED = 2,
    KV_EXISTS_AT_NEW_TIME = 3,
    KV_MOVED_TO_DIFFERENT_INTERVAL = 4,
    KV_TIME_AFTER_DURATION = 5
};

//FIXME: move into class!
enum DeleteKeyValueReturn {
    KV_NOT_THERE = 0,
    KV_IS_THE_ONLY_ONE = 1,
    KV_DELETED = 2
};

template <class T>
class TemplatePropertyTimeline;

/**
 * This class organizes all animationsettings of one property of the type T.
 */
template <class T>
class VRN_CORE_API TemplatePropertyTimelineState : public Serializable {
public:
    /**
     * Destructor.
     */
    virtual ~TemplatePropertyTimelineState();

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

protected:
    friend class TemplatePropertyTimeline<T>;
    friend class XmlDeserializer;
    friend class TemplatePropertyTimelineStateFactory;

    /**
     * Constructor.
     * Parameter is the propertyobject which should be animated.
     */
    TemplatePropertyTimelineState(PropertyKeyValue<T>* kv);

    /**
     * Returns a map of all keyvalues of the timeline.
     */
    virtual const std::map<float,PropertyKeyValue<T>*>& getKeyValues() const;

    /**
     * Adds a new keyvalue at a certain time.
     * If there already exists a Keyvalue at the given time, this function does nothing.
     *
     * The interplationfunction at the time of the new keyvalue will be deletetd and replaced by two default
     * interpolations before and after the new keyvalue.
     * Returns the pointer to the new keyvalue.
     * If there already existed a keyvalue at the given time a nullpointer is returned.
     */
    virtual const PropertyKeyValue<T>* newKeyValue(float time);

    /**
     * Changes the value of a keyvalue:
     * Returns true if the value was changed.
     * Returns false if the given keyvalue couldn't be found in the timeline.
     */
    virtual bool changeValueOfKeyValue(T value, const PropertyKeyValue<T>* keyvalue);

    /**
     * Changes the smooth-variable of a keyvalue:
     * Returns true, if the value was changed.
     * Returns false, if the given keyvalue couldn't be found in the timeline.
     */
    virtual bool changeSmoothnessOfKeyValue(bool smooth, const PropertyKeyValue<T>* keyvalue);

    /**
     * Changes the time of a keyvalue:
     * If there is already a keyvalue at the given time, this function deletes the given keyvalue
     * and sets the value of the already existing keyvalue to the deleted value.
     * If the order of the keyvalues changes by setting the new time, the function reacts as if
     * the given keyvalue was deleted and a new one was added at the given time with the same value as the deleted one.
     *
     * returns KV_NOT_FOUND                   if the given keyvalue couldn't be found in the timeline
     * returns KV_EQUAL_TO_OLD                if the new time is equal to the old one
     * returns KV_TIME_CHANGED                if the new time is in the same intervall between the same other keyvalues
     * returns KV_EXISTS_AT_NEW_TIME          if there already exists a new keyvalue at the new time
     * returns KV_MOVED_TO_DIFFERENT_INTERVAL if the new time is in another interval between two other keyvalues.
     */
    virtual ChangeTimeOfKeyValueReturn changeTimeOfKeyValue(float time, const PropertyKeyValue<T>* keyvalue);

    /**
     * Deletes the given keyvalue:
     * The two interpolationfunctions before and after the deleted keyvalue will be deleted too and replaced by one default interpolation.
     * returns KV_NOT_FOUND       if the given keyvalue couldn't be found in the timeline
     * returns KV_IS_THE_ONLY_ONE if teh given keyvalue is the only value in the timeline and mustn't be deleted
     * returns KV_DELETED         if the keyvalue was deleted successfully
    */
    virtual DeleteKeyValueReturn deleteKeyValue(const PropertyKeyValue<T>* keyvalue);

    /**
     * Sets the interpolationfunction between the given keyvalue and the successor.
     */
    virtual void setInterpolationFunctionAfter(InterpolationFunction<T>* func, PropertyKeyValue<T>* keyvalue);

    /**
     * Sets the interpolationfunction btween the given keyvalue and its predecessor.
     */
    virtual void setInterpolationFunctionBefore(InterpolationFunction<T>* func, PropertyKeyValue<T>* keyvalue);

    /**
     * Deletes all keyvalues after time of duration.
     */
    virtual void setDuration(float duration);

    /**
     * Returns a clone of the timeline.
     */
    virtual TemplatePropertyTimelineState<T>* clone() const;

    /**
     * Calculates the animated value to a certain point of time.
     */
    const T getPropertyAt(float time);

     /**
      * Default constructor for clone-method.
      */
    TemplatePropertyTimelineState();

    /**
     * Map of all the keyvalues in this timelinestate.
     */
    std::map<float,PropertyKeyValue<T>*> values_;
};

/**
 * Special implementation of the timeline for transfunc-property,
 * because of the fact that the template-variable is a pointer.
 * For a description of the functions see further above in superclass.
 */
class TransFuncPropertyTimelineState : public TemplatePropertyTimelineState<TransFunc*> {
public:
    TransFuncPropertyTimelineState(PropertyKeyValue<TransFunc*>* kv);
    ~TransFuncPropertyTimelineState();
protected:
    friend class TemplatePropertyTimeline<TransFunc*>;
    friend class XmlDeserializer;
    friend class TemplatePropertyTimelineStateFactory;
    TransFuncPropertyTimelineState();
    const PropertyKeyValue<TransFunc*>* newKeyValue(float time);
    bool changeValueOfKeyValue(TransFunc* value, const PropertyKeyValue<TransFunc*>* keyvalue);
    DeleteKeyValueReturn deleteKeyValue(const PropertyKeyValue<TransFunc*>* keyvalue);
    const TransFunc* getPropertyAt(float time);
    TemplatePropertyTimelineState<TransFunc*>* clone();
    virtual void serialize(XmlSerializer& s) const;
    virtual void deserialize(XmlDeserializer& s);
};

/**
 * Special implementation of the timeline for tgt::Camera*-property
 * because of the fact that the template-variable is a pointer.
 * For a description of the functions see further above in superclass.
 */
class CameraPropertyTimelineState : public TemplatePropertyTimelineState<tgt::Camera> {
public:
    CameraPropertyTimelineState(PropertyKeyValue<tgt::Camera>* kv);
    CameraPropertyTimelineState(std::map<float,PropertyKeyValue<tgt::Camera>*> values);

    ~CameraPropertyTimelineState();
protected:
    friend class TemplatePropertyTimeline<tgt::Camera>;
    friend class XmlDeserializer;
    friend class TemplatePropertyTimelineStateFactory;

    CameraPropertyTimelineState();

    const PropertyKeyValue<tgt::Camera>* newKeyValue(float time);
    bool changeValueOfKeyValue(tgt::Camera value, const PropertyKeyValue<tgt::Camera>* keyvalue);
    DeleteKeyValueReturn deleteKeyValue(const PropertyKeyValue<tgt::Camera>* keyvalue);
    const tgt::Camera getPropertyAt(float time);
    TemplatePropertyTimelineState<tgt::Camera>* clone();

    virtual void serialize(XmlSerializer& s) const;
    virtual void deserialize(XmlDeserializer& s);
};

} // namespace voreen

#endif
