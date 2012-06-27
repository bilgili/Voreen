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

#ifndef VRN_VOREENPAINTER_H
#define VRN_VOREENPAINTER_H

#include "tgt/glcanvas.h"
#include "tgt/vector.h"
#include "tgt/navigation/trackball.h"
#include "tgt/painter.h"

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/message.h"
#include "voreen/core/vis/property.h"

#ifdef VRN_WITH_DEVIL
    #include <IL/il.h>
    #include <IL/ilu.h>
    #include <IL/ilut.h>
#endif

#include <list>

namespace voreen {

class VoreenPainter;
class OverlayManager;
class NetworkEvaluator;

/**
 * VoreenPainterOverlay takes care of rendering additional content to a frame, like a FPS-display.
 */
class VoreenPainterOverlay : public MessageReceiver {
public:
    /**
     * Constructor. Initializes the Overlay.
     *
     * @param canvas The canvas that this overlay renders on.
     */
    VoreenPainterOverlay(tgt::GLCanvas* canvas);

    virtual ~VoreenPainterOverlay() {}

    /**
     * Takes care of rendering. This has to be implemented in a subclass.
     */
    virtual void paint() = 0;

    /**
     * Resizes the overlay.
     *
     * @param width The new width of the overlay
     * @param height The new height of the overlay
     */
    virtual void resize(int width, int height) { width_ = width; height_ = height; }

    virtual void processMessage(Message* /*msg*/, const Identifier& /*dest=Message::all_*/) {}

    /**
     * Setter to control whether or not the overlay is active.
     */
    void setIsActivated(bool state) { activated_ = state; }

    /**
     * Getter that returns whether or not the overlay is active.
     */
    bool getIsActivated() { return activated_; }

    /**
     * Setter that controls whether or not the overlay can be switched on/off.
     */
    void setActivateable(bool state) { activateable_ = state; }

    /**
     * Getter that returns whether or not the overlay can be switched on/off.
     */
    bool getActivateable() { return activateable_; }

    /**
     * Used to name this overlay, e.g. for debugging.
     */
    void setName(std::string name) { name_ = name;}

    /**
     * Returns the internal name of this overlay.
     */
    std::string getName() const { return name_; }

    /**
     * Returns the properties for this overlay.
     */
    const Properties& getProperties() const { return props_; }

    /**
     * Add a Property to this overlay.
     *
     * @param prop The property to add.
     */
    void addProperty(Property* prop) { props_.push_back(prop); }

protected:
    tgt::GLCanvas* canvas_; ///< The canvas that this painter renders to.
    int width_;             ///< The width of this painter.
    int height_;            ///< The height of this painter.

    bool activated_;        ///< Is this overlay activated?
    bool activateable_;     ///< Can this overlay be turned on/off?

    std::string name_;      ///< The name of this overlay.

    std::vector<Property*> props_;  ///< The Properties that can be used to control this overlay.
};

//------------------------------------------------------------------------

/**
 *    OverlayManager controls all the overlays of a canvas. Every canvas has an OverlayManager.
 */
class OverlayManager : public MessageReceiver {
public:
    OverlayManager();

    void addOverlay(VoreenPainterOverlay* overlay);
    void removeOverlay(VoreenPainterOverlay* overlay);

    /**
     *  Returns all VoreenPainterOverlays that this OverlayManager is responsible for.
     */
    std::vector<VoreenPainterOverlay*> getOverlays();

    /**
     * Takes care of incoming messages. Accepts the following message-ids:
     *    - pushOverlayDirect:  Adds an Overlay to the manager.
     *      Msg-Type is VoreenPainterOverlay*.
     *    - popOverlayDirect:  Remvoes an Overlay from the manager.
     *      Msg-Type is VoreenPainterOverlay*..
     *
     * @param msg The incoming message.
     * @param dest The destination of the message.
     */
    void processMessage(Message* msg, const Identifier& dest);

    /**
     * Resizes all overlays that this OverlayManager contains.
     *
     * @param width The new width of the overlays
     * @param height The new height of the overlays
     */
    void resize(int width, int height);

    /**
     * Calls the paint method of all overlays.
     */
    void paint();

private:
    std::vector<VoreenPainterOverlay*> overlays_;   ///< The overlays that this OverlayManager manages.
};

//------------------------------------------------------------------------
/**
* Specialized painter for voreen. This class takes care of rendering to a given tgt::GLCanvas,
    using its Processors.
*/
class VoreenPainter : public tgt::Painter, public MessageReceiver {
public:
    // FIXME: probably this should defined (and renamed...and converted to enum) in tgt? (jms)
    static const int VRN_MONOSCOPIC            = 0x61;
    static const int VRN_STEREOSCOPIC          = 0x62;
    static const int VRN_AUTOSTEREOSCOPIC      = 0x62;
    /**
     * Constructor. Initializes the VoreenPainter.
     *
     * @param canvas The canvas that this painter renders on.
     * @param tag The Message-Tag for this painter.
     */
    VoreenPainter(tgt::GLCanvas* canvas, tgt::Trackball* track, const voreen::Identifier& tag = Message::all_);

    virtual ~VoreenPainter();

    OverlayManager* getOverlayMgr() { return overlayMgr_; }

