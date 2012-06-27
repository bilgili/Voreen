/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "rptmainwindow.h"
#include "voreen/core/vis/processors/networkevaluator.h" // FIXME: remove after global pointer is gone

#ifdef VRN_SPLASHSCREEN
#include <QSplashScreen>
#endif

using namespace voreen;

namespace voreen {
    NetworkEvaluator* uglyglobalevaluator = 0; // FIXME: remove after we found a way to access the NetEv for Tooltips
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
#ifdef VRN_SPLASHSCREEN
    QPixmap pixmap(":/vrn_app/image/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
#endif
    tgt::Singleton<voreen::MessageDistributor>::init(new MessageDistributor());    
    tgt::init();

    tgt::Log* clog = new tgt::ConsoleLog();
    clog->addCat("", true, tgt::Info);
    LogMgr.addLog(clog);

    // init resources for voreen_qt
    Q_INIT_RESOURCE(vrn_qt);
    // init common application resources
    Q_INIT_RESOURCE(vrn_app);
    QApplication::setOrganizationName("Voreen");
    QApplication::setOrganizationDomain("voreen.org");
    QApplication::setApplicationName("VoreenVE");

// initialize virtual file system for shaders
    // (only in release builds)
#ifdef NDEBUG
    #pragma message("WARNING: Using 'shader.tar'. Is it up to date???")
    std::cout << "Loading file 'shaders.tar' from Qt's virtual file system ..." << std::endl;
    QFile shaderFile(":/shaders.tar");
    if (!shaderFile.open(QIODevice::ReadOnly)) {
        std::cout << "FAILED to load file 'shaders.tar' from Qt's virtual file system!" << std::endl;
    } else {
        std::cout << "Reading data from shader file ..." << std::endl;
        QByteArray dataArray = shaderFile.readAll();
        int dataLength = dataArray.length();
        std::cout << "Creating file 'shader_memory.tar' in tgt's virtual file system ..." << std::endl;
        char* data = new char[dataLength];
        memcpy(data, dataArray.data(), dataLength);
        FileSys.addMemoryFile("shader_memory.tar", data, dataLength);
        FileSys.addPackage("shader_memory.tar", "../../src/core/vis/glsl/");
    }
#endif

    RptMainWindow* mainWindow = new RptMainWindow();
    mainWindow->show();
    tgt::initGL();

    mainWindow->initTextureContainer(); 
    mainWindow->createDockWidgets();
    mainWindow->createConnections();

#ifdef VRN_SPLASHSCREEN
    splash.close();
#endif

    return app.exec();
}
