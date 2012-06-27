/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
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

#ifndef tgtSTEREOPAINTER_H
#define tgtSTEREOPAINTER_H

#include "tgt/painter.h"
#include "tgt/modules/tracking/headtracker.h"

namespace tgt {

/**
 * Derived class for stereoscopic painting on a canvas. This class encapsules various
 * stereoscopic rendering methods and allows to use stereo rendering similar to
 * monoscopic rendering.
 *
 * A tgt application is supposed to derive its own StereoPainter.
 * This class can be used like the Painter class (means that the initialize() and
 * sizeChanged() methods from the Painter class have to be overwritten).
 * However, please overwrite the render() method instead of the paint() method
 * (since the latter is only capable of rendering monoscopic content).
 *
 * Some remarks for stereo rendering:
 * MONOSCOPIC:           simple monoscopic rendering
 * FRAME_SEQUENTIAL:     hardware includes eMagin HMDs etc.
 * SIDE_BY_SIDE:         hardware includes projection walls, ProView SR80 HMDs etc.
 *                       Note that the width of the canvas is 2x the width of the
 *                       supposed rendering area!
 * ANAGLYPH:             simple red-blue anaglyph rendering
 *                       Note that you may want to overwrite the color masks for
 *                       different anaglyph glasses in the paint() method!
 * QUADRO_PAGE_FLIPPING: rendering for nVidia Quadro graphics cards with native
 *                       support for certain hardware
 *                       Note that this does not work with nVidia GeForce cards!
 * VERTICAL_INTERLACED:  hardware includes autostereoscopic displays
 *                       Note that the application must not write to the stencil
 *                       buffer for the interlacing process to work!
 *                       Note that this requires that the stencil buffer is available,
 *                       which can be defined using the ToolkitFactory::createCanvas()
 *                       function.
 * Note that due to stereo rendering issues the application must not clear color
 * and depth buffer prior to rendering!
 * See the tgt sample "tracking" for an example, which shows how to use stereo rendering.
 *
 * Since most stereo setups integrate some form of headtracking, a headtracker object
 * can be assigned to the stereo rendering process. This object handles all head-related
 * relative camera transformations
 */
class StereoPainter : public Painter {
public:
	enum StereoMode {
		MONOSCOPIC,
		FRAME_SEQUENTIAL,
		SIDE_BY_SIDE,
		ANAGLYPH,
		QUADRO_PAGE_FLIPPING,
		VERTICAL_INTERLACED,
	};

    StereoPainter(GLCanvas* canvas, StereoMode stereoMode);

    virtual ~StereoPainter() {}

	/// Derived classes should NOT overwrite this method
    virtual void paint();

	/// Overwrite this method for rendering
    virtual void render() = 0;

    void addHeadTracker(HeadTracker* headTracker){ 
        headTracker_ = headTracker; 
        enableHeadTracking(); 
    }
	void enableHeadTracking() { 
        trackingEnabled_ = true; 
    }
	void disableHeadTracking() {
        trackingEnabled_ = false; 
    }
    bool headTrackingEnabled() const{
        return trackingEnabled_;
    }

    StereoMode getStereoMode() const{
        return stereoMode_;
    }

    void setStereoMode(StereoMode mode){
        stereoMode_ = mode;
    }

protected:
	StereoMode stereoMode_;		///< Stereo rendering mode

	HeadTracker* headTracker_;	///< Optional headtracker object
	bool trackingEnabled_;		///< Headtracking is enabled if this flag is set
};

}

#endif //tgtSTEREOPAINTER_H
