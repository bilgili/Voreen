/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/glut.h>

#include "tgt/shadermanager.h"
#include "tgt/glut/glutcanvas.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/utils/voreenpainter.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/network/workspace.h"
#include "voreen/core/network/processornetwork.h"
#include "voreen/core/processors/canvasrenderer.h"

using namespace voreen;

//---------------------------------------------------------------------------

tgt::GLUTCanvas* canvas = 0;
VoreenApplication* app = 0;

NetworkEvaluator* networkEvaluator = 0;
ProcessorNetwork* network = 0;
VoreenPainter* painter = 0;

//---------------------------------------------------------------------------

void initialize() {
    Workspace* workspace = new Workspace();
    workspace->load(VoreenApplication::app()->getWorkspacePath("/standard.vws"));

    // initialize the network evaluator
    networkEvaluator = new NetworkEvaluator();
    network = workspace->getProcessorNetwork();
    std::vector<CanvasRenderer*> canvasRenderer = network->getProcessorsByType<CanvasRenderer>();

    if (canvasRenderer.size() > 0) {
        // init painter and connect it to the canvas
        painter = new VoreenPainter(canvas, networkEvaluator, canvasRenderer[0]);
        canvas->setPainter(painter);
        canvasRenderer[0]->setCanvas(canvas);
        // give the network to the network evaluator
        networkEvaluator->setProcessorNetwork(network);
    }
}

void finalize() {
    delete painter;
    painter = 0;
    delete network;
    network = 0;
    delete networkEvaluator;
    networkEvaluator = 0;

    if (app) {
        app->deinitGL();
        app->deinit();
    }
    delete app;
    app = 0;
}

void keyPressed(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {
        case '\033': // = ESC
            finalize();
            exit(0);
            break;
        case '1':
            glutReshapeWindow(128,128);
            break;
        case '2':
            glutReshapeWindow(256,256);
            break;
        case '3':
            glutReshapeWindow(512,512);
            break;
        case '4':
            glutReshapeWindow(1024, 1024);
            break;
    }
}

int main(int argc, char** argv) {
    VoreenApplication* app = new VoreenApplication("simple-GLUT", "simple-GLUT", argc, argv,
        VoreenApplication::APP_ALL);
    app->init();

    glutInit(&argc, argv);

    // initialize canvas
    canvas = new tgt::GLUTCanvas("Voreen - The Volume Rendering Engine",
                                  tgt::ivec2(512, 512), tgt::GLCanvas::RGBADD);
    canvas->init();

    glutKeyboardFunc(keyPressed);

    app->initGL();
    initialize();

    glutMainLoop();
    return 0; // will never be reached for standard GLUT
}
