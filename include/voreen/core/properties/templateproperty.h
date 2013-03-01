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

#ifndef VRN_TEMPLATEPROPERTY_H
#define VRN_TEMPLATEPROPERTY_H

#include "voreen/core/properties/templatepropertycondition.h"

namespace voreen {

/**
 * Template for a property that stores a single value.
 */
template<class T>
class TemplateProperty : public Property {
public:
    TemplateProperty(const std::string& id, const std::string& guiText,
                     T value, int = Processor::INVALID_RESULT);
    TemplateProperty();

    //TemplateProperty(const TemplateProperty*);

    virtual ~TemplateProperty();

    void set(const T& value);

    const T& get() const { return value_; }

    void setDefaultValue(const T& value);

    const T& getDefault() const { return defaultValue_; }

    virtual void reset();

    virtual void invalidate();

    // convenience methods for PropertyConditions - return Reference to the added Condition
    // these add a simple way to add Conditions to TemplateProperties
    void onValueEqual(const T& value, const Action& action = NoAction(),
                      const Action& elseaction = NoAction())
    {
        addCondition(EqualCondition<T>(this, value, action, elseaction));
    }

    //void onValueLess(const T& value, const Action& action = NoAction(),
    //                 const Action& elseaction = NoAction())
    //{
    //    addCondition(LessCondition<T>(this, value, action, elseaction));
    //}

    //void onValueLessEqual(const T& value, const Action& action = NoAction(),
    //                      const Action& elseaction = NoAction())
    //{
    //    addCondition(LessEqualCondition<T>(this, value, action, elseaction));
    //}

    //void onValueGreater(const T& value, const Action& action = NoAction(),
    //                    const Action& elseaction = NoAction())
    //{
    //    addCondition(GreaterCondition<T>(this, value, action, elseaction));
    //}

    //void onValueGreaterEqual(const T& value, const Action& action = NoAction(),
    //                         const Action& elseaction = NoAction())
    //{
    //    addCondition(GreaterEqualCondition<T>(this, value, action, elseaction));
    //}

    //void onValueIn(const std::set<T>& values, const Action& action = NoAction(),
    //               const Action& elseaction = NoAction())
    //{
    //    addCondition(InCondition<T>(this, values, action, elseaction));
    //}

    void onChange(const Action& action = NoAction()) {
        addCondition(TrueCondition(action));
    }

    void addCondition(const Condition& condition) {
        conditions_.push_back(condition.clone());
    }

    // convenience methods for validations - return Reference to the TemplateProperty they're added to
    // these add a simple way to add Validations to TemplateProperties
    TemplateProperty<T>& verifiesValueEqual(const T& value);
    //TemplateProperty<T>& verifiesValueLess(const T& value);
    //TemplateProperty<T>& verifiesValueLessEqual(const T& value);
    //TemplateProperty<T>& verifiesValueGreater(const T& value);
    //TemplateProperty<T>& verifiesValueGreaterEqual(const T& value);
    //TemplateProperty<T>& verifiesValueIn(const std::set<T>& values);

    Condition* addValidation(const Condition& condition) {
        validations_.push_back(condition.clone());
        return validations_.back();
    }

    TemplateProperty<T>& verifies(const Condition& condition) {
        addValidation(condition);
        return *this;
    }

    /**
     * Returns whether the given value would be valid for this property.
     * The property is not modified.
     *
     * @param value the value to check
     * @param errorMsg contains the error message, if the validation failed
     */
    bool isValidValue(const T& value, std::string& errorMsg);

protected:
    /**
     * Runs validate() on all Validations. If any of them is not met() a Condition::ValidationFailed
     * exception is thrown.
     */
    void validate(const T& value, std::string& errorMsg, bool restore = true);

    ///Executes the links currently registered at the property
    virtual void executeLinks();

