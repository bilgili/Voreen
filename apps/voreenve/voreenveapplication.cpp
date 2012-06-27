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

#include "voreenveapplication.h"

#include "tgt/filesystem.h"
#include "tgt/logmanager.h"
#include "voreen/core/utils/commandlineparser.h"
#include "voreen/core/version.h"
#include "voreen/core/utils/stringutils.h"
#include "voreen/qt/voreenapplicationqt.h"
#include "voreenmoduleve.h"
#include "voreenvemetadatafactory.h"

#include "gen_moduleregistration_ve.h"

#include <QMessageBox>

namespace voreen {

VoreenVEApplication* VoreenVEApplication::veApp_ = 0;
const std::string VoreenVEApplication::loggerCat_("voreenve.VoreenVEApplication");

VoreenVEApplication::VoreenVEApplication(int& argc, char** argv)
    : QApplication(argc, argv)
#ifdef VRN_ADD_FILE_LOGGER
    , VoreenApplicationQt("voreenve", "VoreenVE", "Voreen Visualization Environment", argc, argv, VoreenApplication::APP_ALL)
#else
    , VoreenApplicationQt("voreenve", "VoreenVE", "Voreen Visualization Environment", argc, argv, VoreenApplication::APP_DEFAULT)
#endif
{
    veApp_ = this;

    registerSerializerFactory(new VoreenVEMetaDataFactory());
}

VoreenVEApplication::~VoreenVEApplication() {
}

void VoreenVEApplication::loadModules() throw (VoreenException) {
    VoreenApplicationQt::loadModules();

    // load VE modules
    if (isModuleLoadingEnabled()) {
        LDEBUG("Loading VoreenVE modules from module registration header");
        registerAllVEModules(this);
    }
    else {
        LDEBUG("Module auto loading disabled");
    }
}

void VoreenVEApplication::initialize() throw (VoreenException) {
    VoreenApplicationQt::initialize();
    if (!isInitialized())
        return;
}

void VoreenVEApplication::deinitialize() throw (VoreenException) {
    if (!isInitialized()) {
        if (tgt::LogManager::isInited())
            LWARNING("deinitialize() Application not initialized. Skip.");
        return;
    }

    veModules_.clear(); //< will be deleted by VoreenApplicationQt::deinitialize();

    VoreenApplicationQt::deinitialize();
}

void VoreenVEApplication::initializeGL() throw (VoreenException) {
    VoreenApplicationQt::initializeGL();
}

void VoreenVEApplication::deinitializeGL() throw (VoreenException) {
    VoreenApplicationQt::deinitializeGL();
}

VoreenVEApplication* VoreenVEApplication::veApp() {
    return veApp_;
}

bool VoreenVEApplication::notify(QObject* receiver, QEvent* event) {
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

void VoreenVEApplication::registerVEModule(VoreenModuleVE* module) {
    tgtAssert(module, "null pointer passed");

    // VE modules are subject to standard module handling
    VoreenApplication::registerModule(module);

    tgtAssert(module, "null pointer passed");
    if (std::find(veModules_.begin(), veModules_.end(), module) == veModules_.end())
        veModules_.push_back(module);
    else
        LWARNING("VoreenVE module '" << module->getName() << "' has already been registered. Skipping.");
}

const std::vector<VoreenModuleVE*>& VoreenVEApplication::getVEModules() const {
    return veModules_;
}

} // namespace
