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

#ifndef TGT_MOUSEEVENT_H
#define TGT_MOUSEEVENT_H

#include "tgt/config.h"
#include "tgt/event/event.h"
#include "tgt/vector.h"

namespace tgt { 

class MouseEvent : public Event {
public:
    enum MouseButtons {
        NO_MOUSE_BUTTON =       0,
        MOUSE_BUTTON_1 =   1 << 0, MOUSE_BUTTON_LEFT =   MOUSE_BUTTON_1,
        MOUSE_BUTTON_2 =   1 << 1, MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_2,
        MOUSE_BUTTON_3 =   1 << 2, MOUSE_BUTTON_RIGHT =  MOUSE_BUTTON_3,
        MOUSE_BUTTON_4 =   1 << 3,
        MOUSE_BUTTON_5 =   1 << 4,
        MOUSE_WHEEL_DOWN = 1 << 5,
        MOUSE_WHEEL_UP =   1 << 6,
        MOUSE_ALL = MOUSE_BUTTON_1 | MOUSE_BUTTON_2 | MOUSE_BUTTON_3 | MOUSE_BUTTON_4 | MOUSE_BUTTON_5 | MOUSE_WHEEL_DOWN | MOUSE_WHEEL_UP
    };

    enum MouseAction {
        MOTION,
        PRESSED,
        RELEASED,
        DOUBLECLICK,
        WHEEL
    };

public:
    MouseEvent(int x, int y, MouseAction action, Event::Modifier mod, MouseButtons button = NO_MOUSE_BUTTON);

    ivec2 coord();
    int x();
    int y();
    MouseButtons button();
    MouseAction action();
    Event::Modifier modifiers();

    // HACK for voreen paper... it shouldn't be possible to set Coordinates after creation... or maybe it should?
    void setCoord(const ivec2& coord) { coord_ = coord; }
	virtual int getEventType();
  
protected:
    ivec2 coord_;
    MouseButtons buttonCode_;
    MouseAction action_;
    Event::Modifier mod_;
};

}

#endif //TGT_MOUSEEVENT_H
