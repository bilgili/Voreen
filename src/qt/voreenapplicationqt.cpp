/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/qt/voreenmoduleqt.h"
#include "modules/core/coremoduleqt.h"
#include "voreen/qt/versionqt.h"
#include "voreen/qt/progressdialog.h"
#include "voreen/core/utils/stringutils.h"
#include "voreen/core/properties/callmemberaction.h"
#include "tgt/init.h"
#include "tgt/qt/qttimer.h"
#include "tgt/filesystem.h"

#include "gen_moduleregistration_qt.h"

#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QDir>
#include <QSettings>

using std::string;


namespace voreen {

VoreenApplicationQt* VoreenApplicationQt::qtApp_ = 0;
const std::string VoreenApplicationQt::loggerCat_ = "voreenqt.VoreenApplicationQt";

VoreenApplicationQt::VoreenApplicationQt(const std::string& name, const std::string& displayName, const std::string& description,
                                         int argc, char** argv, ApplicationFeatures appType)
    : VoreenApplication(name, displayName, description, argc, argv, appType)
    , resetApplicationSettingsButton_("resetApplicationSettings", "Reset Window Settings")
    , mainWindow_(0)
    , clearSettings_(false)
{
    QCoreApplication::setOrganizationName("Voreen");
    QCoreApplication::setOrganizationDomain("voreen.org");
    QCoreApplication::setApplicationName(displayName.c_str());

    addProperty(resetApplicationSettingsButton_);
    resetApplicationSettingsButton_.onClick(
        CallMemberAction<VoreenApplicationQt>(this, &VoreenApplicationQt::queryResetApplicationSettings));
    resetApplicationSettingsButton_.setGroupID("user-interface");

    qtApp_ = this;
}

VoreenApplicationQt::~VoreenApplicationQt() {
    if (clearSettings_) {
        QSettings settings;
        settings.clear();
    }
}

void VoreenApplicationQt::loadModules() throw (VoreenException) {
    VoreenApplication::loadModules();

    // load Qt modules
    if (isModuleLoadingEnabled()) {
        LDEBUG("Loading Voreen Qt modules from module registration header");
        registerAllQtModules(this);
    }
    else {
        LDEBUG("Module auto loading disabled");
        registerQtModule(new CoreModuleQt(getBasePath("modules/core")));   //< core module is always included
    }

}

void VoreenApplicationQt::initialize() throw (VoreenException) {
    VoreenApplication::initialize();
    if (!isInitialized())
        return;

    LINFO("Qt version: " << VoreenVersionQt::getQtVersion());
}

void VoreenApplicationQt::deinitialize() throw (VoreenException) {
    VoreenApplication::deinitialize();

    qtModules_.clear(); //< have been deleted by VoreenApplication::deinitialize();
}

void VoreenApplicationQt::initializeGL() throw (VoreenException) {
    VoreenApplication::initializeGL();
}

void VoreenApplicationQt::deinitializeGL() throw (VoreenException) {
    VoreenApplication::deinitializeGL();
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

void VoreenApplicationQt::registerQtModule(VoreenModuleQt* qtModule) {
    tgtAssert(qtModule, "null pointer passed");

    // qt modules are subject to standard module handling
    VoreenApplication::registerModule(qtModule);

    // additionally store qt modules separately (currently no use for this, though)
    if (std::find(qtModules_.begin(), qtModules_.end(), qtModule) == qtModules_.end())
        qtModules_.push_back(qtModule);
    else
        LWARNING("Qt Module '" << qtModule->getName() << "' has already been registered. Skipping.");
}

const std::vector<VoreenModuleQt*>& VoreenApplicationQt::getQtModules() const {
    return qtModules_;
}

VoreenModuleQt* VoreenApplicationQt::getQtModule(const std::string& moduleName) const {
    for (size_t i = 0 ; i < qtModules_.size() ; ++i) {
        VoreenModuleQt* qtModule = qtModules_.at(i);
        if (qtModule->getName() == moduleName)
            return qtModule;
    }
    return 0;
}

void VoreenApplicationQt::resetApplicationSettings() {
    QSettings settings;
    settings.clear();
    clearSettings_ = true;
}

void VoreenApplicationQt::queryResetApplicationSettings() {
    QMessageBox msgBox(mainWindow_);
    msgBox.setWindowTitle(QApplication::tr("Reset Windows Settings"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(QApplication::tr("This will reset the complete window configuration as well as the default paths."));
    msgBox.setInformativeText(QApplication::tr("Do you want to proceed?"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Ok) {
        resetApplicationSettings();
        QMessageBox::information(mainWindow_, QApplication::tr("Window Settings Restored"),
            QApplication::tr("All window settings have been restored. "
            "Please restart the application for the changes to take effect."));
    }
}

} // namespace

