/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QErrorMessage>
#include <QPushButton>

#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/volume/volumesetcontainer.h"
#include "voreen/qt/ioprogressdialog.h"
#include "voreen/qt/widgets/volumesetwidget.h"

namespace voreen {

// protected methods for VolumeSetTreeWidget

void VolumeSetWidget::VolumeSetTreeWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasFormat("application/x-volumesetitem")) {
        event->setDropAction(Qt::MoveAction);
        if (event->source() == this)
            event->accept();
        else
            event->acceptProposedAction();
    }
    else
        event->ignore();
}

void VolumeSetWidget::VolumeSetTreeWidget::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasFormat("application/x-volumesetitem")) {
        event->setDropAction(Qt::MoveAction);
        if (event->source() == this)
            event->accept();
        else
            event->acceptProposedAction();
    }
    else
        event->ignore();
}

bool VolumeSetWidget::VolumeSetTreeWidget::dropMimeData(QTreeWidgetItem* parent, 
                                                        int /*index*/, const QMimeData* data,
                                                        Qt::DropAction /*action*/)
{
    if (parent == 0 || data == 0 || !data->hasFormat("application/x-volumesetitem"))
        return false;

    // The mime data contain the pointer which has been casted to a qulonglong
    //
    QByteArray itemData = data->data("application/x-volumesetitem");
    AbstractVolumeSetTreeItem* aitem = 
        reinterpret_cast<AbstractVolumeSetTreeItem*>(itemData.toULongLong());

    // The dragable item is either a VolumeSeries- or a VolumeHandleItem.
    // The target item on which it can be dropped is either a VolumeSet- or
    // a VolumeSeriesItem. So try to cast the dragged item and the target.
    // One of the casts will fail and the pointer will be NULL.
    //
    VolumeSeriesItem* draggedVolumeSeries = dynamic_cast<VolumeSeriesItem*>(aitem);
    VolumeHandleItem* draggedVolumeHandle = dynamic_cast<VolumeHandleItem*>(aitem);
    VolumeSetItem* droppedVolumeSet = dynamic_cast<VolumeSetItem*>(parent);
    VolumeSeriesItem* droppedVolumeSeries = dynamic_cast<VolumeSeriesItem*>(parent);
    
    // If an item containing a VolumeSeries is dropped onto an item containing 
    // a VolumeSet, move the series to the set, if it is not already a child of
    // it!
    //
    if (draggedVolumeSeries != 0 && droppedVolumeSet != 0
        && draggedVolumeSeries->parent() != droppedVolumeSet)
    {
        moveItem(draggedVolumeSeries, droppedVolumeSet);
        return true;
    }

    // If an item containing a VolumeHandle is dropped onto an item containing
    // a VolumeSeries, move the handle to the series, if it is not already a child
    // of it!
    //
    if (draggedVolumeHandle != 0 && droppedVolumeSeries != 0
        && draggedVolumeHandle->parent() != droppedVolumeSeries)
    {
        moveItem(draggedVolumeHandle, droppedVolumeSeries);
        return true;
    }
    return false;
}

void VolumeSetWidget::VolumeSetTreeWidget::mousePressEvent(QMouseEvent* event) {
    QTreeWidget::mousePressEvent(event);

    if (((event->buttons() & Qt::LeftButton) == 0))
        return;

    AbstractVolumeSetTreeItem* item = dynamic_cast<AbstractVolumeSetTreeItem*>(itemAt(event->pos()));
    if (item == 0)
        return;
    
    if ((item->getType() != typeid(VolumeSeries*))
        && (item->getType() != typeid(VolumeHandle*)))
        return;

    // Send the pointer to the item as the data which is dropped. Therefore
    // cast the pointer to a qulonglong and pack it into a QByteArray.
    //
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData();
    QByteArray ba = QByteArray::number(reinterpret_cast<qulonglong>(item));
    mimeData->setData("application/x-volumesetitem", ba);
    drag->setMimeData(mimeData);
    drag->start(Qt::MoveAction);
}

// private methods for VolumeSetTreeWidget
//

