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

#ifndef VRN_CONDITION_H
#define VRN_CONDITION_H

#include "voreen/core/vis/properties/action.h"

namespace voreen {
/**
 * A Condition can check any Condition and trigger Actions
 * if the condition is met. The foremost purpose is to monitor the state of
 * a Property and react to its changes (Look at TemplateProperty and TemplateProperty(\w*)Condition).
 */
class Condition {
public:
    Condition(const Action& action = NoAction(), const Action& elseaction = NoAction());
    Condition(const Condition& condition);

    virtual ~Condition();
    virtual Condition* clone() const = 0;

    /**
     * Add an arbitrary number of Actions to the Condition. These will be exec'd every time
     * the method check() is called and met() returns true. Equivalent to thenDo().
     */
    Condition& addAction(const Action& action);

    /**
     * Add an arbitrary number of ElseActions to the Condition. These will be exec'd every time
     * the method exec() is called and met() returns false. Equivalent to elseDo().
     */
    Condition& addElseAction(const Action& action);

    /**
     * This implements the actual checking of the Condition.
     */
    virtual bool met() const throw() = 0;

    /**
     * Overloading the bool() operator to call met().
     */
    operator bool() const { return met(); }

    /**
     * Checks if Condition is met and execs the right Actions.
     * If met() returns true Actions are exec'd, and if met() returns false ElseActions are exec'd.
     */
    void exec();

    /**
     * Exception thrown when validation of the Condition failed.
     */
    class ValidationFailed : public std::exception {
    public:
        ValidationFailed(const Condition* condition = 0);
        virtual ~ValidationFailed() throw () {}

        virtual const char* what() const throw();

    protected:
        std::string conditionMsg_;
    };

    /**
     * Checks if the Conditions is met and throws a Condition::ValidationFailed if not.
     * This is used for validation of TemplateProperties and should not be abused.
     */
    void validate() const throw (Condition::ValidationFailed);

    /**
     * Returns a string describing the condition. This is added to the exception message when
     * validation failed.
     */
    virtual std::string description() const { return ""; }

private:
    std::vector<Action*> actions_;
    std::vector<Action*> elseactions_;
};

// ----------------------------------------------------------------------------

/**
 * This Condition is always met() and thus always execs its Actions when check()ed.
 * Used to implement the onChange()-Mechanism in TemplateProperty
 */
class TrueCondition : public Condition {
public:
    TrueCondition(const Action& action = NoAction())
        : Condition(action) {}
    virtual ~TrueCondition() {}
    virtual bool met() const throw() { return true; }
    virtual TrueCondition* clone() const { return new TrueCondition(*this); }
};

// ----------------------------------------------------------------------------

template<typename T> class NumericProperty;

template<class T>
class NumericPropertyValidation : public Condition {
public:
    NumericPropertyValidation(NumericProperty<T>* const observed)
        : Condition(NoAction(), NoAction()), observed_(observed)
    {}

    virtual ~NumericPropertyValidation() {}
    virtual NumericPropertyValidation* clone() const { return new NumericPropertyValidation(*this); }
    virtual bool met() const throw ();

protected:
    NumericProperty<T>* observed_;
};

// ----------------------------------------------------------------------------

template<typename T> class OptionProperty;

template<class T>
class OptionPropertyValidation : public Condition {
public:
    OptionPropertyValidation(OptionProperty<T>* observed)
        : Condition(NoAction(), NoAction()), observed_(observed)
    {}

    virtual ~OptionPropertyValidation() {}
    virtual OptionPropertyValidation* clone() const { return new OptionPropertyValidation(*this); }

    virtual bool met() const throw();

protected:
    OptionProperty<T>* observed_;
};

// ----------------------------------------------------------------------------

template<typename T>
bool OptionPropertyValidation<T>::met() const throw() {
    std::set<std::string> allowedValues = observed_->allowedIds();
    return (allowedValues.find(observed_->get()) != allowedValues.end());
}

} // namespace voreen

#endif // VRN_CONDITION_H
