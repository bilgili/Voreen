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

#ifndef VRN_VOLUMESETWIDGET_H
#include "voreen/qt/widgets/volumesetwidget.h"

#ifndef VRN_VOLUMESERIALIZER_H
#include "voreen/core/io/volumeserializer.h"
#endif

namespace voreen
{

// public methods
//

VolumeSetWidget::VolumeSetWidget(VolumeSetContainer* const volumeSetContainer,
                                 QWidget* parent, int levels, Qt::WFlags flags) : 
    QWidget(parent, flags),
    ioObserver_(new IOObserver(this)),
    volumeSetContainer_(volumeSetContainer),
    generateVolumeGL_(true), 
    availableLevelsMask_(levels),
    fileExtFilter_("Volume data (*.DAT *.I4D *.PVM *.RDM *.RDI *.HDR *.SW *.SEG *.TUV *.ZIP *.TIFF *.TIF *.MAT *.HV *.NRRD *.NHDR)"),
    currentDir_(QDir::currentPath().toStdString()),
    allowAddingMultipleFiles_(true)
{
    volumeSerializerPopulator_ = new VolumeSerializerPopulator(ioObserver_);
    createWidgets();

    connect(btnLoad_, SIGNAL(clicked()), this, SLOT(buttonAddClicked()));
    connect(btnUnload_, SIGNAL(clicked()), this, SLOT(buttonRemoveClicked()));
    connect(treeVolumeSets_, SIGNAL(itemSelectionChanged()), this, SLOT(treeItemSelectionChanged()));

    VolumeRootItem* root = new VolumeRootItem("VolumeSetContainer", 0);
    treeVolumeSets_->addTopLevelItem(root);
    root->setExpanded(true);
    root->setSelected(true);
}

VolumeSetWidget::~VolumeSetWidget() {
    delete lblVolumeSets_;
    delete treeVolumeSets_;
    delete btnLoad_;
    delete btnUnload_;
    delete ioObserver_;
}

void VolumeSetWidget::updateContent() {
    if( (volumeSetContainer_ == 0) || (treeVolumeSets_ == 0) )
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

void VolumeSetWidget::setGenerateVolumeGL(const bool generateVolumeGL) {
    generateVolumeGL_ = generateVolumeGL;
}

bool VolumeSetWidget::getGenerateVolumeGL() const {
    return generateVolumeGL_;
}

std::vector<std::string> VolumeSetWidget::openFileDialog() {
    QFileDialog dlg(this, tr("Open File(s) for adding"), QString(currentDir_.c_str()), QString(fileExtFilter_.c_str()));
    dlg.setViewMode(QFileDialog::Detail);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    if( allowAddingMultipleFiles_ == true )
        dlg.setFileMode(QFileDialog::ExistingFiles);
    else
        dlg.setFileMode(QFileDialog::ExistingFile);

    std::vector<std::string> filenames;
    if( dlg.exec() == QDialog::Accepted ) {
        QDir dir = dlg.directory();
        currentDir_ = dir.canonicalPath().toStdString();
        const QStringList& lst = dlg.selectedFiles();
        QStringList::const_iterator it = lst.begin();
        for( ; it != lst.end(); ++it ) {
            filenames.push_back(it->toStdString());
        }
    }
    return filenames;
}

void VolumeSetWidget::addVolumeSets(const std::vector<std::string>& filenames) {
    if( (volumeSerializerPopulator_ == 0) || (volumeSetContainer_ == 0) )
        return;

    VolumeSerializer* serializer = volumeSerializerPopulator_->getVolumeSerializer();
    if( serializer == 0 )
        return;

    for( size_t i = 0; i < filenames.size(); i++ ) {
        const QString qname(filenames[i].c_str());
        QFileInfo fi(qname);
        if( fi.exists() == false ) {
            printf("\tERROR: File '%s' does not exist!\n", filenames[i].c_str());
            continue;
        }
        
        ioObserver_->show(filenames[i]);
        VolumeSet* volumeset = serializer->load(filenames[i], generateVolumeGL_);
        ioObserver_->hide();

        if( volumeset == 0 ) {
            printf("\tERROR: Deserialization of VolumeSet '%s' failed!\n", filenames[i].c_str());
            continue;
        }

        if( volumeSetContainer_->addVolumeSet(volumeset) == false )
            printf("\tWARNING: VolumeSet '%s' could not be added to VolumeSetContainer. It may already be contained.\n", volumeset->getName().c_str());
    }
}

void VolumeSetWidget::addVolumeSeries(const std::vector<std::string>& filenames, VolumeSet* volumeSet) {
    if( (volumeSerializerPopulator_ == 0) || (volumeSet == 0) )
        return;

    VolumeSerializer* serializer = volumeSerializerPopulator_->getVolumeSerializer();
    for( size_t i = 0; i < filenames.size(); i++ ) {
        const QString qname(filenames[i].c_str());
        QFileInfo fi(qname);
        if( fi.exists() == false ) {
            printf("\tERROR: File '%s' does not exist!\n", filenames[i].c_str());
            continue;
        }

        ioObserver_->show(filenames[i]);
        VolumeSet* vs = serializer->load(filenames[i], generateVolumeGL_);
        ioObserver_->hide();

        if( vs == 0 ) {
            printf("\tERROR: Deserialization of VolumeSet '%s' failed!\n", filenames[i].c_str());
            continue;
        }

        const VolumeSeries::SeriesSet& series = vs->getSeries();
        VolumeSeries::SeriesSet::const_iterator it = series.begin();
        for( ; it != series.end(); ++it ) {
            VolumeSeries* s = *it;
            if( volumeSet->addSeries(s) == false )
                printf("\tWARNING: VolumeSeries '%s' could not be added to VolumeSet '%s'. It may already be contained.\n", s->getName().c_str(), vs->getName().c_str());
//printf("\tadded VolumeSeries '%s' to VolumeSet '%s'...\n", s->getName().c_str(), volumeSet->getName().c_str());
        }
    }
}

void VolumeSetWidget::addVolumeHandles(const std::vector<std::string>& filenames, VolumeSeries* volumeSeries) {
    if( (volumeSerializerPopulator_ == 0) || (volumeSeries == 0) )
        return;

    VolumeSerializer* serializer = volumeSerializerPopulator_->getVolumeSerializer();
    for( size_t i = 0; i < filenames.size(); i++ ) {
        const QString qname(filenames[i].c_str());
        QFileInfo fi(qname);
        if( fi.exists() == false ) {
            printf("\tERROR: File '%s' does not exist!\n", filenames[i].c_str());
            continue;
        }

        ioObserver_->show(filenames[i]);
        VolumeSet* volumeset = serializer->load(filenames[i], generateVolumeGL_);
        ioObserver_->hide();

        if( volumeset == 0 ) {
            printf("\tERROR: Deserialization of VolumeSet '%s' failed!\n", filenames[i].c_str());
            continue;
        }

        const VolumeSeries::SeriesSet& series = volumeset->getSeries();
        VolumeSeries::SeriesSet::const_iterator it = series.begin();
        for( ; it != series.end(); ++it ) {
            VolumeSeries* s = *it;
            if( s == 0 )
                continue;

            const VolumeHandle::HandleSet& handles = s->getVolumeHandles();
            VolumeHandle::HandleSet::const_iterator itHandles = handles.begin();
            for( ; itHandles != handles.end(); ++itHandles ) {
                VolumeHandle* handle = *itHandles;
                if( volumeSeries->addVolumeHandle(handle) == false )
                    printf("\tWARNING: VolumeHandle '%f' could not be added to VolumeSeries '%s'. It may already be contained.\n", handle->getTimestep(), volumeSeries->getName().c_str());
//printf("\tadded VolumeHandle '%f' to VolumeSeries '%s'...\n", handle->getTimestep(), s->getName().c_str());
            }
        }
    }
}

VolumeSet* VolumeSetWidget::loadVolumeSet(const std::string& filename) {
    if( (volumeSerializerPopulator_ == 0) 
        || (volumeSetContainer_ == 0)
        || (filename.empty() == true) ) {
        return 0;
    }

    QString qname(filename.c_str());
    QFileInfo fi(qname);
    currentDir_ = fi.canonicalPath().toStdString();

    VolumeSerializer* serializer = volumeSerializerPopulator_->getVolumeSerializer();
    if( serializer == 0 )
        return 0;

    ioObserver_->show(fi.canonicalFilePath().toStdString());
    VolumeSet* volumeset = serializer->load(fi.canonicalFilePath().toStdString(), generateVolumeGL_);
    ioObserver_->hide();

    if( volumeset == 0 ) {
        printf("\tERROR: Deserialization of VolumeSet '%s' failed!\n", filename.c_str());
        return 0;
    }

    if( volumeSetContainer_->addVolumeSet(volumeset) == false )
        printf("\tWARNING: VolumeSet '%s' could not be added to VolumeSetContainer. It may already be contained.\n", volumeset->getName().c_str());
    else
        updateContent();

    return volumeset;
}

// slots
//

void VolumeSetWidget::buttonAddClicked() {
    if( treeVolumeSets_ == 0 )
        return;

    const QList<QTreeWidgetItem*>& items = treeVolumeSets_->selectedItems();
    if( items.empty() == true )
        return;

    AbstractVolumeSetTreeItem* item = dynamic_cast<AbstractVolumeSetTreeItem*>(items.first());
    if( item != 0 ) {
        std::vector<std::string> filenames = openFileDialog();
        if( filenames.empty() == false ) {
            if( item->getType() == typeid(void*) ) {
                addVolumeSets(filenames);
            } else if( item->getType() == typeid(VolumeSet*) ) {
                VolumeSetItem* volsetItem = dynamic_cast<VolumeSetItem*>(item);
                if( volsetItem != 0 )
                    addVolumeSeries(filenames, volsetItem->getItemData());
            } else if( item->getType() == typeid(VolumeSeries*) ) {
                VolumeSeriesItem* volseriesItem = dynamic_cast<VolumeSeriesItem*>(item);
                if( volseriesItem != 0 )
                    addVolumeHandles(filenames, volseriesItem->getItemData());
            }
            updateContent();
        }
    }
}

void VolumeSetWidget::buttonRemoveClicked() {
    if( treeVolumeSets_ == 0 )
        return;

    const QList<QTreeWidgetItem*>& items = treeVolumeSets_->selectedItems();
    if( items.empty() == true )
        return;

    AbstractVolumeSetTreeItem* item = dynamic_cast<AbstractVolumeSetTreeItem*>(items.first());
    if( item == 0 )
        return;

    if( item->getType() == typeid(VolumeSet*) ) {
        VolumeSetItem* volsetItem = dynamic_cast<VolumeSetItem*>(item);
        if( volsetItem != 0 ) {
            volumeSetContainer_->deleteVolumeSet(volsetItem->getItemData());
            if( volsetItem->parent() != 0 )
                volsetItem->parent()->removeChild(volsetItem);
            // finally delete the item itself
            //
            delete volsetItem;
        }
    } else if( item->getType() == typeid(VolumeSeries*) ) {
        // if the item is a VolumeSeriesItem, cast it
        //
        VolumeSeriesItem* seriesItem = dynamic_cast<VolumeSeriesItem*>(item);
        if( seriesItem != 0 ) {
            // if the item has been successfully casted, get its parent item which should
            // be a VolumeSetItem.
            //
            VolumeSetItem* volsetItem = dynamic_cast<VolumeSetItem*>(seriesItem->parent());
            if( (volsetItem != 0) && (volsetItem->getItemData() != 0) ) {
                // now delete the VolumeSeries contained in the VolumeSeriesItem 
                // from the parent VolumeSet contained in the parent VolumeSetItem
                //
                volsetItem->getItemData()->deleteSeries(seriesItem->getItemData());
                volsetItem->removeChild(seriesItem);
            }
            // finally delete the item itself
            //
            delete seriesItem;
        }
    } else if( item->getType() == typeid(VolumeHandle*) ) {
        // if the item is a VolumeHandleItem, cast it
        //
        VolumeHandleItem* handleItem = dynamic_cast<VolumeHandleItem*>(item);
        if( handleItem != 0 ) {
            // if the item has been successfully casted, get its parent item which should
            // be a VolumeSeriesItem.
            //
            VolumeSeriesItem* seriesItem = dynamic_cast<VolumeSeriesItem*>(handleItem->parent());
            if( (seriesItem != 0) && (seriesItem->getItemData() != 0) ) {
                // now delete the VolumeHandle contained in the VolumeHandleItem 
                // from the parent VolumeSeries contained in the parent VolumeSeriesItem
                //
                seriesItem->getItemData()->deleteVolumeHandle(handleItem->getItemData());
                seriesItem->removeChild(handleItem);
            }
            // finally delete the item itself
            //
            delete handleItem;
        }
    }
}

void VolumeSetWidget::treeItemSelectionChanged() {
    if( (treeVolumeSets_ == 0) || (btnLoad_ == 0) )
        return;

    const QList<QTreeWidgetItem*>& items = treeVolumeSets_->selectedItems();
    if( items.empty() == true )
        return;

    AbstractVolumeSetTreeItem* item = dynamic_cast<AbstractVolumeSetTreeItem*>(items.first());
    if( item->getType() == typeid(void*) ) {
        btnLoad_->setEnabled(true);
        btnLoad_->setText("+ add VolumeSet");
        btnUnload_->setEnabled(false);
        btnUnload_->setText("not possible");
    } else if( item->getType() == typeid(VolumeSet*) ) {
        btnLoad_->setEnabled(true);
        btnLoad_->setText("+ add VolumeSeries");
        btnUnload_->setEnabled(true);
        btnUnload_->setText("- remove VolumeSet");
    } else if( item->getType() == typeid(VolumeSeries*) ) {
        btnLoad_->setEnabled(true);
        btnLoad_->setText("+ add VolumeHandle");
        btnUnload_->setEnabled(true);
        btnUnload_->setText("- remove VolumeSeries");
    } else if( item->getType() == typeid(VolumeHandle*) ) {
        btnLoad_->setEnabled(false);
        btnLoad_->setText("not possible");
        btnUnload_->setEnabled(true);
        btnUnload_->setText("- remove VolumeHandle");
    }
}

// private methods
//

void VolumeSetWidget::appendVolumeSets(VolumeRootItem* node) {
    if( (node == 0) || (volumeSetContainer_ == 0) )
        return;

    const VolumeSet::VSPSet& volsets = volumeSetContainer_->getVolumeSets();
    VolumeSet::VSPSet::const_iterator it = volsets.begin();
    for( ; it != volsets.end(); ++it ) {
        VolumeSet* volumeSet = *it;
        if( volumeSet == 0 )
            continue;
              
        if( (availableLevelsMask_ & VolumeSetWidget::LEVEL_VOLUMESETS) != 0 ) {
            QTreeWidgetItem* volsetItem = new VolumeSetItem(volumeSet->getName(), volumeSet);
            appendVolumeSeries(volsetItem, volumeSet);
            node->addChild( volsetItem );
        } else {
            appendVolumeSeries(node, volumeSet);
        }
    }
}

void VolumeSetWidget::appendVolumeSeries(QTreeWidgetItem* node, VolumeSet* const volumeSet) {
    if( (node == 0) || (volumeSet == 0) )
        return;

    const VolumeSeries::SeriesSet& series = volumeSet->getSeries();
    VolumeSeries::SeriesSet::const_iterator it = series.begin();
    for( ; it != series.end(); ++it ) {
        VolumeSeries* series = *it;
        if( series == 0 )
            continue;

        if( (availableLevelsMask_ & VolumeSetWidget::LEVEL_VOLUMESERIES) != 0 ) {
            QTreeWidgetItem* seriesItem = new VolumeSeriesItem(series->getName(), series);
            appendVolumeHandles(seriesItem, series);
            node->addChild( seriesItem );
        } else {
            appendVolumeHandles(node, series);
        }
    }

}

void VolumeSetWidget::appendVolumeHandles(QTreeWidgetItem* node, VolumeSeries* const volumeSeries) {
    if( (node == 0) || (volumeSeries == 0) )
        return;

    const VolumeHandle::HandleSet& handles = volumeSeries->getVolumeHandles();
    VolumeHandle::HandleSet::const_iterator it = handles.begin();
    for( ; it != handles.end(); ++it ) {
        VolumeHandle* handle = *it;
        if( handle == 0 )
            continue;

        if( (availableLevelsMask_ & VolumeSetWidget::LEVEL_VOLUMEHANDLES) != 0 ) {
            char buffer[20] = {0};
            sprintf(buffer, "%f", handle->getTimestep());
            QTreeWidgetItem* handleItem = new VolumeHandleItem(std::string(buffer), handle);
            node->addChild( handleItem );
        }
    }
}

void VolumeSetWidget::createWidgets() {
	lblVolumeSets_ = new QLabel("currently loaded volume sets:", this);
    treeVolumeSets_ = new VolumeSetTreeWidget(this);

	btnLoad_ = new QPushButton("+ add", this);
    btnLoad_->setEnabled(true);
	btnUnload_ = new QPushButton("- remove", this);
    btnUnload_->setEnabled(true);

	QGridLayout* layout = new QGridLayout(this);
	layout->addWidget(lblVolumeSets_, 0, 0, 1, 2);
    layout->addWidget(treeVolumeSets_, 1, 0, 1, 2);
	layout->addWidget(btnLoad_, 2, 0, 1, 1);
	layout->addWidget(btnUnload_, 2, 1, 1, 1);
    setLayout(layout);
    adjustSize();
}

// ---------------------------------------------------------------------------

}   // namespace

#endif
