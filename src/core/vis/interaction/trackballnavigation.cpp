/**********************************************************************
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

#include "voreen/core/vis/interaction/trackballnavigation.h"

#include "voreen/core/vis/network/networkevaluator.h"
#include "tgt/camera.h"

namespace voreen {

using tgt::vec3;
using tgt::vec2;
using tgt::ivec2;
using tgt::quat;
using tgt::MouseEvent;
using tgt::KeyEvent;

const std::string TrackballNavigation::resetTrackball_("reset.trackball");
const std::string TrackballNavigation::toggleTrackball_("toggle.trackball");

const std::string TrackballNavigation::loggerCat_ = "voreen.vis.Trackballnavigation";

TrackballNavigation::TrackballNavigation(tgt::Camera* camera, float minDist, float maxDist)
    : trackball_(new VoreenTrackball(camera))
    , minDistance_(minDist)
    , maxDistance_(maxDist)
{

    initializeEventHandling();

    setMouseRotate(tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::Event::NONE);
    setMouseZoom(tgt::MouseEvent::MOUSE_BUTTON_RIGHT, tgt::vec2(0.f, 0.5f), tgt::Event::NONE);

    setMouseMove(tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::Event::SHIFT);
    setMouseRoll(tgt::MouseEvent::MOUSE_BUTTON_RIGHT, 0.5f, tgt::Event::SHIFT);

    setMouseWheelZoom(10.f, true, tgt::Event::NONE);
    setMouseWheelRoll(10.f, true, tgt::Event::SHIFT);
    trackball_->setSize(0.7f); // sets trackball sensitivity
    resetButton_ = tgt::MouseEvent::NO_MOUSE_BUTTON;

    trackball_->setMoveCenter(false);
//        trackball_->setContinuousSpin(false);

    wheelCounter_   = -1;
    spinCounter_    = -1;
    moveCounter_    = -1;

    spinit_ = true;

    trackballEnabled_ = true;
}

TrackballNavigation::~TrackballNavigation() {
    delete trackball_;
}

void TrackballNavigation::resetTrackball() {
    wheelCounter_   = -1;
    spinCounter_    = -1;
    moveCounter_    = -1;
    spinit_ = true;
}

void TrackballNavigation::mousePressEvent(tgt::MouseEvent* e) {

    if (!trackball_)
        return;

    if (trackballEnabled_) {

        startMouseDrag( e );
        e->ignore();

        /*if (e->button() == getRotateButton() && getContinuousSpin()) {
            spinit_ = true;
            moveCounter_ = 0;
        }*/
    }
}

void TrackballNavigation::mouseReleaseEvent(tgt::MouseEvent* e) {

    if (!trackball_)
        return;

    if (trackballEnabled_) {
        endMouseDrag( e );
        e->ignore();

        moveCounter_ = -1;
    }
}

void TrackballNavigation::mouseMoveEvent(tgt::MouseEvent* e) {

    e->ignore();

    if (!trackball_)
        return;

    if (trackballEnabled_) {

        vec2 newMouse = scaleMouse( ivec2(e->x(), e->y()), e->viewport() );

        if ( mouseRotateButton_ & e->button() &&
            (mouseRotateMod_ == e->modifiers() || mouseRotateMod_ & e->modifiers()) ) {
            trackball_->rotate(newMouse, lastMousePosition_);
            e->accept();
        }
        if ( mouseMoveButton_ & e->button() &&
            (mouseMoveMod_ == e->modifiers() || mouseMoveMod_ & e->modifiers()) ) {
                trackball_->move(newMouse, lastMousePosition_);
                e->accept();
        }
        if ( mouseZoomButton_ & e->button() &&
            (mouseZoomMod_ == e->modifiers() || mouseZoomMod_ & e->modifiers()) ) {
                trackball_->zoom(newMouse, lastMousePosition_, mouseZoomInDirection_);
                e->accept();
        }
        if ( mouseRollButton_ & e->button() &&
            (mouseRollMod_ == e->modifiers() || mouseRollMod_ & e->modifiers()) ) {
                rollCameraHorz((newMouse.x-lastMousePosition_.x) / mouseRollAcuteness_);
                e->accept();
        }

        lastMousePosition_ = newMouse;

        // restrict distance within specified range
        if (trackball_->getCenterDistance() < minDistance_)
            trackball_->zoomAbsolute(minDistance_);
        if (trackball_->getCenterDistance() > maxDistance_)
            trackball_->zoomAbsolute(maxDistance_);

        /*if (trackball_->getContinuousSpin())
            moveCounter_ = 0; */
    }
}


