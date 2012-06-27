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

#ifndef TGT_EVENTLISTENER_H
#define TGT_EVENTLISTENER_H

#include "tgt/types.h"

namespace tgt {

class Event;
class KeyEvent;
class MouseEvent;
class TimeEvent;

class TGT_API EventListener {
public:
    EventListener();
    virtual ~EventListener();

    virtual void mousePressEvent(MouseEvent* e);
    virtual void mouseReleaseEvent(MouseEvent* e);
    virtual void mouseMoveEvent(MouseEvent* e);
    virtual void mouseDoubleClickEvent(MouseEvent* e);
    virtual void wheelEvent(MouseEvent* e);
    virtual void timerEvent(TimeEvent* e);
    virtual void keyEvent(KeyEvent* e);

    virtual void onEvent(Event* e);

    void setEventTypes(int eventTypes);
    int getEventTypes() const;

private:
    /*
     * bitmask in which all eventTypes this EventListener is listening to are stored
     * could be used to deactivate an EventListener by setting eventTypes_ = 0
     */
    int eventTypes_;
};

}

#endif //TGT_EVENTLISTENER_H
