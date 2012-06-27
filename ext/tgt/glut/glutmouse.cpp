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

#include "tgt/glut/glutmouse.h"

namespace tgt {

GLUTMouse::GLUTMouse(float rate, bool show)
  : Mouse(rate, show)
{
    if (show)
        glutSetCursor( GLUT_CURSOR_INHERIT );
    else
        glutSetCursor( GLUT_CURSOR_NONE );
}

ivec2 GLUTMouse::getPosition() const {
    return position_;
}

void GLUTMouse::setPosition(const ivec2& pos) {
    position_ = pos;
    glutWarpPointer( pos.x, pos.y );
}

void GLUTMouse::hideCursor() {
    glutSetCursor( GLUT_CURSOR_INHERIT );
    showCursor_ = false;
}

void GLUTMouse::showCursor() {
    glutSetCursor( GLUT_CURSOR_NONE );
    showCursor_ = true;
}

}