void TrackballNavigation::mouseDoubleClickEvent(tgt::MouseEvent* e) {

    e->ignore();

    if (!trackball_ )
        return;

    if (trackballEnabled_ && (tgt::MouseEvent::MouseButtons)e->button() == resetButton_) {
        resetTrackball();
        e->accept();
    }
}

void TrackballNavigation::wheelEvent(tgt::MouseEvent* e) {

    e->ignore();

    if (!trackball_ )
        return;

    wheelCounter_ = 0;

    if (trackballEnabled_) {

        if ( mouseWheelZoom_ &&
            (mouseWheelZoomMod_ == e->modifiers() || mouseWheelZoomMod_ & e->modifiers()) ) {
                trackball_->zoom( getZoomFactor( mouseWheelZoomAcuteness_,
                    ( mouseWheelUpZoomIn_ && (e->button()&MouseEvent::MOUSE_WHEEL_UP)) ||
                    (!mouseWheelUpZoomIn_ && (e->button()&MouseEvent::MOUSE_WHEEL_DOWN))
                    ));
                e->accept();
        }
        else if ( mouseWheelRoll_ &&
            (mouseWheelRollMod_ == e->modifiers() || mouseWheelRollMod_ & e->modifiers()) ) {
                rollCameraHorz( getRollAngle( mouseWheelRollAcuteness_,
                    ( mouseWheelUpRollLeft_ && (e->button()&MouseEvent::MOUSE_WHEEL_UP)) ||
                    (!mouseWheelUpRollLeft_ && (e->button()&MouseEvent::MOUSE_WHEEL_DOWN))
                    ) );
                e->accept();
        }


        // restrict distance within specified range
        if (trackball_->getCenterDistance() < minDistance_)
            trackball_->zoomAbsolute(minDistance_);
        if (trackball_->getCenterDistance() > maxDistance_)
            trackball_->zoomAbsolute(maxDistance_);

    }
}

void TrackballNavigation::timerEvent(tgt::TimeEvent* e) {

    e->ignore();

    if (!trackball_)
        return;

    if (wheelCounter_ >= 0)
        wheelCounter_++;
    if (moveCounter_ >= 0)
        moveCounter_++;

    if (wheelCounter_ == 70) {
        wheelCounter_ = -1;
    }
    if (moveCounter_ == 10) {
        spinit_ = false;
        moveCounter_ = -1;
    }

    /*if (trackball_->getContinuousSpin() && spinit_ &&
        e->getTimer() == trackball_->getContinuousSpinTimer())
    {
        trackball_->timerEvent(e);
        distributeCamera(trackball_->getCamera());
    } */
}

