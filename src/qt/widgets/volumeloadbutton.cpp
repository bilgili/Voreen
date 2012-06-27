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

#include <QAction>
#include <QApplication>
#include <QErrorMessage>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QUrl>

#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/datastructures/volume/volumecontainer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/voreenapplication.h"

#include "voreen/qt/widgets/rawvolumewidget.h"
#include "voreen/qt/widgets/volumeloadbutton.h"

#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/qt/progressdialog.h"

#ifdef _MSC_VER
#include "tgt/gpucapabilitieswindows.h"
#endif

namespace voreen {

const std::string VolumeLoadButton::loggerCat_("voreen.qt.VolumeContainerWidget");

VolumeLoadButton::VolumeLoadButton(VolumeContainer* vc, QWidget* parent)
    : QToolButton(parent)
    , volumeContainer_(vc)
    , rawVolumeFilesFilter_("Raw Volume Files (*)")
    , philipsUSFilesFilter_("Philips US Files (*.dcm)")
    , rawSliceFilesFilter_("Raw Slice Files (*)")
#ifdef VRN_MODULE_DICOM
    , dicomDirDialog_(0)
#endif
{
    setMinimumWidth(100);
    setText(tr("Load Volume"));
    setIcon(QPixmap(":/voreenve/icons/open-volume.png"));
    volumeFilesFilter_ = QString::fromStdString(getExtensions());
    selectedFilter_ = volumeFilesFilter_;

    loadDatAction_ = new QAction("Load Volume", this);
    loadRawAction_ = new QAction("Load Raw Volume", this);
    #ifdef VRN_MODULE_DICOM
        loadDicomAction_ = new QAction("Load DICOM Slices", this);
    #endif
    QMenu* loadMenu = new QMenu(this);
    loadMenu->addAction(loadDatAction_);
    loadMenu->addAction(loadRawAction_);
    #ifdef VRN_MODULE_DICOM
        loadMenu->addAction(loadDicomAction_);
    #endif
    setMenu(loadMenu);
    setPopupMode(QToolButton::MenuButtonPopup);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(loadMenu, SIGNAL(triggered(QAction*)), this, SLOT(loadActionTriggered(QAction*)));
    connect(this, SIGNAL(clicked()), this, SLOT(loadVolume()));
}

void VolumeLoadButton::loadRawVolume(const std::string&  filenameStd) {

    QString filename = QString::fromStdString(filenameStd);
    std::string objectModel;
    std::string format;
    tgt::ivec3 dim;
    tgt::vec3 spacing;
    int headerSkip;
    bool bigEndian;
    tgt::mat4 trafoMat = tgt::mat4::identity;

    if (!filename.isEmpty()){
        RawVolumeWidget* rawVW = new RawVolumeWidget(this, tr("Please enter the properties for <br><strong>")+filename+"</strong>",
            objectModel, format, dim, spacing, headerSkip, bigEndian, trafoMat);
        if (rawVW->exec() == QDialog::Accepted) {

            // dervive expected file size from provided properties
            uint formatBytes = 1;;
            if (format == "USHORT" || format == "USHORT_12" || format == "SHORT")
                formatBytes = 2;
            else if (format == "FLOAT8" || format == "FLOAT16" || format == "FLOAT")
                formatBytes = 4;
            else if (format == "UINT" || format == "INT")
                formatBytes = 4;

            int numChannels = 1;
            if (objectModel == "RGB")
                numChannels = 3;
            else if (objectModel == "RGBA")
                numChannels = 4;

            uint rawSize = headerSkip + formatBytes * numChannels * (dim.x * dim.y * dim.z);

            // inform/query user, if file size does not match
            if (QFile(filename).size() != rawSize) {
                QMessageBox::StandardButton retButton = QMessageBox::Yes;
                if(QFile(filename).size() > rawSize) {
                    QString msg = tr("The provided properties result in a size smaller\nthan the actual file size. Do you want to continue?");
                    retButton = QMessageBox::question(this, tr("Size mismatch"), msg,
                        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
                }
                else if (QFile(filename).size() < rawSize) {
                    QString msg = tr("The provided properties result in a size\ngreater than the actual file size.");
                    retButton = QMessageBox::warning(this, tr("Size mismatch"), msg,
                        QMessageBox::Cancel);
                }
                if (retButton != QMessageBox::Yes && retButton != QMessageBox::Ok)
                    return;
            }

            qApp->processEvents();
            VolumeHandle* vol;
            try {
                vol = volumeContainer_->loadRawVolume(filename.toStdString(), objectModel, format, dim, spacing,
                    headerSkip, bigEndian);
                vol->getVolume()->setTransformation(trafoMat);
            }
            catch (tgt::FileException& e) {
                LERROR(e.what());
                QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
                errorMessageDialog->showMessage(e.what());
            }
            catch (std::bad_alloc&) {
                LERROR("std::Error BAD ALLOCATION");
                QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
                errorMessageDialog->showMessage("std::Error BAD ALLOCATION, File: " + filename);
            }
        }
    }
    emit VolumeAdded(volumeContainer_->size());
}

void VolumeLoadButton::loadRawSlices(std::vector<std::string> sliceFiles){

    tgtAssert(!sliceFiles.empty(), "No slice files");

    QString filename = QString::fromStdString(sliceFiles[0]);
    std::string objectModel;
    std::string format;
    tgt::ivec3 dim;
    tgt::vec3 spacing;
    int headerSkip;
    bool bigEndian;
    tgt::mat4 trafoMat = tgt::mat4::identity;

    if (!sliceFiles.empty()){
        RawVolumeWidget* rawVW = new RawVolumeWidget(this, tr("Please enter the properties for slice stack<br><strong>")
                                                     + QString::fromStdString(sliceFiles[0])+"</strong>",
            objectModel, format, dim, spacing, headerSkip, bigEndian, trafoMat, sliceFiles.size());
        if (rawVW->exec() == QDialog::Accepted) {

            // dervive expected file size from provided properties
            uint formatBytes = 1;;
            if (format == "USHORT" || format == "USHORT_12" || format == "SHORT")
                formatBytes = 2;
            else if (format == "FLOAT8" || format == "FLOAT16" || format == "FLOAT")
                formatBytes = 4;
            else if (format == "UINT" || format == "INT")
                formatBytes = 4;

            //int numChannels = 1;
            //if (objectModel == "RGB")
            //    numChannels = 3;
            //else if (objectModel == "RGBA")
            //    numChannels = 4;

            uint rawSize = headerSkip + formatBytes * (dim.x * dim.y);

            // inform/query user, if file size does not match
            if (QFile(filename).size() != rawSize) {
                QMessageBox::StandardButton retButton = QMessageBox::Yes;
                if(QFile(filename).size() > rawSize) {
                    QString msg = tr("The provided properties result in a size smaller\nthan the actual filesize. Do you want to continue?");
                    retButton = QMessageBox::question(this, tr("Size mismatch"), msg,
                        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
                }
                else if (QFile(filename).size() < rawSize) {
                    QString msg = tr("The provided properties result in a size\ngreater than the actual filesize.");
                    retButton = QMessageBox::warning(this, tr("Size mismatch"), msg,
                        QMessageBox::Cancel);
                }
                if (retButton != QMessageBox::Yes && retButton != QMessageBox::Ok)
                    return;
            }

            qApp->processEvents();
            try {
                RawVolumeReader* rawReader = new RawVolumeReader();
                dim.z = 1;
                RawVolumeReader::ReadHints readHints(dim, spacing,  0, objectModel, format, headerSkip, bigEndian);
                rawReader->setReadHints(readHints);
                VolumeHandle* compositedVolume = rawReader->readSliceStack(sliceFiles);
                if (compositedVolume)
                    volumeContainer_->add(compositedVolume);
            }
            catch (tgt::FileException e) {
                LERROR(e.what());
                QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
                errorMessageDialog->showMessage(e.what());
            }
            catch (std::bad_alloc) {
                LERROR("std::Error BAD ALLOCATION");
            }
        }
    }
    emit VolumeAdded(volumeContainer_->size());
}

std::string VolumeLoadButton::getExtensions() const {
    VolumeSerializerPopulator* populator = new VolumeSerializerPopulator();
    std::vector<std::string> extensionVec = populator->getSupportedReadExtensions();
    delete populator;
    std::string out = "Volume Files (";
    for(size_t i = 0; i < extensionVec.size(); i++) {
        out+= "*."+extensionVec[i]+" ";
    }

#ifdef VRN_MODULE_DICOM
    out+=" DICOMDIR";
#endif
    out+=")";
    return out;
}

void VolumeLoadButton::setVolumeContainer(VolumeContainer* volumeContainer) {
    // do not deregister from previously assigned container,
    // since it may not be valid anymore!

    // assign new container
    volumeContainer_ = volumeContainer;
    update();

}

void VolumeLoadButton::loadActionTriggered(QAction* action) {
    if (action == loadDatAction_)
        loadVolume();
    else if (action == loadRawAction_)
        loadVolumeRawFilter();
#ifdef VRN_MODULE_DICOM
    else if (action == loadDicomAction_)
        loadDicomFiles();
#endif
    }

void VolumeLoadButton::loadVolume() {
    std::vector<std::string> files = openFileDialog();
    if (!files.empty())
        addMultipleVolumes(files);
}

void VolumeLoadButton::loadVolumeRawFilter() {
    QString saveFilter = selectedFilter_;
    selectedFilter_ = rawVolumeFilesFilter_;
    loadVolume();
    selectedFilter_ = saveFilter;
}

void VolumeLoadButton::addMultipleVolumes(std::vector<std::string> filenames) {
    if (selectedFilter_ == volumeFilesFilter_) {
    for (size_t i = 0; i < filenames.size(); i++) {
        if (filenames[i] != "") {
            VolumeCollection* collection = 0;
            try {
                // Special handling for bricked data sets
                if (tgt::FileSystem::fileExtension(filenames.at(i), true) == "bvi") {
                    bool ok;
                    int i = QInputDialog::getInteger(this, tr("Bricked Volume"),
                                                     tr("Please specify the maximum amount of <b>main memory</b><br />"
                                                        "to be used for rendering this bricked data set (in MB).<br />"
                                                        "If the value is chosen too large, the application may crash<br />"
                                                        "with an 'out of memory' exception.<br >"
                                                        "Select 'Cancel' if you don't want to load this data set right now."),
                                                     LargeVolumeManager::getMaxMemory(), 10, 16000, 1, &ok);
                    if (ok)
                        LargeVolumeManager::setMaxMemory(i);
                    else
                        return;

                    QString gpuMem;
#ifdef _MSC_VER
                    if (GpuCapsWin.getVideoRamSize() > 100)
                        gpuMem = QString(" (estimated as %1 MB on this<br />"
                                         "system, which may be inaccurate)").arg(LargeVolumeManager::estimateMaxGpuMemory());
#endif

                    i = QInputDialog::getInteger(this, tr("Bricked Volume"),
                                                 tr("Please specify the maximum amount of <b>graphics card</b><br />"
                                                    "<b>memory</b> to be used for rendering this bricked data set (in MB).<br />"
                                                    "This value must be smaller than the chosen amount of main memory and<br />"
                                                    "smaller than the usable free GPU memory%1.<br />"
                                                    "Select 'Cancel' if you don't want to load this data set right now.")
                                                 .arg(gpuMem), i, 0, i, 1, &ok);
                    if (ok)
                        LargeVolumeManager::setMaxGpuMemory(i);
                    else
                        return;
                }

#ifdef VRN_MODULE_DICOM
                if (tgt::FileSystem::fileName(filenames[i]) == "DICOMDIR" || tgt::FileSystem::fileName(filenames[i]) == "dicomdir")
                    loadDicomDir(filenames[i]);
                else
#endif
                    collection = volumeContainer_->loadVolume(filenames.at(i));

                update();
            }
            catch (std::bad_alloc) {
                LERROR("std::Error BAD ALLOCATION");
                QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
                errorMessageDialog->showMessage("std::Error BAD ALLOCATION");
            }
            catch (const std::exception& e) {
                LERROR(e.what());
                QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
                errorMessageDialog->showMessage(e.what());
            }
            delete collection;
        }
    }

    }
    else if (selectedFilter_ == rawVolumeFilesFilter_) {
        for (size_t i = 0; i < filenames.size(); i++) {
            if (filenames[i] != "") {
                loadRawVolume(filenames[i]);
            }
        }

    }
    else if (selectedFilter_ == philipsUSFilesFilter_) {
        // we need to add a protocol prefix for philips us format,
        // since the file ending "dcm" is ambiguous
        for (size_t i = 0; i < filenames.size(); i++) {
            if (filenames[i] != "") {
                try {
                    volumeContainer_->loadVolume("phus://" + filenames.at(i));
                }
                catch (std::bad_alloc) {
                    LERROR("std::Error BAD ALLOCATION");
                    QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
                    errorMessageDialog->showMessage("std::Error BAD ALLOCATION");
                }
                catch (const std::exception& e) {
                    LERROR(e.what());
                    QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
                    errorMessageDialog->showMessage(e.what());
                }
            }
        }
    }
    else if (selectedFilter_ == rawSliceFilesFilter_) {
       if (!filenames.empty()) {
            loadRawSlices(filenames);
       }
    }
    emit VolumeAdded(volumeContainer_->size());
}

std::vector<std::string> VolumeLoadButton::openFileDialog() {
    std::string openPath;
    if (volumeContainer_ && !volumeContainer_->empty()) {
        QFileInfo info(volumeContainer_->at(volumeContainer_->size()-1)->getOrigin().getPath().c_str());
        openPath = info.canonicalPath().toStdString(); // we only want the directory not the filename
    }
    else {
        openPath = VoreenApplication::app()->getVolumePath();
    }
    QDir dir(QString::fromStdString(openPath)); // we want absolute path name to prevent problems when the
                                                // file dialog perform chdir()

    QStringList filters;
    std::string ext = getExtensions();
    if (selectedFilter_ == volumeFilesFilter_)
        filters << volumeFilesFilter_ << rawVolumeFilesFilter_ << philipsUSFilesFilter_ << rawSliceFilesFilter_;
    else if (selectedFilter_ == rawVolumeFilesFilter_)
        filters << rawVolumeFilesFilter_ << volumeFilesFilter_ << philipsUSFilesFilter_ << rawSliceFilesFilter_;
    else if (selectedFilter_ == philipsUSFilesFilter_)
        filters << philipsUSFilesFilter_ << volumeFilesFilter_ << rawVolumeFilesFilter_  << rawSliceFilesFilter_;
    else if (selectedFilter_ == rawSliceFilesFilter_)
        filters << rawSliceFilesFilter_ << volumeFilesFilter_ << rawVolumeFilesFilter_ << philipsUSFilesFilter_;

    QFileDialog dlg(this, tr("Load Volume..."), dir.absolutePath(), "");
    dlg.setNameFilters(filters);
    dlg.setViewMode(QFileDialog::Detail);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);

    connect(&dlg, SIGNAL(filterSelected(QString)), this, SLOT(filterChanged(QString)));

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getVolumePath().c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getDataPath().c_str());
    dlg.setSidebarUrls(urls);

    std::vector<std::string> filenames;
    if (dlg.exec() == QDialog::Accepted) {
        const QStringList& lst = dlg.selectedFiles();
        QStringList::const_iterator it = lst.begin();
        for (; it != lst.end(); ++it)
            filenames.push_back(it->toStdString());
    }

    return filenames;
}

void VolumeLoadButton::filterChanged(QString filter) {
    selectedFilter_ = filter;
}

void VolumeLoadButton::loadDicomFiles() {
#ifdef VRN_MODULE_DICOM
    std::string openPath;
    if (volumeContainer_ && !volumeContainer_->empty())
        openPath = volumeContainer_->at(volumeContainer_->size()-1)->getOrigin().getPath();
    else
        openPath = VoreenApplication::app()->getVolumePath();
    QString tmp = QFileDialog::getExistingDirectory(this, tr("Load Volume from DICOM Slices..."),
                                                    QString::fromStdString(openPath),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!tmp.isEmpty()) {
        if (!tmp.endsWith("/"))
            tmp += "/";
        loadDicomFiles(tmp.toStdString());
    }
#else
    QMessageBox::information(this, "Voreen", tr("Application was compiled without DICOM support."));
#endif
}

#ifdef VRN_MODULE_DICOM

void VolumeLoadButton::loadDicomDir(const std::string& file) {

    if (file.empty())
        return;

    ProgressBar* progress = new ProgressDialog(reinterpret_cast<QWidget *>(VoreenApplicationQt::qtApp()->getMainWindow()));

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    DicomVolumeReader dicomReader(progress);
    std::vector<voreen::DicomSeriesInfo> series = dicomReader.listSeries(file);
    QApplication::restoreOverrideCursor();

    if (series.size() > 0) {
        delete dicomDirDialog_;
        dicomDirDialog_ = new DicomDirDialog(this);
        connect(dicomDirDialog_, SIGNAL(dicomDirFinished()), this, SLOT(dicomDirDialogFinished()));
        dicomDirDialog_->setSeries(series, file);
        dicomDirDialog_->exec();
    }
    else
        QMessageBox::warning(this, "Voreen", "No DICOM series found.");
}

void VolumeLoadButton::dicomDirDialogFinished() {

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    ProgressBar* progress = new ProgressDialog(VoreenApplicationQt::qtApp()->getMainWindow());
    DicomVolumeReader dicomReader(progress);
    VolumeCollection* volumeCollection = 0;
    try {
        volumeCollection = dicomReader.read(dicomDirDialog_->getFilename());
    }
    catch (tgt::FileException e) {
        LERROR(e.what());
        QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
        errorMessageDialog->showMessage(e.what());
    }
    catch (std::bad_alloc) {
        LERROR("std::Error BAD ALLOCATION");
        QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
        errorMessageDialog->showMessage("std::Error BAD ALLOCATION, File: "
            + QString(dicomDirDialog_->getFilename().c_str()));
    }
    QApplication::restoreOverrideCursor();

    delete progress;

    if (volumeCollection)
        volumeContainer_->add(volumeCollection);
    delete volumeCollection;
    emit VolumeAdded(volumeContainer_->size());
}

void VolumeLoadButton::loadDicomFiles(const std::string& dir) {

    tgtAssert(volumeContainer_, "No volume container");

    if (dir.empty())
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    ProgressBar* progress = new ProgressDialog(VoreenApplicationQt::qtApp()->getMainWindow());
    DicomVolumeReader volumeReader(progress);
    VolumeCollection* volumeCollection = 0;
    try {
        volumeCollection = volumeReader.read(dir);
    }
    catch (tgt::FileException e) {
        LERROR(e.what());
        QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
        errorMessageDialog->showMessage(e.what());
    }
    catch (std::bad_alloc) {
        LERROR("std::Error BAD ALLOCATION");
        QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
        errorMessageDialog->showMessage("std::Error BAD ALLOCATION, File: "
            + QString(dicomDirDialog_->getFilename().c_str()));
    }
    QApplication::restoreOverrideCursor();

    delete progress;

    if (volumeCollection)
        volumeContainer_->add(volumeCollection);
    delete volumeCollection;
    emit VolumeAdded(volumeContainer_->size());
}

#else

void VolumeLoadButton::dicomDirDialogFinished() { }

void VolumeLoadButton::loadDicomDir(const std::string& /*file*/) { }

void VolumeLoadButton::loadDicomFiles(const std::string& /*dir*/) { }

#endif // VRN_MODULE_DICOM

} // namespace voreen
