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
#include "tgt/navigation/trackball.h"

#include "voreen/core/vis/voreenpainter.h"

namespace voreen {

/**
 * A class that makes it possible to use a trackball-metaphor to rotate a dataset.
 */
class TrackballNavigation : public tgt::EventListener, public MessageReceiver {
public:
    /**
     * The Constructor.
     *
     * @param track the trackball to modifie for navigation
     * @param minDist the minimum allowed orthogonal distance to the center of the trackball
     * @param maxDist the maximum allowed orthogonal distance to the center of the trackball
     */
    TrackballNavigation(tgt::Trackball* track, bool defaultBehavior = true, float minDist = 0.f, float maxDist = 100.f);
    virtual ~TrackballNavigation() {}

    /**
     * React to a press-Event. In this case, remember the current mouse-position, so we can
     * calculate relative movement in mouseMoveEvent.
     *
     * @param e The event to be processed.
     */
    virtual void mousePressEvent(tgt::MouseEvent* e);

    /**
     * React to a release-Event. In this case, reset \a leftPressed_ if necessary and switch
     * coarseness off.
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
     * React to a double-click-Event. In this case, this actually resets the camera's position
     * and orientation
     *
     * @param e The event to be processed.
     */
    virtual void mouseDoubleClickEvent(tgt::MouseEvent* e);

   /**
     * React to a mouse-wheel-Event.  In this case, this actually causes the object to rotate sideways.
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
     *  Takes care of incoming messages.  Accepts the following message-ids:
     *      - switch.trackballSpinning, which switches auto-spinning of the trackball on/off.  Msg-Type: bool
     *      - Identifier::resetTrackball, which resets the trackball's camera to its initial position / orientation
     *      - Identifier::toggleTrackball, which disables / enables the reaction of this navigation completetly
     *
     *   @param msg The incoming message.
     *   @param dest The destination of the message.
     */
    virtual void processMessage(Message* msg, const Identifier& dest);

    /**
     * Returns the internally used trackball.
     */
    tgt::Trackball* getTrackball();

    /**
     * Adds a MessageReceiver to the set of receivers that are notified when the trackball's
     * state changes.
     */
    void addReceiver(MessageReceiver* receiver);

    static const Identifier resetTrackball_;
    static const Identifier toggleTrackball_;

protected:

    /// Resets position and orientation of the trackball's camera to the initial parameters the camera had
    /// when passed to the trackball.
    /// Projective parameters (frustum) are not touched.
    virtual void resetTrackball();

    tgt::Trackball* trackball_;                 ///< The trackball that is modified when navigating

    tgt::MouseEvent::MouseButtons resetButton_; ///< If this button is double-clicked, reset trackball to initial
                                                /// position and orientation

    std::vector<MessageReceiver*> additionalReceivers_;     ///< Additional receivers that are notified on state changes

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

    static const std::string loggerCat_;
};

} // namespace

#endif //VRN_TRACKBALL_NAVIGATION_H