void TrackballNavigation::keyEvent(tgt::KeyEvent* e) {

    e->ignore();
    bool accepted = false;

    if ( (keyRotateMod_ == e->modifiers() || keyRotateMod_ & e->modifiers())
         && e->pressed() == keyRotatePressed_) {
            if (e->keyCode() == keyRotateLeft_) {
                trackball_->rotate(vec3(0.f, 1.f, 0.f),  getRotationAngle(keyRotateAcuteness_));
                accepted = true;
            }
            else if (e->keyCode() == keyRotateRight_) {
                trackball_->rotate(vec3(0.f, 1.f, 0.f), -getRotationAngle(keyRotateAcuteness_));
                accepted = true;
            }
            else if (e->keyCode() == keyRotateUp_) {
                trackball_->rotate(vec3(1.f, 0.f, 0.f), -getRotationAngle(keyRotateAcuteness_));
                accepted = true;
            }
            else if (e->keyCode() == keyRotateDown_) {
                trackball_->rotate(vec3(1.f, 0.f, 0.f),  getRotationAngle(keyRotateAcuteness_));
                accepted = true;
            }
    }
    if ( (keyMoveMod_ == e->modifiers() || keyMoveMod_ & e->modifiers())
         && e->pressed() == keyMovePressed_) {
            if (e->keyCode() == keyMoveLeft_) {
                trackball_->move(getMovementLength(keyMoveAcuteness_), vec3(-1.f, 0.f, 0.f));
                accepted = true;
            }
            else if (e->keyCode() == keyMoveRight_) {
                trackball_->move(getMovementLength(keyMoveAcuteness_), vec3( 1.f, 0.f, 0.f));
                accepted = true;
            }
            else if (e->keyCode() == keyMoveUp_) {
                trackball_->move(getMovementLength(keyMoveAcuteness_), vec3(0.f,  1.f, 0.f));
                accepted = true;
            }
            else if (e->keyCode() == keyMoveDown_) {
                trackball_->move(getMovementLength(keyMoveAcuteness_), vec3(0.f, -1.f, 0.f));
                accepted = true;
            }
    }
    if ( (keyZoomMod_ == e->modifiers() || keyZoomMod_ & e->modifiers())
         && e->pressed() == keyZoomPressed_) {
            if (e->keyCode() == keyZoomIn_) {
                trackball_->zoom(getZoomFactor(keyZoomAcuteness_, true));
                accepted = true;
            }
            else if (e->keyCode() == keyZoomOut_) {
                trackball_->zoom(getZoomFactor(keyZoomAcuteness_, false));
                accepted = true;
            }
    }
    if ( (keyRollMod_ == e->modifiers() || keyRollMod_ & e->modifiers())
         && e->pressed() == keyRollPressed_) {
            if (e->keyCode() == keyRollLeft_) {
                rollCameraHorz(getRollAngle(keyRollAcuteness_, true));
                accepted = true;
            }
            else if (e->keyCode() == keyRollRight_) {
                rollCameraHorz(getRollAngle(keyRollAcuteness_, false));
                accepted = true;
            }
    }

    if (accepted)
        e->accept();

}
VoreenTrackball* TrackballNavigation::getTrackball() {
    return trackball_;
}

void TrackballNavigation::startMouseDrag(tgt::MouseEvent* e) {

    lastMousePosition_ = scaleMouse( ivec2(e->x(), e->y()), e->viewport() );

    /*if (continuousSpin_ && continuousSpinTimer_) {
        if ( mouseRotateButton_ & e->button() &&
            (mouseRotateMod_ == e->modifiers() || mouseRotateMod_ & e->modifiers()) ) {
                // keep track how long last roation took
                continuousSpinTimer_->stop();
                continuousSpinStopwatch_->reset();
                continuousSpinLastOrientationChangeMSecs_ = 0;
                continuousSpinStopwatch_->start();
        }
    }

    tracking_ = true; */

}

