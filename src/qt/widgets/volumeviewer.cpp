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

#include "tgt/matrix.h"

#include "voreen/qt/widgets/volumeviewer.h"
#include "voreen/qt/widgets/volumeviewhelper.h"
#include "voreen/qt/widgets/rawvolumewidget.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/io/progressbar.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/network/processornetwork.h"
#include "voreen/core/ports/volumeport.h"
#include "voreen/core/voreenapplication.h"

#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/qt/progressdialog.h"

#include <set>

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
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QMessageBox>

#include <QApplication>

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
    setTextElideMode(Qt::ElideMiddle);
}

void QRCTreeWidget::contextMenuEvent(QContextMenuEvent* cmevent) {
    if (itemAt(cmevent->pos())){
        QMenu* menu = new QMenu();
        menu->addAction(QPixmap(":/voreenve/icons/export.png"), "Export...");
        QAction* action = menu->exec(mapToGlobal(cmevent->pos()));
        if(action) {
            if(action->text() == "Export...")
                emit exportVolumes();
            }
    }
}

void QRCTreeWidget::keyPressEvent(QKeyEvent* event) {
    QTreeWidget::keyPressEvent(event);
}

// ---------------------------------------------------------------------------

const std::string VolumeViewer::loggerCat_("voreen.qt.VolumeContainerWidget");

VolumeViewer::VolumeViewer(QWidget* parent)
    : QWidget(parent, Qt::Tool)
    , NetworkEvaluator::ProcessWrapper()
    , evaluator_(0)
    , volumeIOHelper_(parent)
    , updateRequired_(true)
{
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    volumeInfos_ = new QRCTreeWidget();
    QTreeWidgetItem* header = volumeInfos_->headerItem();
    header->setText(0, tr("Volumes"));
    volumeInfos_->header()->hide();
    volumeInfos_->setColumnCount(1);
    volumeInfos_->show();
    volumeInfos_->setIconSize(QSize(63,63));

    updateButton_ = new QPushButton(tr("Update"), this);
    updateButton_->setIcon(QPixmap(":/icons/reload.png"));
    updateButton_->setToolTip(tr("Update view from network"));

    autoUpdateCheckbox_ = new QCheckBox(tr("auto"));
    autoUpdateCheckbox_->setToolTip(tr("Automatically update view after each network processing (may be slow!)"));
    autoUpdateCheckbox_->setChecked(true);

    containerInfo_ = new QLabel(this);

    buttonLayout->addWidget(updateButton_);
    buttonLayout->addWidget(autoUpdateCheckbox_);
    buttonLayout->addStretch();
    buttonLayout->addWidget(containerInfo_);

    mainLayout->addWidget(volumeInfos_);
    mainLayout->addLayout(buttonLayout);

    connect(updateButton_, SIGNAL(clicked()),
        this, SLOT(updateFromNetwork()));
    connect(autoUpdateCheckbox_, SIGNAL(stateChanged(int)),
        this, SLOT(updateStateChanged(int)));

    connect(volumeInfos_, SIGNAL(exportVolumes()),
        this, SLOT(exportVolumes()));
    connect(&volumeIOHelper_, SIGNAL(volumeSaved(const VolumeBase*, const std::string&)),
        this, SLOT(volumeSaved(const VolumeBase*, const std::string&)));

    setMinimumWidth(250);
    setMinimumHeight(125);
}

VolumeViewer::~VolumeViewer() {
}

void VolumeViewer::resizeOnCollapse(bool) {
    //adjustSize();
}

QSize VolumeViewer::sizeHint() const {
    return QSize(300, 300);
}

void VolumeViewer::setNetworkEvaluator(NetworkEvaluator* evaluator) {
    if (evaluator_)
        evaluator_->removeProcessWrapper(this);

    evaluator_ = evaluator;

    if (evaluator_)
        evaluator_->addProcessWrapper(this);

    updateRequired_ = true;
    update();
}

void VolumeViewer::volumeHandleDelete(const VolumeBase* /*source*/) {
    // force update even with auto-update disabled,
    // since we do not want to store invalid handle pointers
    if (isVisible()) {
        updateRequired_ = true;
        update();
    }
}

void VolumeViewer::volumeChange(const VolumeBase* /*source*/) {
    // force update even with auto-update disabled,
    // since we do not want to store invalid handle pointers
    if (isVisible()) {
        updateRequired_ = true;
        update();
    }
}

