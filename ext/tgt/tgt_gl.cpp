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

#include "tgt/tgt_gl.h"

#include "tgt/logmanager.h"

namespace tgt {

GLenum _lGLError(int line, const char* file) {
    GLenum err = glGetError();

    if (err != GL_NO_ERROR) {
        const GLubyte* exp = gluErrorString(err);

        std::ostringstream tmp1, tmp2, loggerCat;
        tmp2 << " File: " << file << "@" << line;
        tmp1 << (exp ? (const char*) exp : "unknown");
        loggerCat << "gl-error:" << file << ':' << line;

        LogMgr.log(loggerCat.str(), tgt::Error, tmp1.str(), tmp2.str());
    }

    return err;
}

} // namespace tgt