void TrackballNavigation::endMouseDrag(tgt::MouseEvent* /*e*/) {

    /*if (continuousSpin_ && continuousSpinStopwatch_) {
        // Make sure this endMouseDrag belongs to a rotation
        if ((mouseRotateButton_ & e->button()) &&
            (mouseRotateMod_ == e->modifiers() || mouseRotateMod_ & e->modifiers())) {
                // Make sure mouse has been moved within the last time.
                if (continuousSpinStopwatch_->getRuntime() < 2.*continuousSpinLastOrientationChangeMSecs_) {
                    if (continuousSpinLastOrientationChangeMSecs_ < 1000.f / 25.f) {
                        // Last rotation took less than 40 msecs. As all rotations with more than
                        // 25 steps per second (less than 40 msecs) will look smooth, we do not
                        // want to rotate with a frequency that high. We will scale the last
                        // orientation change to be so large that rotation with 25 Hz will make
                        // the trackball rotate with the according speed.
                        float scale = (1000.f / 25.f) / static_cast<float>(continuousSpinLastOrientationChangeMSecs_);
                        //                    lastOrientationChange_.w = cosf( scale * acos( lastOrientationChange_.w ) );
                        lastOrientationChange_ = expQuat( scale * logQuat( lastOrientationChange_ ) );
                        lastOrientationChange_.normalize();
                        continuousSpinTimer_->start(1000 / 25, 0);
                    } else {
                        continuousSpinTimer_->start(continuousSpinLastOrientationChangeMSecs_, 0);
                    }
                }
                if (1 - lastOrientationChange_.w < 0.000002) {
                    // Rotation is very small. Probably user did not mean to spin the trackball.
                    lastOrientationChange_.x = 0.f;
                    lastOrientationChange_.y = 0.f;
                    lastOrientationChange_.z = 0.f;
                    lastOrientationChange_.w = 1.f;
                    continuousSpinTimer_->stop();
                }
        }
        continuousSpinStopwatch_->stop();
        continuousSpinStopwatch_->reset();
        continuousSpinLastOrientationChangeMSecs_ = 0;
    } */

    tracking_ = false;
}

// This can be used to rotate (or "roll") the camera left and right, angle in radian measure
void TrackballNavigation::rollCameraHorz(float angle) {
    vec3 up = normalize( quat::rotate(trackball_->getCamera()->getUpVector(), angle, trackball_->getCamera()->getLook()) );
    trackball_->getCamera()->setUpVector(up);
}

// This can be used to rotate (or "roll") the camera forwards and backwards, angle in radian measure
void TrackballNavigation::rollCameraVert(float angle) {
    vec3 up = normalize( quat::rotate(trackball_->getCamera()->getUpVector(), angle, trackball_->getCamera()->getStrafe()) );
    trackball_->getCamera()->setUpVector(up);
    vec3 look = cross(up, trackball_->getCamera()->getStrafe());
    trackball_->getCamera()->setFocus(trackball_->getCamera()->getPosition() + look);
}


tgt::vec2 TrackballNavigation::scaleMouse(const tgt::ivec2& coords, const tgt::ivec2& viewport) const {
    return vec2( static_cast<float>(coords.x*2.f) / static_cast<float>(viewport.x) - 1.f,
                 1.f - static_cast<float>(coords.y*2.f) / static_cast<float>(viewport.y) );
}



float TrackballNavigation::getRotationAngle(float acuteness) const {
    return 1.f / acuteness;
}

float TrackballNavigation::getMovementLength(float acuteness) const {
    return trackball_->getSize() / acuteness;
}

float TrackballNavigation::getZoomFactor(float acuteness, bool zoomIn) const {
    if (zoomIn) {
        return 1.f + 1.f/acuteness;
    }
    else {
        return 1.f - 1.f/acuteness;
    }
}

float TrackballNavigation::getRollAngle(float acuteness, bool left) const {
    if (left) {
        return - 1.f / acuteness;
    }
    else {
        return 1.f / acuteness;
    }
}

void TrackballNavigation::initializeEventHandling() {
    mouseRotateButton_ = mouseMoveButton_ = mouseZoomButton_ = mouseRollButton_
        = MouseEvent::NO_MOUSE_BUTTON;
    keyRotateLeft_ = keyRotateRight_ = keyRotateUp_ = keyRotateDown_
        = keyMoveLeft_ = keyMoveRight_ = keyMoveUp_ = keyMoveDown_
        = keyZoomIn_ = keyZoomOut_
        = keyRollLeft_ = keyRollRight_
        = KeyEvent::K_LAST;
    mouseWheelZoom_ = mouseWheelRoll_ = false;
}

void TrackballNavigation::setMouseRotate(MouseEvent::MouseButtons button, int mod) {
    mouseRotateButton_ = button;
    mouseRotateMod_ = mod;
}