void VolumeSetWidget::VolumeSetTreeWidget::moveItem(VolumeSeriesItem* child, VolumeSetItem* newParent) {
    // move the data contained in the item
    //
    VolumeSetItem* oldParent = dynamic_cast<VolumeSetItem*>(child->parent());
    if (oldParent == 0)
        return;

    VolumeSet* oldVolumeSet = oldParent->getItemData();
    VolumeSet* newVolumeSet = newParent->getItemData();
    if ((oldVolumeSet != 0) && (newVolumeSet != 0)) {
        VolumeSeries* volumeSeries = oldVolumeSet->removeSeries(child->getItemData());
        newVolumeSet->addSeries(volumeSeries);
        // As the name of the VolumeSeries may change due to insertion into another
        // VolumeSet, reset the name of the item. E.g. if there is a VolumeSeries
        // named "unknown" already contained within the VolumeSet and another
        // VolumeSeries of the same name will be inserted, the name will be
        // changed to "unknown 2" by addSeries()
        //
        child->setText(0, QString(volumeSeries->getLabel().c_str()));
    }

    // move the item itself
#if QT_VERSION >= 0x040300    
    child->parent()->removeChild(child);
#else
    child->parent()->takeChild(child->parent()->indexOfChild(child));
#endif
    newParent->addChild(child);
}

void VolumeSetWidget::VolumeSetTreeWidget::moveItem(VolumeHandleItem* child, VolumeSeriesItem* newParent) {
    // move the data contained in the item
    //
    VolumeSeriesItem* oldParent = dynamic_cast<VolumeSeriesItem*>(child->parent());
    if (oldParent == 0)
        return;

    VolumeSeries* oldVolumeSeries = oldParent->getItemData();
    VolumeSeries* newVolumeSeries = newParent->getItemData();
    if ((oldVolumeSeries != 0) && (newVolumeSeries != 0)) {
        VolumeHandle* volumeHandle = oldVolumeSeries->removeVolumeHandle(child->getItemData());
        newVolumeSeries->addVolumeHandle(volumeHandle);
        // As the timestep of the VolumeHandle may change due to insertion into another
        // VolumeSet, reset the name of the item. E.g. if there is a VolumeHandle for
        // timestep 0.0f already contained within the VolumeSeries and another
        // VolumeHandle of the same timestep will be inserted, the timestep will be
        // changed to 1.0f by addVolumeHandle()
        //
        char name[20] = {0};
        sprintf(name, "%f", volumeHandle->getTimestep());//FIXME: use c++
        child->setText(0, QString(name));
    }

    // move the item itself
    //
#if QT_VERSION >= 0x040300    
    child->parent()->removeChild(child);
#else
    child->parent()->takeChild(child->parent()->indexOfChild(child));
#endif
    newParent->addChild(child);
}

// ---------------------------------------------------------------------------

// public methods
//

VolumeSetWidget::VolumeSetWidget(VolumeSetContainer* const volumeSetContainer,
                                 QWidget* parent, int levels, bool allowClose, Qt::WFlags flags)
    : QDialog(parent, flags)
    , volumeSetContainer_(volumeSetContainer)
    , availableLevelsMask_(levels)
    , fileExtFilter_("Volume data (*.DAT *.I4D *.PVM *.RDM *.RDI *.HDR *.SW *.SEG *.TUV "
                     "*.ZIP *.TIFF *.TIF *.MAT *.HV *.NRRD *.NHDR)")
    , currentDir_(QDir::currentPath().toStdString())
    , allowAddingMultipleFiles_(true)
    , progress_(new IOProgressDialog(this))
    , allowClose_(allowClose)
    , useProgress_(true)
{
    setWindowTitle(tr("Data Sets"));
#ifdef VRN_WITH_DCMTK
    dicomDirDialog_ = 0;
#endif
    volumeSerializerPopulator_ = new VolumeSerializerPopulator(progress_);
    createWidgets();

    connect(btnLoad_, SIGNAL(clicked()), 
        this, SLOT(buttonAddClicked()));
    connect(btnLoadDICOM_, SIGNAL(clicked()), 
        this, SLOT(buttonAddDICOMClicked()));
    connect(btnLoadDICOMDir_, SIGNAL(clicked()), 
        this, SLOT(buttonAddDICOMDirClicked()));
    connect(btnUnload_, SIGNAL(clicked()), 
        this, SLOT(buttonRemoveClicked()));
    connect(comboModality_, SIGNAL(currentIndexChanged(const QString&)), 
        this, SLOT(comboModalitySelectionChanged(const QString&)));
    connect(treeVolumeSets_, SIGNAL(itemSelectionChanged()), 
        this, SLOT(treeItemSelectionChanged()));
    connect(treeVolumeSets_, SIGNAL(itemChanged(QTreeWidgetItem*, int)), 
        this, SLOT(treeItemChanged(QTreeWidgetItem*, int)));

    VolumeRootItem* root = new VolumeRootItem("VolumeSetContainer", 0);
    treeVolumeSets_->addTopLevelItem(root);
    root->setExpanded(true);
    root->setSelected(true);
}

