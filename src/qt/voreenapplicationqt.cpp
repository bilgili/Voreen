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

#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/qt/versionqt.h"
#include "voreen/qt/ioprogressdialog.h"
#include "tgt/init.h"
#include "tgt/qt/qttimer.h"
#include "tgt/filesystem.h"
#include "tgt/shadermanager.h"

#ifdef VRN_WITH_PYTHON
#include "voreen/qt/pyvoreenqt.h"
#endif

#include <QApplication>
#include <QMainWindow>
#include <QDir>

using std::string;

namespace {

string findShaderPath(const string& basePath) {
#ifdef VRN_INSTALL_PREFIX
    return basePath + "/share/voreen/shaders";
#else
    return basePath + "/src/qt/glsl";
#endif
}

} // namespace


namespace voreen {

VoreenApplicationQt* VoreenApplicationQt::qtApp_ = 0;

VoreenApplicationQt::VoreenApplicationQt(const std::string& name, const std::string& displayName,
                                         int argc, char** argv, ApplicationType appType)
    : VoreenApplication(name, displayName, argc, argv, appType)
    , mainWindow_(0)
{
    QCoreApplication::setOrganizationName("Voreen");
    QCoreApplication::setOrganizationDomain("voreen.org");
    QCoreApplication::setApplicationName(displayName.c_str());

    qtApp_ = this;
}

void VoreenApplicationQt::init() {
    VoreenApplication::init();

    VoreenVersionQt::logAll(name_ + "." + displayName_);

    // Set the path for temporary files
    temporaryPath_ = QDir::tempPath().toStdString();

#ifdef VRN_WITH_PYTHON
    tgt::Singleton<VoreenPythonQt>::init(new VoreenPythonQt());
#endif

    //
    // Path detection
    //

    // shader path
    if (appType_ & APP_SHADER) {
        shaderPathQt_ = findShaderPath(basePath_);
    }
}

void VoreenApplicationQt::initGL() {
    VoreenApplication::initGL();
    ShdrMgr.addPath(getShaderPathQt());
}

void VoreenApplicationQt::setMainWindow(QMainWindow* mainWindow) {
    mainWindow_ = mainWindow;
}

QMainWindow* VoreenApplicationQt::getMainWindow() const {
    return mainWindow_;
}

VoreenApplicationQt* VoreenApplicationQt::qtApp() {
    return qtApp_;
}

tgt::Timer* VoreenApplicationQt::createTimer(tgt::EventHandler* handler) const {
    return new tgt::QtTimer(handler);
}

IOProgressDialog* VoreenApplicationQt::createProgressDialog() const {
    return new IOProgressDialog(getMainWindow());
}

std::string VoreenApplicationQt::getShaderPathQt(const std::string& filename) const {
    return shaderPathQt_ + (filename.empty() ? "" : "/" + filename);
}

} // namespace

