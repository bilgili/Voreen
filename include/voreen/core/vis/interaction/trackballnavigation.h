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

#ifndef VRN_TRACKBALL_NAVIGATION_H
#define VRN_TRACKBALL_NAVIGATION_H

#include "tgt/event/eventlistener.h"
#include "tgt/vector.h"
#include "tgt/event/mouseevent.h"
#include "tgt/glcanvas.h"
#include "voreen/core/vis/interaction/voreentrackball.h"

#include "voreen/core/vis/voreenpainter.h"

namespace tgt {
class Camera;
}

namespace voreen {

/**
 * A class that makes it possible to use a trackball-metaphor to rotate a dataset.
 */
class TrackballNavigation : public tgt::EventListener {
public:
    /**
     * The Constructor.
     *
     * @param camera the camera that is to be modified by the navigation
     * @param minDist the minimum allowed orthogonal distance to the center of the trackball
     * @param maxDist the maximum allowed orthogonal distance to the center of the trackball
     */
    TrackballNavigation(tgt::Camera* camera, float minDist = 0.01f, float maxDist = 50.f);
    virtual ~TrackballNavigation();

    /**
     * React to a press-Event.
     *
     * @param e The event to be processed.
     */
    virtual void mousePressEvent(tgt::MouseEvent* e);

    /**
     * React to a release event.
     *
     * @param e The event to be processed.
     */
    virtual void mouseReleaseEvent(tgt::MouseEvent* e);

    /**
     * React to a move-Event.  In this case, this actually causes the object to rotate.
     *
     * @param e The event to be processed.
     */
    virtual void mouseMoveEvent(tgt::MouseEvent* e);

    /**
     * React to a double-click-Event.
     *
     * @param e The event to be processed.
     */
    virtual void mouseDoubleClickEvent(tgt::MouseEvent* e);

   /**
     * React to a mouse-wheel-Event.
     *
     * @param e The event to be processed.
     */
    virtual void wheelEvent(tgt::MouseEvent* e);

   /**
     * React to a time-Event. This does a number of things:
     *
     *  - If necessary auto-spin the trackball.
     *  - If the mouse-wheel was used, switch coarseness off after a certain amount of time.
     *  - If auto-spinning of the trackball is active, don't spin the trackball if the user has
     *    waited too long after moving the mouse.
     *
     * @param e The event to be processed.
     */
    virtual void timerEvent(tgt::TimeEvent* e);

    /**
     * React to key event.
     */
    virtual void keyEvent(tgt::KeyEvent* e);

    /**
     * Returns the internally used trackball.
     */
    VoreenTrackball* getTrackball();

    tgt::MouseEvent::MouseButtons getRotateButton() const { return mouseRotateButton_; }
    tgt::MouseEvent::MouseButtons getZoomButton() const { return mouseZoomButton_; }
    tgt::MouseEvent::MouseButtons getMoveButton() const { return mouseMoveButton_; }
    tgt::MouseEvent::MouseButtons getRollButton() const { return mouseRollButton_; }

    /**
     *  Let trackball react on mouse movements when button is pressed and modifiers are suitable
     *  to mod with rotating.
     *  Use MouseEvent::NO_MOUSE_BUTTON as button to disable mouse rotation.
     */
    void setMouseRotate(tgt::MouseEvent::MouseButtons button = tgt::MouseEvent::MOUSE_BUTTON_LEFT,
                        int mod = tgt::Event::NONE);
    /**
     *  Let trackball react on key pressures with rotating
     *  @param acuteness The per-keypress angle of rotation will be smaller at greater acuteness. Use acuteness = 0.f to disable key rotation.
     *  @param left, right, up, down keycode which should cause suitable rotation
     *  @param mod which modifiers must be set
     *  @param pressed rotate on key-pressed-event when true, on key-release-event when false
     *
     */
    void setKeyRotate(float acuteness = 10.f,
                      tgt::KeyEvent::KeyCode left  = tgt::KeyEvent::K_LEFT,
                      tgt::KeyEvent::KeyCode right = tgt::KeyEvent::K_RIGHT,
                      tgt::KeyEvent::KeyCode up    = tgt::KeyEvent::K_UP,
                      tgt::KeyEvent::KeyCode down  = tgt::KeyEvent::K_DOWN,
                      int mod = tgt::Event::NONE,
                      bool pressed = false);


    /**
     * Let trackball react on mouse movements when button is pressed and modifiers are suitable
     * to mod with moving
     *
     * Use MouseEvent::NO_MOUSE_BUTTON as button to disable mouse movement
     */
    void setMouseMove(tgt::MouseEvent::MouseButtons button = tgt::MouseEvent::MOUSE_BUTTON_LEFT,
                      int mod = tgt::Event::LSHIFT);

