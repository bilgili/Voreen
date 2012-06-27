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
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QToolButton>
#include <QTreeWidgetItem>
#include <QUrl>
#include <QVBoxLayout>

#include <QApplication>

#include "tgt/matrix.h"

#include "voreen/qt/widgets/volumeloadbutton.h"
#include "voreen/qt/widgets/volumecontainerwidget.h"
#include "voreen/qt/widgets/volumeviewhelper.h"
#include "voreen/qt/widgets/rawvolumewidget.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/bricking/largevolumemanager.h"
#include "voreen/core/io/datvolumewriter.h"
#include "voreen/core/io/dicomvolumereader.h"
#include "voreen/core/io/ioprogress.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/voreenapplication.h"

#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/qt/ioprogressdialog.h"

#ifdef _MSC_VER
#include "tgt/gpucapabilitieswindows.h"
#endif

#include "voreen/core/io/rawvolumereader.h"

namespace {
#ifdef __APPLE__
    int fontSize = 13;
#else
    int fontSize = 8;
#endif
}

namespace voreen {

QRCTreeWidget::QRCTreeWidget(QWidget* parent) : QTreeWidget(parent) {
    setSelectionMode(QAbstractItemView::ExtendedSelection);
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
        menu->addAction(QPixmap(":/voreenve/icons/open-volume.png"), "Load Volume...");
        QAction* action = menu->exec(mapToGlobal(cmevent->pos()));
        if (action && action->text() == "Load Volume...") {
            emit add();
        }
    }
}

// ---------------------------------------------------------------------------

const std::string VolumeContainerWidget::loggerCat_("voreen.qt.VolumeContainerWidget");

VolumeContainerWidget::VolumeContainerWidget(VolumeContainer* volumeContainer, QWidget* parent)
        : QWidget(parent, Qt::Tool)
        , volumeContainer_(volumeContainer)
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

    volumeLoadButton_ = new VolumeLoadButton(volumeContainer_, this);
    volumeLoadButton_->setMinimumWidth(125);

    connect(volumeInfos_, SIGNAL(refresh(QTreeWidgetItem*)), this, SLOT(volumeRefresh(QTreeWidgetItem*)));
    connect(volumeInfos_, SIGNAL(add()), volumeLoadButton_, SLOT(loadVolume()));
    connect(volumeInfos_, SIGNAL(remove()), this, SLOT(removeVolume()));
    connect(volumeInfos_, SIGNAL(exportDat()), this, SLOT(exportDat()));

    buttonLayout->addWidget(volumeLoadButton_);

    buttonLayout->addStretch();

    mainLayout->addWidget(volumeInfos_);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(dicomLayout);

    setMinimumWidth(250);
    setMinimumHeight(200);
    update();
}

VolumeContainerWidget::~VolumeContainerWidget() {
}

void VolumeContainerWidget::resizeOnCollapse(bool) {
    adjustSize();
}

QSize VolumeContainerWidget::sizeHint() const {
    return QSize(300, 300);
}

void VolumeContainerWidget::setVolumeContainer(VolumeContainer* volumeContainer) {

    // do not deregister from previously assigned container,
    // since it may not be valid anymore!

    // assign new container
    volumeContainer_ = volumeContainer;
    volumeLoadButton_->setVolumeContainer(volumeContainer);

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


        uint stringLength = 24;
        if(name.size() > stringLength) {
            int end = name.size();
            std::string startString;
            std::string endString;
            for(size_t i = 0; i < 12; i++){
                 startString += name.at(i);
                 endString += name.at(end-12+i);
            }
            name = startString+"..."+endString;
        }
        if(path.size() > stringLength) {
            int end = path.size();
            std::string startString;
            std::string endString;
            for(size_t i = 0; i < 12; i++){
                 startString += path.at(i);
                 endString += path.at(end-12+i);
            }
            path = startString+"..."+endString;
        }

        QFontInfo fontInfo(qtwi->font(0));
        qtwi->setFont(0, QFont(fontInfo.family(), fontSize));
        //qtwi->setFont(0, QFont(QString("Arial"), 7));
        qtwi->setText(0, QString::fromStdString(name
            + " ("+VolumeViewHelper::getVolumeType(volume)+")"+"\n"+path
            +"\nDimensions: " + VolumeViewHelper::getVolumeDimension(volume) + "\nVoxel Spacing: "
            + VolumeViewHelper::getVolumeSpacing(volume) +"\nMemory Size: "
            + VolumeViewHelper::getVolumeMemorySize(volume)));

        qtwi->setIcon(0, QIcon(VolumeViewHelper::generateBorderedPreview(volume, 63, 0)));
        qtwi->setSizeHint(0,QSize(65,65));
        qtwi->setToolTip(0, QString::fromStdString(VolumeViewHelper::getStrippedVolumeName(handle)
            + " ("+VolumeViewHelper::getVolumeType(volume)+")"+"\n"+VolumeViewHelper::getVolumePath(handle)
            +"\nDimensions: " + VolumeViewHelper::getVolumeDimension(volume) + "\nVoxel Spacing: "
            + VolumeViewHelper::getVolumeSpacing(volume) +"\nMemory Size: "
            + VolumeViewHelper::getVolumeMemorySize(volume)));

        volumeInfos_->addTopLevelItem(qtwi);
    }
}

void VolumeContainerWidget::removeVolume() {
    QList<QTreeWidgetItem *> allItems = volumeInfos_->selectedItems();
    QList<QTreeWidgetItem *>::iterator it = allItems.begin();
    VolumeCollection* volumes = new VolumeCollection();
    while(it != allItems.end()) {
        if ((*it)->parent() == 0) {
            volumeInfos_->removeItemWidget(*it, 0);
            volumes->add(volumeContainer_->at(volumeInfos_->indexOfTopLevelItem(*it)));
        }
        it++;
    }
    volumeContainer_->remove(volumes);
    delete volumes;
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

void VolumeContainerWidget::volumeChanged(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/) {
    update();
}

void VolumeContainerWidget::volumeRefresh(QTreeWidgetItem* /*item*/) {
    qApp->processEvents();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QList<QTreeWidgetItem *> allItems = volumeInfos_->selectedItems();
    QList<QTreeWidgetItem *>::iterator it = allItems.begin();
    while(it != allItems.end())
    {
        volumeContainer_->at(volumeInfos_->indexOfTopLevelItem(*it))->reloadVolume();
        it++;
    }
    QApplication::restoreOverrideCursor();
}

void VolumeContainerWidget::loadDicomFiles() {
    volumeLoadButton_->loadDicomFiles();
}

void VolumeContainerWidget::loadVolume() {
    volumeLoadButton_->loadVolume();
}

void VolumeContainerWidget::loadVolumeRawFilter() {
    volumeLoadButton_->loadVolumeRawFilter();
}

void VolumeContainerWidget::exportDat() {
    QList<QTreeWidgetItem *> allItems = volumeInfos_->selectedItems();
    QList<QTreeWidgetItem *>::iterator it = allItems.begin();
    while(it != allItems.end())
    {

        QFileDialog saveAsDialog(0, "Export Volume ");
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
                    volumeContainer_->at(volumeInfos_->indexOfTopLevelItem( (*it))));
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

    it++;
    }

}

} // namespace
