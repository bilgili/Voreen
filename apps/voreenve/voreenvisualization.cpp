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

#include "voreenvisualization.h"

#include "tgt/gpucapabilities.h"
#include "tgt/ziparchive.h"

#include "voreen/core/network/workspace.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/datastructures/volume/volumecontainer.h"
#include "voreen/core/properties/link/linkevaluatorid.h"

#include "voreen/qt/voreenapplicationqt.h"

#include "voreen/qt/widgets/volumecontainerwidget.h"
#include "voreen/qt/widgets/processor/qprocessorwidgetfactory.h"
#include "voreen/qt/widgets/processorlistwidget.h"
#include "voreen/qt/widgets/propertylistwidget.h"
#include "networkeditor/networkeditor.h"
#include "voreen/qt/widgets/inputmappingdialog.h"

#include <QString>

namespace voreen {

const std::string VoreenVisualization::loggerCat_ = "voreenve.VoreenVisualization";

VoreenVisualization::VoreenVisualization(tgt::GLCanvas* sharedContext)
    : QObject()
    , evaluator_(new NetworkEvaluator(sharedContext))
    , workspace_(new Workspace(sharedContext))
    , sharedContext_(sharedContext)
    , networkEditorWidget_(0)
    , propertyListWidget_(0)
    , processorListWidget_(0)
    , inputMappingDialog_(0)
    , readOnlyWorkspace_(false)
    , modified_(false)
{
    // create Qt-specific factory for processor widgets (used by Processor::initialize)
    VoreenApplication::app()->setProcessorWidgetFactory(
        new QProcessorWidgetFactory(VoreenApplicationQt::qtApp()->getMainWindow(), evaluator_));

    // assign network evaluator to application
    VoreenApplication::app()->setNetworkEvaluator(evaluator_);
}

VoreenVisualization::~VoreenVisualization() {

    propagateVolumeContainer(0);
    propagateNetwork(0);
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
            connect(networkEditorWidget_, SIGNAL(processorsSelected(const QList<Processor*>&)),
                    propertyListWidget_, SLOT(processorsSelected(const QList<Processor*>&)));
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

void VoreenVisualization::setInputMappingDialog(InputMappingDialog* inputMappingDialog) {
    inputMappingDialog_ = inputMappingDialog;
}

void VoreenVisualization::importNetwork(const QString& filename)
    throw (SerializationException)
{
    NetworkSerializer networkSerializer;
    ProcessorNetwork* net = networkSerializer.readNetworkFromFile(filename.toStdString());
    propagateNetwork(0);
    delete workspace_->getProcessorNetwork();
    workspace_->setProcessorNetwork(net);
    propagateVolumeContainer(workspace_->getVolumeContainer());
    propagateNetwork(workspace_->getProcessorNetwork());
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

void VoreenVisualization::exportWorkspaceToZipArchive(const QString& filename, bool overwrite)
    throw (SerializationException)
{
    LINFO("Exporting workspace " << filename.toStdString() << " to archive " << filename.toStdString());
    workspace_->exportToZipArchive(filename.toStdString(), overwrite);
}

#ifdef VRN_WITH_ZLIB
QString VoreenVisualization::extractWorkspaceArchive(const QString& archiveName, const QString& path,
                                                     bool overwrite)
    throw (SerializationException)
{

    LINFO("Extracting workspace archive " << archiveName.toStdString() << " to " << path.toStdString());

    // Open archive and detect contained files
    tgt::ZipArchive zip(archiveName.toStdString());
    if (!zip.archiveExists()) {
        LERROR("Archive does not exist: " << archiveName.toStdString());
        throw SerializationException("Archive does not exist:\n" + archiveName.toStdString());
    }

    std::vector<std::string> archiveFiles = zip.getContainedFileNames();
    if (archiveFiles.empty()) {
        LERROR("Archive does not contain any files: " << archiveName.toStdString());
        throw SerializationException("Archive does not contain any files:\n" + archiveName.toStdString());
    }

    // Extract files from the archive
    std::string currentDir = tgt::FileSystem::currentDirectory();
    if (tgt::FileSystem::changeDirectory(path.toStdString()) == false) {
        LERROR("Failed to locate target directory " << path.toStdString());
        throw SerializationException("Failed to open target directory:\n" + path.toStdString());
    }

    // prepare progress dialog
    ProgressBar* progressDialog = VoreenApplicationQt::app()->createProgressDialog();
    tgtAssert(progressDialog, "no progress dialog created");
    progressDialog->setTitle("Extract Archive '" + tgt::FileSystem::fileName(archiveName.toStdString()) + "'");
    progressDialog->show();
    qApp->setOverrideCursor(Qt::WaitCursor);
    qApp->processEvents();

    // extract files
    size_t numFiles = 0;
    for (std::vector<std::string>::const_iterator it = archiveFiles.begin(); it != archiveFiles.end(); ++it) {
        progressDialog->setMessage("Extracting '" + *it + "' ...");
        progressDialog->setProgress((float)numFiles / (archiveFiles.size() - 1));
        qApp->processEvents();
        tgt::File* xFile = zip.extractFile(*it, tgt::ZipArchive::TARGET_DISK, "", true, overwrite);
        if (xFile) {
            xFile->close();
            delete xFile;
            ++numFiles;
        }
    }

    // clean up
    progressDialog->hide();
    delete progressDialog;
    qApp->restoreOverrideCursor();
    qApp->processEvents();
    tgt::FileSystem::changeDirectory(currentDir);

    if (numFiles != zip.getNumFilesInArchive()) {
        LWARNING("Some files could not be extracted.");
        //throw SerializationException("Some files could not be extracted from '" + archiveName + "' to " + path);
    }

    // detect workspace file in extracted archive files
    size_t index;
    for (index=0; index < archiveFiles.size(); ++index) {
        std::string fileExt = tgt::FileSystem::fileExtension(archiveFiles[index]);
        if (fileExt == "vws")
            break;
    }

    // success: return workspace file path
    if (index < archiveFiles.size()) {
        return path + "/" + QString::fromStdString(archiveFiles[index]);
    }
    // workspace file not found: remove extracted files and throw exception
    else {
        LERROR("No workspace file (*.vws) found in archive: " << archiveName.toStdString() << ". Extracted files are removed.");
        cleanupTempFiles(archiveFiles, path.toStdString());
        throw SerializationException("No workspace file (*.vws) found in archive:\n" + archiveName.toStdString());
    }
}
#else
QString VoreenVisualization::extractWorkspaceArchive(const QString& /*archiveName*/, const QString& /*path*/,
                                                bool /*overwrite*/)
    throw (SerializationException)
{
    throw SerializationException("The application has been compiled without Zip support.");
}
#endif

void VoreenVisualization::newWorkspace() {
    tgtAssert(evaluator_, "No network evaluator");

    readOnlyWorkspace_ = false;

    blockSignals(true);

    // clear workspace resources
    evaluator_->unlock();
    propagateVolumeContainer(0);
    propagateNetwork(0);
    workspace_->clear();

    // generate new resources
    workspace_->setProcessorNetwork(new ProcessorNetwork());
    workspace_->setVolumeContainer(new VolumeContainer());

    blockSignals(false);

    // propagate resources
    propagateVolumeContainer(workspace_->getVolumeContainer());
    propagateNetwork(workspace_->getProcessorNetwork());
}

void VoreenVisualization::openWorkspace(const QString& filename) throw (SerializationException) {

    LINFO("Loading workspace " << tgt::FileSystem::absolutePath(filename.toStdString()));
    if (!VoreenApplication::app()->getProcessorWidgetFactory())
        LWARNING("No ProcessorWidgetFactory assigned to VoreenApplication: No ProcessorWidgets are generated!");

    blockSignals(true);

    // clear workspace resources
    evaluator_->unlock();
    propagateVolumeContainer(0);
    propagateNetwork(0);
    workspace_->clear();

    blockSignals(false);

    try {
        workspace_->load(filename.toStdString());
    }
    catch (SerializationException& e) {
        LERROR("Could not open workspace: " << e.what());
        throw;
    }

    workspaceErrors_ = workspace_->getErrors();
    readOnlyWorkspace_ = workspace_->readOnly();

    propagateVolumeContainer(workspace_->getVolumeContainer());
    propagateNetwork(workspace_->getProcessorNetwork());
}

void VoreenVisualization::saveWorkspace(const QString& filename, bool overwrite) throw (SerializationException) {

    try {
        // zipped workspace?
        if (filename.endsWith(".zip", Qt::CaseInsensitive))
            exportWorkspaceToZipArchive(filename, overwrite);
        else {
            LINFO("Saving workspace to " << filename.toStdString());
            readOnlyWorkspace_ = false;
            workspace_->save(filename.toStdString(), overwrite);
        }
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

void VoreenVisualization::propagateNetwork(ProcessorNetwork* network) {

    if (network) {
        network->addObserver(this);
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
    evaluator_->setProcessorNetwork(network);
    qApp->processEvents();

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

void VoreenVisualization::propagateVolumeContainer(VolumeContainer* container) {
    if (volumeContainerWidget_)
        volumeContainerWidget_->setVolumeContainer(container);

    if (propertyListWidget_)
        propertyListWidget_->setVolumeContainer(container);
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
