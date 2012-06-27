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

#include "voreen/qt/widgets/property/volumehandlepropertywidget.h"

#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/volume/volumecontainer.h"
#include "voreen/core/application.h"

#include "voreen/qt/widgets/rawvolumewidget.h"
#include "voreen/qt/widgets/volumecontainerwidget.h"
#include "voreen/qt/widgets/volumeviewhelper.h"
#include "voreen/qt/voreenapplicationqt.h"

#include <QApplication>
#include <QDialog>
#include <QErrorMessage>
#include <QFileDialog>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include <math.h>

namespace voreen {


const std::string VolumeHandlePropertyWidget::loggerCat_("voreen.qt.VolumeHandlePropertyWidget");

VolumeHandlePropertyWidget::VolumeHandlePropertyWidget(VolumeHandleProperty* volumeHandleProp, QWidget* parent)
    : QPropertyWidget(volumeHandleProp, parent, false)
    , volumeContainer_(0)
    , rawSelected_(false)
{

    if (!volumeHandleProp) {
        tgtAssert(false, "No volume handle property");
        LERROR("No volume handle property");
        return;
    }

    QVBoxLayout* mainLayout = new QVBoxLayout();

    layout_->addLayout(mainLayout);

    QHBoxLayout* previewLayout = new QHBoxLayout();
    QHBoxLayout* volumeSelectorLayout = new QHBoxLayout();
    QGridLayout* infoLayout = new QGridLayout();

    loadVolumeButton_ = new QPushButton(this);
    previewLabel_ = new QLabel(this);

    loadVolumeButton_->setText(tr("Load Volume"));
    loadVolumeButton_->setIcon(QPixmap(":/voreenve/icons/open-volume.png"));
    volumeSelectorBox_ = new QComboBox(this);
    volumeSelectorBox_->setIconSize(QSize(26, 26));
    volumeSelectorBox_->setMinimumSize(QSize(180, 30));

    QLabel* dimensionLabel = new QLabel(this);
    QLabel* spacingLabel = new QLabel(this);
    QLabel* memSizeLabel = new QLabel(this);

    dimensionLabel->setText(" Dimension");
    spacingLabel->setText(" Spacing");
    memSizeLabel->setText(" MemSize");

    nameLabel_ = new QLabel(this);
    pathLabel_ = new QLabel(this);
    dimensionLabel_ = new QLabel(this);
    spacingLabel_ = new QLabel(this);
    memSizeLabel_ = new QLabel(this);

    nameLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    nameLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    pathLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dimensionLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    spacingLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    memSizeLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    volumeSelectorLayout->addWidget(volumeSelectorBox_);
    volumeSelectorLayout->addStretch();
    volumeSelectorLayout->addWidget(loadVolumeButton_);

    previewLayout->addWidget(previewLabel_);
    previewLayout->addLayout(infoLayout);
    infoLayout->addWidget(pathLabel_, 0, 0, 1, 2, 0);
    infoLayout->addWidget(dimensionLabel, 1, 0);
    infoLayout->addWidget(spacingLabel, 2, 0);
    infoLayout->addWidget(memSizeLabel, 3, 0);

    infoLayout->addWidget(dimensionLabel_, 1, 1);
    infoLayout->addWidget(spacingLabel_, 2, 1);
    infoLayout->addWidget(memSizeLabel_, 3, 1);

    previewLayout->addStretch();

    loadVolumeButton_->setDefault(true);

    connect(loadVolumeButton_,SIGNAL(clicked()),this,SLOT(newFileDialog()));
    connect(volumeSelectorBox_, SIGNAL(activated(int)), this, SLOT(changeVolume(int)));

    mainLayout->addLayout(volumeSelectorLayout);
    mainLayout->addSpacerItem(new QSpacerItem(0,10));
    QHBoxLayout* separatorLayout = new QHBoxLayout();
    QFrame* frame = new QFrame();
    frame->setFrameShape(QFrame::HLine);
    separatorLayout->addWidget(frame);
    separatorLayout->addWidget(nameLabel_);
    frame = new QFrame();
    frame->setFrameShape(QFrame::HLine);
    separatorLayout->addWidget(frame);
    mainLayout->addLayout(separatorLayout);
    mainLayout->addSpacerItem(new QSpacerItem(0,5));
    mainLayout->addLayout(previewLayout);

    mainLayout->setContentsMargins(10,3,3,10);
    updateFromProperty();
}

void VolumeHandlePropertyWidget::newFileDialog() {

    VolumeHandleProperty* handleProp = dynamic_cast<VolumeHandleProperty*>(prop_);
    if (!handleProp) {
        tgtAssert(false, "No volume handle property");
        LERROR("No volume handle property");
        return;
    }

    std::string openPath;
    // use path of currently loaded volume, if one is present,
    // or data path otherwise
    if (handleProp->get()) {
        openPath = handleProp->get()->getOrigin().getPath();
        openPath = tgt::FileSystem::dirName(openPath);
    }
    else {
        openPath = VoreenApplication::app()->getDataPath();
    }
    QDir dir(QString::fromStdString(openPath)); // we want absolute path name to prevent problems when the
                                   // file dialog perform chdir()

    // retrieve the supported extensions from the VolumeSerializerPopulator
    VolumeSerializerPopulator* populator = new VolumeSerializerPopulator();
    std::vector<std::string> extensionVec = populator->getSupportedReadExtensions();
    delete populator;
    std::string extensions = "Volume Files (";
    for(size_t i = 0; i < extensionVec.size(); i++) {
        extensions+= "*."+extensionVec[i]+" ";
    }
    extensions+=")";

    QStringList filters;

    if(rawSelected_)
        filters << "Raw Files (*)" << QString::fromStdString(extensions);
    else
        filters << QString::fromStdString(extensions) << "Raw Files (*)";

    QFileDialog dlg(this, tr("Open file(s) for adding"), dir.absolutePath(), "");
    dlg.setNameFilters(filters);
    dlg.setViewMode(QFileDialog::Detail);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFiles);

