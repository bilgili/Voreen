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

#ifndef VRN_FIRSTPERSONNAVIGATION_H
#define VRN_FIRSTPERSONNAVIGATION_H

#include "tgt/event/eventlistener.h"

namespace tgt{
    class Camera;
    class EventHandler;
    class KeyEvent;
    class MouseEvent;
    class Stopwatch;
    class Timer;
}

namespace voreen{
/**
 * A class that makes it possible to navigate freely through datasets like in first-person games.
 */
class FirstPersonNavigation : public tgt::EventListener {
public:
     /**
     * The Constructor.
     *
     * @param camera the camera that is to be modified by the navigation
     */
    FirstPersonNavigation(tgt::Camera* camera);
    ~FirstPersonNavigation();

    /**
     * React to a press-Event. This stores the current mouseposition in FirstPersonNavigation::lastMousePosition_.
     *
     * @param e The event to be processed.
     */
    virtual void mousePressEvent(tgt::MouseEvent* e);

    /**
     * React to a move-Event. This rotates the camera around its axis depending on the pressed mousebutton.
     *
     * @param e The event to be processed.
     */
    virtual void mouseMoveEvent(tgt::MouseEvent* e);

    /**
     * React to a timer-Event. This is needed for refreshing current camera in FirPersonNavigation::camera_ every
     * 40 ms (25 fps), if a move-key was pressed before.
     *
     * @param e The event to be processed.
     */
    virtual void timerEvent(tgt::TimeEvent* e);

    /**
     * React to a key-Event. This moves the current camera in FirstPersonNavigation::camera_ initialy.
     *
     * @param e The event to be processed.
     * @see move
     */
    virtual void keyEvent(tgt::KeyEvent* e);

    tgt::Camera* getCamera() const;
    void setCamera(tgt::Camera* camera);

    void initialize();

    /** Load some established standard settings **/
    void loadDefaultKeysetting();
    void loadDefaultMousesetting();
    void loadDefaultCamerasetting();

    void setKeysetting(tgt::KeyEvent::KeyCode keyMoveForward,
                       tgt::KeyEvent::KeyCode keyMoveBackward,
                       tgt::KeyEvent::KeyCode keyMoveLeft,
                       tgt::KeyEvent::KeyCode keyMoveRight,
                       tgt::KeyEvent::KeyCode keyMoveUp,
                       tgt::KeyEvent::KeyCode keyMoveDown);

    void setMousesetting(tgt::MouseEvent::MouseButtons buttonRotateUpAndStrafe,
                         tgt::MouseEvent::MouseButtons buttonRotateLook);

    /**
     * @param minMouseSteps Specifies the minimal count of pixels (along one axis) at which the FirstPersonNavigation::mouseMoveEvent(tgt::MouseEvent* e)
     * shall react and rotate the camera.
     * @param maxMouseSteps Specifies the maximal count of pixels (along one axis) at which the FirstPersonNavigation::mouseMoveEvent(tgt::MouseEvent* e)
     * shall react and rotate the camera.
     * @param minRotationAngle Specifies the rotationangle the camera is rotated with, if the mousemovement was minMouseSteps along one
     * axis
     * @param maxRotationAngle Specifies the rotationangle the camera is rotated with, if the mousemovement was maxMouseSteps along one
     * axis
     * @param minAccuteness Specifies the minimal stepsize for one movement
     * @param maxAccuteness Specifies the maximal stepsize for one movement
     */
    void setCameraSetting(int minMouseSteps,
                          int maxMouseSteps,
                          float minRotationAngle,
                          float maxRotationAngle,
                          float minAccuteness,
                          float maxAccutness);

    /**
     * This moves the camera. Beside this an internal stopwatch is started to calculate the elapsed time until
     * the next call of FirstPersonNavigation::move(). This is needed for smooth camera movements.
     */
    void move();

    /**
     * @return Specifies, if the camera is still moving, i.e. if a moving key was pressed, but not released yet.
     */
    bool isMoving();

protected:
    void move(float length, tgt::vec3 direction);

    /**
     * Methods for rotating the camera around its axis.
     */
    void rollCameraAroundUp(float angle);
    void rollCameraAroundStrafe(float angle);
    void rollCameraAroundLook(float angle);

    /**
     * @return Rotationangle for the given mousemovement along one axis. For movement lengths between FirstPersonNavigation::minMouseSteps_
     * and FirstPersonNavigation::maxMouseSteps_ the angle is interpolated linear between FirstPersonNavigation::minRotationAngle_ and
     * FirstPersonNavigation::maxRotationAngle_.
     */
    float computeRotationAngle(int oldMouse, int newMouse);

    tgt::Camera* camera_;
    tgt::Stopwatch* stopwatch_;

    tgt::vec3 movingDirection_;
    tgt::ivec2 lastMousePosition_;

    /**
     * Stores, if the next call of FirstPersonNavigation::move() is an initial move. If not, it is necessary to calculate the stepsize
     * through the elapsed time since last call of FirstPersonNavigation::move().
     */
    bool firstMove_;

    int minMouseSteps_, maxMouseSteps_;
    float minRotationAngle_, maxRotationAngle_;
    float minAccuteness_, maxAccuteness_, curAccuteness_;

    tgt::KeyEvent::KeyCode keyMoveForward_;
    tgt::KeyEvent::KeyCode keyMoveBackward_;
    tgt::KeyEvent::KeyCode keyMoveLeft_;
    tgt::KeyEvent::KeyCode keyMoveRight_;
    tgt::KeyEvent::KeyCode keyMoveUp_;
    tgt::KeyEvent::KeyCode keyMoveDown_;

    tgt::MouseEvent::MouseButtons buttonRotateUpAndStrafe_;
    tgt::MouseEvent::MouseButtons buttonRotateLook_;
};
}

#endif // VRN_FIRSTPERSONNAVIGATION_H
