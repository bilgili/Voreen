/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "tgt/event/mouseevent.h"

namespace tgt {

MouseEvent::MouseEvent(int x, int y, MouseAction action, Event::Modifier mod, MouseButtons button, ivec2 viewport)
    : Event()
    , coord_(x,y)
    , viewport_(viewport)
    , buttonCode_(button)
    , action_(action)
    , mod_(mod)
{}

int MouseEvent::getEventType() {
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
        case ENTER:
            return MOUSEENTEREVENT;
            break;
        case EXIT:
            return MOUSEEXITEVENT;
            break;
        default:
            return 0;
    }
}

ivec2 MouseEvent::coord() const {
    return coord_;
}

int MouseEvent::x() const {
    return coord_.x;
}

int MouseEvent::y() const {
    return coord_.y;
}

ivec2 MouseEvent::viewport() const {
    return viewport_;
}

MouseEvent::MouseButtons MouseEvent::button() const {
    return buttonCode_;
}

MouseEvent::MouseAction MouseEvent::action() const {
    return action_;
}

Event::Modifier MouseEvent::modifiers() const {
    return mod_;
}

}
