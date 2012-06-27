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

#include "voreen/qt/widgets/transfunc/transfunceditorintensity.h"

#include "voreen/qt/widgets/transfunc/colorpicker.h"
#include "voreen/qt/widgets/transfunc/colorluminancepicker.h"
#include "voreen/qt/widgets/transfunc/doubleslider.h"
#include "voreen/qt/widgets/transfunc/transfunctexturepainter.h"
#include "voreen/qt/widgets/transfunc/transfuncmappingcanvas.h"

#include "voreen/core/vis/transfunc/transfuncintensity.h"
#include "voreen/core/volume/volume.h"

#include "tgt/logmanager.h"
#include "tgt/qt/qtcanvas.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QLayout>
#include <QMessageBox>
#include <QSpinBox>
#include <QSplitter>
#include <QToolButton>

namespace voreen {

const std::string TransFuncEditorIntensity::loggerCat_("voreen.qt.transfunceditorintensity");

TransFuncEditorIntensity::TransFuncEditorIntensity(TransFuncProp* prop, QWidget* parent,
                                                   Qt::Orientation orientation)
    : TransFuncEditor(prop, parent)
    , transCanvas_(0)
    , transferFuncIntensity_(0)
    , textureCanvas_(0)
    , texturePainter_(0)
    , doubleSlider_(0)
    , maximumIntensity_(255)
    , orientation_(orientation)
{
    title_ = QString("Intensity");
    transferFuncIntensity_ = dynamic_cast<TransFuncIntensity*>(property_->get());
}

TransFuncEditorIntensity::~TransFuncEditorIntensity() {
}

QLayout* TransFuncEditorIntensity::createMappingLayout() {
    transCanvas_ = new TransFuncMappingCanvas(0, transferFuncIntensity_);
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
    lowerThresholdSpin_ = new QSpinBox();
    lowerThresholdSpin_->setRange(0, maximumIntensity_ - 1);
    lowerThresholdSpin_->setValue(0);
    lowerThresholdSpin_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    upperThresholdSpin_ = new QSpinBox();
    upperThresholdSpin_->setRange(1, maximumIntensity_);
    upperThresholdSpin_->setValue(maximumIntensity_);
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

    // CheckBox for threshold clipping
    QHBoxLayout* hboxClip = new QHBoxLayout();
    checkClipThresholds_ = new QCheckBox(tr("Zoom on threshold area"));
    checkClipThresholds_->setToolTip(tr("Zoom-in on the area between lower and upper thresholds"));
    checkClipThresholds_->setChecked(false);
    hboxClip->addWidget(checkClipThresholds_);
    hboxClip->addStretch();

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
    vBox->addLayout(hboxClip);

    return vBox;
}

QLayout* TransFuncEditorIntensity::createButtonLayout() {
    QBoxLayout* buttonLayout;
    if (orientation_ == Qt::Vertical)
        buttonLayout = new QHBoxLayout();
    else
        buttonLayout = new QVBoxLayout();

    clearButton_ = new QToolButton();
    clearButton_->setIcon(QIcon(":/icons/eraser.png"));
    clearButton_->setToolTip(tr("Reset to default transfer function"));

    loadButton_ = new QToolButton();
    loadButton_->setIcon(QIcon(":/icons/open.png"));
    loadButton_->setToolTip(tr("Load transfer function"));

    saveButton_ = new QToolButton();
    saveButton_->setIcon(QIcon(":/icons/save.png"));
    saveButton_->setToolTip(tr("Save transfer function"));

    if (property_->getManualRepaint()) {
        repaintButton_ = new QToolButton();
        repaintButton_->setIcon(QIcon(":/icons/view-refresh.png"));
        repaintButton_->setToolTip(tr("Repaint the volume rendering"));
    }

    buttonLayout->setSpacing(0);
    buttonLayout->addWidget(clearButton_);
    buttonLayout->addWidget(loadButton_);
    buttonLayout->addWidget(saveButton_);
    if (property_->getManualRepaint())
        buttonLayout->addWidget(repaintButton_);

    buttonLayout->addStretch();

    return buttonLayout;
}

QLayout* TransFuncEditorIntensity::createColorLayout() {
    // ColorPicker
    colorPicker_ = new ColorPicker();
    colorPicker_->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    colorPicker_->setMaximumHeight(160);

    // ColorLuminacePicker
    colorLumPicker_ = new ColorLuminancePicker();
    colorLumPicker_->setFixedWidth(20);
    colorLumPicker_->setMaximumHeight(160);


    QHBoxLayout* hBoxColor = new QHBoxLayout();
    hBoxColor->addWidget(colorPicker_);
    hBoxColor->addWidget(colorLumPicker_);


    if (orientation_ == Qt::Vertical)
        return hBoxColor;
    else {
        QVBoxLayout* vbox = new QVBoxLayout();
        vbox->addLayout(hBoxColor, 1);
        vbox->addStretch();
        return vbox;
    }
}

void TransFuncEditorIntensity::createWidgets() {
    QWidget* mapping = new QWidget();
    QWidget* color = new QWidget();

    QLayout* mappingLayout = createMappingLayout();
    QLayout* colorLayout = createColorLayout();
    QLayout* buttonLayout = createButtonLayout();

    QSplitter* splitter = new QSplitter(orientation_);
    QLayout* buttonColor;
    if (orientation_ == Qt::Vertical) {
        buttonColor = new QVBoxLayout();
        buttonColor->addItem(buttonLayout);
        buttonColor->addItem(mappingLayout);
        mapping->setLayout(buttonColor);
        color->setLayout(colorLayout);
    }
    else {
        buttonColor = new QHBoxLayout();
        buttonColor->addItem(buttonLayout);
        buttonColor->addItem(colorLayout);
        mapping->setLayout(mappingLayout);
        color->setLayout(buttonColor);
    }
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(mapping);
    splitter->addWidget(color);

    splitter->setStretchFactor(0, QSizePolicy::Expanding); // mapping should be stretched
    splitter->setStretchFactor(1, QSizePolicy::Preferred); // color should not be stretched

    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addWidget(splitter);

    setLayout(mainLayout);
}

void TransFuncEditorIntensity::createConnections() {
    // Buttons
    connect(clearButton_, SIGNAL(clicked()), this, SLOT(clearButtonClicked()));
    connect(loadButton_, SIGNAL(clicked()), this, SLOT(loadTransferFunction()));
    connect(saveButton_, SIGNAL(clicked()), this, SLOT(saveTransferFunction()));
    if (property_->getManualRepaint())
        connect(repaintButton_, SIGNAL(clicked()), this, SLOT(causeVolumeRenderingRepaint()));

    // signals from transferMappingCanvas
    connect(transCanvas_, SIGNAL(changed()), this, SLOT(updateTransferFunction()));
    connect(transCanvas_, SIGNAL(loadTransferFunction()), this, SLOT(loadTransferFunction()));
    connect(transCanvas_, SIGNAL(saveTransferFunction()), this, SLOT(saveTransferFunction()));
    connect(transCanvas_, SIGNAL(resetTransferFunction()), this, SLOT(clearButtonClicked()));
    connect(transCanvas_, SIGNAL(switchInteractionMode(bool)), this, SLOT(switchInteractionMode(bool)));

    // signals for colorPicker
    connect(transCanvas_, SIGNAL(colorChanged(const QColor&)),
            colorPicker_, SLOT(setCol(const QColor)));
    connect(transCanvas_, SIGNAL(colorChanged(const QColor&)),
            colorLumPicker_, SLOT(setCol(const QColor)));
    connect(colorPicker_, SIGNAL(newCol(int,int)),
            colorLumPicker_, SLOT(setCol(int,int)));
    connect(colorLumPicker_, SIGNAL(newHsv(int,int,int)),
            this, SLOT(markerColorChanged(int,int,int)));
    connect(colorPicker_, SIGNAL(switchInteractionMode(bool)), this, SLOT(switchInteractionMode(bool)));
    connect(colorLumPicker_, SIGNAL(switchInteractionMode(bool)), this, SLOT(switchInteractionMode(bool)));

    // doubleslider
    connect(doubleSlider_, SIGNAL(valuesChanged(float, float)), this, SLOT(thresholdChanged(float, float)));
    connect(doubleSlider_, SIGNAL(switchInteractionMode(bool)), this, SLOT(switchInteractionMode(bool)));

    // threshold spinboxes
    connect(lowerThresholdSpin_, SIGNAL(valueChanged(int)), this, SLOT(lowerThresholdSpinChanged(int)));
    connect(upperThresholdSpin_, SIGNAL(valueChanged(int)), this, SLOT(upperThresholdSpinChanged(int)));

    connect(checkClipThresholds_, SIGNAL(toggled(bool)), transCanvas_, SLOT(toggleClipThresholds(bool)));
}

void TransFuncEditorIntensity::causeVolumeRenderingRepaint() {
    // this informs the owner about change in transfer function texture
    property_->notifyChange();

    // this signal causes a repaint of the volume rendering
    emit transferFunctionChanged();

    repaintAll();
}

void TransFuncEditorIntensity::clearButtonClicked() {
    resetThresholds();
    resetTransferFunction();

    causeVolumeRenderingRepaint();
}

void TransFuncEditorIntensity::resetTransferFunction() {
    transferFuncIntensity_->createStdFunc();
}

void TransFuncEditorIntensity::resetThresholds() {
    lowerThresholdSpin_->blockSignals(true);
    lowerThresholdSpin_->setValue(0);
    lowerThresholdSpin_->blockSignals(false);

    upperThresholdSpin_->blockSignals(true);
    upperThresholdSpin_->setValue(maximumIntensity_);
    upperThresholdSpin_->blockSignals(false);

    doubleSlider_->blockSignals(true);
    doubleSlider_->setValues(0.f, 1.f);
    doubleSlider_->blockSignals(false);

    transCanvas_->setThreshold(0.f, 1.f);
    transferFuncIntensity_->setThresholds(0.f, 1.f);
}

void TransFuncEditorIntensity::loadTransferFunction() {
    //create filter with supported file formats
    QString filter = "transfer function (";
    for (size_t i = 0; i < transferFuncIntensity_->getLoadFileFormats().size(); ++i) {
        std::string temp = "*." + transferFuncIntensity_->getLoadFileFormats()[i] + " ";
        filter.append(temp.c_str());
    }
    filter.replace(filter.length()-1, 1, ")");

    QString fileName = getOpenFileName(filter);
    if (!fileName.isEmpty()) {
        if (transferFuncIntensity_->load(fileName.toStdString())) {
            restoreThresholds();
            updateTransferFunction();
        }
        else {
            QMessageBox::critical(this, tr("Error"),
                "The selected transfer function could not be loaded.");
            LERROR("The selected transfer function could not be loaded. Maybe the file is corrupt.");
        }
    }
}

void TransFuncEditorIntensity::saveTransferFunction() {
    QStringList filter;
    for (size_t i = 0; i < transferFuncIntensity_->getSaveFileFormats().size(); ++i) {
        std::string temp = "transfer function (*." + transferFuncIntensity_->getSaveFileFormats()[i] + ")";
        filter << temp.c_str();
    }

    QString fileName = getSaveFileName(filter);
    if (!fileName.isEmpty()) {
        //save transfer function to disk
        if (!transferFuncIntensity_->save(fileName.toStdString())) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("The transfer function could not be saved."));
            LERROR("The transfer function could not be saved. Maybe the disk is full?");
        }
    }
}

