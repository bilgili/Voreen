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

#ifndef VRN_ACTION_H
#define VRN_ACTION_H

#include "voreen/core/vis/properties/property.h"

namespace voreen {

class Processor;

/**
 * An Action can be executed by a Property when a Condition is met.
 */
class Action {
public:
    virtual ~Action() {}
    virtual Action* clone() const = 0;
    virtual void exec() = 0;
    virtual bool isNoAction() const { return false; }
};

// ----------------------------------------------------------------------------

/**
 * This Action does nothing.
 */
class NoAction : public Action {
public:
    virtual ~NoAction() {}
    void exec() {} // do nothing
    virtual bool isNoAction() const { return true; }
    virtual NoAction* clone() const { return new NoAction(); }
};

// ----------------------------------------------------------------------------

/**
 * Inherit from a TagetAction<YourTarget> and you will get an action
 * that acts on a target.
 */
template<class T>
class TargetAction : public Action {
public:
    TargetAction() : target_(0) {}
    TargetAction(T* target) : target_(target) {}
    virtual ~TargetAction() {}

    /**
     * Returns a pointer to the target of the action.
     */
    T* target() const { return target_; }

protected:
    T* target_;
};

// ----------------------------------------------------------------------------

/**
 * This Action can call a member function of an Object with signature void mem()
 */
template<class T>
class CallMemberAction : public TargetAction<T> {
protected:
    using TargetAction<T>::target_;

public:

    CallMemberAction(T* target, void (T::*fpt)())
        : TargetAction<T>(target), fpt_(fpt)
    {
    }

    virtual ~CallMemberAction() {}
    virtual CallMemberAction* clone() const { return new CallMemberAction(*this); }

    virtual void exec() {
        if ((target_ != 0) && (fpt_ != 0))
            (target_->*fpt_)();
    }

private:
    void (T::*fpt_)();
};

// ----------------------------------------------------------------------------

/**
 * This Action can call a member function of an Object with signature void mem(P param)
 */
template<class T, class P>
class Call1ParMemberAction : public TargetAction<T> {
protected:
    using TargetAction<T>::target_;

public:
    Call1ParMemberAction(T* target, void (T::*fpt)(P), P param)
        : TargetAction<T>(target), fpt_(fpt), param_(param)
    {
    }

    virtual ~Call1ParMemberAction() {}
    virtual Call1ParMemberAction* clone() const { return new Call1ParMemberAction(*this); }

    virtual void exec() {
        if ((target_ != 0) && (fpt_ != 0))
            (target_->*fpt_)(param_);
    }

private:
    void (T::*fpt_)(P);
    P param_;
};

// ----------------------------------------------------------------------------

class InvalidateOwnerAction : public TargetAction<Property> {
protected:
    using TargetAction<Property>::target_;

public:
    InvalidateOwnerAction(Property* target) : TargetAction<Property>(target) {}
    virtual  ~InvalidateOwnerAction() {}
    virtual InvalidateOwnerAction* clone() const { return new InvalidateOwnerAction(*this); }
    virtual void exec();
};

// ----------------------------------------------------------------------------

class InvalidateOwnerShaderAction : public TargetAction<Property> {
protected:
    using TargetAction<Property>::target_;

public:
    InvalidateOwnerShaderAction(Property* target) : TargetAction<Property>(target) {}
    virtual  ~InvalidateOwnerShaderAction() {}
    virtual InvalidateOwnerShaderAction* clone() const { return new InvalidateOwnerShaderAction(*this); }

    virtual void exec();
};

} // namespace voreen

#endif // VRN_ACTION_H
