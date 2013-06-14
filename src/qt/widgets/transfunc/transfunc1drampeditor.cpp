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

#include "voreen/qt/widgets/transfunc/transfunc1drampeditor.h"

#include "voreen/qt/widgets/transfunc/colorpicker.h"
#include "voreen/qt/widgets/transfunc/colorluminancepicker.h"
#include "voreen/qt/widgets/transfunc/doubleslider.h"
#include "voreen/qt/widgets/transfunc/transfuncmappingcanvasramp.h"
#include "voreen/qt/widgets/transfunc/transfunc1dkeyspainter.h"

#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/volume/volumeram.h"

#include "tgt/logmanager.h"
#include "tgt/qt/qtcanvas.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QToolButton>

namespace voreen {

const std::string TransFunc1DRampEditor::loggerCat_("voreen.qt.TransFunc1DRampEditor");

TransFunc1DRampEditor::TransFunc1DRampEditor(TransFuncProperty* prop, QWidget* parent,
                                                           Qt::Orientation orientation)
    : TransFuncEditor(prop, parent)
    , transCanvas_(0)
    , textureCanvas_(0)
    , texturePainter_(0)
    , doubleSlider_(0)
    , maximumIntensity_(255)
    , orientation_(orientation)
{
    title_ = QString("Intensity Ramp");

    transferFuncIntensity_ = dynamic_cast<TransFunc1DKeys*>(property_->get());
}

TransFunc1DRampEditor::~TransFunc1DRampEditor() {
}

QLayout* TransFunc1DRampEditor::createMappingLayout() {
    transCanvas_ = new TransFuncMappingCanvasRamp(0, transferFuncIntensity_);

    QWidget* additionalSpace = new QWidget();
    additionalSpace->setMinimumHeight(2);

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

    //add canvas that displays the transferfunction as image
    textureCanvas_ = new tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, 0, true);
    texturePainter_ = new TransFunc1DKeysPainter(textureCanvas_);
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

