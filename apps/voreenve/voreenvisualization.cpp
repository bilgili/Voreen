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

#include "voreenvisualization.h"

#include "tgt/gpucapabilities.h"
#include "tgt/ziparchive.h"

#include "voreen/core/vis/workspace/workspace.h"
#include "voreen/core/vis/network/networkevaluator.h"
#include "voreen/core/volume/volumecontainer.h"
#include "voreen/core/vis/properties/link/linkevaluatorid.h"

#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/qt/widgets/animationplugin.h"
#include "voreen/qt/widgets/volumecontainerwidget.h"
#include "voreen/qt/widgets/animationplugin.h"
#include "voreen/qt/widgets/processor/qprocessorwidgetfactory.h"

#include "voreen/qt/widgets/network/processorlistwidget.h"
#include "voreen/qt/widgets/network/propertylistwidget.h"
#include "voreen/qt/widgets/network/editor/networkeditor.h"

#include <QString>

namespace voreen {

const std::string VoreenVisualization::loggerCat_ = "voreenve.VoreenVisualization";

VoreenVisualization::VoreenVisualization(tgt::GLCanvas* sharedContext)
    : QObject()
    , evaluator_(new NetworkEvaluator(sharedContext))
    , workspace_(new Workspace())
    , sharedContext_(sharedContext)
    , networkEditorWidget_(0)
    , propertyListWidget_(0)
    , processorListWidget_(0)
    , animationPlugin_(0)
    , readOnlyWorkspace_(false)
    , modified_(false)
{
    // create Qt-specific factory for processor widgets (used by Processor::initialize)
    VoreenApplication::app()->setProcessorWidgetFactory(
        new QProcessorWidgetFactory(VoreenApplicationQt::qtApp()->getMainWindow(), evaluator_));
}

VoreenVisualization::~VoreenVisualization() {
    cleanupTempFiles();
    delete evaluator_;
    delete workspace_;
}

void VoreenVisualization::createConnections() {

    // create connections
    if (networkEditorWidget_) {
        if (processorListWidget_)  {
            connect(networkEditorWidget_, SIGNAL(processorsSelected(const std::vector<Processor*>&)),
                    processorListWidget_, SLOT(processorsSelected(const std::vector<Processor*>&)));
            connect(networkEditorWidget_, SIGNAL(processorsSelected(const std::vector<Processor*>&)),
                    propertyListWidget_, SLOT(processorsSelected(const std::vector<Processor*>&)));
            connect(propertyListWidget_, SIGNAL(modified()), this, SLOT(setModified()));
        }
        else 
            LWARNINGC("VoreenVisualization", "init(): PropertyListWidget not assigned");
    }
    else {
        LWARNINGC("VoreenVisualization", "init(): Network editor not assigned");
    }
}

void VoreenVisualization::setNetworkEditorWidget(NetworkEditor* networkEditorWidget) {
    networkEditorWidget_ = networkEditorWidget;
}

void VoreenVisualization::setPropertyListWidget(PropertyListWidget* propertyListWidget) {
    propertyListWidget_ = propertyListWidget;
}

void VoreenVisualization::setVolumeContainerWidget(VolumeContainerWidget* volumeContainerWidget) {
    volumeContainerWidget_ = volumeContainerWidget;
}

void VoreenVisualization::openNetwork(const std::string& filename)
    throw (SerializationException)
{
    NetworkSerializer networkSerializer;
    ProcessorNetwork* net = networkSerializer.readNetworkFromFile(filename);
    delete workspace_->getProcessorNetwork();
    workspace_->setProcessorNetwork(net);
    propagateVolumeContainer();
    propagateNetwork();

}

void VoreenVisualization::saveNetwork(const std::string& filename, bool reuseTCTargets)
    throw (SerializationException)
{
    workspace_->getProcessorNetwork()->setReuseTargets(reuseTCTargets);

    try {
        NetworkSerializer().writeNetworkToFile(workspace_->getProcessorNetwork(), filename);
    } catch (SerializationException&) {
//        delete processorNetwork_;
        throw;
    }
}

void VoreenVisualization::exportWorkspaceAsZip(const std::string& filename, bool overwrite)
    throw (SerializationException)
{
    const bool workspaceWasReadOnly = readOnlyWorkspace_;
    readOnlyWorkspace_ = false;
    workspace_->exportZipped(filename, overwrite);
    readOnlyWorkspace_ = workspaceWasReadOnly;
}

#ifdef VRN_WITH_ZLIB
void VoreenVisualization::importZippedWorkspace(const std::string& archiveName, const std::string& path,
                                                bool deflateTemporarily)
    throw (SerializationException)
{

    LINFO("Importing zipped workspace: " << archiveName);

    tgt::ZipArchive zip(archiveName);
    if (!zip.archiveExists()) {
        LERROR("Archive does not exist: " << archiveName);
        throw SerializationException("Archive does not exist: " + archiveName);
        return;
    }

    // Extract files from the archive and enforce overwriting existing ones only
    // if they are extracted for temporary purpose.
    //
    size_t numFiles = zip.extractFilesToDirectory(path, deflateTemporarily);
    if (numFiles != zip.getNumFilesInArchive()) {
        LERROR("An error occured while extracting files from '" << archiveName << "' to " << path << "!");
        LERROR("Aborting.");
        throw SerializationException("An error occured while extracting files from '" + archiveName + "' to " + path + "!");
        return;
    }

    if (numFiles == 0) {
        LWARNING("Archive does not contain any files: " << archiveName);
        throw SerializationException("Archive does not contain any files: " + archiveName);
        return;
    }

    std::vector<std::string> files = zip.getContainedFileNames();
    size_t index = 0;
    for ( ; index < files.size(); ++index) {
        std::string fileExt = tgt::FileSystem::fileExtension(files[index]);
        if (fileExt == "vws")
            break;
    }

    if (index < files.size()) {
        try {
            openWorkspace(QString::fromStdString(path + "/" + files[index]));
            if (deflateTemporarily)
                workspace_->setFilename(archiveName);
        }
        catch (SerializationException& e) {
            LERROR("Failed to import workspace '" << (path + "/" + files[index]) << "'!");
            LERROR("Reason: " << e.what());
            throw e;
        }
    } else {
        if (deflateTemporarily) {
            tmpPath_ = path;
            tmpFiles_ = files;
        }
        LERROR("No workspace file (*.vws) found in archive: " << archiveName);
        throw SerializationException("No workspace file (*.vws) found in archive " + archiveName);
    }

    // delete extracted files
    if (deflateTemporarily) {
        tmpPath_ = path;
        tmpFiles_ = files;
    }
}
#else
void VoreenVisualization::importZippedWorkspace(const std::string& /*archiveName*/, const std::string& /*path*/,
                                                bool /*deflateTemporarily*/)
    throw (SerializationException)
{
}
#endif

void VoreenVisualization::newWorkspace() {
    cleanupTempFiles();

    readOnlyWorkspace_ = false;

    // clear workspace resources and new ones
    sharedContext_->getGLFocus();
    workspace_->clear();
    workspace_->setProcessorNetwork(new ProcessorNetwork());
    workspace_->setVolumeContainer(new VolumeContainer());

    propagateVolumeContainer();
    propagateNetwork();
}

void VoreenVisualization::openWorkspace(const QString& filename)
    throw (SerializationException)
{
    LINFO("Loading workspace " << filename.toStdString());
    if (!VoreenApplication::app()->getProcessorWidgetFactory())
        LWARNING("No ProcessorWidgetFactory assigned to VoreenApplication: No ProcessorWidgets are generated!");

    cleanupTempFiles();

    sharedContext_->getGLFocus();
    // zipped workspace?
    if (filename.endsWith(".zip", Qt::CaseInsensitive))
        importZippedWorkspace(filename.toStdString(), VoreenApplication::app()->getTemporaryPath(), true);
    else
        workspace_->load(filename.toStdString());

    workspaceErrors_ = workspace_->getErrors();
    readOnlyWorkspace_ = workspace_->readOnly();

    propagateVolumeContainer();
    propagateNetwork();
}

void VoreenVisualization::saveWorkspace(const std::string& filename, bool reuseTCTargets, bool overwrite)
    throw (SerializationException)
{
    readOnlyWorkspace_ = false;
    workspace_->getProcessorNetwork()->setReuseTargets(reuseTCTargets);

    // zipped workspace?
    if (QString::fromStdString(filename).endsWith(".zip", Qt::CaseInsensitive))
        exportWorkspaceAsZip(filename, overwrite);
    else
        workspace_->save(filename, overwrite);
}

void VoreenVisualization::cleanupTempFiles() {
    if ((tmpFiles_.empty() == true) || (tmpPath_.empty() == true))
        return;

    // Delete all temporary files.
    //
    for (size_t i = 0; i < tmpFiles_.size(); ++i) {
        tgt::FileSystem::deleteFile(tmpPath_ + "/" + tmpFiles_[i]);

        // Remove file name to obtain the remaing path
        //
        size_t pos = tmpFiles_[i].rfind("/");
        if (pos == std::string::npos)
            pos = 0;
        tmpFiles_[i] = tmpFiles_[i].substr(0, pos);
    }

    // Now, the directory part of the temp. file is either empty,
    // if it has been created by the during the import, or it is not.
    // If it is not empty, the directory path will then be deleted piecewise,
    // and the remaining "file name" will be reduced until all file names are
    // empty, e.g. "/data/foo" => "/data" => "".
    //
    bool foundNonEmptyString = true;
    while (foundNonEmptyString == true) {
        foundNonEmptyString = false;
        for (size_t i = 0; i < tmpFiles_.size(); ++i) {
            if (tmpFiles_[i].empty() == true)
                continue;

            foundNonEmptyString = true;
            tgt::FileSystem::deleteDirectory(tmpPath_ + "/" + tmpFiles_[i]);

            // Reduce remaining path
            //
            size_t pos = tmpFiles_[i].rfind("/");
            if (pos == std::string::npos)
                pos = 0;
            tmpFiles_[i] = tmpFiles_[i].substr(0, pos);
        }   // for (i
    }   // while (

    tmpFiles_.clear();
    tmpPath_.clear();
}

void VoreenVisualization::propagateNetwork() {

    tgtAssert(workspace_, "No workspace");

    if (workspace_->getProcessorNetwork()) {
        workspace_->getProcessorNetwork()->addObserver(this);
    }

    // assign network to network editor
    if (networkEditorWidget_) {
        networkEditorWidget_->newNetwork();
        qApp->processEvents();
    }

    // assign network to evaluator, also initializes the network
    evaluator_->setProcessorNetwork(workspace_->getProcessorNetwork());
    qApp->processEvents();

    // assign network to propertylist widget at last, since
    // property widget generation might be time consuming
    if (propertyListWidget_) {
        propertyListWidget_->setNetwork(workspace_->getProcessorNetwork());
        qApp->processEvents();
    }

    // propagate new network to AnimationPlugin
    if (animationPlugin_)
        animationPlugin_->setNetwork(workspace_->getProcessorNetwork());

    sharedContext_->getGLFocus();

    emit newNetwork(workspace_->getProcessorNetwork());
}

void VoreenVisualization::propagateVolumeContainer() {
    if (volumeContainerWidget_)
        volumeContainerWidget_->setVolumeContainer(workspace_->getVolumeContainer());

    if (propertyListWidget_)
        propertyListWidget_->setVolumeContainer(workspace_->getVolumeContainer());
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

    if (ShdrMgr.rebuildAllShadersFromFile()) {
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

void VoreenVisualization::setAnimationPlugin(AnimationPlugin* animationPlugin) {
    animationPlugin_ = animationPlugin;
}

NetworkEvaluator* VoreenVisualization::getEvaluator() const {
    return evaluator_;
}

Workspace* VoreenVisualization::getWorkspace() const {
    return workspace_;
}

VolumeContainer* VoreenVisualization::getVolumeContainer() const {
    return workspace_->getVolumeContainer();
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
