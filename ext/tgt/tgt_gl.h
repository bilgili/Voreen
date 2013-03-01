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

#ifndef TGT_GL_H
#define TGT_GL_H

#ifdef __APPLE__
    #include <GL/glew.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glew.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif // __APPLE__

#include "tgt/gpucapabilities.h"
#include "tgt/types.h"

namespace tgt {
    TGT_API GLenum _lGLError(int line, const char* file);
} // namespace tgt

#ifdef TGT_DEBUG
    #define LGL_ERROR tgt::_lGLError(__LINE__, __FILE__)
#else
    #define LGL_ERROR
#endif

#endif  //TGT_GL_H
