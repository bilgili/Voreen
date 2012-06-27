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

#include <QCheckBox>
#include <QErrorMessage>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QTreeWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QApplication>

#include "voreen/qt/widgets/volumecontainerwidget.h"
#include "voreen/qt/widgets/volumeviewhelper.h"
#include "voreen/qt/widgets/rawvolumewidget.h"
#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/volume/bricking/largevolumemanager.h"
#include "voreen/core/io/datvolumewriter.h"
#include "voreen/core/io/dicomvolumereader.h"
#include "voreen/core/io/ioprogress.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/application.h"

#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/qt/ioprogressdialog.h"
#include "voreen/qt/voreenapplicationqt.h"

#ifdef _MSC_VER
#include "tgt/gpucapabilitieswindows.h"
#endif


namespace voreen {

QRCTreeWidget::QRCTreeWidget(QWidget* parent) : QTreeWidget(parent) {
}

void QRCTreeWidget::contextMenuEvent(QContextMenuEvent* cmevent) {
    if (itemAt(cmevent->pos())){
        QMenu* menu = new QMenu();
        menu->addAction(QPixmap(":/voreenve/icons/refresh.png"), "Refresh");
        menu->addAction(QPixmap(":/voreenve/icons/export.png"), "Export...");
        menu->addAction(QPixmap(":/voreenve/icons/cancel.png"), "Remove");
        QAction* action = menu->exec(mapToGlobal(cmevent->pos()));
        if(action) {
            if(action->text() == "Export...")
                emit exportDat();
            if(action->text() == "Refresh")
                emit refresh(itemAt(cmevent->pos()));
            if(action->text() == "Remove")
                emit remove();
            }
    } else {
        QMenu* menu = new QMenu();
        menu->addAction(QPixmap(":/voreenve/icons/open-volume.png"), "Volume");
        QAction* action = menu->exec(mapToGlobal(cmevent->pos()));
        if(action && action->text() == "Volume") {
            emit add();
        }
    }
}

// ---------------------------------------------------------------------------

const std::string VolumeContainerWidget::loggerCat_("voreen.qt.VolumeContainerWidget");

VolumeContainerWidget::VolumeContainerWidget(VolumeContainer* volumeContainer, QWidget* parent)
        : QWidget(parent, Qt::Tool)
        , volumeContainer_(volumeContainer)
        , rawSelected_(false)
#ifdef VRN_WITH_DCMTK
        , dicomDirDialog_(0)
#endif
{
    if (volumeContainer_)
        volumeContainer_->addObserver(this);
    else
        LWARNING("No volume container assigned");

    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QHBoxLayout* dicomLayout = new QHBoxLayout();

    volumeInfos_ = new QRCTreeWidget();
    QTreeWidgetItem* header = volumeInfos_->headerItem();
    header->setText(0, tr("Volumes"));
    volumeInfos_->header()->hide();
    volumeInfos_->setColumnCount(1);
    volumeInfos_->show();
    volumeInfos_->setIconSize(QSize(63,63));

    QPushButton* loadButton = new QPushButton();
    loadButton->setText(tr("Load Volume"));
    loadButton->setIcon(QPixmap(":/voreenve/icons/open-volume.png"));

    connect(loadButton, SIGNAL(clicked()), this, SLOT(loadVolume()));
    connect(volumeInfos_, SIGNAL(refresh(QTreeWidgetItem*)), this, SLOT(volumeRefresh(QTreeWidgetItem*)));
    connect(volumeInfos_, SIGNAL(add()), this, SLOT(loadVolume()));
    connect(volumeInfos_, SIGNAL(remove()), this, SLOT(removeVolume()));
    connect(volumeInfos_, SIGNAL(exportDat()), this, SLOT(exportDat()));

    buttonLayout->addWidget(loadButton);

#ifdef VRN_WITH_DCMTK
    QPushButton* btnLoadDICOM = new QPushButton(QPixmap(":/voreenve/icons/open-dicom.png"), tr("Load DICOM Slices"));
    buttonLayout->addWidget(btnLoadDICOM);
    connect(btnLoadDICOM, SIGNAL(clicked()), this, SLOT(loadDicomFiles()));
#endif

    buttonLayout->addStretch();

    mainLayout->addWidget(volumeInfos_);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(dicomLayout);

    setMinimumWidth(300);
    update();
}

VolumeContainerWidget::~VolumeContainerWidget() {
}

void VolumeContainerWidget::resizeOnCollapse(bool) {
    adjustSize();
}

void VolumeContainerWidget::loadRawVolume(std::string filenameStd) {

    QString filename = QString::fromStdString(filenameStd);
    std::string objectModel;
    std::string format;
    tgt::ivec3 dim;
    tgt::vec3 spacing;
    int headerSkip;

    if (!filename.isEmpty()){
        RawVolumeWidget* rawVW = new RawVolumeWidget(this, tr("Please enter the properties for <br><strong>")+filename+"</strong>",
                                                     objectModel, format, dim, spacing, headerSkip);
        if (rawVW->exec() == QDialog::Accepted) {

            // dervive expected file size from provided properties
            uint formatBytes;
            if (format == "USHORT" || format == "USHORT_12")
                formatBytes = 2;
            else if (format == "FLOAT8" || format == "FLOAT16" || format == "FLOAT")
                formatBytes = 4;
            else
                formatBytes = 1;
            uint rawSize = headerSkip + formatBytes * (dim.x * dim.y * dim.z);

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
                volumeContainer_->loadRawVolume(filename.toStdString(), objectModel, format, dim, spacing, headerSkip);
            }
            catch (tgt::FileException e) {
                QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
                errorMessageDialog->showMessage(e.what());
                LWARNING(e.what());
            }
            catch (std::bad_alloc) {
#ifdef VRN_WITH_DCMTK
                QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
                errorMessageDialog->showMessage("std::Error BAD ALLOCATION, File: "
                    + QString(dicomDirDialog_->getFilename().c_str()));
#endif
                LWARNING("std::Error BAD ALLOCATION");
            }
        }
    }
}