VolumeSetWidget::~VolumeSetWidget() {
    delete btnLoad_;
    delete btnLoadDICOM_;
    delete btnLoadDICOMDir_;
    delete btnUnload_;
    delete comboModality_;
    delete groupAdd_;
    delete groupRemove_;
    delete groupModality_;
    delete layout_;
    delete treeVolumeSets_;
    delete progress_;
}

void VolumeSetWidget::updateContent(VolumeSetContainer* vsc) {
    if (vsc)
        volumeSetContainer_ = vsc;
    if ((volumeSetContainer_ == 0) || (treeVolumeSets_ == 0))
        return;

    treeVolumeSets_->clear();
    VolumeRootItem* root = new VolumeRootItem("VolumeSetContainer", 0);
    appendVolumeSets(root);
    treeVolumeSets_->addTopLevelItem(root);
    root->setExpanded(true);
    root->setSelected(true);
    treeVolumeSets_->update();
}

void VolumeSetWidget::setAllowAddingMultipleFiles(const bool allow) {
    allowAddingMultipleFiles_ = allow;
}

bool VolumeSetWidget::getAllowAddingMultipleFiles() const {
    return allowAddingMultipleFiles_;
}

void VolumeSetWidget::setCurrentDirectory(const std::string& dir) {
    currentDir_ = dir;
}

const std::string& VolumeSetWidget::getCurrentDirectory() const {
    return currentDir_;
}

void VolumeSetWidget::setFileExtensionFilter(const std::string& fileExtFilter) {
    fileExtFilter_ = fileExtFilter;
}

const std::string& VolumeSetWidget::getFileExtensionFilter() const {
    return fileExtFilter_;
}

std::vector<std::string> VolumeSetWidget::openFileDialog() {

    QDir dir(currentDir_.c_str()); // we want absolute path name to prevent problems when the
                                   // file dialog perform chdir()

    QFileDialog dlg(this, tr("Open file(s) for adding"), dir.absolutePath(),
                    QString(fileExtFilter_.c_str()));
    dlg.setViewMode(QFileDialog::Detail);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(allowAddingMultipleFiles_ ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile);

    std::vector<std::string> filenames;
    if (dlg.exec() == QDialog::Accepted) {
        currentDir_ = dlg.directory().path().toStdString();
        const QStringList& lst = dlg.selectedFiles();
        QStringList::const_iterator it = lst.begin();
        for (; it != lst.end(); ++it)
            filenames.push_back(it->toStdString());
    }

    return filenames;
}

void VolumeSetWidget::clear(){
    volumeSetContainer_->clear();
    updateContent();
}

void VolumeSetWidget::addVolumeSets(const std::vector<std::string>& filenames) {
    bool update = false;
    for (size_t i = 0; i < filenames.size(); ++i) {
        if (loadVolumeSet(filenames[i]) != 0)
            update = true;
    }
    if (update)
        updateContent();
}