void TransFuncEditorIntensity::updateTransferFunction() {
    transferFuncIntensity_->textureUpdateNeeded();

    if (!property_->getManualRepaint())
        causeVolumeRenderingRepaint();
}

void TransFuncEditorIntensity::markerColorChanged(int h, int s, int v) {
    transCanvas_->changeCurrentColor(QColor::fromHsv(h, s, v));
}

void TransFuncEditorIntensity::thresholdChanged(float min, float max) {
    //convert to integer values
    int val_min = tgt::iround(min * maximumIntensity_);
    int val_max = tgt::iround(max * maximumIntensity_);

    //sync with spinboxes
    if ((val_max != upperThresholdSpin_->value()))
        upperThresholdSpin_->setValue(val_max);

    if ((val_min != lowerThresholdSpin_->value()))
        lowerThresholdSpin_->setValue(val_min);

    //apply threshold to transfer function
    applyThreshold();
}

void TransFuncEditorIntensity::lowerThresholdSpinChanged(int value) {
    if (value+1 < maximumIntensity_) {
        //increment maximum of lower spin when maximum was reached and we are below upper range
        if (value == lowerThresholdSpin_->maximum())
            lowerThresholdSpin_->setMaximum(value+1);

        //update minimum of upper spin
        upperThresholdSpin_->blockSignals(true);
        upperThresholdSpin_->setMinimum(value);
        upperThresholdSpin_->blockSignals(false);
    }
    //increment value of upper spin when it equals value of lower spin
    if (value == upperThresholdSpin_->value()) {
        upperThresholdSpin_->blockSignals(true);
        upperThresholdSpin_->setValue(value+1);
        upperThresholdSpin_->blockSignals(false);
    }

    //update doubleSlider to new minValue
    doubleSlider_->blockSignals(true);
    doubleSlider_->setMinValue(value / static_cast<float>(maximumIntensity_));
    doubleSlider_->blockSignals(false);

    //apply threshold to transfer function
    applyThreshold();
}

