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

#include "voreen/qt/applicationqt.h"
#include "voreen/qt/versionqt.h"

#ifdef VRN_WITH_PYTHON
#include "voreen/qt/pyvoreenqt.h"
#endif

#include <QApplication>
#include <QDir>

namespace voreen {

VoreenApplicationQt::VoreenApplicationQt(const std::string& name, const std::string& displayName,
                                         int argc, char** argv, ApplicationType appType)
    : VoreenApplication(name, displayName, argc, argv, appType)
{
    QApplication::setOrganizationName("Voreen");
    QApplication::setOrganizationDomain("voreen.org");
    QApplication::setApplicationName(displayName.c_str());
}

void VoreenApplicationQt::init() {
    VoreenApplication::init();

    VoreenVersionQt::logAll(name_ + "." + displayName_);

    // Set the path for temporary files
    temporaryPath_ = QDir::tempPath().toStdString();

#ifdef VRN_WITH_PYTHON
    initVoreenqtPythonModule();
#endif
}

} // namespace