    //put widgets in layout
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

QLayout* TransFunc1DRampEditor::createButtonLayout() {
    QBoxLayout* buttonLayout;
    if (orientation_ == Qt::Vertical)
        buttonLayout = new QHBoxLayout();
    else
        buttonLayout = new QVBoxLayout();

    clearButton_ = new QToolButton();
    clearButton_->setIcon(QIcon(":/qt/icons/clear.png"));
    clearButton_->setToolTip(tr("Reset to default transfer function"));

    loadButton_ = new QToolButton();
    loadButton_->setIcon(QIcon(":/qt/icons/open.png"));
    loadButton_->setToolTip(tr("Load transfer function"));

    saveButton_ = new QToolButton();
    saveButton_->setIcon(QIcon(":/qt/icons/save.png"));
    saveButton_->setToolTip(tr("Save transfer function"));

    buttonLayout->setSpacing(0);
    buttonLayout->addWidget(clearButton_);
    buttonLayout->addWidget(loadButton_);
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addStretch();

    return buttonLayout;
}

QLayout* TransFunc1DRampEditor::createColorLayout() {
    // Ramp settings
    QGroupBox* rampBox = new QGroupBox(tr("Ramp Parameters"));
    QGridLayout* rampLayout = new QGridLayout();
    rampLayout->setColumnStretch(0, 0);
    rampLayout->setColumnStretch(1, 1);
    rampLayout->setColumnStretch(2, 0);
    rampLayout->setRowStretch(0, 0);
    rampLayout->setRowStretch(1, 0);
    rampLayout->setRowStretch(2, 0);

    rampLayout->addWidget(new QLabel(tr("Center")), 0, 0);
    sliderRampCenter_ = new QSlider(Qt::Horizontal);
    sliderRampCenter_->setMaximum(maximumIntensity_);
    spinRampCenter_ = new QSpinBox();
    spinRampCenter_->setMaximum(maximumIntensity_);
    rampLayout->addWidget(sliderRampCenter_, 0, 1);
    rampLayout->addWidget(spinRampCenter_, 0, 2);

    rampLayout->addWidget(new QLabel(tr("Width")), 1, 0);
    sliderRampWidth_ = new QSlider(Qt::Horizontal);
    sliderRampWidth_->setMaximum(maximumIntensity_);
    spinRampWidth_ = new QSpinBox();
    spinRampWidth_->setMaximum(maximumIntensity_);
    rampLayout->addWidget(sliderRampWidth_, 1, 1);
    rampLayout->addWidget(spinRampWidth_, 1, 2);
    rampBox->setLayout(rampLayout);

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

    QVBoxLayout* colorLayout = new QVBoxLayout();
    colorLayout->addWidget(rampBox);
    colorLayout->addLayout(hBoxColor, 1);
    colorLayout->addStretch();

    return colorLayout;
}

void TransFunc1DRampEditor::createWidgets() {
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
    splitter->setChildrenCollapsible(true);
    splitter->addWidget(mapping);
    splitter->addWidget(color);

    splitter->setStretchFactor(0, QSizePolicy::Expanding); // mapping should be stretched
    splitter->setStretchFactor(1, QSizePolicy::Preferred); // color should not be stretched

    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addWidget(splitter);

    setLayout(mainLayout);

    //set ramp values to default
    syncRampSliders(128, 255);
}

void TransFunc1DRampEditor::createConnections() {
    // Buttons
    connect(clearButton_, SIGNAL(clicked()), this, SLOT(clearButtonClicked()));
    connect(loadButton_, SIGNAL(clicked()), this, SLOT(loadTransferFunction()));
    connect(saveButton_, SIGNAL(clicked()), this, SLOT(saveTransferFunction()));

    // signals from transferMappingCanvas
    connect(transCanvas_, SIGNAL(changed()), this, SLOT(updateTransferFunction()));
    connect(transCanvas_, SIGNAL(loadTransferFunction()), this, SLOT(loadTransferFunction()));
    connect(transCanvas_, SIGNAL(saveTransferFunction()), this, SLOT(saveTransferFunction()));
    connect(transCanvas_, SIGNAL(resetTransferFunction()), this, SLOT(clearButtonClicked()));
    connect(transCanvas_, SIGNAL(toggleInteractionMode(bool)), this, SLOT(toggleInteractionMode(bool)));

    // signals for colorPicker
    connect(transCanvas_, SIGNAL(colorChanged(const QColor&)),
            colorPicker_, SLOT(setCol(const QColor)));
    connect(transCanvas_, SIGNAL(colorChanged(const QColor&)),
            colorLumPicker_, SLOT(setCol(const QColor)));
    connect(colorPicker_, SIGNAL(newCol(int,int)),
            colorLumPicker_, SLOT(setCol(int,int)));
    connect(colorLumPicker_, SIGNAL(newHsv(int,int,int)),
            this, SLOT(markerColorChanged(int,int,int)));
    connect(colorPicker_, SIGNAL(toggleInteractionMode(bool)), this, SLOT(toggleInteractionMode(bool)));
    connect(colorLumPicker_, SIGNAL(toggleInteractionMode(bool)), this, SLOT(toggleInteractionMode(bool)));

    // doubleslider
    connect(doubleSlider_, SIGNAL(valuesChanged(float, float)), this, SLOT(thresholdChanged(float, float)));
    connect(doubleSlider_, SIGNAL(toggleInteractionMode(bool)), this, SLOT(toggleInteractionMode(bool)));

    // threshold spinboxes
    connect(lowerThresholdSpin_, SIGNAL(valueChanged(int)), this, SLOT(lowerThresholdSpinChanged(int)));
    connect(upperThresholdSpin_, SIGNAL(valueChanged(int)), this, SLOT(upperThresholdSpinChanged(int)));

    connect(checkClipThresholds_, SIGNAL(toggled(bool)), transCanvas_, SLOT(toggleClipThresholds(bool)));

    //ramp slider and spinboxes
    connect(sliderRampCenter_, SIGNAL(valueChanged(int)), this, SLOT(updateRampCenter(int)));
    connect(spinRampCenter_,   SIGNAL(valueChanged(int)), this, SLOT(updateRampCenter(int)));
    connect(spinRampWidth_,    SIGNAL(valueChanged(int)), this, SLOT(updateRampWidth(int)));
    connect(sliderRampWidth_,  SIGNAL(valueChanged(int)), this, SLOT(updateRampWidth(int)));

    connect(sliderRampCenter_, SIGNAL(sliderPressed()),  this, SLOT(startTracking()));
    connect(sliderRampWidth_,  SIGNAL(sliderPressed()),  this, SLOT(startTracking()));
    connect(sliderRampCenter_, SIGNAL(sliderReleased()), this, SLOT(stopTracking()));
    connect(sliderRampWidth_,  SIGNAL(sliderReleased()), this, SLOT(stopTracking()));
}

void TransFunc1DRampEditor::causeVolumeRenderingRepaint() {
    // this informs the owner about change in transfer function texture
    property_->notifyChange();

    // this signal causes a repaint of the volume rendering
    emit transferFunctionChanged();

    repaintAll();
}

void TransFunc1DRampEditor::clearButtonClicked() {
    resetThresholds();
    resetTransferFunction();

    causeVolumeRenderingRepaint();
}

void TransFunc1DRampEditor::resetTransferFunction() {

    if (!transferFuncIntensity_) {
        LWARNING("No valid transfer function assigned");
        return;
    }

    transferFuncIntensity_->setToStandardFunc();

    float rampCenter, rampWidth;
    transCanvas_->calcRampParameterFromKeys();
    transCanvas_->getRampParameter(rampCenter, rampWidth);
    int center = tgt::iround(rampCenter * maximumIntensity_);
    int width = tgt::iround(rampWidth * maximumIntensity_);
    syncRampSliders(center, width);
}

void TransFunc1DRampEditor::resetThresholds() {

    if (!transferFuncIntensity_) {
        LWARNING("No valid transfer function assigned");
        return;
    }

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

void TransFunc1DRampEditor::loadTransferFunction() {

    if (!transferFuncIntensity_) {
        LWARNING("No valid transfer function assigned");
        return;
    }

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
            if (transferFuncIntensity_->getNumKeys() > 2) {
                clearButtonClicked();
                std::string error = "The loaded transfer function contains more than 2 keys.\n";
                error += "The editor does not support this kind of transfer functions.\n";
                error += "The transfer function is reset to default.";
                QMessageBox::critical(this, tr("Error"), tr(error.c_str()));
                LERROR(error);
            }
            else {
                restoreThresholds();
                updateTransferFunction();
            }
        }
        else {
            QMessageBox::critical(this, tr("Error"),
                                  tr("The selected transfer function could not be loaded."));
            LERROR("The selected transfer function could not be loaded. Maybe the file is corrupt.");
        }
    }
}