const std::string VolumeContainerWidget::getExtensions() const{
    VolumeSerializerPopulator* populator = new VolumeSerializerPopulator();
    std::vector<std::string> extensionVec = populator->getSupportedReadExtensions();
    delete populator;
    std::string out = "Volume Files (";
    for(size_t i = 0; i < extensionVec.size(); i++) {
        out+= "*."+extensionVec[i]+" ";
    }

#ifdef VRN_WITH_DCMTK
    out+=" DICOMDIR";
#endif
    out+=")";
    return out;
}

void VolumeContainerWidget::setVolumeContainer(VolumeContainer* volumeContainer) {

    // do not deregister from previously assigned container,
    // since it may not be valid anymore!

    // assign new container
    volumeContainer_ = volumeContainer;

    // register at new container
    if (volumeContainer_)
        volumeContainer_->addObserver(this);

    update();

}

void VolumeContainerWidget::update() {
    volumeInfos_->clear();

    if (!volumeContainer_ || volumeContainer_->empty())
        return;

    for(size_t i = 0 ; i< volumeContainer_->size(); i++) {
        VolumeHandle* handle = volumeContainer_->at(i);
        Volume* volume = volumeContainer_->at(i)->getVolume();
        QTreeWidgetItem* qtwi = new QTreeWidgetItem(volumeInfos_);
        std::string name = VolumeViewHelper::getStrippedVolumeName(handle);
        std::string path = VolumeViewHelper::getVolumePath(handle);

        if(name.size() > 40) {
            int end = name.size();
            std::string startString;
            std::string endString;
            for(size_t i = 0; i < 18; i++){
                 startString += name.at(i);
                 endString += name.at(end-18+i);
            }
            name = startString+"..."+endString;
        }
        if(path.size() > 40) {
            int end = path.size();
            std::string startString;
            std::string endString;
            for(size_t i = 0; i < 18; i++){
                 startString += path.at(i);
                 endString += path.at(end-18+i);
            }
            path = startString+"..."+endString;
        }


        qtwi->setFont(0, QFont(QString("Arial"), 7));
        qtwi->setText(0, QString::fromStdString(name
            + " ("+VolumeViewHelper::getVolumeType(volume)+")"+"\n"+path
            +"\nDimensions: " + VolumeViewHelper::getVolumeDimension(volume) + "\nSpacing: "
            + VolumeViewHelper::getVolumeSpacing(volume) +"\nMemSize: "
            + VolumeViewHelper::getVolumeBytes(volume)));

        qtwi->setIcon(0, QIcon(VolumeViewHelper::generateBorderedPreview(volume, 63, 0)));
        qtwi->setSizeHint(0,QSize(65,65));
        qtwi->setToolTip(0, QString::fromStdString(VolumeViewHelper::getStrippedVolumeName(handle)
            + " ("+VolumeViewHelper::getVolumeType(volume)+")"+"\n"+VolumeViewHelper::getVolumePath(handle)
            +"\nDimensions: " + VolumeViewHelper::getVolumeDimension(volume) + "\nSpacing: "
            + VolumeViewHelper::getVolumeSpacing(volume) +"\nMemSize: "
            + VolumeViewHelper::getVolumeBytes(volume)));

        volumeInfos_->addTopLevelItem(qtwi);
    }
}