void VolumeViewer::afterNetworkProcess() {
    if (isVisible() && autoUpdateCheckbox_->isChecked()) {
        updateRequired_ = true;
        update();
    }
}

void VolumeViewer::afterNetworkInitialize() {
    if (isVisible() && autoUpdateCheckbox_->isChecked()) {
        updateRequired_ = true;
        update();
    }
}

void VolumeViewer::beforeNetworkDeinitialize() {
    clear();
}

void VolumeViewer::clear() {
    volumeHandles_.clear();
    handleToHashMap_.clear();
    handleToPortMap_.clear();

    tgtAssert(volumeInfos_, "no volumeInfos_");
    tgtAssert(containerInfo_, "no containerInfo_");
    volumeInfos_->clear();
    containerInfo_->clear();
}

void VolumeViewer::paintEvent(QPaintEvent* event) {
    if (updateRequired_) {
        if (!evaluator_) {
            updateRequired_ = false;
            clear();
        }
        else if (!evaluator_->isLocked()) {
            updateRequired_ = false;
            updateFromNetwork();
        }
    }

    QWidget::paintEvent(event);
}

void VolumeViewer::showEvent(QShowEvent* /*event*/) {
    if (isVisible() && autoUpdateCheckbox_->isChecked()) {
        updateRequired_ = true;
        update();
    }
}

void VolumeViewer::updateFromNetwork() {

    if (!evaluator_ || !evaluator_->getProcessorNetwork()) {
        clear();
        return;
    }

    const ProcessorNetwork* network = evaluator_->getProcessorNetwork();
    tgtAssert(network, "no network");

    // retrieve volumes from network (outports)
    std::vector<const VolumeBase*> tempVolumes;
    std::map<const VolumeBase*, std::string> tempHashMap_;
    std::map<const VolumeBase*, Port*> tempPortMap_;
    for (size_t i=0; i<network->getProcessors().size(); i++) {
        Processor* processor = network->getProcessors().at(i);
        tgtAssert(processor, "no processor");
        const std::vector<Port*>& ports = processor->getOutports();
        for (size_t j=0; j<ports.size(); j++) {
            if (dynamic_cast<VolumePort*>(ports.at(j))) {
                const VolumeBase* handle = static_cast<VolumePort*>(ports.at(j))->getData();
                if (handle) {
                    tempVolumes.push_back(handle);
                    // TODO hashing seems to be quite slow at times
                    //tempHashMap_.insert(std::make_pair(handle, handle->getHash()));
                    tempPortMap_.insert(std::make_pair(handle, ports.at(j)));
                }
            }
        }
    }

    // check wether network volumes have changed
    bool changed = false;
    if (tempVolumes.size() != volumeHandles_.size())
        changed = true;
    else {
        for (size_t i=0; i<tempVolumes.size() && !changed; i++) {
            if (tempVolumes.at(i) != volumeHandles_.at(i))
                changed = true;
            else {
                // TODO hashing seems to be quite slow at times
                /*tgtAssert(tempHashMap_.find(tempVolumes.at(i)) != tempHashMap_.end(), "missing entry in tempHashMap");
                tgtAssert(handleToHashMap_.find(volumeHandles_.at(i)) != handleToHashMap_.end(), "missing entry in handleToHashMap_");
                if (tempHashMap_[tempVolumes.at(i)] != handleToHashMap_[volumeHandles_.at(i)])
                    changed = true; */
            }
        }
    }

    if (!changed)
        return;

    clear();

    // update widget from new volumes
    volumeHandles_ = tempVolumes;
    handleToHashMap_ = tempHashMap_;
    handleToPortMap_ = tempPortMap_;

    for (size_t i=0; i<volumeHandles_.size(); i++) {
        const VolumeBase* handle = volumeHandles_.at(i);
        if (!handle->isObservedBy(this))
            handle->addObserver(this);

        tgtAssert(handleToPortMap_.find(handle) != handleToPortMap_.end(),
            "missing entry in handleToPortMap");
        QTreeWidgetItem* treeWidgetItem = createTreeWidgetItem(handle, handleToPortMap_[handle]);
        volumeInfos_->addTopLevelItem(treeWidgetItem);
    }

    if (!volumeHandles_.empty())
        containerInfo_->setText(QString::fromStdString(calculateVolumeSizeString(volumeHandles_)));
}

