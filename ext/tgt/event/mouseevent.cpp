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

#include "tgt/event/mouseevent.h"

namespace tgt {

MouseEvent::MouseEvent(int x, int y, MouseAction action, Event::Modifier mod, MouseButtons button)
    : Event()
    , coord_(x,y)
    , buttonCode_(button)
    , action_(action)
    , mod_(mod)
{}

int MouseEvent::getEventType(){
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
		case DOUBLECLICK:
		    return MOUSEDOUBLECLICKEVENT;
			break;
		case WHEEL:
			return WHEELEVENT;
			break;
		default:
			return 0;
	}
}

ivec2 MouseEvent::coord() {
    return coord_;
}

int MouseEvent::x() {
    return coord_.x;
}

int MouseEvent::y() {
    return coord_.y;
}

MouseEvent::MouseButtons MouseEvent::button() {
    return buttonCode_;
}

MouseEvent::MouseAction MouseEvent::action() {
    return action_;
}

Event::Modifier MouseEvent::modifiers() {
    return mod_;
}

}
