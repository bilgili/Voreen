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

#include "voreen/core/animation/templatepropertytimelinestate.h"
#include "voreen/core/animation/animation.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"
#include "voreen/core/animation/interpolation/camerainterpolationfunctions.h"
#include "voreen/core/animation/interpolation/transfuncinterpolationfunctions.h"
#include "tgt/camera.h"

using tgt::Camera;

namespace voreen {


/////////////////// Special implementation for TransFunc*-Property

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

const PropertyKeyValue<TransFunc*>* TransFuncPropertyTimelineState::newKeyValue(float time) {
    time = round(time);
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
            InterpolationFunction<TransFunc*>* func = new TransFuncInterpolationFunction();
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
            InterpolationFunction<TransFunc*>* func = new TransFuncInterpolationFunction();
            it->second->setForegoingInterpolationFunction(func);
            it--;
            it->second->setFollowingInterpolationFunction(func);
            it++;
        }
        else {
            InterpolationFunction<TransFunc*>* func1 = new TransFuncInterpolationFunction();
            InterpolationFunction<TransFunc*>* func2 = new TransFuncInterpolationFunction();
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
    InterpolationFunction<TransFunc*>* func = new TransFuncInterpolationFunction();
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

TemplatePropertyTimelineState<TransFunc*>* TransFuncPropertyTimelineState::clone() const {
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
        func2 = func->create();
        it2->second->setFollowingInterpolationFunction(func2);
        it2++;
        it++;
        it2->second->setForegoingInterpolationFunction(func2);
    }

    return timeline;
}

} // namespace voreen