    std::list<VoreenPainterOverlay*>& getOverlays() {
        return overlays_;
    }

    const tgt::ivec2& getSize() {
        return size_;
    }

    /**
     * Add a MouseListener to the canvas. Incoming tgt-Events will be broadcast to the listener.
     *
     * @param listener The listener that is to be added.
     */
    void addMouseListener(tgt::EventListener* listener);

    /**
     * Remove a MouseListener from canvas.
     *
     * @param listener The listener that is removed.
     */
    void removeMouseListener(tgt::EventListener* listener);

    /// removes all registered MouseListener from Eventhandler
    void removeAllMouseListener();

    /**
     * Takes care of incoming messages. Accepts the following message-ids: <br>
     *    - repaint, which repaints the canvas. Msg-type: none <br>
     *    - resize, which resizes this painter. Msg-type: ivec2 <br>
     *    - switch.trackballContinuousSpin, which tells the camera whether or not
     *      its trackball is continously spinning. Msg-type: bool <br>
     *    - set.cameraApplyOrientation, which moves the camera of the associated canvas to
     *      a given orientation using quaternions. <br>
     *      Msg-type: std::vector<\c float> <br>
     *    - set.cameraZoom, which sets the distance between the viewer and the center of the
     *      scene. Msg-type: vec3 <br>
     *    - Identifier::delCanvasOverlay, which deletes a certain overlay from \a overlays_.
     *      Msg-type: VoreenPainterOverlay* <br>
     *    - Identifier::addCanvasOverlay, which adds a an overlay to \a overlays_ . Msg-type: VoreenPainterOverlay*
     *
     * This method also redistributes incoming messages to \a processor_ and \a overlayMgr_.
     *
     * @param msg The incoming message.
     * @param dest The destination of the message.
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

    virtual void invalidateRendering();

    /**
      * Initializes this painter and its processors, if present.
      */
    virtual void initialize();

    void setStereoMode(int stereoMode) {stereoMode_ = stereoMode;}

    /**
     * This method gets the focus of the painter's canvas (getCanvas()) , calls the \a processor_ and actually paints something.
     */
    void paint();

	void setEvaluator(NetworkEvaluator* eval);
	NetworkEvaluator* getEvaluator() const {
		return evaluator_;
	}

    /**
     * Sets the trackball that is used for navigation
     */
    void setTrackball(tgt::Trackball* trackball);

    /**
     * Returns the trackball that is used for navigation. May be a null pointer.
     */
    tgt::Trackball* getTrackball() const;

    /**
     * Renders a snapshot to file. Therefore evaluate from \a evaluator_ is called.
     *
     * @param size size of snapshot
     * @param fileName name of the snapshot file
     */
    void renderToSnapshot(tgt::ivec2 size, std::string fileName);

	static const Identifier removeMouseListener_;
    static const Identifier addMouseListener_;
    static const Identifier addCanvasOverlay_;
    static const Identifier addFrameOverlay_;
    static const Identifier delCanvasOverlay_;
    static const Identifier delFrameOverlay_;
    static const Identifier repaint_;
    static const Identifier resize_;
    static const Identifier visibleViews_;
    static const Identifier cameraChanged_;
    static const Identifier switchCoarseness_;

protected:
	NetworkEvaluator* evaluator_;
    /**
     *  A wrapper for convenience that calls the resize method with a tgt::ivec2 as the parameter.
     */
    void sizeChanged(int x, int y) {
        tgt::ivec2 newsize(x, y);
        sizeChanged(newsize);
    }

    virtual void sizeChanged(const tgt::ivec2& size);

    /**
     *  This method is necessary to initialize the processor of this VoreenPainter and to
     *  deinitialize the processor of the old VoreenPainter. This is necessary when using more
     *  than one canvas.
     */
    void switchCanvas();

    void drawMouseCursor();

     // FIXME progress aaLevel not used at the moment
    /**
     * Renders to a DevIL image.
     *
     * @param size Size of the "devil"-canvas independent of the real canvas.
     * @param progress Used to get a callback, for instance for a progress bar.
     * @param aaLevel Used for anti-aliasing.
     */
#ifdef VRN_WITH_DEVIL
    ILuint renderToILImage(	tgt::ivec2 size,
                            bool (progress)(float _progress),
                            int aaLevel);
#endif

    /**
     * Internally used by
     * \a renderToILImage(tgt::ivec2 size, bool (progress)(float _progress), int aaLevel)
     */
#ifdef VRN_WITH_DEVIL
    ILuint renderToILImage(bool (progress)(float _progress), int aaLevel);
#endif

    tgt::Trackball* trackball_; ///< The trackball that will be used to navigate.  trackball_->getCamera() should be equal to getCanvas()->getCamera()

    typedef std::list<VoreenPainterOverlay*> Overlays;
    Overlays overlays_; ///< The overlays for this painter

    static tgt::GLCanvas* lastCanvas_; ///< The canvas that was previously used. @see switchCanvas()

    OverlayManager* overlayMgr_; ///< The manager that takes care of this painters overlays.

    int stereoMode_; ///< This holds the current view-mode: either monoscopic or stereoscopic view.

    tgt::ivec2 size_; ///< The size of this painter.

    static const std::string loggerCat_;
};

} // namespace

#endif // VRN_VOREENPAINTER_H
