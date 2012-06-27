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

#ifndef VRN_EVENTPROPERTY_H
#define VRN_EVENTPROPERTY_H

#include "voreen/core/vis/properties/property.h"
#include "tgt/event/event.h"
#include "tgt/event/keyevent.h"
#include "tgt/event/mouseevent.h"
#include "voreen/core/vis/properties/action.h"
#include "voreen/core/vis/properties/targetaction.h"

namespace voreen {

template<class T, class P>
class EventAction : public TargetAction<T> {
protected:
    using TargetAction<T>::target_;

public:
    EventAction(T* target, void (T::*fpt)(P*))
        : TargetAction<T>(target), fpt_(fpt), param_(0)
    {    }

    virtual ~EventAction() {}
    virtual EventAction* clone() const { return new EventAction(*this); }

    virtual void setParam(P* value) {
        param_ = value;
    }
    virtual void exec() {
        if ((target_ != 0) && (fpt_ != 0))
            (target_->*fpt_)(param_);
    }

private:
    void (T::*fpt_)(P*);
    P* param_;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------


class EventProperty : public Property {
public:
    EventProperty(const std::string& guiText, tgt::Event::Modifier modifier);

    tgt::Event::Modifier getModifier();
    void setModifier(tgt::Event::Modifier value);

    virtual std::string toString() const;

protected:
    tgt::Event::Modifier modifier_;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

class MouseEventProperty : public EventProperty {
public:
    MouseEventProperty(const std::string& guiText, int mouseAction, tgt::Event::Modifier modifier, tgt::MouseEvent::MouseButtons mouseButtons)
        : EventProperty(guiText, modifier)
        , mouseAction_(tgt::MouseEvent::MouseAction(mouseAction))
        , mouseButtons_(mouseButtons)
    {}

    tgt::MouseEvent::MouseButtons getMouseButtons() {
        return mouseButtons_;
    }

    void setMouseButtons(tgt::MouseEvent::MouseButtons value) {
        mouseButtons_ = value;
    }
    bool accepts(tgt::Event* e);

    tgt::MouseEvent::MouseAction getMouseAction() {
        return mouseAction_;
    }

    void setMouseAction(tgt::MouseEvent::MouseAction value) {
        mouseAction_ = value;
    }

    virtual void execute(tgt::MouseEvent* e) = 0;

protected:
    tgt::MouseEvent::MouseAction mouseAction_;
    tgt::MouseEvent::MouseButtons mouseButtons_;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<class T>
class TemplateMouseEventProperty : public MouseEventProperty {
public:
    TemplateMouseEventProperty(const std::string& guiText, EventAction<T, tgt::MouseEvent>* action , int mouseAction, tgt::Event::Modifier modifier, tgt::MouseEvent::MouseButtons mouseButtons)
        : MouseEventProperty(guiText, mouseAction, modifier, mouseButtons)
        , action_(action)
    {}

    virtual void execute(tgt::MouseEvent* e) {
        if ( (e->modifiers() == getModifier()) && (e->action() & getMouseAction()) && (e->button() & getMouseButtons()) ) {
            action_->setParam(e);
            action_->exec();
            //e->accept();
        }
    }

protected:
    EventAction<T, tgt::MouseEvent>* action_;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

class KeyboardEventProperty : public EventProperty {
public:
    KeyboardEventProperty(const std::string& guiText, tgt::Event::Modifier modifier, tgt::KeyEvent::KeyCode keyCode)
        : EventProperty(guiText, modifier)
        , keyCode_(keyCode)
    {}

    tgt::KeyEvent::KeyCode getKeyCode() {
        return keyCode_;
    }
    void setKeyCode(tgt::KeyEvent::KeyCode value) {
        keyCode_ = value;
    }

    virtual void execute(tgt::KeyEvent* e) = 0;

protected:
    tgt::KeyEvent::KeyCode keyCode_;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

template <class T>
class TemplateKeyboardEventProperty : public KeyboardEventProperty {
public:
    TemplateKeyboardEventProperty(const std::string& guiText, EventAction<T, tgt::KeyEvent>* action, tgt::Event::Modifier modifier, tgt::KeyEvent::KeyCode keyCode)
        : EventProperty(guiText, modifier, keyCode)
        , action_(action)
    {}

    virtual void execute(tgt::KeyEvent* e) {
        if (e->keyCode() == getKeyCode()) {
            action_->setParam(e);
            action_->exec();
            //e->accept();
        }
    }

protected:
    EventAction<T, tgt::KeyEvent>* action_;
};

} // namespace

#endif // VRN_EVENTPROPERTY_H