void VolumeSetWidget::addVolumeSeries(const std::vector<std::string>& filenames, 
                                      VolumeSet* parentVolumeSet)
{
    if (parentVolumeSet == 0)
        return;

    bool update = false;
    for (size_t i = 0; i < filenames.size(); ++i) {
        VolumeSet* vs = loadVolumeSet(filenames[i], false);
        if (vs == 0)
            continue;

        const VolumeSeries::SeriesSet& series = vs->getSeries();
        VolumeSeries::SeriesSet::const_iterator it = series.begin();
        for (; it != series.end(); ++it) {
            VolumeSeries* s = *it;
            if (s != 0) {
                if (parentVolumeSet->addSeries(s))
                    update = true;
            }
        }
    }   // for (filenames)

    if (update)
        updateContent();
}

void VolumeSetWidget::addVolumeHandles(const std::vector<std::string>& filenames,
                                       VolumeSeries* parentVolumeSeries)
{
    if (parentVolumeSeries == 0)
        return;

    bool update = false;
    for (size_t i = 0; i < filenames.size(); ++i) {
        VolumeSet* volumeset = loadVolumeSet(filenames[i], false);
        if (volumeset == 0)
            continue;

        const VolumeSeries::SeriesSet& series = volumeset->getSeries();
        VolumeSeries::SeriesSet::const_iterator it = series.begin();
        for (; it != series.end(); ++it) {
            VolumeSeries* s = *it;
            if (s == 0)
                continue;

            const VolumeHandle::HandleSet& handles = s->getVolumeHandles();
            VolumeHandle::HandleSet::const_iterator itHandles = handles.begin();
            for (; itHandles != handles.end(); ++itHandles) {
                VolumeHandle* handle = *itHandles;
                if (handle != 0) {
                    if (parentVolumeSeries->addVolumeHandle(handle))
                        update = true;
                }
            }
        }   // for (series)
    }   // for (filenames)

    if (update)
        updateContent();
}

VolumeSet* VolumeSetWidget::loadVolumeSet(const std::string& filename, const bool addToContainer) {
    if (volumeSetContainer_ == 0 || filename.empty())
        return 0;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if (useProgress_)
        progress_->show(filename);
    VolumeSet* volumeSet = 0;
    try {
        volumeSet = volumeSerializerPopulator_->getVolumeSerializer()->load(filename);
    } catch (tgt::FileException e) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
        LWARNINGC("VolumeSetWidget", e.what());
    } catch (std::bad_alloc) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage("std::Error BAD ALLOCATION, File: " + QString(filename.c_str()));
        LWARNINGC("VolumeSetWidget", "std::Error BAD ALLOCATION");
    }

    if (useProgress_)
        progress_->hide();
    QApplication::restoreOverrideCursor();

    if (volumeSet && addToContainer) {
        if (volumeSetContainer_->addVolumeSet(volumeSet))
            updateContent();
    }

    return volumeSet;
}

#ifdef VRN_WITH_DCMTK

void VolumeSetWidget::loadDicomDir(const std::string& file) {
    if (file.empty())
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    voreen::DicomVolumeReader volumeReader;
    std::vector<voreen::DicomSeriesInfo> series = volumeReader.listSeries(file);
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

void VolumeSetWidget::dicomDirDialogFinished() {
    DicomVolumeReader volumeReader;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if (useProgress_)
        progress_->show(dicomDirDialog_->getFilename());
    VolumeSet* volumeSet = 0;
    try {
        volumeSet = volumeReader.read(dicomDirDialog_->getFilename());
    } catch (tgt::FileException e) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
        LWARNINGC("VolumeSetWidget", e.what());
    } catch (std::bad_alloc) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage("std::Error BAD ALLOCATION, File: "
                                        + QString(dicomDirDialog_->getFilename().c_str()));
        LWARNINGC("VolumeSetWidget", "std::Error BAD ALLOCATION");
    }
    if (useProgress_)
        progress_->hide();
    QApplication::restoreOverrideCursor();

    if (volumeSet == 0)
        return;

    if (volumeSetContainer_->addVolumeSet(volumeSet))
        updateContent();
}

