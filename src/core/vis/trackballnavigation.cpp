/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/trackballnavigation.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/networkevaluator.h"

namespace voreen {

using tgt::vec3;

const Identifier TrackballNavigation::resetTrackball_("reset.trackball");
const Identifier TrackballNavigation::toggleTrackball_("toggle.trackball");

const std::string TrackballNavigation::loggerCat_ = "voreen.vis.Trackballnavigation";

TrackballNavigation::TrackballNavigation(tgt::Trackball* track, bool defaultBehavior, float minDist, float maxDist)
    : trackball_(track)
    , minDistance_(minDist)
    , maxDistance_(maxDist)
{
    if (trackball_) {
        if (defaultBehavior) {
            trackball_->setMouseRotate(tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::Event::NONE);
            trackball_->setMouseMove(tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::Event::SHIFT);
            trackball_->setMouseZoom(tgt::MouseEvent::MOUSE_BUTTON_RIGHT, tgt::vec2(0.f, 0.5f), tgt::Event::NONE);
            trackball_->setMouseWheelZoom(10.f, false, tgt::Event::NONE);
            trackball_->setMouseWheelRoll(10.f, true, tgt::Event::SHIFT);
            trackball_->setSize(0.7f); // sets trackball sensitivity
            resetButton_ = tgt::MouseEvent::MOUSE_BUTTON_RIGHT;
        }
        //else: the above is voreen standard behaviour.  If something else is desired, change behaviour with trackNavi->getTrackball()->setMouseRotate(...)
        trackball_->setMoveCenter(false);
        trackball_->setContinuousSpin(false);
    }

    wheelCounter_   = -1;
    spinCounter_    = -1;
    moveCounter_    = -1;

    spinit_ = true;

    trackballEnabled_ = true;
}

void TrackballNavigation::resetTrackball() {

    wheelCounter_   = -1;
    spinCounter_    = -1;
    moveCounter_    = -1;
    spinit_ = true;

    trackball_->reset();

    if (trackball_->getCanvas()->getPainter()) {
        postMessage( new CameraPtrMsg(VoreenPainter::cameraChanged_, trackball_->getCamera()) );
        trackball_->getCanvas()->repaint();
    }
}

void TrackballNavigation::mousePressEvent(tgt::MouseEvent* e) {

    if (!trackball_ || !trackball_->getCanvas() || !trackball_->getCanvas()->getPainter())
        return;

    if (trackballEnabled_) {
        postMessage( new BoolMsg(VoreenPainter::switchCoarseness_, true) );

        trackball_->mousePressEvent(e);
        if (e->button() == trackball_->getRotateButton() && trackball_->getContinuousSpin()) {
            spinit_ = true;
            moveCounter_ = 0;
        }
        // mid-button? what was supposed to happen in that case?
    }
}

void TrackballNavigation::mouseReleaseEvent(tgt::MouseEvent* e) {

    if (!trackball_ || !trackball_->getCanvas() || !trackball_->getCanvas()->getPainter())
        return;

    if (trackballEnabled_) {

            trackball_->mouseReleaseEvent(e);

            if (!trackball_->getContinuousSpin() || !spinit_)
                postMessage( new BoolMsg(VoreenPainter::switchCoarseness_, false) );

            moveCounter_ = -1;
            trackball_->getCanvas()->update();
    }
}

void TrackballNavigation::mouseMoveEvent(tgt::MouseEvent* e) {

    if (!trackball_ || !trackball_->getCanvas() || !trackball_->getCanvas()->getPainter())
        return;

    if (trackballEnabled_) {

        trackball_->mouseMoveEvent(e);

        // restrict distance within specified range
        if (trackball_->getCenterDistance() < minDistance_)
            trackball_->zoomAbsolute(minDistance_);
        if (trackball_->getCenterDistance() > maxDistance_)
            trackball_->zoomAbsolute(maxDistance_);

        postMessage( new CameraPtrMsg(VoreenPainter::cameraChanged_, trackball_->getCamera()) );
        trackball_->getCanvas()->update();

         if (trackball_->getContinuousSpin()) {
             moveCounter_ = 0;
         }
    }
}


void TrackballNavigation::mouseDoubleClickEvent(tgt::MouseEvent* e) {

    if (!trackball_ || !trackball_->getCanvas() || !trackball_->getCanvas()->getPainter())
        return;

    if (trackballEnabled_ && (tgt::MouseEvent::MouseButtons)e->button() == resetButton_) {
        resetTrackball();
    }

}

void TrackballNavigation::wheelEvent(tgt::MouseEvent* e) {

    if (!trackball_ || !trackball_->getCanvas() || !trackball_->getCanvas()->getPainter())
        return;

    wheelCounter_ = 0;

    if (trackballEnabled_) {
        trackball_->wheelEvent(e);

        // restrict distance within specified range
        if (trackball_->getCenterDistance() < minDistance_)
            trackball_->zoomAbsolute(minDistance_);
        if (trackball_->getCenterDistance() > maxDistance_)
            trackball_->zoomAbsolute(maxDistance_);

        postMessage( new CameraPtrMsg(VoreenPainter::cameraChanged_, trackball_->getCamera()) );
        postMessage( new BoolMsg(VoreenPainter::switchCoarseness_, true) );
        trackball_->getCanvas()->update();
    }
}

void TrackballNavigation::timerEvent(tgt::TimeEvent* e) {

    if (!trackball_ || !trackball_->getCanvas() || !trackball_->getCanvas()->getPainter())
        return;

    if (wheelCounter_ >= 0)
        wheelCounter_++;
    if (moveCounter_ >= 0)
        moveCounter_++;

    if (wheelCounter_ == 70) {
        postMessage( new BoolMsg(VoreenPainter::switchCoarseness_, false) );
        trackball_->getCanvas()->update();
        wheelCounter_ = -1;
    }
    if (moveCounter_ == 10) {
        spinit_ = false;
        moveCounter_ = -1;
    }

    if (trackball_->getContinuousSpin() && spinit_ && e->getTimer() == trackball_->getContinuousSpinTimer()) {
        trackball_->timerEvent(e);
        postMessage( new CameraPtrMsg(VoreenPainter::cameraChanged_, trackball_->getCamera()));
        postMessage( new Message(VoreenPainter::repaint_));
    }
}

void TrackballNavigation::processMessage(Message* msg, const Identifier& dest) {
    MessageReceiver::processMessage(msg, dest);

    if (msg->id_ == "switch.trackballContinuousSpin") {
        trackball_->setContinuousSpin(msg->getValue<bool>());
    }
    else if (msg->id_ == resetTrackball_) {
        resetTrackball();
    }
    else if (msg->id_ == toggleTrackball_) {
        msg->discard();
        trackballEnabled_ = msg->getValue<bool>();   
    }
    else {
        // forward message to painter
        if ( trackball_ && trackball_->getCanvas() && trackball_->getCanvas()->getPainter()) {
            VoreenPainter* painter  = (VoreenPainter*)(trackball_->getCanvas()->getPainter());
            painter->processMessage(msg, dest);
        }
        else {
            LDEBUG("Unable to forward message to renderer: No renderer");
        }
	
        // forward message to additional receivers
        std::vector<MessageReceiver*>::iterator iter;
        for (iter = additionalReceivers_.begin(); iter != additionalReceivers_.end(); iter++) {
            (*iter)->processMessage(msg);
        }
    }

}

tgt::Trackball* TrackballNavigation::getTrackball() {
    return trackball_;
}

void TrackballNavigation::addReceiver(MessageReceiver* receiver) {

    tgtAssert(receiver, "Null-Pointer passed as MessageReceiver");

    additionalReceivers_.push_back(receiver);
}

} // namespace
