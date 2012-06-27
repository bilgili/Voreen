/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef TGT_PAINTER_H
#define TGT_PAINTER_H

#include "tgt/types.h"
#include "tgt/vector.h"

namespace tgt {

class GLCanvas;
class Camera;

/**
 * Base class for all rendering on a canvas. Every canvas has a painter and use its
 * render method for rendering content.
 *
 * A tgt application is supposed to derive it's own Painter(s) and override paint, initialize
 * and sizeChanged method
 */
class TGT_API Painter {
public:
    /// Constructor
    Painter(GLCanvas* canvas = 0);

    /**
     * The owned GLCanvas should be destroyed manually otherwise endless
     * recursions can occur.
     */
    virtual ~Painter() {}

    /**
     * This is meant be overridden to do the according openGL paintings
     * is not meant to be called directly, will be called by GLCanvas object
     */
    virtual void paint() = 0;

    /// This is meant be overridden to adjust camera settings to new canvas dimensions
    virtual void sizeChanged(const ivec2&) {};

    /// This is meant be overridden to prepare OpenGl context for painting (e.g. set background color).
    /// If the user's derived painter class has additional attributes, they may be initialized here as well.
    virtual void init() {};

    /// Set the Canvas on which painter will draw
    void setCanvas(GLCanvas* canvas);

    /// Get the associated Canvas
    GLCanvas* getCanvas() const;

    /// A wrapper to get the camera from the Canvas
    Camera* getCamera() const;

private:
    GLCanvas* canvas_;
};

} // namespace tgt

#endif  //TGT_PAINTER_H