    /**
     * Let trackball react on key presses with moving
     * @param acuteness The per-keypress length of movement will be smaller at greater
     *        acuteness. Use acuteness = 0.f to disable key rotation.
     * @param left, right, up, down keycode which should cause suitable movement
     * @param mod which modifiers must be set
     * @param pressed move on key-pressed-event when true, on key-release-event when false
     */
    void setKeyMove(float acuteness = 100.f,
                    tgt::KeyEvent::KeyCode left  = tgt::KeyEvent::K_LEFT,
                    tgt::KeyEvent::KeyCode right = tgt::KeyEvent::K_RIGHT,
                    tgt::KeyEvent::KeyCode up    = tgt::KeyEvent::K_UP,
                    tgt::KeyEvent::KeyCode down  = tgt::KeyEvent::K_DOWN,
                    int mod = tgt::Event::SHIFT,
                    bool pressed = false);


    /**
     * Let trackball react on mouse movements when button is pressed and modifiers are suitable
     * to mod with zooming (changing the trackball's size)
     *
     * Use MouseEvent::NO_MOUSE_BUTTON as button to disable mouse zooming
     *
     * @param zoomInDirection specifies in which direction mouse must be moved to zoom in.
     *        The greater the length of this vector, the longer the way the mouse must be moved
     *        to achieve a certain zoom factor.
     */
    void setMouseZoom(tgt::MouseEvent::MouseButtons button = tgt::MouseEvent::MOUSE_BUTTON_LEFT,
                      tgt::vec2 zoomInDirection = tgt::vec2( 0.f, 1.f), int mod = tgt::Event::CTRL);

    /**
     * Let trackball react on mouse wheel events when modifiers are suitable to mod with
     * zooming (changing the trackball's size)
     * @param acuteness The zoom factor will be smaller at greater acuteness. Use acuteness = 0.f to disable key rotation.
     * @param wheelUpZoomIn zoom in on wheel up and out on wheel down when true, otherwise when false
     */
    void setMouseWheelZoom(float acuteness = 10.f, bool wheelUpZoomIn = true, int mod = tgt::Event::NONE);

    /// @param acuteness The zoom factor will be smaller at greater acuteness. Use acuteness = 0.f to disable key rotation.
    void setKeyZoom(float acuteness = 10.f,
                    tgt::KeyEvent::KeyCode in  = tgt::KeyEvent::K_UP,
                    tgt::KeyEvent::KeyCode out = tgt::KeyEvent::K_DOWN,
                    int mod = tgt::Event::CTRL,
                    bool pressed = false);

    /**
     * Let trackball react on mouse movements when button is pressed and modifiers are suitable
     * to mod with rolling (tiling camera to left or right)
     *
     * Use MouseEvent::NO_MOUSE_BUTTON as button to disable mouse rolling
     * @param acuteness greater the acuteness means less tiling
     */
    void setMouseRoll(tgt::MouseEvent::MouseButtons button = tgt::MouseEvent::MOUSE_BUTTON_LEFT,
                      float acuteness = .5f,
                      int mod = tgt::Event::CTRL);

    /**
     * Let trackball react on mouse wheel events when modifiers are suitable to mod with
     * rolling  (tiling camera to left or right)
     * @param acuteness Less tiling at greater acuteness. Use acuteness = 0.f to disable key rotation.
     * @param wheelUpRollLeft roll left on wheel up and right on wheel down when true, otherwise when false
     */
    void setMouseWheelRoll(float acuteness = 10.f, bool wheelUpRollLeft = true, int mod = tgt::Event::SHIFT);

    /// @param acuteness Less tiling at greater acuteness. Use acuteness = 0.f to disable key rotation.
    void setKeyRoll(float acuteness = 10.f,
                    tgt::KeyEvent::KeyCode left = tgt::KeyEvent::K_LEFT,
                    tgt::KeyEvent::KeyCode right = tgt::KeyEvent::K_RIGHT,
                    int mod = tgt::Event::ALT,
                    bool pressed = false);

    static const std::string resetTrackball_;
    static const std::string toggleTrackball_;

protected:

    void startMouseDrag(tgt::MouseEvent* eve);
    void endMouseDrag(tgt::MouseEvent* eve);

    /// The following functions may be used to rotate the Up-Vector about
    /// the Strafe- and the Look-Vector.  Use this with care since it may
    /// leave the Camera with a "strange" orientation.
    void rollCameraVert(float angle);
    void rollCameraHorz(float angle);

