/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2005-2013 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "tgt/event/touchevent.h"

namespace tgt {


TouchEvent::TouchEvent(Event::Modifier mod, t_state touchPointStates, DeviceType deviceType, const std::deque<TouchPoint> &touchPoints)
    : Event()
    , mod_(mod)
    , deviceType_(deviceType)
    , touchPointStates_(touchPointStates)
    , touchPoints_(touchPoints)
{
}

TouchEvent::~TouchEvent() {

}

TouchEvent::DeviceType TouchEvent::deviceType() const
{
    return deviceType_;
}

TouchEvent::t_state TouchEvent::touchPointStates() const
{
    return TouchEvent::TouchPointMoved; //< TODO
}

const std::deque<TouchPoint> &TouchEvent::touchPoints() const
{
    return touchPoints_;
}

int TouchEvent::getEventType() {
    switch (action_) {
        case MOTION:
            // motion = move?
            return MOUSEMOVEEVENT;
            break;
        case PRESSED:
            return MOUSEPRESSEVENT;
            break;
        case RELEASED:
            return MOUSERELEASEEVENT;
            break;
        default:
            return 0;
    }
}

TouchEvent::MouseAction TouchEvent::action() const {
    return action_;
}

Event::Modifier TouchEvent::modifiers() const {
    return mod_;
}

/*tgt::QtCanvas * TouchEvent::widget () const {

}*/

}