void TransFuncEditorIntensity::upperThresholdSpinChanged(int value) {
    if (value-1 > 0) {
        //increment minimum of upper spin when minimum was reached and we are above lower range
        if (value == upperThresholdSpin_->minimum())
            upperThresholdSpin_->setMinimum(value-1);

        //update maximum of lower spin
        lowerThresholdSpin_->blockSignals(true);
        lowerThresholdSpin_->setMaximum(value);
        lowerThresholdSpin_->blockSignals(false);
    }
    //increment value of lower spin when it equals value of upper spin
    if (value == lowerThresholdSpin_->value()) {
        lowerThresholdSpin_->blockSignals(true);
        lowerThresholdSpin_->setValue(value-1);
        lowerThresholdSpin_->blockSignals(false);
    }

    //update doubleSlider to new maxValue
    doubleSlider_->blockSignals(true);
    doubleSlider_->setMaxValue(value / static_cast<float>(maximumIntensity_));
    doubleSlider_->blockSignals(false);

    //apply threshold to transfer function
    applyThreshold();
}

void TransFuncEditorIntensity::applyThreshold() {
    float min = doubleSlider_->getMinValue();
    float max = doubleSlider_->getMaxValue();
    transCanvas_->setThreshold(min, max);
    transferFuncIntensity_->setThresholds(min, max);

    updateTransferFunction();
}

