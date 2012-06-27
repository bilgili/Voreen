/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
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

#include <typeinfo>

#include "tgt/event/eventlistener.h"

namespace tgt {

	EventListener::EventListener(){
	    eventTypes_ = Event::ALLEVENTTYPES;
	}

void EventListener::onEvent(Event* e) {
    /*
        since we already checked which type we have we can safely use
        the faster static_cast (rl)
    */
    if (typeid(*e) == typeid(MouseEvent)) {
        MouseEvent* me = static_cast<MouseEvent*>(e);
        if (me->action() == MouseEvent::PRESSED)
            mousePressEvent(me);
        else if (me->action() == MouseEvent::RELEASED)
            mouseReleaseEvent(me);
        else if (me->action() == MouseEvent::MOTION)
            mouseMoveEvent(me);
        else if (me->action() == MouseEvent::DOUBLECLICK)
            mouseDoubleClickEvent(me);
        else if (me->action() == MouseEvent::WHEEL)
            wheelEvent(me);
    }
    else if (typeid(*e) == typeid(KeyEvent)) {
        keyEvent(static_cast<KeyEvent*>(e));
    }
    else if (typeid(*e) == typeid(TimeEvent)) {
        timerEvent(static_cast<TimeEvent*>(e));
    }
}

void EventListener::setEventTypes(int eventTypes){
    eventTypes_=eventTypes;
}

int EventListener::getEventTypes(){
	return eventTypes_;
}

} // namespace tgt
