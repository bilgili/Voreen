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

#ifdef VRN_MODULE_PYTHON
// include this at first
#include "voreen/modules/python/pythonmodule.h"
#include "voreen/modules/python/qt/pyvoreenqt.h"
#endif

#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/qt/versionqt.h"
#include "voreen/qt/progressdialog.h"
#include "tgt/init.h"
#include "tgt/qt/qttimer.h"
#include "tgt/filesystem.h"
#include "tgt/shadermanager.h"

#include <QApplication>
#include <QMainWindow>
#include <QDir>

using std::string;

namespace {

} // namespace


namespace voreen {

VoreenApplicationQt* VoreenApplicationQt::qtApp_ = 0;

VoreenApplicationQt::VoreenApplicationQt(const std::string& name, const std::string& displayName,
                                         int argc, char** argv, ApplicationType appType)
    : VoreenApplication(name, displayName, argc, argv, appType)
    , mainWindow_(0)
#ifdef VRN_MODULE_PYTHON
    , pythonQt_(0)
#endif
{
    QCoreApplication::setOrganizationName("Voreen");
    QCoreApplication::setOrganizationDomain("voreen.org");
    QCoreApplication::setApplicationName(displayName.c_str());

    qtApp_ = this;
}

VoreenApplicationQt::~VoreenApplicationQt() {
#ifdef VRN_MODULE_PYTHON
    delete pythonQt_;
    pythonQt_ = 0;
#endif
}

void VoreenApplicationQt::init() {
    VoreenApplication::init();
    if (!initialized_)
        return;

    VoreenVersionQt::logAll(name_ + "." + displayName_);

    // Set the path for temporary files
    temporaryPath_ = QDir::tempPath().toStdString();

    //
    // Path detection
    //

    // shader path
    if (appType_ & APP_SHADER) {
#ifdef VRN_INSTALL_PREFIX
        shaderPathQt_ = basePath_ + "/share/voreen/shaders";
#else
    #if defined(__APPLE__) && defined(VRN_DEPLOYMENT)
        shaderPathQt_ = appBundleResourcesPath_ + "/glsl/qt";
    #elif defined(VRN_DEPLOYMENT)
        shaderPathQt_ = basePath_ + "/glsl";
    #else
        shaderPathQt_ = basePath_ + "/src/qt/glsl";
    #endif
#endif
    }
}

void VoreenApplicationQt::initGL() throw (VoreenException) {
    VoreenApplication::initGL();

    ShdrMgr.addPath(getShaderPathQt());

#ifdef VRN_MODULE_PYTHON
    if (PythonModule::getInstance() && PythonModule::getInstance()->isInitialized())
        pythonQt_ = new PyVoreenQt();
    else
        LWARNING("Skipped instantiation of VoreenPythonQt: Python module not initialized");
#endif
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

ProgressDialog* VoreenApplicationQt::createProgressDialog() const {
    return new ProgressDialog(getMainWindow());
}

std::string VoreenApplicationQt::getShaderPathQt(const std::string& filename) const {
    return shaderPathQt_ + (filename.empty() ? "" : "/" + filename);
}

} // namespace

