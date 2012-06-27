/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/properties/action.h"
#include "voreen/core/vis/properties/condition.h"
#include "voreen/core/vis/properties/numericproperty.h"

#include "tgt/logmanager.h"

namespace voreen {

Condition::Condition(const Action& action, const Action& elseaction) {
    addAction(action);
    addElseAction(elseaction);
}

Condition::Condition(const Condition& condition) {
    for(size_t i = 0; i < condition.actions_.size(); ++i)
        actions_.push_back(condition.actions_[i]->clone());

    for(size_t i = 0; i < condition.elseactions_.size(); ++i)
        elseactions_.push_back(condition.elseactions_[i]->clone());
}

Condition::~Condition() {
    for(size_t i = 0; i < actions_.size(); ++i)
        delete actions_[i];

    for(size_t i = 0; i < elseactions_.size(); ++i)
        delete elseactions_[i];

    actions_.clear();
    elseactions_.clear();
}

Condition& Condition::addAction(const Action& action) {
    if (!action.isNoAction())
        actions_.push_back(action.clone());

    return *this;
}

Condition& Condition::addElseAction(const Action& action) {
    if (!action.isNoAction())
        elseactions_.push_back(action.clone());

    return *this;
}

void Condition::exec() {
    if (met()) {
        for (size_t i = 0; i < actions_.size(); ++i)
            actions_[i]->exec();
    } else {
        for (size_t i = 0; i < elseactions_.size(); ++i)
            elseactions_[i]->exec();
    }
}

bool Condition::validate() const {

    bool valid = met();
    if (!valid) {
        std::string msg = "Property validation failed";
        if (!description().empty())
            msg += ": " + description();
        LWARNINGC("voreen.core.Condition", msg);
    }
    return met();
}

// ----------------------------------------------------------------------------

Condition::ValidationFailed::ValidationFailed(const Condition* condition)
    : std::exception()
{
    conditionMsg_ = "ValidationFailed";
    if ((condition != 0) && (condition->description().empty() == false))
        conditionMsg_ += ": " + condition->description();
}

const char* Condition::ValidationFailed::what() const throw() {
    return conditionMsg_.c_str();
}

// ============================================================================

template<typename T>
bool NumericPropertyValidation<T>::met() const throw() {
    const T& min = observed_->minValue_;
    const T& max = observed_->maxValue_;

    // do min and max make sense at all?
    // If not just pretend the validation holds
    for (size_t i = 0; i < min.size; ++i) {
        if (min.elem[i] > max.elem[i])
            return true;
    }

    // If we have come this far we can do real validations:
    // test if any component is < min or > max
    const T& val = observed_->value_;
    for (size_t i = 0; i < val.size; ++i) {
        if ((val.elem[i] < min.elem[i]) || (val.elem[i] > max.elem[i]))
            return false;
    }
    return true;
}

template<>
bool NumericPropertyValidation<float>::met() const throw() {
    // if min and max make sense do a real validation
    if (observed_->minValue_ <= observed_->maxValue_) {
        return ((observed_->value_ >= observed_->minValue_)
                && (observed_->value_ <= observed_->maxValue_));
    }
    else
        return true;
}

template<>
bool NumericPropertyValidation<int>::met() const throw() {
    // if min and max make sense do a real validation
    if (observed_->minValue_ <= observed_->maxValue_) {
        return ((observed_->value_ >= observed_->minValue_)
                && (observed_->value_ <= observed_->maxValue_));
    }
    else
        return true;
}

template<class T>
std::string voreen::NumericPropertyValidation<T>::description() const{
    std::stringstream stream;
    stream << observed_->value_ << " out of valid range ["
           << observed_->minValue_ << "," << observed_->maxValue_ << "]"
           << "  (" << observed_->getOwner()->getName() << "." << observed_->getId() << ")";
    return stream.str();
}


// explicit template instantiation to enable distribution of
// implementation of template class methods over .h and .cpp files
//
template class NumericPropertyValidation<float>;
template class NumericPropertyValidation<int>;
template class NumericPropertyValidation<tgt::vec2>;
template class NumericPropertyValidation<tgt::vec3>;
template class NumericPropertyValidation<tgt::vec4>;
template class NumericPropertyValidation<tgt::ivec2>;
template class NumericPropertyValidation<tgt::ivec3>;
template class NumericPropertyValidation<tgt::ivec4>;

}   // namespace