void VolumeSetWidget::loadDicomFiles(const std::string& dir) {
    if (dir.empty())
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if (useProgress_)
        progress_->show(dir);
    DicomVolumeReader volumeReader(progress_);
    VolumeSet* volumeSet = 0;
    try {
        volumeSet = volumeReader.read(dir);
    } catch (tgt::FileException e) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
        LWARNINGC("VolumeSetWidget", e.what());
    } catch (std::bad_alloc) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage("std::Error BAD ALLOCATION, File: " + QString(dir.c_str()));
        LWARNINGC("VolumeSetWidget", "std::Error BAD ALLOCATION");
    }
    if (useProgress_)
        progress_->hide();
    QApplication::restoreOverrideCursor();

    if (volumeSet == 0)
        return;

    if (volumeSetContainer_->addVolumeSet(volumeSet))
        updateContent();
}

#else

void VolumeSetWidget::dicomDirDialogFinished() {
}

void VolumeSetWidget::loadDicomDir(const std::string& /*file*/) {
}

void VolumeSetWidget::loadDicomFiles(const std::string& /*dir*/) {
}

#endif // VRN_WITH_DCMTK

// public slots:

void VolumeSetWidget::buttonAddClicked() {
    if (treeVolumeSets_ == 0)
        return;

    const QList<QTreeWidgetItem*>& items = treeVolumeSets_->selectedItems();
    if (items.empty())
        return;

    QTreeWidgetItem* item = items.first();
    std::vector<std::string> filenames = openFileDialog();
    if (!filenames.empty()) {
            VolumeRootItem* rootItem = dynamic_cast<VolumeRootItem*>(item);
            VolumeSetItem* volsetItem = dynamic_cast<VolumeSetItem*>(item);
            VolumeSeriesItem* volseriesItem = dynamic_cast<VolumeSeriesItem*>(item);
            if (volsetItem != 0) {
                addVolumeSeries(filenames, volsetItem->getItemData());
                updateContent();
            }
            else if (volseriesItem != 0) {
                addVolumeHandles(filenames, volseriesItem->getItemData());
                updateContent();
            }
            else if (rootItem != 0) {
                addVolumeSets(filenames);
                updateContent();
            }
    }
}