    connect(&dlg, SIGNAL(filterSelected(QString)), this, SLOT(filterChanged(QString)));

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getVolumePath().c_str());
    dlg.setSidebarUrls(urls);

    std::vector<std::string> files;
    if (dlg.exec() == QDialog::Accepted) {
        const QStringList& lst = dlg.selectedFiles();
        QStringList::const_iterator it = lst.begin();
        for (; it != lst.end(); ++it)
            files.push_back(it->toStdString());
    }

    for(uint i = 0; i < files.size(); i++) {
        QString filename = QString::fromStdString(files.at(i));
        if(!rawSelected_) {
        if (filename.toStdString() != ""){
            try {
                if (volumeContainer_) {
                    VolumeCollection* collection = volumeContainer_->loadVolume(filename.toStdString());
                    if (!collection->empty()) {
                        handleProp->set(collection->first());
                        updateFromProperty();
                    }
                }
                else {
                    handleProp->loadVolume(filename.toStdString());
                }
            }
            catch(tgt::FileException e) {
                QMessageBox error;
                error.setText(QString::fromStdString(e.what()));
                error.exec();
            }
            catch(std::bad_alloc) {
                QMessageBox error;
                error.setText(QString::fromStdString("std::Error BAD ALLOCATION"));
                error.exec();
            }
        }
    }
    else if (rawSelected_) {

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
                LWARNING("std::Error BAD ALLOCATION");
            }
        }
    }
    }
    }
}


VolumeHandle* VolumeHandlePropertyWidget::getVolume() const {

    VolumeHandleProperty* handleProp = dynamic_cast<VolumeHandleProperty*>(prop_);
    if (!handleProp) {
        LWARNING("No volume handle property");
        return 0;
    }

    return (handleProp->get() ? handleProp->get() : 0);
}

void VolumeHandlePropertyWidget::updateFromProperty() {
    VolumeHandle* handle = getVolume();
    if (handle) {
        dimensionLabel_->show();
        pathLabel_->show();
        spacingLabel_->show();
        memSizeLabel_->show();
        previewLabel_->show();

        std::string name = VolumeViewHelper::getStrippedVolumeName(handle);
        std::string path = VolumeViewHelper::getVolumePath(handle);
        if(name.size() > 30) {
            nameLabel_->setToolTip(QString::fromStdString(name));
            int end = name.size();
            std::string startString;
            std::string endString;
            for(size_t i = 0; i < 14; i++){
                 startString += name.at(i);
                 endString += name.at(end-14+i);
            }
            name = startString+"..."+endString;
        }
        if (path.size() > 30) {
            pathLabel_->setToolTip(QString::fromStdString(path));
            int end = path.size();
            std::string startString;
            std::string endString;
            for(size_t i = 0; i < 14; i++){
                 startString += path.at(i);
                 endString += path.at(end-14+i);
            }
            path = startString+"..."+endString;
        }

        nameLabel_->setText(QString::fromStdString(" " + name+ " ("+ VolumeViewHelper::getVolumeType(handle->getVolume())+") "));
        pathLabel_->setText(QString::fromStdString(" "+path));
        dimensionLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeDimension(handle->getVolume())));
        spacingLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeSpacing(handle->getVolume())));
        memSizeLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeBytes(handle->getVolume())));
        previewLabel_->setPixmap(VolumeViewHelper::generateBorderedPreview(handle->getVolume(), 70, 0));

        // adjust selected index of volume selector box to currently assigned volume
        volumeSelectorBox_->setCurrentIndex(0);
        if (volumeContainer_) {
            tgtAssert(volumeContainer_->size()+1 == static_cast<size_t>(volumeSelectorBox_->count()),
                "Sizes of volume container and volume selector box do not match");
            for (size_t i=0; i < volumeContainer_->size(); ++i) {
                if (handle == volumeContainer_->at(i)) {
                    volumeSelectorBox_->setCurrentIndex(i+1);
                    break;
                }

            }
        }

    }
    else {
        nameLabel_->setText(tr(" no volume"));
        nameLabel_->adjustSize();
        pathLabel_->hide();
        previewLabel_->setPixmap(QPixmap());
        dimensionLabel_->hide();
        spacingLabel_->hide();
        memSizeLabel_->hide();
        previewLabel_->hide();
    }
}

