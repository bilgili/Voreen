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

#ifdef VRN_WITH_PYTHON
// this must come first
#include "tgt/scriptmanager.h"
#endif // VRN_WITH_PYTHON

#include "voreenmainwindow.h"

#include "tgt/filesystem.h"

#include "voreen/qt/applicationqt.h"

#ifdef VRN_SPLASHSCREEN
#include <QSplashScreen>
#endif

using namespace voreen;

class VoreenVEApplication : public VoreenApplicationQt {
public:
    VoreenVEApplication(int argc, char** argv, const std::string& logDir)
        : VoreenApplicationQt("voreenve", "VoreenVE", argc, argv, VoreenApplication::APP_DEFAULT, logDir)
    {}

    virtual void prepareCommandParser() {
        VoreenApplicationQt::prepareCommandParser();

        getCommandLineParser()->addCommand(new Command_LoadDatasetSingle(&datasetFilename_));
        getCommandLineParser()->addCommandForNamelessArguments(new Command_LoadDatasetSingle(&datasetFilename_));

        getCommandLineParser()->addCommand(new Command_LoadNetworkSingle(&networkFilename_));
    }

    std::string datasetFilename_;
    std::string networkFilename_;
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    std::string logDir;
#ifdef VRN_DISTRIBUTION_MODE
    // detect user's home directory for log file
    logDir = QDir::toNativeSeparators(QDir::homePath() + "/").toStdString();
#endif

    VoreenVEApplication vapp(argc, argv, logDir);
    vapp.init();

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

    // initialize virtual file system for shaders
    // (only in distribution mode)
#ifdef VRN_DISTRIBUTION_MODE
    #pragma message("NOTICE: Using 'shaders.tar'. Is it up to date?")
    LINFOC("voreenve.main", "Loading shaders.tar into virtual file system ...");
    FileSys.addPackage(vapp.getBasePath() + "/shaders.tar", "/src/core/vis/glsl/");
#endif

    VoreenMainWindow mainWindow(vapp.networkFilename_, vapp.datasetFilename_);
    mainWindow.show();

    vapp.initGL();
    mainWindow.init();

#ifdef VRN_SPLASHSCREEN
    splash.close();
#endif

    return app.exec();
}
