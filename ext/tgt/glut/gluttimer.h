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

#ifndef TGT_GLUTTIMER_H
#define TGT_GLUTTIMER_H

#include "tgt/timer.h"

namespace tgt {

class EventHandler;

class GLUTTimer : public Timer {
public:
    // FIXME: strange comment ...
    /// used to hold the canvases registered in our app.
    /// must be global for the glut wrapper.
    /// see GLUTApplication for more details
    static GLUTTimer* GLUTTimers_[];
    static int GLUTTimerCounter_;

    GLUTTimer( EventHandler* eh );
    virtual ~GLUTTimer();
    virtual void start( const int msec, const int limit = 0 );
    virtual void stop();
    virtual void setTickTime( const int msec );
    virtual void timerEvent();
    static void timerEventFunc( int id );

protected:
    int id_;
};

} // namespace tgt

#endif //TGT_GLUTTIMER_H
