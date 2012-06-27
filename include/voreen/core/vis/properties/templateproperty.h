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

#ifndef VRN_TEMPLATEPROPERTY_H
#define VRN_TEMPLATEPROPERTY_H

#include "voreen/core/vis/properties/templatepropertycondition.h"

namespace voreen {

/**
 * Stores a parameter value and additional information regarding the parameter.
 *
 * If shader recompiling is needed when the parameter changed, pass a pointer
 * to the appropriate shader variable.
 * If the parameter should not be changeable, the isChangeable to false.
 */
template<class T>
class TemplateProperty : public Property {
public:
    TemplateProperty(const std::string& id, const std::string& guiText,
                     const T& value, bool invalidate = true, bool invalidateShader = false);
    virtual ~TemplateProperty();

    /**
     * Indicates whether this Property invalidate its owner.
     * TODO: should be renamed, as this has nothing to do with validate()
     */
    bool invalidates() const { return invalidates_; }

    /**
     * Indicates whether this Property invalidate the shader of its owner.
     */
    bool invalidatesShader() const { return invalidatesShader_; }

    void set(const T& value, bool updateWidgets = true);

    /**
     * Sets the visibility of this property in the gui.
     */
    virtual void setVisible(bool state);
    const T& get() const { return value_; }

    // convenience methods for PropertyConditions - return Reference to the added Condition
    // these add a simple way to add Conditions to TemplateProperties
    void onValueEqual(const T& value, const Action& action = NoAction(),
                      const Action& elseaction = NoAction())
    {
        addCondition(EqualCondition<T>(this, value, action, elseaction));
    }

    void onValueLess(const T& value, const Action& action = NoAction(),
                     const Action& elseaction = NoAction())
    {
        addCondition(LessCondition<T>(this, value, action, elseaction));
    }

    void onValueLessEqual(const T& value, const Action& action = NoAction(),
                          const Action& elseaction = NoAction())
    {
        addCondition(LessEqualCondition<T>(this, value, action, elseaction));
    }

    void onValueGreater(const T& value, const Action& action = NoAction(),
                        const Action& elseaction = NoAction())
    {
        addCondition(GreaterCondition<T>(this, value, action, elseaction));
    }

    void onValueGreaterEqual(const T& value, const Action& action = NoAction(),
                             const Action& elseaction = NoAction())
    {
        addCondition(GreaterEqualCondition<T>(this, value, action, elseaction));
    }

    void onValueIn(const std::set<T>& values, const Action& action = NoAction(),
                   const Action& elseaction = NoAction())
    {
        addCondition(InCondition<T>(this, values, action, elseaction));
    }

    void onChange(const Action& action = NoAction()) {
        addCondition(TrueCondition(action));
    }

    void addCondition(const Condition& condition) {
        conditions_.push_back(condition.clone());
    }

    // convenience methods for validations - return Reference to the TemplateProperty they're added to
    // these add a simple way to add Validations to TemplateProperties
    TemplateProperty<T>& verifiesValueEqual(const T& value);
    TemplateProperty<T>& verifiesValueLess(const T& value);
    TemplateProperty<T>& verifiesValueLessEqual(const T& value);
    TemplateProperty<T>& verifiesValueGreater(const T& value);
    TemplateProperty<T>& verifiesValueGreaterEqual(const T& value);
    TemplateProperty<T>& verifiesValueIn(const std::set<T>& values);

    Condition* addValidation(const Condition& condition) {
        validations_.push_back(condition.clone());
        return validations_.back();
    }

    TemplateProperty<T>& verifies(const Condition& condition) {
        addValidation(condition);
        return *this;
    }

    /**
     * Checks if a value is valid for a Property, which means all Validations are met().
     * Does _not_ set the Property.
     */
    //bool valid(const T& value);

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f) = 0;

    virtual std::string toString() const = 0;

protected:
    /**
     * Runs validate() on all Validations. If any of them is not met() a Condition::ValidationFailed
     * exception is thrown.
     */
    void validate(const T& value, bool restore = true);

    T value_;
    const bool invalidates_;
    const bool invalidatesShader_;
    std::vector<Condition*> conditions_;
    std::vector<Condition*> validations_;

private:
    std::string getTypename() const;
};

//---------------------------------------------------------------------------

template<class T>
TemplateProperty<T>::TemplateProperty(const std::string& id, const std::string& guiText,
                                      const T& value, bool invalidate, bool invalidateShader)
  : Property(id, guiText),
    value_(value),
    invalidates_(invalidate),
    invalidatesShader_(invalidateShader)
{
    if (invalidates_ == true) {
        onChange(InvalidateOwnerAction(this));
    }

    if (invalidatesShader_ == true) {
        onChange(InvalidateOwnerShaderAction(this));
    }
}

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
void TemplateProperty<T>::setVisible(bool state) {
    visible_ = state;
    // display widgets
    setWidgetsVisible(state);
}

template<class T>
void TemplateProperty<T>::set(const T& value, bool updateWidgets) {
    if (value_ != value) {
        validate(value, false);

        // notify widgets of updated values
        if (updateWidgets)
            this->updateWidgets();

        // check if conditions are met and exec actions
        for(size_t j = 0; j < conditions_.size(); ++j)
            conditions_[j]->exec();
    }
}

template<class T>
void TemplateProperty<T>::validate(const T& value, bool restore) {
    // save value
    T temp(value_);
    value_ = value;
    try {
        // validate all validations
        for (size_t j = 0; j < validations_.size(); ++j)
            validations_[j]->validate();
    } catch (Condition::ValidationFailed&) {
        value_ = temp; // restore if invalid
        throw;
    }
    if (restore) // everything validated ok, restore value if requested
        value_ = temp;
}

//---------------------------------------------------------------------------

template<class T>
TemplateProperty<T>& TemplateProperty<T>::verifiesValueEqual(const T& value) {
    return verifies(EqualCondition<T>(this, value));
}

template<class T>
TemplateProperty<T>& TemplateProperty<T>::verifiesValueLess(const T& value) {
    return verifies(LessCondition<T>(this, value));
}

template<class T>
TemplateProperty<T>& TemplateProperty<T>::verifiesValueLessEqual(const T& value) {
    return verifies(LessEqualCondition<T>(this, value));
}

template<class T>
TemplateProperty<T>& TemplateProperty<T>::verifiesValueGreater(const T& value) {
    return verifies(GreaterCondition<T>(this, value));
}

template<class T>
TemplateProperty<T>& TemplateProperty<T>::verifiesValueGreaterEqual(const T& value) {
    return verifies(GreaterEqualCondition<T>(this, value));
}

template<class T>
TemplateProperty<T>& TemplateProperty<T>::verifiesValueIn(const std::set<T>& values) {
    return verifies(InCondition<T>(this, values));
}

} // namespace voreen

#endif // VRN_TEMPLATEPROPERTY_H