    void initializeEventHandling();

    /// scale screen-coodinates of mouse to intervall [-1, 1]x[-1, 1]
    tgt::vec2 scaleMouse(const tgt::ivec2& coords, const tgt::ivec2& viewport) const;

    float getRotationAngle(float acuteness) const;
    float getMovementLength(float acuteness) const;
    float getZoomFactor(float acuteness, bool zoomIn) const;
    float getRollAngle(float acuteness, bool left) const;

    /// Resets position and orientation of the trackball's camera to the initial parameters the camera had
    /// when passed to the trackball.
    /// Projective parameters (frustum) are not touched.
    virtual void resetTrackball();

    VoreenTrackball* trackball_;                 ///< The trackball that is modified when navigating

    /// Last mouse coordinates to allow calculation of the relative change.
    /// Ought to be relative coordinates within range [-1, 1]x[-1, 1].
    tgt::vec2 lastMousePosition_;

    float minDistance_;     ///< minimal allowed orthogonal distance to center of trackball
    float maxDistance_;     ///< maximal allowed orthogonal distance to center of trackball

    int wheelCounter_; ///< Counts how many time-ticks have passed since the mouse-wheel was used.
    int spinCounter_;  ///< Counts how many time-ticks have passed since the trackball was spun.
    int moveCounter_;  ///< Counts how many time-ticks have passed since the user has moved the mouse.

    int wheelID_;
    int spinID_;
    int moveID_;
    int overlayTimerID_;

    bool spinit_; ///< Helper member to control auto-spinning of trackball.
    bool trackballEnabled_; ///< Is the trackball enabled?

    bool tracking_; ///< Are we tracking mouse move events? Only when we received a mousePressEvent before.

    static const std::string loggerCat_;


    /// used to store settings how to react on certain events
    tgt::MouseEvent::MouseButtons mouseRotateButton_ ; ///< at which mouse button to rotate
    int mouseRotateMod_                         ; ///< at which modifiers to rotate by mouse
    float keyRotateAcuteness_                   ; ///< acuteness of rotation steps of key presses
    tgt::KeyEvent::KeyCode keyRotateLeft_       ; ///< at which key code rotate left
    tgt::KeyEvent::KeyCode keyRotateRight_      ; ///< at which key code rotate right
    tgt::KeyEvent::KeyCode keyRotateUp_         ; ///< at which key code rotate up
    tgt::KeyEvent::KeyCode keyRotateDown_       ; ///< at which key code rotate down
    int keyRotateMod_                           ; ///< at which modifiers to rotate by keys
    bool keyRotatePressed_                      ;
    tgt::MouseEvent::MouseButtons mouseMoveButton_;
    int mouseMoveMod_                           ;
    float keyMoveAcuteness_                     ;
    tgt::KeyEvent::KeyCode keyMoveLeft_         ;
    tgt::KeyEvent::KeyCode keyMoveRight_        ;
    tgt::KeyEvent::KeyCode keyMoveUp_           ;
    tgt::KeyEvent::KeyCode keyMoveDown_         ;
    int keyMoveMod_                             ;
    bool keyMovePressed_                        ;
    tgt::MouseEvent::MouseButtons mouseZoomButton_;
    tgt::vec2 mouseZoomInDirection_             ;
    int mouseZoomMod_                           ;
    bool mouseWheelZoom_                        ;
    float mouseWheelZoomAcuteness_              ;
    bool mouseWheelUpZoomIn_                    ;
    int mouseWheelZoomMod_                      ;
    float keyZoomAcuteness_                     ;
    tgt::KeyEvent::KeyCode keyZoomIn_           ;
    tgt::KeyEvent::KeyCode keyZoomOut_          ;
    int keyZoomMod_                             ;
    bool keyZoomPressed_                        ;
    tgt::MouseEvent::MouseButtons mouseRollButton_;
    float mouseRollAcuteness_                   ;
    int mouseRollMod_                           ;
    bool mouseWheelRoll_                        ;
    float mouseWheelRollAcuteness_              ;
    bool mouseWheelUpRollLeft_                  ;
    int mouseWheelRollMod_                      ;
    float keyRollAcuteness_                     ;
    tgt::KeyEvent::KeyCode keyRollLeft_         ;
    tgt::KeyEvent::KeyCode keyRollRight_        ;
    int keyRollMod_                             ;
    bool keyRollPressed_                        ;
    tgt::MouseEvent::MouseButtons resetButton_  ; ///< If this button is double-clicked, reset trackball to initial
                                                  /// position and orientation

};

} // namespace

#endif //VRN_TRACKBALL_NAVIGATION_H