void TransFuncEditorIntensity::update() {
    // check whether the volume associated with the TransFuncProperty has changed
    Volume* newVol = property_->getVolume();
    if (newVol && (newVol != volume_)) {
        volume_ = newVol;
        volumeChanged();
    }

    if (transferFuncIntensity_) {
        // update treshold widgets from tf
        restoreThresholds();

        // repaint control elements
        repaintAll();
    }
    else
        resetEditor();
}

void TransFuncEditorIntensity::restoreThresholds() {
    tgt::vec2 thresh = transferFuncIntensity_->getThresholds();
    // set value for doubleSlider
    doubleSlider_->blockSignals(true);
    doubleSlider_->setValues(thresh.x, thresh.y);
    doubleSlider_->blockSignals(false);

    // set value for spinboxes
    int val_min = tgt::iround(thresh.x * maximumIntensity_);
    int val_max = tgt::iround(thresh.y * maximumIntensity_);
    lowerThresholdSpin_->blockSignals(true);
    upperThresholdSpin_->blockSignals(true);
    lowerThresholdSpin_->setValue(val_min);
    upperThresholdSpin_->setValue(val_max);
    lowerThresholdSpin_->blockSignals(false);
    upperThresholdSpin_->blockSignals(false);
    // test whether to update minimum and/or maximum of spinboxes
    if (val_min+1 < maximumIntensity_) {
        //increment maximum of lower spin when maximum was reached and we are below upper range
        if (val_min == lowerThresholdSpin_->maximum())
            lowerThresholdSpin_->setMaximum(val_min+1);

        //update minimum of upper spin
        upperThresholdSpin_->blockSignals(true);
        upperThresholdSpin_->setMinimum(val_min);
        upperThresholdSpin_->blockSignals(false);
    }

    if (val_max-1 > 0) {
        //increment minimum of upper spin when minimum was reached and we are above lower range
        if (val_max == upperThresholdSpin_->minimum())
            upperThresholdSpin_->setMinimum(val_max-1);

        //update maximum of lower spin
        lowerThresholdSpin_->blockSignals(true);
        lowerThresholdSpin_->setMaximum(val_max);
        lowerThresholdSpin_->blockSignals(false);
    }

    // propagate threshold to mapping canvas
    transCanvas_->setThreshold(thresh.x, thresh.y);
}

