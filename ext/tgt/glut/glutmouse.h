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

#ifndef TGT_GLUTMOUSE_H
#define TGT_GLUTMOUSE_H

#include "tgt/mouse.h"
#include <GL/glut.h>


/**
    This class implements the abstract class Mouse by using GLUT-Functions.
*/
namespace tgt {

class GLUTMouse : public Mouse
{
protected:
    ivec2 position_;

public:

    /// Constructor / Destructor
    GLUTMouse(float rate = 5.f, bool show = true);
    ~GLUTMouse() {};

    /// These methods implement the abstract functions of super-class Mouse
    virtual ivec2 getPosition() const;
    virtual void setPosition(const ivec2& pos);
    virtual void hideCursor();
    virtual void showCursor();
};

}

#endif //tgt_GLUTMOUSE_H
