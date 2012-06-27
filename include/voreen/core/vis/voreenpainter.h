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

#ifndef VRN_VOREENPAINTER_H
#define VRN_VOREENPAINTER_H

#include "tgt/exception.h"
#include "tgt/glcanvas.h"
#include "tgt/vector.h"
#include "tgt/painter.h"

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/properties/property.h"

#include <vector>
#include <list>

namespace voreen {

class NetworkEvaluator;
class CanvasRenderer;

class OverlayManager;
class VoreenPainterOverlay;

/**
 * Specialized painter for voreen. This class takes care of rendering to a given tgt::GLCanvas,
 * using the assigned NetworkEvaluator.
 */
class VoreenPainter : public tgt::Painter {
public:

    // FIXME: probably this should defined (and renamed...and converted to enum) in tgt? (jms)
    static const int VRN_MONOSCOPIC            = 0x61;
    static const int VRN_STEREOSCOPIC          = 0x62;
    static const int VRN_AUTOSTEREOSCOPIC      = 0x62;

    /**
     * Constructor. Initializes the VoreenPainter.
     *
     * @param canvas The canvas that this painter renders on
     * @param evaluator the network evaluator whose process() function if to be called on canvas repaints.
     * @param canvasRenderer the canvas renderer associated with the canvas
     */
    VoreenPainter(tgt::GLCanvas* canvas, NetworkEvaluator* evaluator, CanvasRenderer* canvasRenderer);

    virtual ~VoreenPainter();

    virtual void initialize();

    void repaint();

    virtual void sizeChanged(const tgt::ivec2& size);

    /**
     * This method gets the focus of the painter's canvas (getCanvas()) , calls the \a processor_ and actually paints something.
     */
    void paint();

    /**
     * Returns the associated NetworkEvaluator.
     */
    NetworkEvaluator* getEvaluator() const;

    /**
     * Returns the associated CanvasRenderer.
     */
    CanvasRenderer* getCanvasRenderer() const;

    /**
     * Renders a snapshot to a file.
     *
     * The snapshot generation is delegated to the associated CanvasRenderer.
     *
     * @note requires Voreen to be compiled with DevIL support
     *
     * @param fileName name of the snapshot file
     * @param size size of the snapshot
     */
    void renderToSnapshot(const std::string& fileName, const tgt::ivec2& size)
        throw (std::bad_alloc, tgt::FileException);

    /**
     * Currently not functional.
     */
    void setStereoMode(int stereoMode);

    // overlays (deprecated) ---------------------------------------
    OverlayManager* getOverlayMgr() { return overlayMgr_; }

    std::list<VoreenPainterOverlay*>& getOverlays() {
        return overlays_;
    }

    void addCanvasOverlay(VoreenPainterOverlay* overlay);
    void delCanvasOverlay(VoreenPainterOverlay* overlay);
    // end of overlays ---------------------------------------------


protected:

    NetworkEvaluator* evaluator_;

    CanvasRenderer* canvasRenderer_;

    int stereoMode_; ///< The current view-mode: either monoscopic or stereoscopic view.

    static const std::string loggerCat_;

    // overlays (deprecated) ---------------------------------------
    typedef std::list<VoreenPainterOverlay*> Overlays;
    Overlays overlays_; ///< The overlays for this painter
    OverlayManager* overlayMgr_; ///< The manager that takes care of this painters overlays.
    // end of overlays ---------------------------------------------

};


//------------------------------------------------------------------------

/**
 * VoreenPainterOverlay takes care of rendering additional content to a frame, like a FPS-display.
 *
 * @deprecated: Use a processor instead
 */
class VoreenPainterOverlay {
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
 * OverlayManager controls all the overlays of a canvas. Every canvas has an OverlayManager.
 *
 * @deprecated: overlays are deprecated. Use a processor instead.
 */
class OverlayManager {
public:
    OverlayManager();

    void addOverlay(VoreenPainterOverlay* overlay);
    void removeOverlay(VoreenPainterOverlay* overlay);

    /**
     *  Returns all VoreenPainterOverlays that this OverlayManager is responsible for.
     */
    std::vector<VoreenPainterOverlay*> getOverlays();

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


} // namespace voreen

#endif // VRN_VOREENPAINTER_H