void TransFuncEditorIntensity::volumeChanged() {
    int bits = volume_->getBitsStored() / volume_->getNumChannels();
    int maxNew = static_cast<int>(pow(2.f, static_cast<float>(bits)))-1;
    if (maxNew != maximumIntensity_) {
        float lowerRelative = lowerThresholdSpin_->value() / static_cast<float>(maximumIntensity_);
        float upperRelative = upperThresholdSpin_->value() / static_cast<float>(maximumIntensity_);
        maximumIntensity_ = maxNew;
        lowerThresholdSpin_->blockSignals(true);
        lowerThresholdSpin_->setRange(0, maximumIntensity_-1);
        lowerThresholdSpin_->setValue(tgt::iround(lowerRelative*maximumIntensity_));
        lowerThresholdSpin_->updateGeometry();
        lowerThresholdSpin_->blockSignals(false);

        upperThresholdSpin_->blockSignals(true);
        upperThresholdSpin_->setRange(1, maximumIntensity_);
        upperThresholdSpin_->setValue(tgt::iround(upperRelative*maximumIntensity_));
        upperThresholdSpin_->updateGeometry();
        upperThresholdSpin_->blockSignals(false);
    }

    //propagate new volume to transfuncMappingCanvas
    transCanvas_->volumeChanged(volume_, maximumIntensity_+1);
}

void TransFuncEditorIntensity::resetEditor() {
    if (property_->get() != transferFuncIntensity_) {
        LDEBUG("The pointers of property and transfer function do not match."
                << "Creating new transfer function object.....");
        transferFuncIntensity_ = new TransFuncIntensity(maximumIntensity_ + 1);
        property_->set(transferFuncIntensity_);

        // propagate transfer function to mapping canvas and texture painter
        texturePainter_->setTransFunc(transferFuncIntensity_);
        transCanvas_->setTransFunc(transferFuncIntensity_);
    }

    checkClipThresholds_->setChecked(false);
    // reset transfer function and thresholds
    resetThresholds();
    resetTransferFunction();

    causeVolumeRenderingRepaint();
}

void TransFuncEditorIntensity::repaintAll() {
    transCanvas_->repaint();
    doubleSlider_->repaint();
    textureCanvas_->update();
}

} // namespace voreen
