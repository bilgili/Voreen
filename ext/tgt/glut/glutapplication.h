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

#ifndef TGT_GLUTAPPLICATION_H
#define TGT_GLUTAPPLICATION_H

#include "tgt/guiapplication.h"
#include "tgt/glut/glutcanvas.h"
#include "tgt/glut/gluttimer.h"

#include <GL/glut.h>

namespace tgt {

class GLUTApplication : public GUIApplication {
public:
    /// Constructor
    /// @param argc number of command line arguments
    /// @param vector of command line arguments
    GLUTApplication(int& argc, char** argv);

    /**
     * Destructor.
     *
     * Deletes all GLCanvases and deinits tgt. This will never be called in a normal way
     * because of GLUT's restrictions
     * @see onExitFunction
     */
    virtual ~GLUTApplication();

    /// initialize tgt and glut, create a window for every GLUTCanvas and stuff ...
    virtual int init(InitFeature::Features featureset = InitFeature::ALL);

    /// enter glut mainloop (glutMainLoop)
    virtual int run();

    /// tells glut to quit mainloop and exit program
    virtual void quit(int status = 0);

}; // class GLUTApplication

} // namespace tgt

#endif // TGT_GLUTAPPLICATION_H