    T value_;
    T defaultValue_;
    std::vector<Condition*> conditions_;
    std::vector<Condition*> validations_;

//private:
//    std::string getTypename() const;
};

//---------------------------------------------------------------------------

template<class T>
TemplateProperty<T>::TemplateProperty(const std::string& id, const std::string& guiText,
                                      T value, int invalidationLevel)
  : Property(id, guiText, invalidationLevel)
  , value_(value)
  , defaultValue_(value)
{}

template<class T>
TemplateProperty<T>::TemplateProperty() :
    Property()
{}

template<class T>
TemplateProperty<T>::~TemplateProperty() {
    size_t i;
    // delete conditions
    for (i = 0; i < conditions_.size() ;++i)
        delete conditions_.at(i);

    // delete validations
    for (i = 0; i < validations_.size(); ++i)
        delete validations_.at(i);

    conditions_.clear();
    validations_.clear();
}

template<class T>
void TemplateProperty<T>::set(const T& value) {
    if (value_ != value) {
        std::string errorMsg;
        validate(value, errorMsg, false);

        if (!errorMsg.empty())
            LWARNINGC("voreen.TemplateProperty", errorMsg);
        if (value_ != value)
            return;

        Property::invalidate();  // issues invalidateOwner and updateWidgets

        // execute links
        executeLinks();

        // check if conditions are met and exec actions
        for (size_t j = 0; j < conditions_.size(); ++j)
            conditions_[j]->exec();
    }
}

template<class T>
void TemplateProperty<T>::setDefaultValue(const T& value) {
    defaultValue_ = value;
}

template<class T>
void TemplateProperty<T>::reset() {
    set(defaultValue_);
}

template<class T>
void TemplateProperty<T>::validate(const T& value, std::string& errorMsg, bool restore) {
    // save value
    T temp(value_);
    value_ = value;
    bool valid = true;
    for (size_t j = 0; j < validations_.size() && valid; ++j)
        valid &= validations_[j]->validate(errorMsg);

    if (!valid || restore) // restore if new value valid or requested
        value_ = temp;
}

template<class T>
void voreen::TemplateProperty<T>::invalidate() {

    Property::invalidate();   // issues invalidateOwner and updateWidgets

    // check if conditions are met and exec actions
    for (size_t j = 0; j < conditions_.size(); ++j)
        conditions_[j]->exec();

    // execute links: we do not know the old value here, so just assign the current one twice
    executeLinks();
}

template<class T>
void voreen::TemplateProperty<T>::executeLinks() {
    if (links_.empty())
        return;

    // pass change data object to links
    for (std::vector<PropertyLink*>::iterator it = links_.begin(); it != links_.end(); it++) {
        try {
            (*it)->onChange();
        }
        catch (const VoreenException& e) {
            LERRORC("voreen.TemplateProperty", "executeLinks(): " << e.what());
        }
    }
}

template<class T>
bool TemplateProperty<T>::isValidValue(const T& value, std::string& errorMsg) {
    // save value
    T temp(value_);
    value_ = value;
    bool valid = true;
    for (size_t j = 0; j < validations_.size() && valid; ++j)
        valid &= validations_[j]->validate(errorMsg);
    value_ = temp;
    return valid;
}

//---------------------------------------------------------------------------

template<class T>
TemplateProperty<T>& TemplateProperty<T>::verifiesValueEqual(const T& value) {
    return verifies(EqualCondition<T>(this, value));
}

//template<class T>
//TemplateProperty<T>& TemplateProperty<T>::verifiesValueLess(const T& value) {
//    return verifies(LessCondition<T>(this, value));
//}
//
//template<class T>
//TemplateProperty<T>& TemplateProperty<T>::verifiesValueLessEqual(const T& value) {
//    return verifies(LessEqualCondition<T>(this, value));
//}
//
//template<class T>
//TemplateProperty<T>& TemplateProperty<T>::verifiesValueGreater(const T& value) {
//    return verifies(GreaterCondition<T>(this, value));
//}
//
//template<class T>
//TemplateProperty<T>& TemplateProperty<T>::verifiesValueGreaterEqual(const T& value) {
//    return verifies(GreaterEqualCondition<T>(this, value));
//}
//
//template<class T>
//TemplateProperty<T>& TemplateProperty<T>::verifiesValueIn(const std::set<T>& values) {
//    return verifies(InCondition<T>(this, values));
//}

} // namespace voreen

#endif // VRN_TEMPLATEPROPERTY_H
