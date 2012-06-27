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

#include "tgt/glut/gluttimer.h"

#include <map>
#include <GL/glut.h>

#include "tgt/event/timeevent.h"
#include "tgt/event/eventhandler.h"


namespace tgt {

typedef std::map< int, GLUTTimer* > GLUTTimerMap;
GLUTTimerMap GLUTTimerMap_;
int GLUTTimer::GLUTTimerCounter_ = 0;

GLUTTimer::GLUTTimer( EventHandler* eh )
    : Timer( eh )
{
    id_ = GLUTTimerCounter_;
    ++GLUTTimerCounter_;

    GLUTTimerMap_.insert( std::make_pair( id_, this ) );
}

GLUTTimer::~GLUTTimer() {
    GLUTTimerMap_.erase( id_ );
}

void GLUTTimer::start( const int msec, const int limit ) {
    if ( stopped_ ) {
        limit_ = limit;
        tickTime_ = msec;
        stopped_ = false;
        glutTimerFunc(tickTime_, timerEventFunc, id_ );
    }
}

void GLUTTimer::stop(){
    // GLUT timer can't be stopped. We'll just ignore
    stopped_ = true;
}

void GLUTTimer::setTickTime( const int msec ){
    if ( !stopped_) {
        tickTime_ = msec;

        // unfortunaly there is no way to stop a glut timer
        glutTimerFunc(tickTime_, timerEventFunc, id_ );
    }
}

void GLUTTimer::timerEvent(){
    ++count_;

    if ( ( limit_ == 0 || count_ <= limit_ ) && !stopped_ ) {
        tgt::TimeEvent* te = new tgt::TimeEvent( this );
        eventHandler_->broadcast(te);

        // shoot next event:
        if ( count_ < limit_ || limit_ == 0 ) {
            glutTimerFunc(tickTime_, timerEventFunc, id_ );
        }
    } else {
        stop();
    }
}

void GLUTTimer::timerEventFunc( int id ) {
    GLUTTimerMap_.find( id )->second->timerEvent();
}

}
