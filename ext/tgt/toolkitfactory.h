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

#ifndef TGT_TOOLKITFACTORY_H
#define TGT_TOOLKITFACTORY_H

#include "tgt/config.h"
#include "tgt/glcanvas.h"
#include "tgt/vector.h"

namespace tgt {

class GUIApplication;
class Timer;

class ToolkitFactory {
public:

    static GUIApplication* createApplication(int& argc, char** argv);

    /// create a Canvas object of the according GUI toolkit
    static GLCanvas* createCanvas( const std::string& title = "",
                                   const ivec2& size = ivec2(GLCanvas::DEFAULT_WINDOW_WIDTH,
                                                             GLCanvas::DEFAULT_WINDOW_HEIGHT),
                                   const GLCanvas::Buffers buffers = GLCanvas::RGBADD );

    /// Creates and returns a QtTimer Object.
    /// @param eh   see tgt::QtTimer.
    /// @return tgt::QtTimer
    static Timer* createTimer(EventHandler* eh);
};

} // namespace tgt

#endif // TGT_TOOLKITFACTORY_H
