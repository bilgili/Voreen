#/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/vis/interaction/camerainteractionhandler.h"
#include "voreen/core/vis/interaction/trackballnavigation.h"
#include "voreen/core/vis/properties/cameraproperty.h"

#include "tgt/event/mouseevent.h"
#include "tgt/event/keyevent.h"
#include "tgt/event/timeevent.h"

using tgt::Event;
using tgt::MouseEvent;
using tgt::KeyEvent;
using tgt::TimeEvent;

namespace voreen {

CameraInteractionHandler::CameraInteractionHandler(CameraProperty* cameraProp) :
    InteractionHandler(),
    active_("camerahandler.active", "Camera handler active", true, Processor::VALID),
    shareEvents_("camerahandler.shareEvents", "Share events", false, Processor::VALID)
{

    tgtAssert(cameraProp, "No camera property");
    tgtAssert(cameraProp->get(), "No camera");

    cameraProp_ = cameraProp;
    trackNavi_ = new TrackballNavigation(cameraProp_->get(), 0.05f, 15.f);

    addProperty(active_);
    addProperty(shareEvents_);
}

CameraInteractionHandler::~CameraInteractionHandler() {

    delete trackNavi_;
}

void CameraInteractionHandler::onEvent(Event* eve) {

    tgtAssert(cameraProp_, "No camera property");
    tgtAssert(trackNavi_, "No trackball navigation");
    tgtAssert(trackNavi_->getTrackball(), "No trackball");

    // assign new camera object to navigation, if it has changed
    if (trackNavi_->getTrackball()->getCamera() != cameraProp_->get()) {
        trackNavi_->getTrackball()->setCamera(cameraProp_->get());
    }

    // ignore event, if handler not active
    if (active_.get() == false) {
        eve->ignore();
        return;
    }

    // propagate event to trackball navigation
    if (MouseEvent* mouseEve = dynamic_cast<MouseEvent*>(eve)) {
        if (mouseEve->action() == MouseEvent::PRESSED) {
            cameraProp_->toggleInteractionMode(true, this);
            trackNavi_->mousePressEvent(mouseEve);
        }
        else if (mouseEve->action() == MouseEvent::RELEASED) {
            cameraProp_->toggleInteractionMode(false, this);
            trackNavi_->mouseReleaseEvent(mouseEve);
        }
        else if (mouseEve->action() == MouseEvent::MOTION)
            trackNavi_->mouseMoveEvent(mouseEve);
        else if (mouseEve->action() == MouseEvent::DOUBLECLICK)
            trackNavi_->mouseDoubleClickEvent(mouseEve);
        else if (mouseEve->action() == MouseEvent::WHEEL)
            trackNavi_->wheelEvent(mouseEve);
    }
    else if (TimeEvent* timeEve = dynamic_cast<TimeEvent*>(eve)) {
        trackNavi_->timerEvent(timeEve);
    }
    else if (KeyEvent* keyEve = dynamic_cast<KeyEvent*>(eve)) {
        trackNavi_->keyEvent(keyEve);
    }

    // invalidate processor and update camera prop widgets, if event has been accepted
    if (eve->isAccepted()) {
        cameraProp_->notifyChange();
        cameraProp_->updateWidgets();

        // share event
        if (shareEvents_.get())
            eve->ignore();
    }
}

} // namespace
