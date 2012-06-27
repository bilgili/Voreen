/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/network/networkevaluator.h"
#include "voreen/core/vis/workspace/workspace.h"
#include "voreen/core/vis/processors/image/canvasrenderer.h"

using namespace voreen;

//---------------------------------------------------------------------------

tgt::GLUTCanvas* canvas_ = 0;

//---------------------------------------------------------------------------

void init() {
    // add shader path to the shader manager
    ShdrMgr.addPath(VoreenApplication::app()->getShaderPath());

    Workspace* workspace = new Workspace();
    workspace->load(VoreenApplication::app()->getWorkspacePath() + "/standard.vws");

    // initialize the network evaluator
    NetworkEvaluator* networkEvaluator = new NetworkEvaluator();
    ProcessorNetwork* network = workspace->getProcessorNetwork();
    std::vector<CanvasRenderer*> canvasRenderer = network->getProcessorsByType<CanvasRenderer>();

    if(canvasRenderer.size() > 0) {
        // init painter and connect it to the canvas
        VoreenPainter* painter = new VoreenPainter(canvas_, networkEvaluator, canvasRenderer[0]);
        canvas_->setPainter(painter);
        canvasRenderer[0]->setCanvas(canvas_);
        // give the network to the network evaluator
        networkEvaluator->setProcessorNetwork(network);
    }
}

void keyPressed(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {
        case '\033': // = ESC
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
    VoreenApplication app("simple-GLUT", "simple-GLUT", argc, argv);
    app.init();

    glutInit(&argc, argv);

    // initialize canvas
    canvas_ = new tgt::GLUTCanvas("Voreen - The Volume Rendering Engine", tgt::ivec2(512, 512), tgt::GLCanvas::RGBADD);
    canvas_->init();

    glutKeyboardFunc(keyPressed);

    app.initGL();

    init();

    glutMainLoop();

    return 0;
}