void TrackballNavigation::setKeyRotate(float acuteness,
                             KeyEvent::KeyCode left,
                             KeyEvent::KeyCode right,
                             KeyEvent::KeyCode up,
                             KeyEvent::KeyCode down,
                             int mod, bool pressed) {
    if (acuteness != 0.f) {
        keyRotateAcuteness_ = acuteness;
        keyRotateLeft_ = left;
        keyRotateRight_ = right;
        keyRotateUp_ = up;
        keyRotateDown_ = down;
        keyRotateMod_ = mod;
        keyRotatePressed_ = pressed;
    }
    else {
        keyRotateLeft_ = keyRotateRight_ = keyRotateUp_ = keyRotateDown_ = KeyEvent::K_LAST;
    }
}

void TrackballNavigation::setMouseMove(MouseEvent::MouseButtons button, int mod) {
    mouseMoveButton_ = button;
    mouseMoveMod_ = mod;
}

void TrackballNavigation::setKeyMove(float acuteness,
                           KeyEvent::KeyCode left,
                           KeyEvent::KeyCode right,
                           KeyEvent::KeyCode up,
                           KeyEvent::KeyCode down,
                           int mod, bool pressed) {
    if (acuteness != 0.f) {
        keyMoveAcuteness_ = acuteness;
        keyMoveLeft_ = left;
        keyMoveRight_ = right;
        keyMoveUp_ = up;
        keyMoveDown_ = down;
        keyMoveMod_ = mod;
        keyMovePressed_ = pressed;
    }
    else {
        keyMoveLeft_ = keyMoveRight_ = keyMoveUp_ = keyMoveDown_ = KeyEvent::K_LAST;
    }
}

void TrackballNavigation::setMouseZoom(MouseEvent::MouseButtons button,
                             vec2 zoomInDirection,
                             int mod) {
    mouseZoomButton_ = button;
    mouseZoomInDirection_ = normalize(zoomInDirection);
    mouseZoomMod_ = mod;
}

void TrackballNavigation::setMouseWheelZoom(float acuteness, bool wheelUpZoomIn, int mod) {
    if (acuteness != 0.f) {
        mouseWheelZoom_ = true;
        mouseWheelZoomAcuteness_ = acuteness;
        mouseWheelUpZoomIn_ = wheelUpZoomIn;
        mouseWheelZoomMod_ = mod;
    }
    else {
        mouseWheelZoom_ = false;
    }
}

void TrackballNavigation::setKeyZoom(float acuteness, KeyEvent::KeyCode in, KeyEvent::KeyCode out,
                                     int mod, bool pressed) {
    if (acuteness != 0.f) {
        keyZoomAcuteness_ = acuteness;
        keyZoomIn_ = in;
        keyZoomOut_ = out;
        keyZoomMod_ = mod;
        keyZoomPressed_ = pressed;
    }
    else {
        keyZoomIn_ = keyZoomOut_ = KeyEvent::K_LAST;
    }
}
void TrackballNavigation::setMouseRoll(MouseEvent::MouseButtons button, float acuteness, int mod) {
    mouseRollButton_ = button;
    mouseRollAcuteness_ = acuteness;
    mouseRollMod_ = mod;
}

void TrackballNavigation::setMouseWheelRoll(float acuteness, bool wheelUpRollLeft, int mod) {
    if (acuteness != 0.f) {
        mouseWheelRoll_ = true;
        mouseWheelRollAcuteness_ = acuteness;
        mouseWheelUpRollLeft_ = wheelUpRollLeft;
        mouseWheelRollMod_ = mod;
    }
    else {
        mouseWheelRoll_ = false;
    }
}

void TrackballNavigation::setKeyRoll(float acuteness, KeyEvent::KeyCode left, KeyEvent::KeyCode right,
                                     int mod, bool pressed) {
    if (acuteness != 0.f) {
        keyRollAcuteness_ = acuteness;
        keyRollLeft_ = left;
        keyRollRight_ = right;
        keyRollMod_ = mod;
        keyRollPressed_ = pressed;
    }
    else {
        keyRollLeft_ = keyRollRight_ = KeyEvent::K_LAST;
    }

}


} // namespace
