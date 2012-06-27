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

#include "tgt/init.h"
#include "tgt/logmanager.h"
#include "tgt/memorymanager.h"
#include "tgt/singleton.h"

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/qt/versionqt.h"

#ifdef VRN_WITH_PYTHON
#include "voreen/core/vis/pyvoreen.h"
#include "voreen/qt/pyvoreenqt.h"
#endif

#ifdef WIN32
    #include "tgt/gpucapabilitieswindows.h"
#endif

#if defined(VRN_WITH_BUGTRAP) && defined(WIN32)
#include "BugTrap.h"
#include <TCHAR.H>
#endif

#include <QSplashScreen>
#include <QTranslator>

#include "mainframe.h"

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

void initGL() {
    tgt::initGL();

    // set shader source path
    ShdrMgr.addPath("../../src/core/vis/glsl");


    // initialize OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

int start(int argc, char** argv) {
    tgt::Singleton<voreen::MessageDistributor>::init(new MessageDistributor());

#ifdef VRN_WITH_PYTHON
    ScriptMgr.addPath("");
    initVoreenPythonModule();
    initVoreenqtPythonModule();
#endif // VRN_WITH_PYTHON

#define VRN_SHOWCONSOLE
#if defined(VRN_SHOWCONSOLE) && defined(WIN32)
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif

#if defined(VRN_WITH_BUGTRAP) && defined(WIN32)
    // install bugtrap
    BT_InstallSehFilter();
    BT_SetAppName(_T("Voreen development application"));
    BT_SetSupportEMail(_T("jennis.meyer-spradow@math.uni-muenster.de"));
    BT_SetFlags(BTF_DETAILEDMODE | BTF_EDITMAIL | BTF_ATTACHREPORT| BTF_SCREENCAPTURE);
    //BT_SetSupportServer(_T("localhost"), 9999);
    BT_SetSupportURL(_T("http://www.voreen.org"));
#endif

    // initialize virtual file system for shaders
    // (only in release builds)
#ifdef NDEBUG
#pragma message("WARNING: Using 'shaders.tar'. Is it up to date???")
    FileSys.addPackage("shaders.tar", "../../");
#endif

    QApplication a(argc, argv);

    if (!checkVoreenPath("voreendev"))
        return 0;
    
#ifdef VRN_USE_TRANSLATIONS
    // load and install qt translations
    QTranslator qtTranslator;
#ifdef NDEBUG
    qtTranslator.load("lang/qt_" + QLocale::system().name());
#else
    qtTranslator.load("../translations/qt_" + QLocale::system().name());
#endif
    a.installTranslator(&qtTranslator);

    // load and install voreen translations
    QTranslator voreenTranslator;
#ifdef NDEBUG
    voreenTranslator.load("lang/voreen_" + QLocale::system().name());
#else
    voreenTranslator.load("../translations/voreen_" + QLocale::system().name());
#endif
    a.installTranslator(&voreenTranslator);
#endif

#ifdef VRN_SPLASHSCREEN
    QPixmap pixmap(":/vrn_app/image/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
#endif

    QStringList args(a.arguments());
    if (!QGLFormat::hasOpenGL())
        qFatal("This system has no OpenGL support");

    VoreenMainFrame mainFrame(&args);

    // init tgt logging system:
    CmdLineParser& clp = mainFrame.getCmdLineParser();

    // add a console logger
    tgt::Log* clog = new tgt::ConsoleLog();
    clog->addCat(clp.getCategory().toStdString(), true, clp.getDebugLevel());
    LogMgr.addLog(clog);

#ifdef VRN_ADD_FILE_LOGGER
    // add a file logger
    tgt::Log* log = new tgt::HtmlLog("voreendev-log.html");
    log->addCat(clp.getCategory().toStdString(), true, clp.getDebugLevel());
    LogMgr.addLog(log);
#endif

    VoreenVersionQt::logAll("voreen.VoreenDev");

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#if defined(VRN_SPLASHSCREEN) && defined(WIN32)
    mainFrame.showMinimized();
#else
    mainFrame.show();
#endif

    initGL();
#ifdef WIN32
    tgt::GpuCapabilitiesWindows::getInstance()->logCapabilities(false, true);
#else
    GpuCaps.logCapabilities(false, true);
#endif
    mainFrame.init();
    QApplication::restoreOverrideCursor();

    
#ifdef VRN_SPLASHSCREEN
    a.processEvents();
    splash.close();
	mainFrame.showMaximized();
#endif

    int result = 0;

    // Start main loop unless we are benchmarking
    if (!clp.getBenchmark()) {
        result = a.exec();
    }
    
    mainFrame.deinit();
    tgt::Singleton<voreen::MessageDistributor>::deinit();

    return result;
}

int main(int argc, char** argv) {
#ifdef TGT_USE_MEMORY_MANAGER
	// the memory manager can be used to fin memory leaks
    tgt::MemoryManager::init();
#endif

    int result;
    tgt::init();
    // init resources for voreen_qt
    Q_INIT_RESOURCE(vrn_qt);
    // init common application resources
    Q_INIT_RESOURCE(vrn_app);
    result = start(argc, argv);
    tgt::deinitGL();
    tgt::deinit();

#ifdef TGT_USE_MEMORY_MANAGER
    tgt::MemoryManager::deinit();
#endif

    return result;
}
