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

#include "voreen/core/properties/link/dependencylinkevaluator.h"

namespace voreen {

DependencyLinkEvaluator::DependencyLinkEvaluator()
    : LinkEvaluatorBase()
    , keys_(std::vector<BoxObject>())
    , values_(std::vector<BoxObject>())
    , historyLength_(-1)
{}

BoxObject DependencyLinkEvaluator::eval(const BoxObject& sourceOld, const BoxObject& sourceNew, const BoxObject& targetOld, Property* /*src*/, Property* /*dest*/) {
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");

    setKeyValue(sourceOld, targetOld);

    // look for the key in the array of saved keys
    for (size_t i = 0; i < keys_.size(); ++i) {
        const BoxObject& key = keys_[i];

        if (key == sourceNew)
            return values_[i];
    }

    return targetOld.deepCopy();
}

std::string DependencyLinkEvaluator::name() const {
    return "DependencyLink";
}

void DependencyLinkEvaluator::setHistoryLength(int newLength) {
    historyLength_ = newLength;
}

int DependencyLinkEvaluator::getHistoryLength() const {
    return historyLength_;
}

void DependencyLinkEvaluator::setKeyValue(BoxObject key, BoxObject value) {
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");

    if (historyLength_ >= 0) {
        // limit the size of the arrays to historyLength_
        while (static_cast<int>(keys_.size()) > historyLength_)
            removeFrontItemFromMap();
    }

    // look for an old value of the key
    for (size_t i = 0; i < keys_.size(); ++i) {
        if (key == keys_[i]) {
            BoxObject copy = BoxObject(value);
            values_[i] = copy.deepCopy();
            addObserver(values_[i]);
            return;
        }
    }

    // if no existing key was found, insert a new pair
    BoxObject keyCopy = key.deepCopy();
    addObserver(keyCopy);
    BoxObject valueCopy = value.deepCopy();
    addObserver(valueCopy);
    keys_.push_back(keyCopy);
    values_.push_back(valueCopy);
}

void DependencyLinkEvaluator::removeFrontItemFromMap() {
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");

    for (size_t i = 0; i < keys_.size() - 1; ++i) {
        keys_[i] = keys_[i+1];
        values_[i] = values_[i+1];
    }

    keys_.pop_back();
    values_.pop_back();
}

void DependencyLinkEvaluator::clearDependencyMap() {
    keys_.clear();
    values_.clear();
}

void DependencyLinkEvaluator::addObserver(const BoxObject& box) {
    if (box.getType() == BoxObject::VOLUMEHANDLE) {
        const VolumeHandle* handle = box.getVolumeHandle();
        if (handle)
            handle->addObserver(this);
    }
}

void DependencyLinkEvaluator::volumeHandleDelete(const VolumeHandle* source) {
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");
    for (size_t i = 0; i < keys_.size(); ++i) {
        const VolumeHandle* keyHandle = keys_[i].getVolumeHandle();

        if (keyHandle == source) {
            keys_.erase(keys_.begin() + i);
            values_.erase(values_.begin() + i);
            break;
        }
    }
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");
}

void DependencyLinkEvaluator::volumeChange(const VolumeHandle*) {}

void DependencyLinkEvaluator::serialize(XmlSerializer& s) const {
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");
    s.serialize("keys", keys_);
    s.serialize("values", values_);
}

void DependencyLinkEvaluator::deserialize(XmlDeserializer& d) {
    d.deserialize("keys", keys_);
    d.deserialize("values", values_);
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");
}


} // namespace