void VolumeContainerWidget::loadVolume() {
    std::vector<std::string> files = openFileDialog();
    if (!files.empty())
        addMultipleVolumes(files);
}

void VolumeContainerWidget::addMultipleVolumes(std::vector<std::string> filenames) {
    if(rawSelected_ == false) {
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

#ifdef VRN_WITH_DCMTK
                if (tgt::FileSystem::fileName(filenames[i]) == "DICOMDIR" || tgt::FileSystem::fileName(filenames[i]) == "dicomdir")
                    loadDicomDir(filenames[i]);
                else
#endif
                    collection = volumeContainer_->loadVolume(filenames.at(i));
                
                update();
            }
            catch (tgt::FileException e) {
                QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
                errorMessageDialog->showMessage(e.what());
                LWARNING(e.what());
            }
            catch (std::bad_alloc) {
#ifdef VRN_WITH_DCMTK
                QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
                errorMessageDialog->showMessage("std::Error BAD ALLOCATION");
#endif
                LWARNING("std::Error BAD ALLOCATION");
            }
            delete collection;
        }
    }

    }
    else if(rawSelected_ == true) {
        for (size_t i = 0; i < filenames.size(); i++) {
            if (filenames[i] != "") {
                loadRawVolume(filenames[i]);
            }
        }

    }
}

void VolumeContainerWidget::removeVolume() {
    QTreeWidgetItem* currentItem = volumeInfos_->currentItem();
    if (currentItem != 0 && currentItem->parent() == 0) {
        volumeInfos_->removeItemWidget(currentItem, 0);
        volumeContainer_->remove(volumeContainer_->at(volumeInfos_->indexOfTopLevelItem(currentItem)));
    }
}

void VolumeContainerWidget::keyPressEvent(QKeyEvent* keyEvent) {
    if((keyEvent->key() == Qt::Key_Minus) || (keyEvent->key() == Qt::Key_Delete)) {
        removeVolume();
    }
}


void VolumeContainerWidget::volumeAdded(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/) {
    update();
}

void VolumeContainerWidget::volumeRemoved(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/) {
    update();
}

void VolumeContainerWidget::volumeRefresh(QTreeWidgetItem* item) {
    qApp->processEvents();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    volumeContainer_->at(volumeInfos_->indexOfTopLevelItem(item))->reloadVolume();
    QApplication::restoreOverrideCursor();
}

void VolumeContainerWidget::exportDat() {

    QFileDialog saveAsDialog(0, "Export Volume...");
    saveAsDialog.setFileMode(QFileDialog::AnyFile);
    saveAsDialog.setNameFilter("dat+raw Volume Files (*.dat)");
    saveAsDialog.setDefaultSuffix("dat");
    saveAsDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveAsDialog.setConfirmOverwrite(true);
    QStringList filename;
    if(saveAsDialog.exec())
        filename = saveAsDialog.selectedFiles();
    if (!filename.empty()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        qApp->processEvents();
        DatVolumeWriter* dvw = new DatVolumeWriter();
        try {
            dvw->write(filename[0].toStdString(),
                volumeContainer_->at(volumeInfos_->indexOfTopLevelItem(volumeInfos_->currentItem())));
        }
        catch(tgt::IOException e) {
            QApplication::restoreOverrideCursor();
            QMessageBox error;
            error.setText(QString(e.what()));
            error.exec();
        }
        delete dvw;
        QApplication::restoreOverrideCursor();
    }

}

