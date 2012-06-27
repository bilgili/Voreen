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

// do this at very first
#ifdef VRN_WITH_PYTHON
#include "tgt/scriptmanager.h"
#endif // VRN_WITH_PYTHON

#include "rptmainwindow.h"

#include "tgt/init.h"
#include "tgt/logmanager.h"
#include "tgt/filesystem.h"

#include "voreen/core/cmdparser/commandlineparser.h"
#include "voreen/core/vis/messagedistributor.h"

#include "voreen/qt/versionqt.h"

#ifdef VRN_WITH_PYTHON
#include "voreen/core/vis/pyvoreen.h"
#include "voreen/qt/pyvoreenqt.h"
#endif

#ifdef VRN_SPLASHSCREEN
#include <QSplashScreen>
#endif

#include "tgt/gpucapabilities.h"
#ifdef WIN32
    #include "tgt/gpucapabilitieswindows.h"
#endif

using namespace voreen;

// Check whether working directory is set correctly and try to fix it for some common
// situations.
bool checkVoreenPath(const QString& appname = "") {
    const QString shaderPath = "../../src/core/vis/glsl";
    QDir dir;
    if (!dir.exists(shaderPath)) {
        QDir dir;
        LWARNINGC("voreenve.main", "shader path " << shaderPath.toStdString()
                  << " not found from current directory "
                  << dir.canonicalPath().toStdString());

        if (dir.cdUp() && dir.exists(shaderPath)) {
            // found from parent directory
            LWARNINGC("voreenve.main", "changing working directory to "
                      << dir.absolutePath().toStdString());

            QDir::setCurrent(dir.path());
            return true;
        }
        else if (!appname.isEmpty()) {
            // try directory named like the application
            QDir dir;
            if (dir.cd(appname) && dir.exists(shaderPath)) {
                // found from appname directory
                LWARNINGC("voreenve.main", "changing working directory to "
                          << dir.absolutePath().toStdString());
                QDir::setCurrent(dir.path());
                return true;
            }            
        }
    } else {
        return true;
    }

    LERRORC("voreenve.main", "shader path not found, check your working directory");
    QMessageBox::critical(0, QObject::tr("Error"),
                          QObject::tr("Could not find the shader path %1.\n"
                                      "Check your working directory!\n"
                                      "(currently set to: %2)\n")
                          .arg(shaderPath).arg(dir.canonicalPath()));
    return false;
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    CommandlineParser cmdparser("VoreenVE");
    cmdparser.setCommandLine(argc, argv);

    tgt::LogLevel logLevel = tgt::Info;
    cmdparser.addCommand(new Command_LogLevel(&logLevel));

    std::string datasetFilename;
    cmdparser.addCommand(new Command_LoadDataset(&datasetFilename));

    std::string networkFilename;
    cmdparser.addCommand(new Command_LoadNetwork(&networkFilename));

    cmdparser.execute();   

    tgt::Singleton<voreen::MessageDistributor>::init(new MessageDistributor());    
    tgt::init();

    tgt::Log* clog = new tgt::ConsoleLog();
    clog->addCat("", true, logLevel);
    LogMgr.addLog(clog);

#ifdef VRN_ADD_FILE_LOGGER
    // add a file logger
    tgt::Log* log = new tgt::HtmlLog("voreenve-log.html");
    log->addCat("", true, logLevel);
    LogMgr.addLog(log);
#endif

    VoreenVersionQt::logAll("voreen.VoreenVE");

#ifdef VRN_SPLASHSCREEN
    QPixmap pixmap(":/vrn_app/image/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
#endif
    
#if (QT_VERSION >= 0x040400) && !defined(__APPLE__)
    // load and set style sheet (only on Qt 4.4 or newer)
	QFile file(":/widgetstyle/voreen.qss");
	file.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(file.readAll());
	app.setStyleSheet(styleSheet);
#endif

// init resources for voreen_qt
    Q_INIT_RESOURCE(vrn_qt);
    // init common application resources
    Q_INIT_RESOURCE(vrn_app);
    QApplication::setOrganizationName("Voreen");
    QApplication::setOrganizationDomain("voreen.org");
    QApplication::setApplicationName("VoreenVE");

#ifdef VRN_WITH_PYTHON
    ScriptMgr.addPath("");
    initVoreenPythonModule();
    initVoreenqtPythonModule();
#endif // VRN_WITH_PYTHON

    // initialize virtual file system for shaders
    // (only in release builds)
#ifdef NDEBUG
    #pragma message("WARNING: Using 'shaders.tar'. Is it up to date???")
    LINFOC("voreenve.main", "Loading file 'shaders.tar' from Qt's virtual file system ...");
    QFile shaderFile(":/shaders.tar");
    if (!shaderFile.open(QIODevice::ReadOnly)) {
        LERRORC("voreenve.main", "FAILED to load file 'shaders.tar' from Qt's virtual file system!");
        if (!checkVoreenPath("voreenve"))
            return 0;
    } else {
        LDEBUGC("voreenve.main", "Reading data from shader file ...");
        QByteArray dataArray = shaderFile.readAll();
        int dataLength = dataArray.length();
        LINFOC("voreenve.main", "Creating file 'shader_memory.tar' in tgt's virtual file system ...");
        char* data = new char[dataLength];
        memcpy(data, dataArray.data(), dataLength);
        FileSys.addMemoryFile("shader_memory.tar", data, dataLength);
        FileSys.addPackage("shader_memory.tar", "../../src/core/vis/");
    }
#else
    if (!checkVoreenPath("voreenve"))
        return 0;
#endif

    RptMainWindow mainWindow(networkFilename, datasetFilename);
    mainWindow.show();
    tgt::initGL();

#ifdef WIN32
    tgt::GpuCapabilitiesWindows::getInstance()->logCapabilities(false, true);
#else
    GpuCaps.logCapabilities(false, true);
#endif

    mainWindow.initGL(); 
    mainWindow.createConnections();
    
#ifdef VRN_SPLASHSCREEN
    splash.close();
#endif

    return app.exec();
}