void VolumeHandlePropertyWidget::updateFromContainer() {

    if (!volumeContainer_)
        return;

    volumeSelectorBox_->clear();
    QPixmap white = QPixmap(30, 30);
    white.fill();
    volumeSelectorBox_->addItem(white, tr("Select volume ..."));
    for (size_t i = 0; i < volumeContainer_->size(); i++) {
        VolumeHandle* handle = volumeContainer_->at(i);
        volumeSelectorBox_->addItem(VolumeViewHelper::generateBorderedPreview(handle->getVolume(), 30, 1),
            QString::fromStdString(VolumeViewHelper::getStrippedVolumeName(handle)),
            QString::fromStdString(VolumeViewHelper::getVolumeName(handle)));
    }

    // disable selector box, when no volume available
    volumeSelectorBox_->setEnabled(volumeSelectorBox_->count() > 1);


}

void VolumeHandlePropertyWidget::setVolumeContainer(VolumeContainer* volumeContainer) {

    stopObservation(volumeContainer_);

    volumeContainer_ = volumeContainer;
    updateFromContainer();
    updateFromProperty();

    if (volumeContainer_)
        volumeContainer_->addObserver(this);
}


void VolumeHandlePropertyWidget::volumeAdded(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/) {
    int i = volumeContainer_->size()-1;
    VolumeHandle* handle = volumeContainer_->at(i);
    volumeSelectorBox_->addItem(VolumeViewHelper::generateBorderedPreview(handle->getVolume(), 30, 1),
        QString::fromStdString(VolumeViewHelper::getStrippedVolumeName(handle)),
        QString::fromStdString(VolumeViewHelper::getVolumeName(handle)));

    // disable selector box, when no volume available
    volumeSelectorBox_->setEnabled(volumeSelectorBox_->count() > 1);

    emit modified();    
}

void VolumeHandlePropertyWidget::volumeRemoved(const VolumeCollection* /*source*/, const VolumeHandle* handle) {

    VolumeHandleProperty* handleProp = dynamic_cast<VolumeHandleProperty*>(prop_);          // TODO: Make this consistent. VolumeFilenames aren't unique! e.g. take the path into account
    if (!handleProp)
        return;

    int selected = volumeSelectorBox_->currentIndex();
    int removed = -1;
    for (int i = 0; i < volumeSelectorBox_->count(); i++) {
        if (volumeSelectorBox_->itemText(i).toStdString() == VolumeViewHelper::getStrippedVolumeName(const_cast<VolumeHandle*>(handle)) && volumeSelectorBox_->findData(QString::fromStdString(VolumeViewHelper::getVolumeName(const_cast<VolumeHandle*>(handle))))== i ){
            volumeSelectorBox_->removeItem(i);
            removed = i;
        }
    }

    // update volume selector box
    if (selected == removed) {
        volumeSelectorBox_->setCurrentIndex(0);
        changeVolume(0);
    }
    else if (selected > 0 && removed <= selected) {
        volumeSelectorBox_->setCurrentIndex(selected-1);
    }

    updateFromProperty();

    // disable selector box, when no volume available
    volumeSelectorBox_->setEnabled(volumeSelectorBox_->count() > 1);

    emit modified();
}

void VolumeHandlePropertyWidget::changeVolume(int volumeIndex) {
    VolumeHandleProperty* handleProp = dynamic_cast<VolumeHandleProperty*>(prop_);
    if (!handleProp) {
        tgtAssert(false, "No volume handle property");
        LERROR("No volume handle property");
        return;
    }
    if (volumeIndex != 0) {
        handleProp->set(volumeContainer_->at(volumeIndex - 1)); // -1 offset because there is a none selection available
    }
    else {
        handleProp->set(0);
    }
    emit modified();    
}

void VolumeHandlePropertyWidget::filterChanged(QString filter) {
    VolumeSerializerPopulator* populator = new VolumeSerializerPopulator();
    std::vector<std::string> extensionVec = populator->getSupportedReadExtensions();
    delete populator;
    std::string extensions = "Volume Files (";
    for(size_t i = 0; i < extensionVec.size(); i++) {
        extensions+= "*."+extensionVec[i]+" ";
    }
    extensions+=")";
    if(filter == QString::fromStdString(extensions))
        rawSelected_ = false;
    if(filter == "Raw Files (*)")
        rawSelected_ = true;
    std::cout << std::endl << "jep";

}

} //namespace voreen
