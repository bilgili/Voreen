/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#include "voreenveplugin.h"

#include "voreen/core/network/workspace.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/qt/widgets/voreentoolwindow.h"
#include "voreenmainwindow.h"

#include <QWidget>
#include <QIcon>

namespace voreen {

VoreenVEPlugin::VoreenVEPlugin(QWidget* parent /*=0*/)
    : QWidget(parent)
    , initialized_(false)
    , mainWindow_(0)
    , parentWindow_(0)
    , evaluator_(0)
    , workspace_(0)
{}

VoreenVEPlugin::~VoreenVEPlugin() {
}

Qt::DockWidgetAreas VoreenVEPlugin::getAllowedDockWidgetAreas() const {
    return Qt::NoDockWidgetArea;
}

Qt::DockWidgetArea VoreenVEPlugin::getInitialDockWidgetArea() const {
    return Qt::NoDockWidgetArea;
}

bool VoreenVEPlugin::isInitialized() const {
    return initialized_;
}

void VoreenVEPlugin::initialize() throw (tgt::Exception) {
}

void VoreenVEPlugin::deinitialize() throw (tgt::Exception) {
}

void VoreenVEPlugin::setMainWindow(VoreenMainWindow* mainWindow) {
    tgtAssert(mainWindow, "null pointer passed");
    mainWindow_ = mainWindow;
}

void VoreenVEPlugin::setParentWindow(VoreenToolWindow* window) {
    tgtAssert(window, "null pointer passed");
    parentWindow_ = window;
}

void VoreenVEPlugin::setNetworkEvaluator(NetworkEvaluator* evaluator) {
    tgtAssert(evaluator, "null pointer passed");
    evaluator_ = evaluator;
}

void VoreenVEPlugin::setWorkspace(Workspace* workspace) {
    workspace_ = workspace;
}

VoreenMainWindow* VoreenVEPlugin::getMainWindow() const {
    return mainWindow_;
}

VoreenToolWindow* VoreenVEPlugin::getParentWindow() const {
    return parentWindow_;
}

NetworkEvaluator* VoreenVEPlugin::getNetworkEvaluator() const  {
    return evaluator_;
}

Workspace* VoreenVEPlugin::getWorkspace() const {
    return workspace_;
}

}
