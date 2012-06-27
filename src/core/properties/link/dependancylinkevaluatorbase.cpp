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

#include "voreen/core/properties/link/dependancylinkevaluatorbase.h"
#include "voreen/core/datastructures/transfunc/transfuncfactory.h"
#include "voreen/core/properties/property.h"

namespace voreen {

DependancyLinkEvaluatorBase::DependancyLinkEvaluatorBase()
    : LinkEvaluatorBase()
    , historyLength_(-1)
{}

void DependancyLinkEvaluatorBase::eval(Property* src, Property* dst) throw (VoreenException) {
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");

    std::string sourceNew = serializeProperty(src);
    std::string targetOld = serializeProperty(dst);

    setKeyValue(sourceOld_, targetOld);

    // look for the key in the array of saved keys
    for (size_t i = 0; i < keys_.size(); ++i) {
        std::string key = keys_[i];

        if (key == sourceNew) {
            deserializeProperty(dst, values_[i]);
        }
    }
    sourceOld_ = sourceNew;
}

std::string DependancyLinkEvaluatorBase::serializeProperty(Property* p) const {
    XmlSerializer s;
    s.registerFactory(TransFuncFactory::getInstance());

    p->serializeValue(s);

    std::stringstream stream;
    s.write(stream);
    return stream.str();
}

void DependancyLinkEvaluatorBase::deserializeProperty(Property* p, std::string s) const {
    XmlDeserializer d;
    d.registerFactory(TransFuncFactory::getInstance());

    std::stringstream stream(s);
    d.read(stream);

    p->deserializeValue(d);
}

std::string DependancyLinkEvaluatorBase::name() const {
    return "DependancyLink";
}

void DependancyLinkEvaluatorBase::setHistoryLength(int newLength) {
    historyLength_ = newLength;
}

int DependancyLinkEvaluatorBase::getHistoryLength() const {
    return historyLength_;
}

void DependancyLinkEvaluatorBase::setKeyValue(std::string key, std::string value) {
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");

    if (historyLength_ >= 0) {
        // limit the size of the arrays to historyLength_
        while (static_cast<int>(keys_.size()) > historyLength_)
            removeFrontItemFromMap();
    }

    // look for an old value of the key
    for (size_t i = 0; i < keys_.size(); ++i) {
        if (key == keys_[i]) {
            values_[i] = value;
            //addObserver(values_[i]); // TODO
            return;
        }
    }

    keys_.push_back(key);
    values_.push_back(value);
}

void DependancyLinkEvaluatorBase::removeFrontItemFromMap() {
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");

    for (size_t i = 0; i < keys_.size() - 1; ++i) {
        keys_[i] = keys_[i+1];
        values_[i] = values_[i+1];
    }

    keys_.pop_back();
    values_.pop_back();
}

void DependancyLinkEvaluatorBase::clearDependancyMap() {
    keys_.clear();
    values_.clear();
}

void DependancyLinkEvaluatorBase::serialize(XmlSerializer& s) const {
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");
    s.serialize("keys", keys_);
    s.serialize("values", values_);
}

void DependancyLinkEvaluatorBase::deserialize(XmlDeserializer& d) {
    d.deserialize("keys", keys_);
    d.deserialize("values", values_);
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");
}

void DependancyLinkEvaluatorBase::propertiesChanged(Property* src, Property* dst) {
    LinkEvaluatorBase::propertiesChanged(src, dst);

    sourceOld_ = serializeProperty(src);
}

} // namespace
