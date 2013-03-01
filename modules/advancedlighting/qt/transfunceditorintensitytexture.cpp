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

#include "voreen/qt/widgets/transfunc/doubleslider.h"
#include "voreen/qt/widgets/transfunc/transfunctexturepainter.h"

#include "voreen/core/datastructures/transfunc/transfuncintensity.h"
#include "modules/sphericalharmonics/include/shraycaster.h"
#include "modules/sphericalharmonics/include/transfunceditorintensitytexture.h"
#include "modules/sphericalharmonics/include/transfuncmappingcanvastexture.h"


#include "tgt/qt/qtcanvas.h"

#include <QHBoxLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QToolButton>
#include <QSplitter>

namespace voreen {

//const std::string TransFuncEditorIntensityTexture::loggerCat_("voreen.qt.transfunceditorintensitytexture");

TransFuncEditorIntensityTexture::TransFuncEditorIntensityTexture(TransFuncProperty* prop, QWidget* parent,
                                                   Qt::Orientation orientation)
    : TransFuncEditorIntensity(prop, parent, orientation){}

QLayout* TransFuncEditorIntensityTexture::createMappingLayout() {
    transCanvas_ = new TransFuncMappingCanvasTexture(0, transferFuncIntensity_, property_);
    transCanvas_->setMinimumWidth(200);

    QWidget* additionalSpace = new QWidget();
    additionalSpace->setMinimumHeight(13);

    // threshold slider
    QHBoxLayout* hboxSlider = new QHBoxLayout();
    doubleSlider_ = new DoubleSlider();
    doubleSlider_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    doubleSlider_->setOffsets(12, 27);
    hboxSlider->addWidget(doubleSlider_);

    //spinboxes for threshold values
    lowerThresholdSpin_ = new QDoubleSpinBox();
    lowerThresholdSpin_->setRange(0.0, 1.0);
    lowerThresholdSpin_->setValue(0.0);
    lowerThresholdSpin_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    upperThresholdSpin_ = new QDoubleSpinBox();
    upperThresholdSpin_->setRange(0.0, 1.0f);
    upperThresholdSpin_->setValue(1.0);
    upperThresholdSpin_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QHBoxLayout* hboxSpin = new QHBoxLayout();
    //the spacing is added so that spinboxes and doubleslider are aligned vertically
    hboxSpin->addSpacing(6);
    hboxSpin->addWidget(lowerThresholdSpin_);
    hboxSpin->addStretch();
    hboxSpin->addWidget(upperThresholdSpin_);
    hboxSpin->addSpacing(21);

    //add gradient that displays the transferfunction as image
    textureCanvas_ = new tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, 0, true);
    texturePainter_ = new TransFuncTexturePainter(textureCanvas_);
    texturePainter_->initialize();
    texturePainter_->setTransFunc(transferFuncIntensity_);
    textureCanvas_->setPainter(texturePainter_, false);
    textureCanvas_->setFixedHeight(15);
    textureCanvas_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    // put widgets in layout
    QVBoxLayout* vBox = new QVBoxLayout();
    vBox->setMargin(0);
    vBox->setSpacing(1);
    vBox->addStretch();
    vBox->addWidget(transCanvas_, 1);
    vBox->addWidget(additionalSpace);
    vBox->addLayout(hboxSlider);
    vBox->addLayout(hboxSpin);
    vBox->addSpacing(1);
    vBox->addWidget(textureCanvas_);

    return vBox;
}

void TransFuncEditorIntensityTexture::createConnections() {
    TransFuncEditorIntensity::createConnections();
    repaintButton_ = new QToolButton();
    repaintButton_->setIcon(QIcon(":/qt/icons/view-refresh.png"));
    repaintButton_->setToolTip(tr("Repaint the volume rendering"));
    connect(repaintButton_, SIGNAL(clicked()), this, SLOT(causeVolumeRenderingRepaint()));
}

QLayout* TransFuncEditorIntensityTexture::createButtonLayout() {
    QBoxLayout* buttonLayout;
    if (orientation_ == Qt::Vertical)
        buttonLayout = new QHBoxLayout();
    else
        buttonLayout = new QVBoxLayout();

    clearButton_ = new QToolButton();
    clearButton_->setIcon(QIcon(":/qt/icons/eraser.png"));
    clearButton_->setToolTip(tr("Reset to default transfer function"));

    loadButton_ = new QToolButton();
    loadButton_->setIcon(QIcon(":/qt/icons/open.png"));
    loadButton_->setToolTip(tr("Load transfer function"));

    saveButton_ = new QToolButton();
    saveButton_->setIcon(QIcon(":/qt/icons/save.png"));
    saveButton_->setToolTip(tr("Save transfer function"));

    repaintButton_ = new QToolButton();
    repaintButton_->setIcon(QIcon(":/qt/icons/view-refresh.png"));
    repaintButton_->setToolTip(tr("Repaint the volume rendering"));

    buttonLayout->setSpacing(0);
    buttonLayout->addWidget(clearButton_);
    buttonLayout->addWidget(loadButton_);
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addWidget(repaintButton_);

    buttonLayout->addStretch();

    return buttonLayout;
}

void TransFuncEditorIntensityTexture::causeVolumeRenderingRepaint() {
    //SHRaycaster* shrc = dynamic_cast<SHRaycaster*>(property_->getOwner());
    //if(shrc)
        //shrc->getSHClass()->setSHInteraction(false);
}

} // namespace voreen
