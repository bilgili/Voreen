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

#ifndef TGT_EVENTHANDLER_H
#define TGT_EVENTHANDLER_H

#include <deque>

#include "tgt/event/eventlistener.h"
#include "tgt/event/event.h"

namespace tgt {

class TGT_API EventHandler {
private:
    std::deque<EventListener*> listeners_; ///< The deque containing all the listeners.

public:
    EventHandler();
    virtual ~EventHandler() {}

    /**
     * Add a listener to the \a listeners_ deques back.
     */
    void addListenerToBack(EventListener* e);

    /**
     * Add a listener to the \a listeners_ deques front.
     */
    void addListenerToFront(EventListener* e);

    /**
     * Remove a listener from the \a listeners_ deque.
     */
    void removeListener(EventListener* e);

    /**
     * Removes all listeners from \a listeners_ deque.
     */
    void clear();

    /**
     * Send an event to all listeners in the \a listeners_ deque.  If a listener accepts the event,
     * the broadcast is aborted.  All broadcasted events are deleted at the end of this method.
     */
    virtual void broadcast(Event* e);

    /**
     * Returns the number of listeners associated.
     */
    size_t getListenerNumber() const;
};

}

#endif //TGT_EVENTHANDLER_H
