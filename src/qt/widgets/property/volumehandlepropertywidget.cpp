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

#include "voreen/qt/widgets/property/volumehandlepropertywidget.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumecontainer.h"
#include "voreen/core/voreenapplication.h"

#include "voreen/qt/widgets/volumecontainerwidget.h"
#include "voreen/qt/widgets/volumeviewhelper.h"
#include "voreen/qt/voreenapplicationqt.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QErrorMessage>
#include <QFileDialog>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "tgt/vector.h"
#include "tgt/matrix.h"

#include <math.h>

namespace voreen {


const std::string VolumeHandlePropertyWidget::loggerCat_("voreen.qt.VolumeHandlePropertyWidget");

VolumeHandlePropertyWidget::VolumeHandlePropertyWidget(VolumeHandleProperty* volumeHandleProp, QWidget* parent)
    : QPropertyWidget(volumeHandleProp, parent, false)
    , volumeContainer_(0)
{

    if (!volumeHandleProp) {
        tgtAssert(false, "No volume handle property");
        LERROR("No volume handle property");
        return;
    }
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setAlignment(Qt::AlignLeft);
    mainLayout->setContentsMargins(0, 2, 0, 0);

    layout_->addLayout(mainLayout);

    QHBoxLayout* previewLayout = new QHBoxLayout();
    previewLayout->setContentsMargins(2, 0, 2, 2);
    QGridLayout* infoLayout = new QGridLayout();
    infoLayout->setContentsMargins(2, 0, 2, 2);

    previewLabel_ = new QLabel(this);

    volumeSelectorBox_ = new QComboBox(this);

    volumeLoadButton_ = new VolumeLoadButton(volumeContainer_, this);

    connect(volumeLoadButton_, SIGNAL(VolumeAdded(int)), this, SLOT(changeVolume(int)));

    CustomLabel* dimensionLabel = new CustomLabel(this);
    CustomLabel* spacingLabel = new CustomLabel(this);
    CustomLabel* memSizeLabel = new CustomLabel(this);

    dimensionLabel->setText(" Dimensions");
    spacingLabel->setText(" Spacing");
    memSizeLabel->setText(" MemSize");

    volumeNameLabel_ = new CustomLabel(this);
    pathLabel_ = new CustomLabel(this);
    dimensionLabel_ = new CustomLabel(this);
    spacingLabel_ = new CustomLabel(this);
    memSizeLabel_ = new CustomLabel(this);

    volumeNameLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    volumeNameLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    pathLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dimensionLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    spacingLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    memSizeLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QHBoxLayout* volumeLayout = new QHBoxLayout();
    volumeLayout->setContentsMargins(0, 0, 4, 0);
    volumeLayout->setSpacing(4);
    volumeLayout->setMargin(0);
    volumeLayout->addWidget(volumeSelectorBox_, 2);
    volumeLayout->addWidget(volumeLoadButton_, 1);
    volumeSelectorBox_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    volumeSelectorBox_->setMinimumWidth(100);
    volumeLoadButton_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    volumeLoadButton_->setMinimumWidth(105);
    mainLayout->addLayout(volumeLayout);

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

    //loadVolumeButton_->setDefault(true);

    connect(volumeSelectorBox_, SIGNAL(activated(int)), this, SLOT(changeVolume(int)));
    connect(volumeSelectorBox_, SIGNAL(activated(int)), this, SIGNAL(widgetChanged()));

    QHBoxLayout* separatorLayout = new QHBoxLayout();
    QFrame* frame = new QFrame();
    frame->setFrameShape(QFrame::HLine);
    separatorLayout->addWidget(frame);
    separatorLayout->addWidget(volumeNameLabel_);
    frame = new QFrame();
    frame->setFrameShape(QFrame::HLine);
    separatorLayout->addWidget(frame);
    mainLayout->addLayout(separatorLayout);
    mainLayout->addLayout(previewLayout);

    updateFromProperty();
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
            volumeNameLabel_->setToolTip(QString::fromStdString(name));
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

        volumeNameLabel_->setText(QString::fromStdString(" " + name+ " ("+ VolumeViewHelper::getVolumeType(handle->getVolume())+") "));
        pathLabel_->setText(QString::fromStdString(" "+path));
        dimensionLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeDimension(handle->getVolume())));
        spacingLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeSpacing(handle->getVolume())));
        memSizeLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeMemorySize(handle->getVolume())));
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
        volumeNameLabel_->setText(tr(" no volume"));
        volumeNameLabel_->adjustSize();
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
    volumeLoadButton_->setVolumeContainer(volumeContainer);
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

    VolumeHandleProperty* handleProp = dynamic_cast<VolumeHandleProperty*>(prop_);
    if (!handleProp)
        return;

    int selected = volumeSelectorBox_->currentIndex();
    int removed = -1;
    for (int i = 0; i < volumeSelectorBox_->count(); i++) {
        if (volumeSelectorBox_->itemText(i).toStdString() == VolumeViewHelper::getStrippedVolumeName(const_cast<VolumeHandle*>(handle)) && volumeSelectorBox_->findData(QString::fromStdString(VolumeViewHelper::getVolumeName(const_cast<VolumeHandle*>(handle))))== i ){
            volumeSelectorBox_->removeItem(i);
            removed = i;
            break;
        }
    }

    tgtAssert(volumeContainer_->size()+1 == static_cast<size_t>(volumeSelectorBox_->count()),
        "Sizes of volume container and volume selector box do not match");

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

void VolumeHandlePropertyWidget::volumeChanged(const VolumeCollection* /*source*/, const VolumeHandle* /*handle*/) {
    updateFromContainer();
    updateFromProperty();

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

void VolumeHandlePropertyWidget::showNameLabel(bool) {
    if(nameLabel_)
        nameLabel_->hide();
}

CustomLabel* VolumeHandlePropertyWidget::getNameLabel() const {
    return 0;
}


} //namespace voreen
