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

#ifndef VRN_CALLMEMBERACTION_H
#define VRN_CALLMEMBERACTION_H

#include "voreen/core/properties/targetaction.h"

namespace voreen {

#define ON_PROPERTY_CHANGE(PROPERTY,PROPERTYOWNER,FUNCTION) PROPERTY.onChange(CallMemberAction<PROPERTYOWNER>(this, &PROPERTYOWNER::FUNCTION));

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

// ============================================================================

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

// ============================================================================

}   // namespace

#endif
