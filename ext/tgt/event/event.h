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

#ifndef TGT_EVENT_H
#define TGT_EVENT_H

#include "tgt/config.h"

namespace tgt {
	
/**
 *  The base class for all events. Provides the accepted/ignore mechanism.
 */
class Event {
public:
    enum Modifier {
        NONE  = 0x0000,
        LSHIFT= 0x0001,
        RSHIFT= 0x0002,
        LCTRL = 0x0040,
        RCTRL = 0x0080,
        LALT  = 0x0100,
        RALT  = 0x0200,
        LMETA = 0x0400,
        RMETA = 0x0800,
        NUM   = 0x1000,
        CAPS  = 0x2000,
        MODE  = 0x4000,
        RESERVED = 0x8000,
        SHIFT = LSHIFT | RSHIFT,
        CTRL = LCTRL | RCTRL,
        ALT = LALT | RALT,
        META = LMETA | RMETA
    };

    /*
     * enum to select to which eventType has to be broadcast, see EventHandler::broadcast
     */
	enum eventType{
    	MOUSEPRESSEVENT       = 0x0001,
	    MOUSERELEASEEVENT     = 0x0002,
	    MOUSEMOVEEVENT        = 0x0004,
	    MOUSEDOUBLECLICKEVENT = 0x0008,
	    WHEELEVENT            = 0x0010,
	    TIMEREVENT            = 0x0020,
	    KEYEVENT              = 0x0040,
        ROTATIONEVENT         = 0x0080,
	    ZOOMEVENT             = 0x0100,
	    SIZINGEVENT           = 0x0200,
        TRANSLATIONEVENT      = 0x0400,

	    ALLEVENTTYPES = MOUSEPRESSEVENT | MOUSERELEASEEVENT | MOUSEMOVEEVENT | MOUSEDOUBLECLICKEVENT | WHEELEVENT| TIMEREVENT | KEYEVENT | ROTATIONEVENT | ZOOMEVENT | SIZINGEVENT | TRANSLATIONEVENT
	};


    /**
     * The Constructor. The use of the \a accepted_ variable is modeled after qt-events; \a accepted_ being on true 
     * as the default-value just opens the possibility to pass on the event using ignore().
     */
    Event() {
        accepted_ =  true;
    }

    virtual ~Event() {}

    bool isAccepted() {
        return accepted_;
    }
    void accept() {
        accepted_ = true;
    }

    void ignore() {
        accepted_ = false;
    }

	virtual int getEventType() =0;

    bool accepted_;

};

}

#endif //TGT_EVENT_H
