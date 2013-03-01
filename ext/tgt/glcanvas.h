/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2005-2013 Visualization and Computer Graphics Group, *
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

#ifndef TGT_GLCANVAS_H
#define TGT_GLCANVAS_H

#include "tgt/tgt_gl.h"

#include "tgt/types.h"
#include "tgt/vector.h"
#include "tgt/event/event.h"
#include "tgt/event/timeevent.h"
#include "tgt/event/mouseevent.h"
#include "tgt/event/keyevent.h"
#include "tgt/event/eventhandler.h"

namespace tgt {

class Camera;
class Painter;

/**
 * This class is the base class for all tgt-Canvases. It provides the functionality of using
 * Painter-objects to actually render things to the canvas. The methods one has to implement in a
 * subclass are, most importantly, those taking care of turning incoming events into tgt-events.
 * This makes sure that all kinds of APIs can be used with tgt without specializing
 * a lot of code.
 */
class TGT_API GLCanvas {
public:
    enum {
        DEFAULT_WINDOW_WIDTH  = 512,
        DEFAULT_WINDOW_HEIGHT = 512
    };

    enum Buffers {
        RGB_BUFFER          = 1 << 0,
        ALPHA_BUFFER        = 1 << 1,
        DEPTH_BUFFER        = 1 << 2,
        DOUBLE_BUFFER       = 1 << 3,
        STENCIL_BUFFER      = 1 << 4,
        ACCUM_BUFFER        = 1 << 5,
        STEREO_VIEWING      = 1 << 6,
        MULTISAMPLING       = 1 << 7,
        RGBA_BUFFER         = RGB_BUFFER | ALPHA_BUFFER,

        // frequently used settings
        RGBD    = RGB_BUFFER | DEPTH_BUFFER,
        RGBDS   = RGB_BUFFER | DEPTH_BUFFER | STENCIL_BUFFER,
        RGBDD   = RGB_BUFFER | DEPTH_BUFFER | DOUBLE_BUFFER,
        RGBDDS  = RGB_BUFFER | DEPTH_BUFFER | DOUBLE_BUFFER | STENCIL_BUFFER,
        RGBA    = RGB_BUFFER | ALPHA_BUFFER,
        RGBAD   = RGB_BUFFER | ALPHA_BUFFER | DEPTH_BUFFER,
        RGBADS  = RGB_BUFFER | ALPHA_BUFFER | DEPTH_BUFFER | STENCIL_BUFFER,
        RGBADD  = RGB_BUFFER | ALPHA_BUFFER | DEPTH_BUFFER | DOUBLE_BUFFER,
        RGBADDS = RGB_BUFFER | ALPHA_BUFFER | DEPTH_BUFFER | DOUBLE_BUFFER | STENCIL_BUFFER
    };

    /// A Constructor
    /// @param title     window title if canvas is standalone window
    /// @param size      size of canvas in pixels
    /// @param buffers   which buffer setting to use, default is to use double-buffered
    ///                  RGB buffer with alpha and depth buffer
    ///                  @see GLCanvas::Buffers
    GLCanvas(const std::string& title = "",
             const ivec2& size = ivec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT),
             const Buffers buffers = RGBADD );

    /**
     * The GLCanvas and the Painter which owns the GLCanvas should be destroyed
     * manually otherwise endless recursions can occur.
     */
    virtual ~GLCanvas();

    /// This is the method that swaps the front- and backbuffer. To be overridden by derived
    /// Canvas subclasses.
    virtual void swap() = 0;

    /**
     * If we use several GLCanvases, we have to switch the OpenGL context when we switch between
     * canvases; this method sets the context of this canvas as the current one.
     *
     * To be overridden by derived Canvas classes.
     */
    virtual void getGLFocus() = 0;

    /**
     * Toggles fullscreen mode.
     *
     * To be overridden by derived Canvas classes.
     */
    virtual void toggleFullScreen() = 0;

    /**
     * Should be called when the canvas was resized.
     * To be called by the according toolkit (Qt, GLUT, SDL, ...).
     */
    void sizeChanged(const ivec2& size);

    /// to be called by application to cause (re)painting on the canvas
    virtual void repaint() = 0;

    /// to be called by application to cause the canvas to repaint when idle next time
    virtual void update() = 0;

    /// initialize the canvas -- e.g. create window, set it's size and title
    virtual void init();

    /// Set the painter the Canvas will use to draw it's content
    /// @param p the Painter
    /// @initPainter wheather to initialize the painter, @see GLCanvas::initPainter()
    void setPainter(Painter* p, bool initPainter = true);

    Painter* getPainter() const;

    /// Calls init()- and sizeChanged()-method of the canvas' painter. These methods shall
    /// initializes OpenGL context (and maybe some user dependant stuff of the painter) and
    /// make the painter adapt to the current width and height of the canvas.
    /// @see Painter::init()
    /// @see Painter::sizeChanged()
    void initPainter();

    EventHandler* getEventHandler() const;
    void setEventHandler(EventHandler* handler);

    /// make the canvas call glFlush/swap automatically or not
    void setAutoFlush(bool a) { autoFlush_ = a; };

    /// wheater canvas automatically calls glFlush or swap
    bool getAutoFlush();

    ///Take a screenshot and save it as TGA file.
    ///@param fname Target filename
    bool takeScreenshot(std::string fname);

    /// Getter
    const ivec4& getRgbaSize() const;

    /// Getter
    int getDepthSize() const;

    /// Getter
    int getStencilSize() const;

    /// Getter
    ivec2 getSize() const;

    /// Getter
    int getWidth() const;

    /// Getter
    int getHeight() const;

    /// Getter
    Buffers getBuffers() const;

    /*
        FIXME It is stupid to have the camera included in the GLCanvas
        or the Painter. If you need the canvas in a Navigation or a class which
        inherits from Painter, GLCanvas or what not include it there! (rl)
    */
    /// Setter
    void setCamera(Camera* cam);

    /// Getter
    Camera* getCamera() const;

    /// Getter
    bool isDoubleBuffered() const { return doubleBuffered_; }

    /// Getter
    bool isInitialized() const { return initialized_; }

protected:

    /// Use the painter_ to actually paint something on the canvas
    /// For internal use, not to be called by application directly. Call repaint instead.
    virtual void paint();

    std::string title_; /// window title if canvas is represented by a window
    ivec2   size_;    /// the size of the canvas
    Buffers buffers_; /// the kind of buffers that will be used

    ivec4   rgbaSize_;
    int     depthSize_;
    int     stencilSize_;
    bool    doubleBuffered_;
    bool    stereoViewing_;
    bool    fullscreen_;
    bool    autoFlush_; ///< whether to call glFlush or swap automatically
    bool    initialized_; // FIXME: does this make sense?

    Camera* camera_; ///< the camera is used to look around

    Painter* painter_;  ///< the painter that will be used for rendering

    EventHandler* eventHandler_;  ///< the eventHandler that will distribute incoming events to
                                  ///< its eventListeners
};

}

#endif // TGT_GLCANVAS_H