QTreeWidgetItem* VolumeViewer::createTreeWidgetItem(const VolumeBase* handle, const Port* port) {
    tgtAssert(handle, "no handle");
    tgtAssert(port, "no port");

    const VolumeRAM* volume = handle->getRepresentation<VolumeRAM>();
    tgtAssert(volume, "no volume");

    QTreeWidgetItem* treeItem = new QTreeWidgetItem(volumeInfos_);
    std::string name = VolumeViewHelper::getStrippedVolumeName(handle);
    std::string path = VolumeViewHelper::getVolumePath(handle);
    if (name.empty())
        name = "<unnamed>";

    QFontInfo fontInfo(treeItem->font(0));

    //QLabel* infos = new QLabel(QString(name.c_str()) + " (" + QString(VolumeViewHelper::getVolumeType(volume).c_str()) + ")" + QString(QChar::LineSeparator)
    //              + QString(path.c_str()) + QString(QChar::LineSeparator)
    //              + "Dimension: " + QString(VolumeViewHelper::getVolumeDimension(volume).c_str()));
    //infos->setWordWrap(true);

    treeItem->setFont(0, QFont(fontInfo.family(), fontSize));
    treeItem->setText(0, QString::fromStdString(port->getQualifiedName()) + QString(QChar::LineSeparator)
        + QString::fromStdString(name) + " (" + QString::fromStdString(VolumeViewHelper::getVolumeType(volume)) + ")" + QString(QChar::LineSeparator)
        + QString::fromStdString(path) + QString(QChar::LineSeparator)
        + "Dimension: " + QString::fromStdString(VolumeViewHelper::getVolumeDimension(handle)));

    treeItem->setIcon(0, QIcon(VolumeViewHelper::generateBorderedPreview(handle, 63, 0)));
    treeItem->setSizeHint(0,QSize(65,65));
    treeItem->setToolTip(0, QString::fromStdString(port->getQualifiedName()) + "\n"
        + QString::fromStdString(name
            + " ("+VolumeViewHelper::getVolumeType(volume)+")"+ "\n"+ path
            + "\nDimensions: " + VolumeViewHelper::getVolumeDimension(handle) + "\nVoxel Spacing: "
            + VolumeViewHelper::getVolumeSpacing(handle) +"\nMemory Size: "
            + VolumeViewHelper::getVolumeMemorySize(volume)));

    return treeItem;
}

std::string VolumeViewer::calculateVolumeSizeString(const std::vector<const VolumeBase*>& handles) {

    size_t volumeSize = 0;

    // use set to eliminate duplicates
    std::set<const VolumeBase*> handleSet;
    handleSet.insert(handles.begin(), handles.end());

    for (std::set<const VolumeBase*>::iterator it = handleSet.begin(); it != handleSet.end(); ++it) {
        const VolumeRAM* volume = (*it)->getRepresentation<VolumeRAM>();
        volumeSize += VolumeViewHelper::getVolumeMemorySizeByte(volume);
    }
    int volumeCount = static_cast<int>(handleSet.size());

    size_t bytes = volumeSize;
    std::stringstream out;
    if(volumeCount == 1) {
        out << volumeCount << " Volume (";
    }
    else if (volumeCount > 1) {
        out << volumeCount << " Volumes (";
    }
    float mb = tgt::round(bytes/104857.6f) / 10.f;    //calculate mb with 0.1f precision
    float kb = tgt::round(bytes/102.4f) / 10.f;
    if (mb >= 0.5f) {
        out << mb << " MB)";
    }
    else if (kb >= 0.5f) {
        out << kb << " kB)";
    }
    else {
        out << bytes << " bytes)";
    }
    return out.str();
}

void VolumeViewer::exportVolumes() {
    QList<QTreeWidgetItem *> allItems = volumeInfos_->selectedItems();
    QList<QTreeWidgetItem *>::iterator it = allItems.begin();
    while (it != allItems.end()) {
        int itemIndex = volumeInfos_->indexOfTopLevelItem((*it));
        tgtAssert(itemIndex < (int)volumeHandles_.size(), "invalid item index");
        tgtAssert(volumeHandles_.at(itemIndex), "volume handle null pointer");
        volumeIOHelper_.showFileSaveDialog(volumeHandles_.at(itemIndex));
        it++;
    }
}

void VolumeViewer::updateStateChanged(int /*state*/) {
    if (isVisible() && autoUpdateCheckbox_->isChecked()) {
        updateRequired_ = true;
        update();
    }
}

void VolumeViewer::volumeSaved(const VolumeBase* /*handle*/, const std::string& exportPath) {
    QMessageBox::information(this, "Volume Saved",
        QString::fromStdString("Saved volume to: " + exportPath));
}

} // namespace
