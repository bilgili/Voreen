/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/core/voreenapplication.h"
#include "voreen/core/interaction/camerainteractionhandler.h"
#include "voreen/core/interaction/trackballnavigation.h"
#include "voreen/core/interaction/firstpersonnavigation.h"
#include "voreen/core/properties/cameraproperty.h"

#include "tgt/event/mouseevent.h"
#include "tgt/event/keyevent.h"
#include "tgt/event/timeevent.h"
#include "tgt/timer.h"
#include "voreen/core/properties/eventproperty.h"

using tgt::Event;
using tgt::MouseEvent;
using tgt::KeyEvent;
using tgt::TimeEvent;

namespace voreen {

CameraInteractionHandler::CameraInteractionHandler() :
    InteractionHandler("dummy", "dummy"),
    navigationMetaphor_("naviMetaphor", "Navigation Metaphor", Processor::VALID)
{}

CameraInteractionHandler::CameraInteractionHandler(const std::string& id, const std::string& guiName,
                                                   CameraProperty* cameraProp, bool sharing, bool enabled)
    : InteractionHandler(id, guiName)
    , cameraProp_(cameraProp)
    , navigationMetaphor_(id + ".interactionMetaphor", guiName + " Interaction", Processor::VALID)
{
    tgtAssert(cameraProp, "No camera property");
    tgtAssert(cameraProp->get(), "No camera");
    cameraProp_ = cameraProp;

    navigationMetaphor_.addOption("trackball",     "Trackball",     TRACKBALL);
    navigationMetaphor_.addOption("first-person",  "First-Person",  FIRST_PERSON);
    navigationMetaphor_.select("trackball");
    navigationMetaphor_.onChange(CallMemberAction<CameraInteractionHandler>(this, &CameraInteractionHandler::adjustWidgetStates));
    addProperty(navigationMetaphor_);

    // navigations
    rotateNavi_ = new TrackballNavigation(cameraProp_->get(), TrackballNavigation::ROTATE_MODE, 0.05f, 15.f);
    zoomNavi_ = new TrackballNavigation(cameraProp_->get(),   TrackballNavigation::ZOOM_MODE, 0.05f, 15.f);
    shiftNavi_ = new TrackballNavigation(cameraProp_->get(),  TrackballNavigation::SHIFT_MODE, 0.05f, 15.f);
    fpNavi_ = new FirstPersonNavigation(cameraProp_->get());

    // event properties trackball
    rotateEvent_ = new EventProperty<CameraInteractionHandler>(id + ".rotate", guiName + " Rotate", this,
        &CameraInteractionHandler::rotateEvent,
        MouseEvent::MOUSE_BUTTON_LEFT,
        MouseEvent::ACTION_ALL,
        tgt::Event::MODIFIER_NONE, sharing, enabled);
    addEventProperty(rotateEvent_);

    zoomEvent_ = new EventProperty<CameraInteractionHandler>(id + ".zoom", guiName + " Zoom", this,
        &CameraInteractionHandler::zoomEvent,
        MouseEvent::MOUSE_BUTTON_RIGHT,
        MouseEvent::ACTION_ALL,
        tgt::Event::MODIFIER_NONE, sharing, enabled);
    addEventProperty(zoomEvent_);

    shiftEvent_ = new EventProperty<CameraInteractionHandler>(id + ".shift", guiName + " Shift", this,
        &CameraInteractionHandler::shiftEvent,
        MouseEvent::MOUSE_BUTTON_LEFT,
        MouseEvent::ACTION_ALL,
        tgt::Event::SHIFT, sharing, enabled);
    addEventProperty(shiftEvent_);

    wheelZoomEvent_ = new EventProperty<CameraInteractionHandler>(id + ".wheelZoom", guiName + " Wheel Zoom", this,
        &CameraInteractionHandler::zoomEvent,
        MouseEvent::MOUSE_WHEEL,
        MouseEvent::WHEEL,
        tgt::Event::MODIFIER_NONE, sharing, enabled);
    addEventProperty(wheelZoomEvent_);

    // event properties first-person navigation
    moveForwardEvent_ = new EventProperty<CameraInteractionHandler>(id + ".moveForward", guiName + " Move Forward", this,
        &CameraInteractionHandler::keyEvent,
        tgt::KeyEvent::K_W,
        tgt::Event::MODIFIER_NONE, sharing, enabled);
    moveForwardEvent_->onChange(&CameraInteractionHandler::updateFpKeySettings);
    addEventProperty(moveForwardEvent_);

    moveLeftEvent_ = new EventProperty<CameraInteractionHandler>(id + ".moveLeft", guiName + " Move Left", this,
        &CameraInteractionHandler::keyEvent,
        tgt::KeyEvent::K_A,
        tgt::Event::MODIFIER_NONE, sharing, enabled);
    moveLeftEvent_->onChange(&CameraInteractionHandler::updateFpKeySettings);
    addEventProperty(moveLeftEvent_);

    moveBackwardEvent_ = new EventProperty<CameraInteractionHandler>(id + ".moveBackward", guiName + " Move Backward", this,
        &CameraInteractionHandler::keyEvent,
        tgt::KeyEvent::K_S,
        tgt::Event::MODIFIER_NONE, sharing, enabled);
    moveBackwardEvent_->onChange(&CameraInteractionHandler::updateFpKeySettings);
    addEventProperty(moveBackwardEvent_);

    moveRightEvent_ = new EventProperty<CameraInteractionHandler>(id + ".moveRight", guiName + " Move Right", this,
        &CameraInteractionHandler::keyEvent,
        tgt::KeyEvent::K_D,
        tgt::Event::MODIFIER_NONE, sharing, enabled);
    moveRightEvent_->onChange(&CameraInteractionHandler::updateFpKeySettings);
    addEventProperty(moveRightEvent_);

    moveUpEvent_ = new EventProperty<CameraInteractionHandler>(id + ".moveUp", guiName + " Move Up", this,
        &CameraInteractionHandler::keyEvent,
        tgt::KeyEvent::K_R,
        tgt::Event::MODIFIER_NONE, sharing, enabled);
    moveUpEvent_->onChange(&CameraInteractionHandler::updateFpKeySettings);
    addEventProperty(moveUpEvent_);

    moveDownEvent_ = new EventProperty<CameraInteractionHandler>(id + ".moveDown", guiName + " Move Down", this,
        &CameraInteractionHandler::keyEvent,
        tgt::KeyEvent::K_F,
        tgt::Event::MODIFIER_NONE, sharing, enabled);
    moveDownEvent_->onChange(&CameraInteractionHandler::updateFpKeySettings);
    addEventProperty(moveDownEvent_);

    // timer
    timerEventHandler_ = new tgt::EventHandler;
    motionTimer_ = VoreenApplication::app()->createTimer(timerEventHandler_);
    tgtAssert(timerEventHandler_, "No timerEventHandler");
    timerEventHandler_->addListenerToFront(this);

    adjustWidgetStates();
}

CameraInteractionHandler::~CameraInteractionHandler() {

    // event properties are deleted by base class InteractionHandler

    delete rotateNavi_;
    delete zoomNavi_;
    delete shiftNavi_;
    delete fpNavi_;
    delete timerEventHandler_;
    delete motionTimer_;
}

// TODO: left clicked mousedragging is catched by fpNavi, too. Maybe this method should get a proper name?!
void CameraInteractionHandler::rotateEvent(tgt::MouseEvent* e) {
    if (navigationMetaphor_.isSelected("trackball")){
        tgtAssert(cameraProp_, "No camera property");
        tgtAssert(rotateNavi_, "No trackball navigation");

        // assign new camera object to navigation, if it has changed
        if (rotateNavi_->getTrackball()->getCamera() != cameraProp_->get()) {
            rotateNavi_->getTrackball()->setCamera(cameraProp_->get());
        }

        // propagate event to trackball navigation
        if (e->action() == MouseEvent::PRESSED) {
            cameraProp_->toggleInteractionMode(true, this);
            rotateNavi_->mousePressEvent(e);
        }
        else if (e->action() == MouseEvent::RELEASED) {
            cameraProp_->toggleInteractionMode(false, this);
            rotateNavi_->mouseReleaseEvent(e);
        }
        else if (e->action() == MouseEvent::MOTION)
            rotateNavi_->mouseMoveEvent(e);
        else if (e->action() == MouseEvent::DOUBLECLICK)
            rotateNavi_->mouseDoubleClickEvent(e);
        else if (e->action() == MouseEvent::WHEEL)
            rotateNavi_->wheelEvent(e);
    }
    else if (navigationMetaphor_.isSelected("first-person")){
        if (e->action() == MouseEvent::PRESSED){
            cameraProp_->toggleInteractionMode(true, this);
            fpNavi_->mousePressEvent(e);
        }
        else if (e->action() == MouseEvent::RELEASED){
            if (!fpNavi_->isMoving())
                cameraProp_->toggleInteractionMode(false, this);

            e->accept();
        }
        else if (e->action() == MouseEvent::MOTION){
            motionTimer_->stop();

            fpNavi_->mouseMoveEvent(e);

            // Restart motionTimer_, if necessary
            if (fpNavi_->isMoving())
                motionTimer_->start(40, 0);
        }
    }

    // invalidate processor and update camera prop widgets, if event has been accepted
    if (e->isAccepted()) {
        cameraProp_->invalidate();
    }
}

// TODO: right clicked mousedragging is catched by fpNavi, too. Maybe this method should get a proper name?!
void CameraInteractionHandler::zoomEvent(tgt::MouseEvent* e) {
    if (navigationMetaphor_.isSelected("trackball")){
        tgtAssert(cameraProp_, "No camera property");
        tgtAssert(zoomNavi_, "No trackball navigation");

        // assign new camera object to navigation, if it has changed
        if (zoomNavi_->getTrackball()->getCamera() != cameraProp_->get()) {
            zoomNavi_->getTrackball()->setCamera(cameraProp_->get());
        }

        // propagate event to trackball navigation
        if (e->action() == MouseEvent::PRESSED) {
            cameraProp_->toggleInteractionMode(true, this);
            zoomNavi_->mousePressEvent(e);
        }
        else if (e->action() == MouseEvent::RELEASED) {
            cameraProp_->toggleInteractionMode(false, this);
            zoomNavi_->mouseReleaseEvent(e);
        }
        else if (e->action() == MouseEvent::MOTION)
            zoomNavi_->mouseMoveEvent(e);
        else if (e->action() == MouseEvent::DOUBLECLICK)
            zoomNavi_->mouseDoubleClickEvent(e);
        else if (e->action() == MouseEvent::WHEEL)
            zoomNavi_->wheelEvent(e);
    }
    else if (navigationMetaphor_.isSelected("first-person")){
        if (e->action() == MouseEvent::PRESSED){
            cameraProp_->toggleInteractionMode(true, this);
            fpNavi_->mousePressEvent(e);
        }
        else if (e->action() == MouseEvent::RELEASED){
            cameraProp_->toggleInteractionMode(false, this);
            e->accept();
        }
        else if (e->action() == MouseEvent::MOTION){
            motionTimer_->stop();

            fpNavi_->mouseMoveEvent(e);

            // Restart motionTimer_, if necessary
            if (fpNavi_->isMoving())
                motionTimer_->start(40, 0);
        }
    }

    // invalidate processor and update camera prop widgets, if event has been accepted
    if (e->isAccepted()) {
        cameraProp_->invalidate();
    }
}

void CameraInteractionHandler::shiftEvent(tgt::MouseEvent* e) {
    if (navigationMetaphor_.isSelected("trackball")){
        tgtAssert(cameraProp_, "No camera property");
        tgtAssert(shiftNavi_, "No trackball navigation");

        // assign new camera object to navigation, if it has changed
        if (shiftNavi_->getTrackball()->getCamera() != cameraProp_->get()) {
            shiftNavi_->getTrackball()->setCamera(cameraProp_->get());
        }

        // propagate event to trackball navigation
        if (e->action() == MouseEvent::PRESSED) {
            cameraProp_->toggleInteractionMode(true, this);
            shiftNavi_->mousePressEvent(e);
        }
        else if (e->action() == MouseEvent::RELEASED) {
            cameraProp_->toggleInteractionMode(false, this);
            shiftNavi_->mouseReleaseEvent(e);
        }
        else if (e->action() == MouseEvent::MOTION)
            shiftNavi_->mouseMoveEvent(e);
        else if (e->action() == MouseEvent::DOUBLECLICK)
            shiftNavi_->mouseDoubleClickEvent(e);
        else if (e->action() == MouseEvent::WHEEL)
            shiftNavi_->wheelEvent(e);
    }
    else if (navigationMetaphor_.isSelected("first-person")){
        // Hack: prevents camerainteraction handler from executing shift events, if fpNavi is activated
        e->accept();
    }

    // invalidate processor and update camera prop widgets, if event has been accepted
    if (e->isAccepted()) {
        cameraProp_->invalidate();
    }
}

void CameraInteractionHandler::keyEvent(tgt::KeyEvent* e){
    tgtAssert(cameraProp_, "No camera property");

    tgtAssert(fpNavi_, "No first person Navigation");

    if (fpNavi_->getCamera() != cameraProp_->get())
        fpNavi_->setCamera(cameraProp_->get());

    if (navigationMetaphor_.isSelected("first-person")){
        fpNavi_->keyEvent(e);

        if (e->pressed()){
            if (fpNavi_->isMoving()){
                cameraProp_->toggleInteractionMode(true, this);
                motionTimer_->start(40, 0);
            }
            else
                 motionTimer_->stop();
        }
        else{
            if (!fpNavi_->isMoving()){
                cameraProp_->toggleInteractionMode(pressedMouseButtons_.any(), this);
                    motionTimer_->stop();
            }
        }

        e->accept();
    }
}

void CameraInteractionHandler::onEvent(Event* eve) {
    tgtAssert(cameraProp_, "No camera property");
    tgtAssert(fpNavi_, "No first person Navigation");

    if (fpNavi_->getCamera() != cameraProp_->get())
        fpNavi_->setCamera(cameraProp_->get());

    if (navigationMetaphor_.isSelected("first-person")){
        if (tgt::TimeEvent* timeEve = dynamic_cast<tgt::TimeEvent*>(eve)){
            if (fpNavi_->isMoving()){
                cameraProp_->toggleInteractionMode(true, this);
                fpNavi_->timerEvent(timeEve);

                eve->accept();
            }
        }
    }

    // invalidate processor and update camera prop widgets, if event has been accepted
    if (eve->isAccepted()) {
        cameraProp_->invalidate();
    }
}

void CameraInteractionHandler::setNavigationMode(NavigationMetaphor behaviour){
    navigationMetaphor_.selectByValue(behaviour);
}

CameraInteractionHandler::NavigationMetaphor CameraInteractionHandler::getNavigationMode() const {
    return navigationMetaphor_.getValue();
}

void CameraInteractionHandler::updateFpKeySettings() {
    tgtAssert(fpNavi_, "no first-person navigation");
    tgtAssert(moveForwardEvent_ && moveLeftEvent_ && moveBackwardEvent_ , "missing event property");
    tgtAssert(moveRightEvent_ && moveUpEvent_ && moveDownEvent_ , "missing event property");

    fpNavi_->setKeysetting(moveForwardEvent_->getKeyCode(), moveBackwardEvent_->getKeyCode(),
        moveLeftEvent_->getKeyCode(), moveRightEvent_->getKeyCode(),
        moveUpEvent_->getKeyCode(), moveDownEvent_->getKeyCode());
}

void CameraInteractionHandler::adjustWidgetStates() {
    tgtAssert(rotateEvent_ && zoomEvent_ && shiftEvent_ && wheelZoomEvent_, "missing event property");
    tgtAssert(moveForwardEvent_ && moveLeftEvent_ && moveBackwardEvent_ , "missing event property");
    tgtAssert(moveRightEvent_ && moveUpEvent_ && moveDownEvent_ , "missing event property");

    bool trackballSelected = navigationMetaphor_.isSelected("trackball");
    bool firstPersonSelected = navigationMetaphor_.isSelected("first-person");

    // trackball properties
    rotateEvent_->setVisible(trackballSelected);
    zoomEvent_->setVisible(trackballSelected);
    shiftEvent_->setVisible(trackballSelected);
    wheelZoomEvent_->setVisible(trackballSelected);

    // first-person properties
    moveForwardEvent_->setVisible(firstPersonSelected);
    moveLeftEvent_->setVisible(firstPersonSelected);
    moveBackwardEvent_->setVisible(firstPersonSelected);
    moveRightEvent_->setVisible(firstPersonSelected);
    moveUpEvent_->setVisible(firstPersonSelected);
    moveDownEvent_->setVisible(firstPersonSelected);
}

} // namespace
