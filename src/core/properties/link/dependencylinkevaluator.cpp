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

#include "voreen/core/properties/link/dependencylinkevaluator.h"
#include "voreen/core/datastructures/transfunc/transfuncfactory.h"
#include "voreen/core/properties/property.h"

namespace voreen {

DependencyLinkEvaluator::DependencyLinkEvaluator()
    : LinkEvaluatorBase()
    , historyLength_(-1)
{}

void DependencyLinkEvaluator::eval(Property* src, Property* dst) throw (VoreenException) {
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

std::string DependencyLinkEvaluator::serializeProperty(Property* p) const {
    XmlSerializer s;

    p->serializeValue(s);

    std::stringstream stream;
    s.write(stream);
    return stream.str();
}

void DependencyLinkEvaluator::deserializeProperty(Property* p, std::string s) const {
    XmlDeserializer d;

    std::stringstream stream(s);
    d.read(stream);

    p->deserializeValue(d);
}

std::string DependencyLinkEvaluator::getGuiName() const {
    return "DependencyLink";
}

void DependencyLinkEvaluator::setHistoryLength(int newLength) {
    historyLength_ = newLength;
}

int DependencyLinkEvaluator::getHistoryLength() const {
    return historyLength_;
}

void DependencyLinkEvaluator::setKeyValue(std::string key, std::string value) {
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

void DependencyLinkEvaluator::serialize(XmlSerializer& s) const {
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");
    s.serialize("keys", keys_);
    s.serialize("values", values_);
    s.serialize("historylength", historyLength_);
}

void DependencyLinkEvaluator::deserialize(XmlDeserializer& d) {
    d.deserialize("keys", keys_);
    d.deserialize("values", values_);
    d.deserialize("historylength", historyLength_);
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");
}

void DependencyLinkEvaluator::propertiesChanged(Property* src, Property* dst) {
    LinkEvaluatorBase::propertiesChanged(src, dst);

    sourceOld_ = serializeProperty(src);
}

std::string DependencyLinkEvaluator::getClassName() const {
    return "DependencyLinkEvaluator";
}

LinkEvaluatorBase* DependencyLinkEvaluator::create() const {
    return new DependencyLinkEvaluator;
}

bool DependencyLinkEvaluator::arePropertiesLinkable( const Property* /*p1*/, const Property* /*p2*/ ) const {
    return true;
}

} // namespace