void VolumeSetWidget::buttonAddDICOMClicked() {
#ifdef VRN_WITH_DCMTK
    QString tmp = QFileDialog::getExistingDirectory(this, "Choose a Directory",
        QString(currentDir_.c_str()), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (tmp != "") {
#ifdef WIN32
        if (!tmp.endsWith("/"))
            tmp += "/";
#endif
        loadDicomFiles(tmp.toStdString());
    }
#else
    QMessageBox::information(this, "Voreen", tr("Application was compiled without DICOM support."));
#endif
}

void VolumeSetWidget::buttonAddDICOMDirClicked() {
#ifdef VRN_WITH_DCMTK
    QString tmp = QFileDialog::getOpenFileName(this, "Choose a File to Open",
                                               QString(currentDir_.c_str()), "DICOMDIR File");
    loadDicomDir(tmp.toStdString());
#else
    QMessageBox::information(this, "Voreen", tr("Application was compiled without DICOM support."));
#endif
}

void VolumeSetWidget::buttonRemoveClicked() {
    if (treeVolumeSets_ == 0)
        return;

    const QList<QTreeWidgetItem*>& items = treeVolumeSets_->selectedItems();
    if (items.empty())
        return;

    AbstractVolumeSetTreeItem* item = dynamic_cast<AbstractVolumeSetTreeItem*>(items.first());
    if (item == 0)
        return;

    if (item->getType() == typeid(VolumeSet*)) {
        VolumeSetItem* volsetItem = dynamic_cast<VolumeSetItem*>(item);
        if (volsetItem != 0) {
            volumeSetContainer_->deleteVolumeSet(volsetItem->getItemData());
            // finally delete the item itself
            treeVolumeSets_->removeItemWidget(volsetItem, treeVolumeSets_->currentColumn());
            delete volsetItem;
        }
    }
    else if (item->getType() == typeid(VolumeSeries*)) {
        // if the item is a VolumeSeriesItem, cast it
        //
        VolumeSeriesItem* seriesItem = dynamic_cast<VolumeSeriesItem*>(item);
        if (seriesItem != 0) {
            // if the item has been successfully casted, get its parent item which should
            // be a VolumeSetItem.
            //
            VolumeSetItem* volsetItem = dynamic_cast<VolumeSetItem*>(seriesItem->parent());
            if ((volsetItem != 0) && (volsetItem->getItemData() != 0)) {
                // now delete the VolumeSeries contained in the VolumeSeriesItem 
                // from the parent VolumeSet contained in the parent VolumeSetItem
                volsetItem->getItemData()->deleteSeries(seriesItem->getItemData());
            }
            // finally delete the item itself
            treeVolumeSets_->removeItemWidget(seriesItem, treeVolumeSets_->currentColumn());
            delete seriesItem;
        }
    }
    else if (item->getType() == typeid(VolumeHandle*)) {
        // if the item is a VolumeHandleItem, cast it
        //
        VolumeHandleItem* handleItem = dynamic_cast<VolumeHandleItem*>(item);
        if (handleItem != 0) {
            // if the item has been successfully casted, get its parent item which should
            // be a VolumeSeriesItem.
            //
            VolumeSeriesItem* seriesItem = dynamic_cast<VolumeSeriesItem*>(handleItem->parent());
            if ((seriesItem != 0) && (seriesItem->getItemData() != 0)) {
                // now delete the VolumeHandle contained in the VolumeHandleItem 
                // from the parent VolumeSeries contained in the parent VolumeSeriesItem
                seriesItem->getItemData()->deleteVolumeHandle(handleItem->getItemData());
            }
            // finally delete the item itself
            treeVolumeSets_->removeItemWidget(handleItem, treeVolumeSets_->currentColumn());
            delete handleItem;
        }
    }
}

void VolumeSetWidget::comboModalitySelectionChanged(const QString& text) {
    if (treeVolumeSets_ == 0 || text == "")
        return;

    const QList<QTreeWidgetItem*>& items = treeVolumeSets_->selectedItems();
    if (items.empty())
        return;

    VolumeSeriesItem* vsi = dynamic_cast<VolumeSeriesItem*>(items.first());
    if (vsi == 0)
        return;

    VolumeSeries* series = vsi->getItemData();
    series->setModality(Modality(text.toStdString()));
    treeVolumeSets_->blockSignals(true);
    vsi->setText(0, QString(series->getLabel().c_str()));
    treeVolumeSets_->blockSignals(false);
}

void VolumeSetWidget::treeItemSelectionChanged() {
    if (treeVolumeSets_ == 0 || btnLoad_ == 0)
        return;

    const QList<QTreeWidgetItem*>& items = treeVolumeSets_->selectedItems();
    if (items.empty())
        return;

    AbstractVolumeSetTreeItem* item = dynamic_cast<AbstractVolumeSetTreeItem*>(items.first());
    if (item->getType() == typeid(void*)) {
        btnLoad_->setEnabled(true);
        btnLoad_->setText("add VolumeSet");
        btnLoadDICOM_->setEnabled(true);
        btnLoadDICOMDir_->setEnabled(true);
        btnUnload_->setEnabled(false);
        btnUnload_->setText("not possible");
        comboModality_->setEnabled(false);
    }
    else {
        btnLoadDICOM_->setEnabled(false);
        btnLoadDICOMDir_->setEnabled(false);
        btnUnload_->setEnabled(true);

        if (item->getType() == typeid(VolumeSet*)) {
            btnLoad_->setEnabled(true);
            btnLoad_->setText("add VolumeSeries");
            btnUnload_->setText("remove VolumeSet");
            comboModality_->setEnabled(false);
        }
        else if (item->getType() == typeid(VolumeSeries*)) {
            btnLoad_->setEnabled(true);
            btnLoad_->setText("add VolumeHandle");
            btnUnload_->setText("remove VolumeSeries");
            comboModality_->setEnabled(true);
            VolumeSeriesItem* vsi = dynamic_cast<VolumeSeriesItem*>(item);
            if (vsi != 0)
                setModalityComboIndex(vsi->getItemData()->getModality().getName());
        }
        else if (item->getType() == typeid(VolumeHandle*)) {
            btnLoad_->setEnabled(false);
            btnLoad_->setText("not possible");
            btnUnload_->setText("remove VolumeHandle");
            comboModality_->setEnabled(false);
        }
    }
}

void VolumeSetWidget::treeItemChanged(QTreeWidgetItem* item, int column) {
    if (column != 0 || item == 0)
        return;

    VolumeSeriesItem* volumeSeriesItem = dynamic_cast<VolumeSeriesItem*>(item);
    VolumeHandleItem* volumeHandleItem = dynamic_cast<VolumeHandleItem*>(item);

    // Set the names for the data underlying the items. If this fails,
    // the name will be reset to the original one and no changes will be
    // made.
    // Note that renaming the items and their data requires no other data
    // with the same name (or timestep) to exist in the parent container.
    // Otherwise the std::set would crash.
    // This is ensured within the renaming methods of the item data.
    //
    if (volumeSeriesItem != 0) {
        VolumeSeries* series = volumeSeriesItem->getItemData();
        std::string prevName = series->getLabel();
        std::string name = volumeSeriesItem->text(column).toStdString();
        if (name == prevName)
            return;

        treeVolumeSets_->blockSignals(true);
        if (!series->setName(name))
            volumeSeriesItem->setText(0, QString(prevName.c_str()));
        else
            volumeSeriesItem->setText(0, QString(series->getLabel().c_str()));
        treeVolumeSets_->blockSignals(false);

    }
    else if (volumeHandleItem != 0) {
        VolumeHandle* handle = volumeHandleItem->getItemData();
        float prevTimestep = handle->getTimestep();
        float timestep = volumeHandleItem->text(column).toFloat();
        if (timestep == prevTimestep)
            return;

        treeVolumeSets_->blockSignals(true);
        if (!handle->setTimestep(timestep)) {
            char name[20] = {0};
            sprintf(name, "%f", prevTimestep);//FIXME: use c++
            volumeHandleItem->setText(0, name);
        }
        treeVolumeSets_->blockSignals(false);
    }
}

// private methods
//

void VolumeSetWidget::appendVolumeSets(VolumeRootItem* node) {
    if ((node == 0) || (volumeSetContainer_ == 0))
        return;

    const VolumeSet::VSPSet& volsets = volumeSetContainer_->getVolumeSets();
    VolumeSet::VSPSet::const_iterator it = volsets.begin();
    for (; it != volsets.end(); ++it) {
        VolumeSet* volumeSet = *it;
        if (volumeSet == 0)
            continue;
              
        if ((availableLevelsMask_ & VolumeSetWidget::LEVEL_VOLUMESETS) != 0) {
            QTreeWidgetItem* volsetItem = new VolumeSetItem(volumeSet->getName(), volumeSet);
            volsetItem->setFlags(Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            appendVolumeSeries(volsetItem, volumeSet);
            node->addChild(volsetItem);
        }
        else
            appendVolumeSeries(node, volumeSet);
    }
}

void VolumeSetWidget::appendVolumeSeries(QTreeWidgetItem* node, VolumeSet* const volumeSet) {
    if ((node == 0) || (volumeSet == 0))
        return;

    const VolumeSeries::SeriesSet& series = volumeSet->getSeries();
    VolumeSeries::SeriesSet::const_iterator it = series.begin();
    for (; it != series.end(); ++it) {
        VolumeSeries* series = *it;
        if (series == 0)
            continue;

        if ((availableLevelsMask_ & VolumeSetWidget::LEVEL_VOLUMESERIES) != 0) {
            QTreeWidgetItem* seriesItem = new VolumeSeriesItem(series->getLabel(), series);
            seriesItem->setFlags(Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled
                | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            appendVolumeHandles(seriesItem, series);
            node->addChild(seriesItem);
        }
        else
            appendVolumeHandles(node, series);
    }
}

void VolumeSetWidget::appendVolumeHandles(QTreeWidgetItem* node,
                                          VolumeSeries* const volumeSeries) {
    if ((node == 0) || (volumeSeries == 0))
        return;

    const VolumeHandle::HandleSet& handles = volumeSeries->getVolumeHandles();
    VolumeHandle::HandleSet::const_iterator it = handles.begin();
    for (; it != handles.end(); ++it) {
        VolumeHandle* handle = *it;
        if (handle == 0)
            continue;

        if ((availableLevelsMask_ & VolumeSetWidget::LEVEL_VOLUMEHANDLES) != 0) {
            char buffer[20] = {0};
            sprintf(buffer, "%f", handle->getTimestep());//FIXME: use c++
            QTreeWidgetItem* handleItem = new VolumeHandleItem(std::string(buffer), handle);
            handleItem->setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsEditable 
                | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            node->addChild(handleItem);
        }
    }
}

void VolumeSetWidget::createWidgets() {
    treeVolumeSets_ = new VolumeSetTreeWidget(this);

    groupAdd_ = new QGroupBox(tr("add"), this);
    QVBoxLayout* groupLayout = new QVBoxLayout(groupAdd_);
	btnLoad_ = new QPushButton(tr("add"), groupAdd_);
    btnLoadDICOM_ = new QPushButton(tr("add DICOM slices"), groupAdd_);
    btnLoadDICOMDir_ = new QPushButton(tr("add DICOMDIR"), groupAdd_);
    groupLayout->addWidget(btnLoad_);
    groupLayout->addWidget(btnLoadDICOM_);
    groupLayout->addWidget(btnLoadDICOMDir_);
    groupAdd_->setLayout(groupLayout);
    groupAdd_->adjustSize();

    groupRemove_ = new QGroupBox(tr("remove"), this);
    groupLayout = new QVBoxLayout(groupRemove_);
	btnUnload_ = new QPushButton(tr("remove"), groupRemove_);
    groupLayout->addWidget(btnUnload_);
    groupRemove_->setLayout(groupLayout);
    groupRemove_->adjustSize();

    groupModality_ = new QGroupBox(tr("modality"), this);
    groupLayout = new QVBoxLayout(groupModality_);
    comboModality_ = new QComboBox(groupModality_);

    // add the names of all available modalities to the combo box
    //
    const std::vector<std::string>& modNames = Modality::getModalities();
    modalityIndices_.clear();
    for (size_t i = 0; i < modNames.size(); ++i) {
        modalityIndices_.insert(std::pair<std::string, int>(modNames[i], static_cast<int>(i)));
        comboModality_->addItem(QString(modNames[i].c_str()));
    }
    
    
    groupLayout->addWidget(comboModality_);
    groupModality_->setLayout(groupLayout);
    groupModality_->adjustSize();

	layout_ = new QGridLayout(this);
    layout_->addWidget(treeVolumeSets_, 0, 0, allowClose_ ? 4 : 3, 1);
	layout_->addWidget(groupAdd_, 0, 1);
	layout_->addWidget(groupRemove_, 1, 1);
    layout_->addWidget(groupModality_, 2, 1);

    if (allowClose_) {
        QPushButton* closeBtn = new QPushButton(tr("Close"));
        connect(closeBtn, SIGNAL(clicked()), this,  SLOT(accept()));
        layout_->addWidget(closeBtn, 3, 1);
    }
    
    setLayout(layout_);
    adjustSize();
}

void VolumeSetWidget::setModalityComboIndex(const std::string& modalityName) {
    if ((modalityName.empty()) || (comboModality_ == 0)
        || (modalityIndices_.empty()))
        return;

    ModalityIndexMap::iterator it = modalityIndices_.find(modalityName);
    if (it != modalityIndices_.end())
        comboModality_->setCurrentIndex(it->second);
}

} // namespace
