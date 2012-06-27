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

#include <list>

#include "tgt/glut/glutapplication.h"
#include "tgt/glut/tgt_glut.h"
#include "tgt/glut/glutcanvas.h"

namespace tgt {

GLUTApplication::GLUTApplication(int& argc, char** argv)
  : GUIApplication(argc, argv)
{}


GLUTApplication::~GLUTApplication(){
    deinitGL();
    deinit();
}

int GLUTApplication::init(InitFeature::Features featureset) {
    tgt::init(featureset);
    glutInit(argc_, argv_);
    initCanvases();
    tgt::initGL(featureset);
    return 0;
}

int GLUTApplication::run() {
    if (hasRun) {
        std::cout << "Failure: GLUTApplication::run() should be called only once.\n";
        return -1;
    }
    hasRun = true;
    glutMainLoop();
    return 0;
}

void GLUTApplication::quit(int status) {
    exit(status); // FIXME: exiting a glut application is not done correct. no good concept how to use destructors, atexit, canvas-application, and so on. glut will never return form mainloop. one has to use atexit, which differs from usage of other toolkits in factory. freeglut offers the ability to return from mainloop. perhaps we should limit tgt to use freeglut only, no glut...
}

}
