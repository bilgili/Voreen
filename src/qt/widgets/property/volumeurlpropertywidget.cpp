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

#include "voreen/qt/widgets/property/volumeurlpropertywidget.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/voreenapplication.h"

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
#include <QToolButton>
#include <QAction>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "tgt/vector.h"
#include "tgt/matrix.h"

#include <math.h>

#ifdef VRN_MODULE_GDCM
#include "modules/gdcm/qt/dicomconnectiondialog.h"
#endif

namespace voreen {


const std::string VolumeURLPropertyWidget::loggerCat_("voreen.qt.VolumeHandlePropertyWidget");

VolumeURLPropertyWidget::VolumeURLPropertyWidget(VolumeURLProperty* volumeHandleProp, QWidget* parent)
    : QPropertyWidget(volumeHandleProp, parent, false)
    , volumeIOHelper_(parent, VolumeIOHelper::SINGLE_FILE)
#ifdef VRN_MODULE_GDCM
    , dicomConnectionDialog_(new DicomConnectionDialog(this))
#else
    , dicomConnectionDialog_(0)
#endif
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

    //loadButton_ = new QToolButton(tr("Load Volume..."));
    loadButton_ = new QToolButton();
    loadButton_->setIcon(QPixmap(":/qt/icons/open-volume.png"));
    loadButton_->setText(tr("Load Volume..."));
    loadButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    loadButton_->setPopupMode(QToolButton::MenuButtonPopup);

    QMenu* loadMenu = new QMenu(loadButton_);
    loadButton_->setMenu(loadMenu);
    QAction* loadVolumeAction = new QAction(tr("Load Volume..."), loadButton_);
    loadVolumeAction->setIcon(QPixmap(":/qt/icons/open-volume.png"));
    loadMenu->addAction(loadVolumeAction);
#ifdef VRN_GDCM_VERSION_22
    QAction* dicomServerAction = new QAction(tr("Load from DICOM Server..."), loadButton_);
    //dicomServerAction->setIcon(QPixmap(":/qt/icons/open-volume.png"));
    loadMenu->addAction(dicomServerAction);
#endif

    clearButton_ = new QPushButton(tr("Clear Volume"));

    dimensionLabelCaption_ = new CustomLabel(this);
    spacingLabelCaption_ = new CustomLabel(this);
    memSizeLabelCaption_ = new CustomLabel(this);

    dimensionLabelCaption_->setText(" Dimensions");
    spacingLabelCaption_->setText(" Spacing");
    memSizeLabelCaption_->setText(" MemSize");

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
    volumeLayout->addWidget(loadButton_, 1);
    volumeLayout->addWidget(clearButton_, 1);

    loadButton_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    loadButton_->setMinimumWidth(105);
    clearButton_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    //clearButton_->setMinimumWidth(105);
    mainLayout->addLayout(volumeLayout);

    previewLayout->addWidget(previewLabel_);
    previewLayout->addLayout(infoLayout);
    infoLayout->addWidget(pathLabel_, 0, 0, 1, 2, 0);
    infoLayout->addWidget(dimensionLabelCaption_, 1, 0);
    infoLayout->addWidget(spacingLabelCaption_, 2, 0);
    infoLayout->addWidget(memSizeLabelCaption_, 3, 0);

    infoLayout->addWidget(dimensionLabel_, 1, 1);
    infoLayout->addWidget(spacingLabel_, 2, 1);
    infoLayout->addWidget(memSizeLabel_, 3, 1);

    previewLayout->addStretch();

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

    connect(loadButton_, SIGNAL(clicked()),
        &volumeIOHelper_, SLOT(showFileOpenDialog()));
    connect(loadVolumeAction, SIGNAL(triggered()),
        &volumeIOHelper_, SLOT(showFileOpenDialog()));
    connect(&volumeIOHelper_, SIGNAL(volumeLoaded(const VolumeBase*)),
        this, SLOT(volumeLoaded(const VolumeBase*)));
    connect(clearButton_, SIGNAL(clicked()),
        this, SLOT(clearVolume()));

#ifdef VRN_GDCM_VERSION_22
    tgtAssert(dicomConnectionDialog_, "DicomConnectionDialog is null");
    connect(dicomServerAction, SIGNAL(triggered()),
        this, SLOT(showDicomConnectionDialog()));
    connect(dicomConnectionDialog_, SIGNAL(loadFromURL(const std::string&, VolumeReader*)),
        &volumeIOHelper_, SLOT(loadURL(const std::string&, VolumeReader*)));
#elif defined VRN_MODULE_GDCM
    tgtAssert(dicomConnectionDialog_, "DicomConnectionDialog is null");
    connect(dicomConnectionDialog_, SIGNAL(loadFromURL(const std::string&, VolumeReader*)),
        &volumeIOHelper_, SLOT(loadURL(const std::string&, VolumeReader*)));
#endif

