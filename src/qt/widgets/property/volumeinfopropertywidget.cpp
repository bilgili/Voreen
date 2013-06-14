/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#include "voreen/qt/widgets/property/volumeinfopropertywidget.h"

#include "voreen/core/datastructures/volume/volume.h"

#include "voreen/qt/widgets/volumeviewhelper.h"

#include <QLabel>
#include <QVBoxLayout>

namespace voreen {


const std::string VolumeInfoPropertyWidget::loggerCat_("voreen.qt.VolumeInfoPropertyWidget");

VolumeInfoPropertyWidget::VolumeInfoPropertyWidget(VolumeInfoProperty* volumeInfoProp, QWidget* parent)
    : QPropertyWidget(volumeInfoProp, parent, false)
{
    if (!volumeInfoProp) {
        tgtAssert(false, "No volume property");
        LERROR("No volume property");
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

    updateFromProperty();
}

const VolumeBase* VolumeInfoPropertyWidget::getVolumeFromProperty() const {

    VolumeInfoProperty* infoProp = dynamic_cast<VolumeInfoProperty*>(prop_);
    if (!infoProp) {
        LWARNING("No volume property");
        return 0;
    }

    return infoProp->getVolume();
}

void VolumeInfoPropertyWidget::updateFromPropertySlot() {
    const VolumeBase* handle = getVolumeFromProperty();
    if (handle) {
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

        volumeNameLabel_->setText(QString::fromStdString(" " + name + " (" + handle->getFormat() + ") "));

        pathLabel_->setText(QString::fromStdString(" "+path));
        dimensionLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeDimension(handle)));
        spacingLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeSpacing(handle)));
        memSizeLabel_->setText(QString::fromStdString(VolumeViewHelper::getVolumeMemorySize(handle)));
        previewLabel_->setPixmap(VolumeViewHelper::generateBorderedPreview(handle, 70, 0));

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
        dimensionLabelCaption_->hide();
        spacingLabelCaption_->hide();
        memSizeLabelCaption_->hide();
    }
}

void VolumeInfoPropertyWidget::showNameLabel(bool) {
    if (nameLabel_)
        nameLabel_->hide();
}

CustomLabel* VolumeInfoPropertyWidget::getNameLabel() const {
    return 0;
}

} //namespace voreen