void TransFunc1DRampEditor::saveTransferFunction() {

    if (!transferFuncIntensity_) {
        LWARNING("No valid transfer function assigned");
        return;
    }

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

void TransFunc1DRampEditor::updateTransferFunction() {

    if (!transferFuncIntensity_) {
        return;
    }

    transferFuncIntensity_->invalidateTexture();

    // syncronize ramp slider and spinboxes
    float rampCenter, rampWidth;
    transCanvas_->getRampParameter(rampCenter, rampWidth);
    int center = tgt::iround(rampCenter * maximumIntensity_);
    int width = tgt::iround(rampWidth * maximumIntensity_);
    syncRampSliders(center, width);

    property_->notifyChange();
}

void TransFunc1DRampEditor::markerColorChanged(int h, int s, int v) {
    transCanvas_->changeCurrentColor(QColor::fromHsv(h, s, v));
}

void TransFunc1DRampEditor::thresholdChanged(float min, float max) {
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

void TransFunc1DRampEditor::lowerThresholdSpinChanged(int value) {
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

void TransFunc1DRampEditor::upperThresholdSpinChanged(int value) {
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

void TransFunc1DRampEditor::applyThreshold() {

    if (!transferFuncIntensity_) {
        return;
    }

    float min = doubleSlider_->getMinValue();
    float max = doubleSlider_->getMaxValue();
    transCanvas_->setThreshold(min, max);
    transferFuncIntensity_->setThresholds(min, max);

    updateTransferFunction();
}

void TransFunc1DRampEditor::updateFromProperty() {

    // check whether new transfer function object has been assigned
    if (property_->get() != transferFuncIntensity_) {
        transferFuncIntensity_ = dynamic_cast<TransFunc1DKeys*>(property_->get());

        // ramp only applicable for two keys
        if (transferFuncIntensity_ && transferFuncIntensity_->getNumKeys() > 2)
            transferFuncIntensity_ = 0;

        // propagate transfer function to mapping canvas and texture painter
        texturePainter_->setTransFunc(transferFuncIntensity_);
        transCanvas_->setTransFunc(transferFuncIntensity_);

        // disable, if tf type unsupported
        if (property_->get() && !transferFuncIntensity_) {
            if (isEnabled()) {
                LWARNING("Current transfer function not supported by this editor. Disabling.");
                setEnabled(false);
            }
        }
    }

    // check whether the volume associated with the TransFuncProperty has changed
    const VolumeBase* newHandle = property_->getVolumeHandle();
    if (newHandle != volume_) {
        volume_ = newHandle;
        volumeChanged();
    }

    if (transferFuncIntensity_) {
        setEnabled(true);

        // update treshold widgets from tf
        restoreThresholds();

        // repaint control elements
        repaintAll();
    }
    else {
        setEnabled(false);
    }

}

void TransFunc1DRampEditor::volumeChanged() {
    if (volume_ && volume_->getRepresentation<VolumeRAM>()) {
        size_t bits = volume_->getRepresentation<VolumeRAM>()->getBitsAllocated() / volume_->getRepresentation<VolumeRAM>()->getNumChannels();
        int maxNew = static_cast<int>(pow(2.f, static_cast<float>(bits))) - 1;
        if (maxNew != maximumIntensity_) {
            float lowerRelative = lowerThresholdSpin_->value() / static_cast<float>(maximumIntensity_);
            float upperRelative = upperThresholdSpin_->value() / static_cast<float>(maximumIntensity_);
            float widthRelative = spinRampWidth_->value() / static_cast<float>(maximumIntensity_);
            float centerRelative = spinRampCenter_->value() / static_cast<float>(maximumIntensity_);
            maximumIntensity_ = maxNew;

            lowerThresholdSpin_->blockSignals(true);
            lowerThresholdSpin_->setRange(0, maximumIntensity_ - 1);
            lowerThresholdSpin_->setValue(tgt::iround(lowerRelative*maximumIntensity_));
            lowerThresholdSpin_->updateGeometry();
            lowerThresholdSpin_->blockSignals(false);

            upperThresholdSpin_->blockSignals(true);
            upperThresholdSpin_->setRange(1, maximumIntensity_);
            upperThresholdSpin_->setValue(tgt::iround(upperRelative * maximumIntensity_));
            upperThresholdSpin_->updateGeometry();
            upperThresholdSpin_->blockSignals(false);

            sliderRampCenter_->setMaximum(maximumIntensity_);
            sliderRampWidth_->setMaximum(maximumIntensity_);
            spinRampCenter_->setMaximum(maximumIntensity_);
            spinRampWidth_->setMaximum(maximumIntensity_);
            spinRampCenter_->updateGeometry();
            spinRampWidth_->updateGeometry();

            // syncronize ramp slider and spinboxes
            int center = tgt::iround(centerRelative*maximumIntensity_);
            int width = tgt::iround(widthRelative*maximumIntensity_);
            syncRampSliders(center, width);

            transCanvas_->blockSignals(true);
            transCanvas_->setRampParameter(centerRelative, widthRelative);
            transCanvas_->blockSignals(false);
        }
    }

    // propagate new volume to transfuncMappingCanvas
    transCanvas_->volumeChanged(volume_);
}

void TransFunc1DRampEditor::restoreThresholds() {

    if (!transferFuncIntensity_) {
        return;
    }

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

void TransFunc1DRampEditor::updateRampCenter(int center) {
    int width = spinRampWidth_->value();
    float centerf = static_cast<float>(center) / maximumIntensity_;
    float widthf = static_cast<float>(width) / maximumIntensity_;

    bool unblock = false;
    if (signalsBlocked() && !transCanvas_->signalsBlocked()) {
        transCanvas_->blockSignals(true);
        unblock = true;
    }
    transCanvas_->setRampParameter(centerf, widthf);
    if (unblock)
        transCanvas_->blockSignals(false);

    syncRampSliders(center, width);
}

void TransFunc1DRampEditor::updateRampWidth(int width) {
    int center = spinRampCenter_->value();
    float centerf = static_cast<float>(center) / maximumIntensity_;
    float widthf = static_cast<float>(width) / maximumIntensity_;

    bool unblock = false;
    if (signalsBlocked() && !transCanvas_->signalsBlocked()) {
        transCanvas_->blockSignals(true);
        unblock = true;
    }
    transCanvas_->setRampParameter(centerf, widthf);
    if (unblock)
        transCanvas_->blockSignals(false);

    syncRampSliders(center, width);
}

void TransFunc1DRampEditor::syncRampSliders(int rampCenter, int rampWidth) {
    sliderRampCenter_->blockSignals(true);
    sliderRampWidth_->blockSignals(true);
    spinRampCenter_->blockSignals(true);
    spinRampWidth_->blockSignals(true);
    sliderRampCenter_->setValue(rampCenter);
    spinRampCenter_->setValue(rampCenter);
    sliderRampWidth_->setValue(rampWidth);
    spinRampWidth_->setValue(rampWidth);
    sliderRampCenter_->blockSignals(false);
    sliderRampWidth_->blockSignals(false);
    spinRampCenter_->blockSignals(false);
    spinRampWidth_->blockSignals(false);
}

void TransFunc1DRampEditor::startTracking() {
    toggleInteractionMode(true);
}

void TransFunc1DRampEditor::stopTracking() {
    toggleInteractionMode(false);
}

void TransFunc1DRampEditor::resetEditor() {
    if (property_->get() != transferFuncIntensity_) {
        LDEBUG("The pointers of property and transfer function do not match."
                << "Creating new transfer function object.....");
        transferFuncIntensity_ = new TransFunc1DKeys(maximumIntensity_ + 1);
        property_->set(transferFuncIntensity_);

        // propagate transfer function to mapping canvas and gradient
        texturePainter_->setTransFunc(transferFuncIntensity_);
        transCanvas_->setTransFunc(transferFuncIntensity_);
    }

    checkClipThresholds_->setChecked(false);
    // reset transfer function and thresholds
    resetThresholds();
    resetTransferFunction();

    causeVolumeRenderingRepaint();
}

void TransFunc1DRampEditor::repaintAll() {
    transCanvas_->update();
    doubleSlider_->update();
    textureCanvas_->update();
}

tgt::ivec2 TransFunc1DRampEditor::getRampParameters() const {
    return tgt::ivec2(spinRampCenter_->value(), spinRampWidth_->value());
}

void TransFunc1DRampEditor::setTransFuncProp(TransFuncProperty* prop) {

    TransFuncEditor::setTransFuncProp(prop);

    // update widgets
    transferFuncIntensity_ = dynamic_cast<TransFunc1DKeys*>(prop->get());
    texturePainter_->setTransFunc(transferFuncIntensity_);
    transCanvas_->setTransFunc(transferFuncIntensity_);
    updateFromProperty();

}

} // namespace voreen
