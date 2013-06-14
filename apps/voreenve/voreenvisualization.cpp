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

#include "voreenvisualization.h"

#include "tgt/gpucapabilities.h"

#include "voreen/core/network/workspace.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/properties/link/linkevaluatorid.h"

#include "voreen/qt/voreenapplicationqt.h"

#include "voreen/qt/widgets/volumeviewer.h"
#include "voreen/qt/widgets/processorlistwidget.h"
#include "voreen/qt/widgets/propertylistwidget.h"
#include "voreen/qt/widgets/rendertargetviewer.h"
#include "voreen/qt/widgets/inputmappingdialog.h"
#include "voreen/qt/networkeditor/networkeditor.h"

#include <QString>

namespace voreen {

const std::string VoreenVisualization::loggerCat_ = "voreenve.VoreenVisualization";

VoreenVisualization::VoreenVisualization(tgt::GLCanvas* sharedContext)
    : QObject()
    , evaluator_(new NetworkEvaluator(true, sharedContext))
    , workspace_(new Workspace(sharedContext))
    , sharedContext_(sharedContext)
    , networkEditorWidget_(0)
    , propertyListWidget_(0)
    , processorListWidget_(0)
    , renderTargetViewer_(0)
    , volumeViewer_(0)
    , inputMappingDialog_(0)
    , readOnlyWorkspace_(false)
    , modified_(false)
{
    // assign network evaluator to application
    tgtAssert(evaluator_, "no network evaluator");
    VoreenApplication::app()->registerNetworkEvaluator(evaluator_);
    evaluator_->addObserver(static_cast<NetworkEvaluatorObserver*>(this));
}

VoreenVisualization::~VoreenVisualization() {
    evaluator_->setProcessorNetwork(0);
    delete evaluator_;

    workspace_->clear();
    delete workspace_;
}

void VoreenVisualization::createConnections() {
    // create connections
    if (networkEditorWidget_) {
        if (processorListWidget_)  {
            connect(networkEditorWidget_, SIGNAL(processorsSelected(const QList<Processor*>&)),
                    processorListWidget_, SLOT(processorsSelected(const QList<Processor*>&)));
        }
        else {
            LWARNINGC("VoreenVisualization", "init(): ProcessorListWidget not assigned");
        }

        if (propertyListWidget_) {
            connect(networkEditorWidget_, SIGNAL(processorsSelected(const QList<Processor*>&)),
                    propertyListWidget_, SLOT(processorsSelected(const QList<Processor*>&)));
            connect(propertyListWidget_, SIGNAL(modified()), this, SLOT(setModified()));
        }
        else {
            LWARNINGC("VoreenVisualization", "init(): PropertyListWidget not assigned");
        }

        if (renderTargetViewer_) {
            connect(networkEditorWidget_, SIGNAL(processorsSelected(const QList<Processor*>&)),
                renderTargetViewer_, SLOT(processorsSelected(const QList<Processor*>&)));
        }
        else {
            LWARNINGC("VoreenVisualization", "init(): RenderTargetViewer not assigned");
        }

    }
    else {
        LWARNINGC("VoreenVisualization", "init(): Network editor not assigned");
    }
}

void VoreenVisualization::setNetworkEditorWidget(NetworkEditor* networkEditorWidget) {
    networkEditorWidget_ = networkEditorWidget;
}

void VoreenVisualization::setRenderTargetViewer(RenderTargetViewer* renderTargetViewer) {
    renderTargetViewer_ = renderTargetViewer;
}

void VoreenVisualization::setPropertyListWidget(PropertyListWidget* propertyListWidget) {
    propertyListWidget_ = propertyListWidget;
}

void VoreenVisualization::setVolumeViewer(VolumeViewer* volumeViewer) {
    volumeViewer_ = volumeViewer;
}

void VoreenVisualization::setInputMappingDialog(InputMappingDialog* inputMappingDialog) {
    inputMappingDialog_ = inputMappingDialog;
}

void VoreenVisualization::importNetwork(const QString& filename)
    throw (SerializationException)
{
    NetworkSerializer networkSerializer;
    ProcessorNetwork* net = networkSerializer.readNetworkFromFile(filename.toStdString());
    evaluator_->setProcessorNetwork(0);
    delete workspace_->getProcessorNetwork();

    workspace_->setProcessorNetwork(net);
    evaluator_->setProcessorNetwork(workspace_->getProcessorNetwork());
}

void VoreenVisualization::exportNetwork(const QString& filename)
    throw (SerializationException)
{
    try {
        NetworkSerializer().writeNetworkToFile(workspace_->getProcessorNetwork(), filename.toStdString());
    } catch (SerializationException&) {
//        delete processorNetwork_;
        throw;
    }
}

void VoreenVisualization::newWorkspace() {
    tgtAssert(evaluator_, "No network evaluator");

    readOnlyWorkspace_ = false;

    blockSignals(true);

    // clear workspace resources
    evaluator_->unlock();
    evaluator_->setProcessorNetwork(0);
    workspace_->clear();

    // generate new resources
    workspace_->setProcessorNetwork(new ProcessorNetwork());

    blockSignals(false);

    // assign new network
    evaluator_->setProcessorNetwork(workspace_->getProcessorNetwork());
    if (workspace_->hasDescription())
        propagateWorkspaceDescription(workspace_->getDescription());
}

void VoreenVisualization::openWorkspace(const QString& filename, const QString& workDir) throw (SerializationException) {

    LINFO("Loading workspace " << tgt::FileSystem::absolutePath(filename.toStdString()));
    if (!workDir.isEmpty())
        LINFO("Workspace working path: " << tgt::FileSystem::cleanupPath(workDir.toStdString()));

    blockSignals(true);

    // clear workspace resources
    evaluator_->unlock();
    evaluator_->setProcessorNetwork(0);
    workspace_->clear();

    blockSignals(false);

    try {
        workspace_->load(filename.toStdString(), workDir.toStdString());
    }
    catch (SerializationException& e) {
        LERROR("Could not open workspace: " << e.what());
        throw;
    }

    workspaceErrors_ = workspace_->getErrors();
    readOnlyWorkspace_ = workspace_->readOnly();

    // assign new network
    evaluator_->setProcessorNetwork(workspace_->getProcessorNetwork());
    propagateWorkspaceDescription(workspace_->getDescription());
}

void VoreenVisualization::saveWorkspace(const QString& filename, bool overwrite, const QString& workDir) throw (SerializationException) {

    if (workDir.isEmpty())
        LINFO("Saving workspace to " << tgt::FileSystem::cleanupPath(filename.toStdString()));
    else
        LINFO("Saving workspace to " << tgt::FileSystem::cleanupPath(filename.toStdString()) << " with working path " << tgt::FileSystem::cleanupPath(workDir.toStdString()));

    try {
        readOnlyWorkspace_ = false;
        workspace_->save(filename.toStdString(), overwrite, workDir.toStdString());
    }
    catch (SerializationException& e) {
        LERROR("Could not save workspace: " << e.what());
        throw;
    }
}

void VoreenVisualization::cleanupTempFiles(std::vector<std::string> tmpFiles, std::string tmpPath) {
    if ((tmpFiles.empty() == true) || (tmpPath.empty() == true))
        return;

    // Delete all temporary files.
    for (size_t i = 0; i < tmpFiles.size(); ++i) {
        tgt::FileSystem::deleteFile(tmpPath + "/" + tmpFiles[i]);

        // Remove file name to obtain the remaing path
        size_t pos = tmpFiles[i].rfind("/");
        if (pos == std::string::npos)
            pos = 0;
        tmpFiles[i] = tmpFiles[i].substr(0, pos);
    }

    // Now, the directory part of the temp. file is either empty,
    // if it has been created by the during the import, or it is not.
    // If it is not empty, the directory path will then be deleted piecewise,
    // and the remaining "file name" will be reduced until all file names are
    // empty, e.g. "/data/foo" => "/data" => "".
    bool foundNonEmptyString = true;
    while (foundNonEmptyString == true) {
        foundNonEmptyString = false;
        for (size_t i = 0; i < tmpFiles.size(); ++i) {
            if (tmpFiles[i].empty() == true)
                continue;

            foundNonEmptyString = true;
            tgt::FileSystem::deleteDirectory(tmpPath + "/" + tmpFiles[i]);

            // Reduce remaining path
            //
            size_t pos = tmpFiles[i].rfind("/");
            if (pos == std::string::npos)
                pos = 0;
            tmpFiles[i] = tmpFiles[i].substr(0, pos);
        }   // for (i
    }   // while (
}

void VoreenVisualization::networkAssigned(ProcessorNetwork* network, ProcessorNetwork* /*previousNetwork*/) {

    if (network) {
        network->addObserver(static_cast<ProcessorNetworkObserver*>(this));
    }

    if (propertyListWidget_) {
        propertyListWidget_->setProcessorNetwork(network);
        qApp->processEvents();
    }

    if (networkEditorWidget_) {
        networkEditorWidget_->setProcessorNetwork(network);
        qApp->processEvents();
    }

    // assign network to evaluator, also initializes the network
    //evaluator_->setProcessorNetwork(network);
    //qApp->processEvents();

    if (networkEditorWidget_ && network) {
        networkEditorWidget_->selectPreviouslySelectedProcessors();
        qApp->processEvents();
    }

    // assign network to input mapping dialog
    if (inputMappingDialog_) {
        inputMappingDialog_->setProcessorNetwork(network);
        qApp->processEvents();
    }

    sharedContext_->getGLFocus();
    emit(newNetwork(network));
}

void VoreenVisualization::propagateWorkspaceDescription(const std::string& description) {
    // TODO: what to do with the description?
    if (!description.empty())
        LINFO(description);
}

std::vector<std::string> VoreenVisualization::getNetworkErrors() {
    if (workspace_->getProcessorNetwork())
        return workspace_->getProcessorNetwork()->getErrors();
    else
        return std::vector<std::string>();
}

bool VoreenVisualization::rebuildShaders() {
    if (sharedContext_)
        sharedContext_->getGLFocus();
    else
        LWARNING("No shared context object");

    bool allSuccessful = true;

    std::vector<Processor*> procs = workspace_->getProcessorNetwork()->getProcessors();
    for(size_t i = 0; i < procs.size(); i++) {
        std::vector<ShaderProperty*> props = procs.at(i)->getPropertiesByType<ShaderProperty>();
        for(size_t j = 0; j < props.size(); j++) {
            if(!props.at(j)->rebuild())
                allSuccessful = false;
        }
    }

    if (!ShdrMgr.rebuildAllShadersFromFile())
        allSuccessful = false;

    if (allSuccessful) {
        evaluator_->invalidateProcessors();
        return true;
    }
    else {
        return false;
    }
}

void VoreenVisualization::networkChanged() {
    setModified();
    emit networkModified(workspace_->getProcessorNetwork());
}

void VoreenVisualization::processorAdded(const Processor* /*processor*/) {
    networkChanged();
}

void VoreenVisualization::processorRemoved(const Processor* /*processor*/) {
    networkChanged();
}

std::vector<std::string> VoreenVisualization::getWorkspaceErrors() {
    return workspaceErrors_;
}

void VoreenVisualization::setProcessorListWidget(ProcessorListWidget* processorListWidget) {
    processorListWidget_ = processorListWidget;
}

NetworkEvaluator* VoreenVisualization::getEvaluator() const {
    return evaluator_;
}

Workspace* VoreenVisualization::getWorkspace() const {
    return workspace_;
}

bool VoreenVisualization::isModified() const {
    return modified_;
}

void VoreenVisualization::setModified(bool isModified) {
    if (modified_ != isModified) {
        modified_ = isModified;
        if (isModified)
            emit modified();
    }
}

} // namespace