std::vector<std::string> VolumeContainerWidget::openFileDialog() {
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
    if (rawSelected_)
        filters << "Raw Volume Files (*)" << QString::fromStdString(ext);
    else
        filters << QString::fromStdString(ext) << "Raw Volume Files (*)";

    QFileDialog dlg(this, tr("Load Volume..."), dir.absolutePath(), "");
    dlg.setNameFilters(filters);
    dlg.setViewMode(QFileDialog::Detail);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);

    connect(&dlg, SIGNAL(filterSelected(QString)), this, SLOT(filterChanged(QString)));

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getVolumePath().c_str());
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

void VolumeContainerWidget::filterChanged(QString filter) {
    if (filter == "Raw Volume Files (*)") {
        rawSelected_ = true;
    }
    else if (filter == QString::fromStdString(getExtensions())){
        rawSelected_ = false;
    }

}

void VolumeContainerWidget::loadDicomFiles() {
#ifdef VRN_WITH_DCMTK
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

#ifdef VRN_WITH_DCMTK

void VolumeContainerWidget::loadDicomDir(const std::string& file) {

    if (file.empty())
        return;

    IOProgress* progress = new IOProgressDialog(VoreenApplicationQt::qtApp()->getMainWindow());

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    DicomVolumeReader dicomReader(progress);
    std::vector<voreen::DicomSeriesInfo> series = dicomReader.listSeries(file);
    QApplication::restoreOverrideCursor();

    if (series.size() > 0) {
        if (dicomDirDialog_)
            delete dicomDirDialog_;
        dicomDirDialog_ = new DicomDirDialog(this);
        connect(dicomDirDialog_, SIGNAL(dicomDirFinished()), this, SLOT(dicomDirDialogFinished()));
        dicomDirDialog_->setSeries(series, file);
        dicomDirDialog_->exec();
    }
    else
        QMessageBox::warning(this, "Voreen", "No DICOM series found.");
}

void VolumeContainerWidget::dicomDirDialogFinished() {

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    IOProgress* progress = new IOProgressDialog(VoreenApplicationQt::qtApp()->getMainWindow());
    DicomVolumeReader dicomReader(progress);
    VolumeCollection* volumeCollection = 0;
    try {
        volumeCollection = dicomReader.read(dicomDirDialog_->getFilename());
    }
    catch (tgt::FileException e) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
        errorMessageDialog->showMessage(e.what());
        LWARNING(e.what());
    }
    catch (std::bad_alloc) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
        errorMessageDialog->showMessage("std::Error BAD ALLOCATION, File: "
            + QString(dicomDirDialog_->getFilename().c_str()));
        LWARNING("std::Error BAD ALLOCATION");
    }
    QApplication::restoreOverrideCursor();

    delete progress;

    if (volumeCollection)
        volumeContainer_->add(*volumeCollection);
    delete volumeCollection;
}

void VolumeContainerWidget::loadDicomFiles(const std::string& dir) {

    tgtAssert(volumeContainer_, "No volume container");

    if (dir.empty())
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    IOProgress* progress = new IOProgressDialog(VoreenApplicationQt::qtApp()->getMainWindow());
    DicomVolumeReader volumeReader(progress);
    VolumeCollection* volumeCollection = 0;
    try {
        volumeCollection = volumeReader.read(dir);
    }
    catch (tgt::FileException e) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
        errorMessageDialog->showMessage(e.what());
        LWARNING(e.what());
    }
    catch (std::bad_alloc) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(VoreenApplicationQt::qtApp()->getMainWindow());
        errorMessageDialog->showMessage("std::Error BAD ALLOCATION, File: "
            + QString(dicomDirDialog_->getFilename().c_str()));
        LWARNING("std::Error BAD ALLOCATION");
    }
    QApplication::restoreOverrideCursor();

    delete progress;

    if (volumeCollection)
        volumeContainer_->add(*volumeCollection);
    delete volumeCollection;
}

#else

void VolumeContainerWidget::dicomDirDialogFinished() { }

void VolumeContainerWidget::loadDicomDir(const std::string& /*file*/) { }

void VolumeContainerWidget::loadDicomFiles(const std::string& /*dir*/) { }

#endif // VRN_WITH_DCMTK

} // namespace
