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

#ifdef VRN_WITH_PYTHON
// this must come first
#include "tgt/scriptmanager.h"
#endif // VRN_WITH_PYTHON

#include "voreenmainwindow.h"

#include "tgt/filesystem.h"
#include "tgt/ziparchive.h"

#include "voreen/core/version.h"
#include "voreen/qt/voreenapplicationqt.h"

#ifdef VRN_NO_MODULE_AUTO_REGISTRATION
#include "voreen/modules/moduleregistration.h"
#else
#include "voreen/modules/gen_moduleregistration.h"
#endif

using namespace voreen;

class VoreenVEApplication : public VoreenApplicationQt {
public:
    VoreenVEApplication(int argc, char** argv)
        : VoreenApplicationQt("voreenve", "VoreenVE", argc, argv, VoreenApplication::APP_DEFAULT)
    {}

    virtual void prepareCommandParser() {
        VoreenApplicationQt::prepareCommandParser();

        CommandlineParser* p = getCommandLineParser();
        p->addCommand(new Command_LoadDatasetSingle(&datasetFilename_));
        p->addCommandForNamelessArguments(new Command_LoadDatasetSingle(&datasetFilename_));

        p->addCommand(new SingleCommand<std::string>(&workspaceFilename_, "--workspace", "-w",
                                                     "Loads a workspace", "<workspace file>"));
#ifdef VRN_WITH_PYTHON
        p->addCommand(new SingleCommand<std::string>(&scriptFilename_, "--script", "-s",
                                                     "Runs a python script", "<script file>"));
#endif
    }

    std::string datasetFilename_;
    std::string workspaceFilename_;
    std::string scriptFilename_;
};

/// Reimplement QApplication to catch unhandled exceptions
class CatchApp : public QApplication {
public:
    CatchApp(int & argc, char ** argv )
        : QApplication(argc, argv)
    {}

    virtual bool notify(QObject* receiver, QEvent* event) {
        bool result = false;
        try {
            result = QApplication::notify(receiver, event);
        }
        catch (const VoreenException& e) {
            LERRORC("voreenve.main", "Caught unhandled VoreenException: " << e.what());
#ifndef TGT_NON_INTERACTIVE_ASSERT
            int choice = QMessageBox::critical(0, tr("VoreenVE"), tr("Caught unhandled VoreenException:\n\"")
                                               + e.what() + +"\"\n" + tr("Continue?"),
                                               QMessageBox::Ok | QMessageBox::Cancel);
            if (choice == QMessageBox::Cancel) {
  #ifdef VRN_DEBUG
                TGT_THROW_BREAKPOINT;
  #else
                exit(1);
  #endif
            }
#else
            exit(1);
#endif // TGT_NON_INTERACTIVE_ASSERT
        }
        catch (const std::exception& e) {
            LERRORC("voreenve.main", "Caught unhandled std::exception: " << e.what());
#ifndef TGT_NON_INTERACTIVE_ASSERT
            int choice = QMessageBox::critical(0, tr("VoreenVE"), tr("Caught unhandled std::exception:\n\"")
                                               + e.what() + "\"\n" + tr("Continue?"),
                                               QMessageBox::Ok | QMessageBox::Cancel);
            if (choice == QMessageBox::Cancel) {
  #ifdef VRN_DEBUG
                TGT_THROW_BREAKPOINT;
  #else
                exit(1);
  #endif
            }
#else
            exit(1);
#endif // TGT_NON_INTERACTIVE_ASSERT
        }
        catch (...) {
            LERRORC("voreenve.main", "Caught unhandled unknown exception!");
#ifndef TGT_NON_INTERACTIVE_ASSERT
            int choice = QMessageBox::critical(0, tr("VoreenVE"), tr("Caught unhandled unknown exception!\nContinue?"),
                                               QMessageBox::Ok | QMessageBox::Cancel);
            if (choice == QMessageBox::Cancel) {
  #ifdef VRN_DEBUG
                TGT_THROW_BREAKPOINT;
  #else
                exit(1);
  #endif
            }
#else
            exit(1);
#endif // TGT_NON_INTERACTIVE_ASSERT
            throw;
        }
        return result;
    }
};

int main(int argc, char** argv) {
    CatchApp app(argc, argv);

    app.setOverrideCursor(Qt::WaitCursor);

#ifdef VRN_SPLASHSCREEN
    VoreenSplashScreen splash;
    splash.show();
    qApp->processEvents();
#endif

    VoreenVEApplication vapp(argc, argv);
    vapp.init();

#if (QT_VERSION >= 0x040400) && !defined(__APPLE__)  && !defined(VRN_NO_STYLESHEET)
    // load and set style sheet (only on Qt 4.4 or newer)
    QFile file(":/voreenve/widgetstyle/voreen.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    app.setStyleSheet(styleSheet);
#ifdef VRN_SPLASHSCREEN
    splash.showMessage("Starting up...");
#endif
#endif

    // init resources for voreen_qt
    Q_INIT_RESOURCE(vrn_qt);
    // init common application resources
    Q_INIT_RESOURCE(vrn_app);

    // initialize all Voreen modules
#ifdef VRN_SPLASHSCREEN
    splash.showMessage("Initializing modules...");
#endif
    addAllModules(&vapp);

#ifdef VRN_SPLASHSCREEN
    splash.showMessage("Creating main window...");
#endif
    VoreenMainWindow mainWindow(vapp.workspaceFilename_, vapp.datasetFilename_);
    vapp.setMainWindow(&mainWindow);
    mainWindow.show();
#ifdef VRN_SPLASHSCREEN
    mainWindow.init(&splash);  // also calls VoreenApplication::app()->initGL()
#else
    mainWindow.init();
#endif

    app.restoreOverrideCursor();

#ifdef VRN_SPLASHSCREEN
    splash.showMessage("Initialization complete.");
    splash.close();
#endif

    if (!vapp.scriptFilename_.empty()) {
        // first make sure that all Qt events have been processed
        qApp->processEvents();
        mainWindow.runScript(vapp.scriptFilename_.c_str());
    }

    return app.exec();
}