    updateFromProperty();
}

VolumeBase* VolumeURLPropertyWidget::getVolume() const {

    VolumeURLProperty* handleProp = dynamic_cast<VolumeURLProperty*>(prop_);
    if (!handleProp) {
        LWARNING("No volume handle property");
        return 0;
    }

    return handleProp->getVolume();
}

void VolumeURLPropertyWidget::updateFromProperty() {
    VolumeBase* handle = getVolume();
    if (handle) {
        clearButton_->setEnabled(true);

        dimensionLabel_->show();
        pathLabel_->show();
        spacingLabel_->show();
        memSizeLabel_->show();
        previewLabel_->show();
        dimensionLabelCaption_->show();
        spacingLabelCaption_->show();
        memSizeLabelCaption_->show();

        std::string name = VolumeViewHelper::getStrippedVolumeName(handle);
        std::string path = VolumeViewHelper::getVolumePath(handle);
        if(name.size() > 30) {
            volumeNameLabel_->setToolTip(QString::fromStdString(name));
            int end = static_cast<int>(name.size());
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
            int end = static_cast<int>(path.size());
            std::string startString;
            std::string endString;
            for(size_t i = 0; i < 14; i++){
                 startString += path.at(i);
                 endString += path.at(end-14+i);
            }
            path = startString+"..."+endString;
        }

        volumeNameLabel_->setText(QString::fromStdString(" " + name + " ("+ VolumeViewHelper::getVolumeType(static_cast<VolumeBase*>(handle)->getRepresentation<VolumeRAM>())+") "));
        pathLabel_->setText(QString::fromStdString(" "+path));
        dimensionLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeDimension(handle)));
        spacingLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeSpacing(handle)));
        memSizeLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeMemorySize(static_cast<VolumeBase*>(handle)->getRepresentation<VolumeRAM>())));
        previewLabel_->setPixmap(VolumeViewHelper::generateBorderedPreview(handle, 70, 0));

    }
    else {
        clearButton_->setEnabled(false);
        volumeNameLabel_->setText(tr(" no volume"));
        volumeNameLabel_->adjustSize();

        pathLabel_->hide();
        previewLabel_->setPixmap(QPixmap());
        dimensionLabel_->hide();
        spacingLabel_->hide();
        memSizeLabel_->hide();
        previewLabel_->hide();
        dimensionLabelCaption_->hide();
        spacingLabelCaption_->hide();
        memSizeLabelCaption_->hide();
    }
}

void VolumeURLPropertyWidget::showNameLabel(bool) {
    if (nameLabel_)
        nameLabel_->hide();
}

CustomLabel* VolumeURLPropertyWidget::getNameLabel() const {
    return 0;
}

// private slot
void VolumeURLPropertyWidget::volumeLoaded(const VolumeBase* handle) {
    tgtAssert(handle, "null pointer passed");

    VolumeURLProperty* handleProp = dynamic_cast<VolumeURLProperty*>(prop_);
    if (!handleProp) {
        tgtAssert(false, "No volume handle property");
        LERROR("No volume handle property");
        return;
    }

    handleProp->setVolume(const_cast<VolumeBase*>(handle), true);
}

void VolumeURLPropertyWidget::clearVolume() {
    VolumeURLProperty* handleProp = dynamic_cast<VolumeURLProperty*>(prop_);
    if (!handleProp) {
        tgtAssert(false, "No volume handle property");
        LERROR("No volume handle property");
        return;
    }

    handleProp->setVolume(0);
}

void VolumeURLPropertyWidget::showDicomConnectionDialog() {
#ifdef VRN_MODULE_GDCM
    if (!dicomConnectionDialog_)
        dicomConnectionDialog_ = new DicomConnectionDialog(this);
    dicomConnectionDialog_->show();
#else
    QMessageBox::information(this, "DICOM Server Connection",
        "Loading DICOM data from a server requires the GDCM module.");
#endif
}

} //namespace voreen
