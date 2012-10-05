/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2005-2012 Visualization and Computer Graphics Group, *
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

#include "tgt/qt/qttimer.h"
#include "tgt/event/timeevent.h"
#include <QObject>

namespace tgt {

QtTimer::QtTimer(EventHandler* eh)
    : Timer(eh)
{
}

QtTimer::~QtTimer() {
}

void QtTimer::start(const int msec, const int limit) {
    if (stopped_) {
        count_ = 0;
        limit_ = limit;
        tickTime_ = msec;
        stopped_ = false;
        id_ = QObject::startTimer(msec);
    }
}

void QtTimer::stop() {
    if (!stopped_) {
        stopped_ = true;
        QObject::killTimer( id_ );
    }
}

void QtTimer::setTickTime(const int msec) {
    stop();
    start( msec, limit_ );
}

void QtTimer::timerEvent(QTimerEvent* /*e*/) {
    ++count_;

    if ((limit_ == 0 || count_ <= limit_) && !stopped_) {
        tgt::TimeEvent* te = new tgt::TimeEvent(this);
        eventHandler_->broadcast(te);
    } else {
        stop();
    }
}

}
