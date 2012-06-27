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

#include "voreen/qt/voreenapp.h"

#include "tgt/init.h"
#include "tgt/singleton.h"
#include "tgt/shadermanager.h"
#include "tgt/filesystem.h"
#include "voreen/core/vis/messagedistributor.h"

#include <QSplashScreen>
#include <QLocale>
#include <QTranslator>

using namespace voreen;

void VoreenApp::boot(int /*argc*/, char** /*argv*/) {

    /*
        Please note that the order of initialisation is very important... do not change it without knowing what
        you're doing, or the program will crash... tgt::init() has to have been called before this method was called
    */

    // initialize the message-distributor
    tgt::Singleton<MessageDistributor>::init(new MessageDistributor());

#define VRN_SHOWCONSOLE
#if defined(VRN_SHOWCONSOLE) && defined(WIN32)
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif

#ifdef VRN_USE_TRANSLATIONS
    // load and install qt translations
    QTranslator qtTranslator;
    qtTranslator.load("../translations/qt_" + QLocale::system().name());
    app_.installTranslator(&qtTranslator);

    // load and install voreen translations
    QTranslator voreenTranslator;
    voreenTranslator.load("../translations/voreen_" + QLocale::system().name());
    app_.installTranslator(&voreenTranslator);
#endif

#undef VRN_SPLASHSCREEN
#ifdef VRN_SPLASHSCREEN
    QPixmap pixmap("splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
#endif

    //add console-logger
    tgt::Log* clog = new tgt::ConsoleLog();
    clog->addCat(std::string(), true, tgt::Info);
    LogMgr.addLog(clog);

#ifdef VRN_ADD_FILE_LOGGER
    // add a file logger
    tgt::Log* log = new tgt::HtmlLog("voreen-log.html");
    log->addCat(""/*clp.getCategory().toStdString()*/, true/*, clp.getDebugLevel()*/);
    LogMgr.addLog(log);
#endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    frame_->resize(800, 600);

#if defined(VRN_SPLASHSCREEN) && defined(WIN32)
    frame_->showMinimized();
#else
    frame_->show();
#endif

    // the mainframe had to be shown before glewInit(), which is part of tgt::initGL(); now that this is done,
    // we can call tgt::initGL() (otherwise, there is no valid opengl-context)
    tgt::initGL();

    // set shader source path; has to be set after tgt::initGL(), otherwise the Singleton is not initialised
    ShdrMgr.addPath("../../src/core/vis/glsl");

    // initialize virtual file system for shaders
    // (only in release builds)
#ifdef NDEBUG
#pragma message("WARNING: Using 'shaders.tar'. Is it up to date???")
    FileSys.addPackage("shaders.tar", "../../");
#endif
    QStringList args = app_.arguments();
    frame_->init(args);

    QApplication::restoreOverrideCursor();

#ifdef VRN_SPLASHSCREEN
    app_.processEvents();
    splash.close();
#endif

    app_.exec();
}

void VoreenApp::shutdown() {
    // deinit everything
    tgt::Singleton<MessageDistributor>::deinit();
    tgt::deinitGL();
    tgt::deinit();
}
