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

#include "voreen/qt/widgets/transfunc/transfunceditorintensitypet.h"

#include "voreen/qt/widgets/transfunc/doubleslider.h"
#include "voreen/qt/widgets/transfunc/histogrampainter.h"
#include "voreen/qt/widgets/transfunc/transfunctexturepainter.h"

#include "voreen/core/vis/transfunc/transfuncintensity.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"
#include "voreen/core/volume/histogram.h"

#include "tgt/logmanager.h"
#include "tgt/qt/qtcanvas.h"

#include <QBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>

namespace voreen {

const std::string TransFuncEditorIntensityPet::loggerCat_("voreen.qt.transfunceditorintensitypet");

TransFuncEditorIntensityPet::TransFuncEditorIntensityPet(TransFuncProp* prop, QWidget* parent)
    : TransFuncEditor(prop, parent)
    , completeTextureCanvas_(0)
    , completeTexturePainter_(0)
    , expandedTextureCanvas_(0)
    , expandedTexturePainter_(0)
    , transferFuncIntensity_(0)
    , transferFuncGradient_(0)
    , histogramPainter_(0)
    , maximumIntensity_(255)
    , oldThreshold_(0.f, 1.f)
{
    title_ = QString("Intensity Pet");
    currentRange_ = tgt::ivec2(0, maximumIntensity_);

    transferFuncIntensity_ = dynamic_cast<TransFuncIntensity*>(property_->get());
    if (transferFuncIntensity_)
        transferFuncGradient_ = new TransFuncIntensity(*transferFuncIntensity_);
}

TransFuncEditorIntensityPet::~TransFuncEditorIntensityPet() {
    delete transferFuncGradient_;
}

void TransFuncEditorIntensityPet::createWidgets() {
    QVBoxLayout* mainLayout = new QVBoxLayout();

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    resetButton_ = new QToolButton();
    resetButton_->setIcon(QIcon(":/icons/eraser.png"));
    resetButton_->setToolTip(tr("Reset transfer function"));
    buttonLayout->addWidget(resetButton_);

    loadButton_ = new QToolButton();
    loadButton_->setIcon(QIcon(":/icons/open.png"));
    loadButton_->setToolTip(tr("Load transfer function"));
    buttonLayout->addWidget(loadButton_);

    saveButton_ = new QToolButton();
    saveButton_->setIcon(QIcon(":/icons/save.png"));
    saveButton_->setToolTip(tr("Save transfer function"));
    buttonLayout->addWidget(saveButton_);

    if (property_->getManualRepaint()) {
        repaintButton_ = new QToolButton();
        repaintButton_->setIcon(QIcon(":/icons/view-refresh.png"));
        repaintButton_->setToolTip(tr("Repaint the volume rendering"));
        buttonLayout->addWidget(repaintButton_);
    }
    buttonLayout->addStretch();

    histogramPainter_ = new HistogramPainter(this, tgt::vec2(0.f, 1.f), tgt::vec2(0.f, 1.f), 5);
    QPalette p = histogramPainter_->palette();
    p.setBrush(QPalette::Background, Qt::white);
    histogramPainter_->setPalette(p);
    histogramPainter_->setAutoFillBackground(true);

    completeTextureCanvas_ = new tgt::QtCanvas("", tgt::ivec2(1,1), tgt::GLCanvas::RGBADD, 0, true);
    completeTexturePainter_ = new TransFuncTexturePainter(completeTextureCanvas_);
    completeTexturePainter_->initialize();
    completeTexturePainter_->setTransFunc(transferFuncIntensity_);
    completeTextureCanvas_->setPainter(completeTexturePainter_, false);
    completeTextureCanvas_->setContentsMargins(-5, 0, -5, 0);
    completeTextureCanvas_->setFixedHeight(20);

    expandedTextureCanvas_ = new tgt::QtCanvas("", tgt::ivec2(1,1), tgt::GLCanvas::RGBADD, 0, true);
    expandedTexturePainter_ = new TransFuncTexturePainter(expandedTextureCanvas_);
    expandedTexturePainter_->initialize();
    expandedTexturePainter_->setTransFunc(transferFuncGradient_);
    expandedTextureCanvas_->setPainter(expandedTexturePainter_, false);
    expandedTextureCanvas_->setContentsMargins(-5, 0, -5, 0);
    expandedTextureCanvas_->setFixedHeight(20);

    //slider for adjusting transfer function
    doubleSlider_ = new DoubleSlider();
    doubleSlider_->setOffsets(5, 5);

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
    //the spacing is added so that spinboxes and doubleslider are vertically aligned
    hboxSpin->addSpacing(-1);
    hboxSpin->addWidget(lowerThresholdSpin_);
    hboxSpin->addStretch();
    hboxSpin->addWidget(upperThresholdSpin_);
    hboxSpin->addSpacing(-1);

    expandButton_ = new QPushButton(tr("&Expand"));
    collapseButton_ = new QPushButton(tr("&Collapse"));

    mainLayout->addItem(buttonLayout);
    mainLayout->addWidget(histogramPainter_);
    mainLayout->addWidget(completeTextureCanvas_);
    mainLayout->addWidget(expandedTextureCanvas_);
    mainLayout->addWidget(doubleSlider_);
    mainLayout->addItem(hboxSpin);
    mainLayout->addWidget(expandButton_);
    mainLayout->addWidget(collapseButton_);

    setLayout(mainLayout);
}

void TransFuncEditorIntensityPet::createConnections() {
    // buttons
    connect(loadButton_, SIGNAL(clicked()), this, SLOT(loadTransferFunction()));
    connect(saveButton_, SIGNAL(clicked()), this, SLOT(saveTransferFunction()));
    connect(resetButton_, SIGNAL(clicked()), this, SLOT(resetButtonClicked()));
    if (property_->getManualRepaint())
        connect(repaintButton_, SIGNAL(clicked()), this, SLOT(causeVolumeRenderingRepaint()));

    // double slider
    connect(doubleSlider_, SIGNAL(valuesChanged(float, float)), this, SLOT(sliderChanged(float, float)));
    connect(doubleSlider_, SIGNAL(switchInteractionMode(bool)), this, SLOT(switchInteractionMode(bool)));

    // spinboxes
    connect(lowerThresholdSpin_, SIGNAL(valueChanged(int)), this, SLOT(minBoxChanged(int)));
    connect(upperThresholdSpin_, SIGNAL(valueChanged(int)), this, SLOT(maxBoxChanged(int)));

    // expand and collapse button
    connect(expandButton_,   SIGNAL(clicked()), this, SLOT(expandGradient()));
    connect(collapseButton_, SIGNAL(clicked()), this, SLOT(collapseGradient()));
}

void TransFuncEditorIntensityPet::loadTransferFunction() {
    // create filter with supported file formats
    QString filter = "transfer function (";
    for (size_t i = 0; i < transferFuncIntensity_->getLoadFileFormats().size(); ++i) {
        std::string temp = "*." + transferFuncIntensity_->getLoadFileFormats()[i] + " ";
        filter.append(temp.c_str());
    }
    filter.replace(filter.length()-1, 1, ")");

    QString fileName = getOpenFileName(filter);
    if (!fileName.isEmpty()) {
        if (transferFuncIntensity_->load(fileName.toStdString()) &&
            transferFuncGradient_->load(fileName.toStdString()))
        {
            if (!transferFunctionCorrect()) {
                rectifyTransferFunction();
                std::string error = "The transfer function contains keys which alpha value is not ";
                error += "255.\nThe alpha value of these keys will be set to 255.";
                QMessageBox::critical(this, tr("Error"), tr(error.c_str()));
                LINFO(error);
            }
            ranges_.clear();
            currentRange_ = tgt::ivec2(0, maximumIntensity_);
            oldThreshold_ = tgt::vec2(0.f, 1.f);
            if (transferFuncIntensity_->getThresholds() != tgt::vec2(0.f, 1.f))
                restoreThresholds();

            updateTransferFunction();
        }
        else {
            QMessageBox::critical(this, tr("Error"), "The selected transfer function could not be loaded.");
            LERROR("The selected transfer function could not be loaded. Maybe the file is corrupt.");
        }
    }
}

void TransFuncEditorIntensityPet::saveTransferFunction() {
    QStringList filter;
    for (size_t i = 0; i < transferFuncIntensity_->getSaveFileFormats().size(); ++i) {
        std::string temp = "transfer function (*." + transferFuncIntensity_->getSaveFileFormats()[i] + ")";
        filter << temp.c_str();
    }

    QString fileName = getSaveFileName(filter);
    if (!fileName.isEmpty()) {
        // save transfer function to disk
        if (!transferFuncIntensity_->save(fileName.toStdString())) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("The transfer function could not be saved."));
            LERROR("The transfer function could not be saved. Maybe the disk is full?");
        }
    }
}

void TransFuncEditorIntensityPet::causeVolumeRenderingRepaint() {
    // this informs the owner about change in transfer function texture
    property_->notifyChange();

    // this signal causes a repaint of the volume rendering
    emit transferFunctionChanged();

    repaintAll();
}

void TransFuncEditorIntensityPet::rectifyTransferFunction() {
    for (int i = 0; i < transferFuncIntensity_->getNumKeys(); ++i) {
        TransFuncMappingKey* key = transferFuncIntensity_->getKey(i);
        key->setAlphaL(1.f);
        key = transferFuncGradient_->getKey(i);
        key->setAlphaL(1.f);
    }
    transferFuncIntensity_->textureUpdateNeeded();
    transferFuncGradient_->textureUpdateNeeded();
}

bool TransFuncEditorIntensityPet::transferFunctionCorrect() {
    // iterate through all keys and test whether they are set to maximum alpha
    for (int i = 0; i < transferFuncIntensity_->getNumKeys(); ++i) {
        TransFuncMappingKey* key = transferFuncIntensity_->getKey(i);
        if ((key->getAlphaL() != 1.f) || (key->isSplit() && key->getAlphaR() != 1.f)) {
            return false;
        }
    }
    return true;
}

void TransFuncEditorIntensityPet::resetButtonClicked() {
    resetTransferFunction();
    resetThresholds();

    causeVolumeRenderingRepaint();
}

void TransFuncEditorIntensityPet::resetThresholds() {
    ranges_.clear();
    currentRange_ = tgt::ivec2(0, maximumIntensity_);
    oldThreshold_ = tgt::vec2(0.f, 1.f);

    //reset spinboxes to default
    lowerThresholdSpin_->blockSignals(true);
    lowerThresholdSpin_->setRange(0, maximumIntensity_ - 1);
    lowerThresholdSpin_->setValue(0);
    lowerThresholdSpin_->blockSignals(false);
    upperThresholdSpin_->blockSignals(true);
    upperThresholdSpin_->setRange(1, maximumIntensity_);
    upperThresholdSpin_->setValue(maximumIntensity_);
    upperThresholdSpin_->blockSignals(false);

    //reset doubleslider
    doubleSlider_->setMinimalAllowedSliderDistance(1.f / static_cast<float>(maximumIntensity_));
    doubleSlider_->blockSignals(true);
    doubleSlider_->setValues(0.f, 1.f);
    doubleSlider_->blockSignals(false);

    transferFuncIntensity_->setThresholds(0.f, 1.f);
    transferFuncGradient_->setThresholds(0.f, 1.f);

    applyThreshold();
}

void TransFuncEditorIntensityPet::sliderChanged(float min, float max) {
    //consider expand factor
    min = min * (currentRange_.y - currentRange_.x) + currentRange_.x;
    max = max * (currentRange_.y - currentRange_.x) + currentRange_.x;

    //sync with spinboxes
    int val = tgt::iround(max);
    if ((val != upperThresholdSpin_->value()))
        upperThresholdSpin_->setValue(val);

    val = tgt::iround(min);
    if ((val != lowerThresholdSpin_->value()))
        lowerThresholdSpin_->setValue(val);

    //apply threshold to both transfer functions
    applyThreshold();
    updateTransferFunction();
}

void TransFuncEditorIntensityPet::minBoxChanged(int value) {
    if (value+1 < currentRange_.y) {
        //increment maximum of lower spin when maximum was reached and we are below upper range
        if (value == lowerThresholdSpin_->maximum())
            lowerThresholdSpin_->setMaximum(value + 1);

        //update minimum of upper spin
        upperThresholdSpin_->blockSignals(true);
        upperThresholdSpin_->setMinimum(value);
        upperThresholdSpin_->blockSignals(false);
    }
    //increment value of upper spin when it equals value of lower spin
    if (value == upperThresholdSpin_->value()) {
        upperThresholdSpin_->blockSignals(true);
        upperThresholdSpin_->setValue(value + 1);
        upperThresholdSpin_->blockSignals(false);
    }

    //update doubleSlider to new minValue
    float sliderMin = (value - currentRange_.x) / static_cast<float>(currentRange_.y - currentRange_.x);
    doubleSlider_->blockSignals(true);
    doubleSlider_->setMinValue(sliderMin);
    doubleSlider_->blockSignals(false);

    //apply threshold to both transfer functions
    applyThreshold();
    updateTransferFunction();
}

void TransFuncEditorIntensityPet::maxBoxChanged(int value) {
    if (value-1 > currentRange_.x) {
        //increment minimum of upper spin when minimum was reached and we are above lower range
        if (value == upperThresholdSpin_->minimum())
            upperThresholdSpin_->setMinimum(value - 1);

        //update maximum of lower spin
        lowerThresholdSpin_->blockSignals(true);
        lowerThresholdSpin_->setMaximum(value);
        lowerThresholdSpin_->blockSignals(false);
    }
    //increment value of lower spin when it equals value of upper spin
    if (value == lowerThresholdSpin_->value()) {
        lowerThresholdSpin_->blockSignals(true);
        lowerThresholdSpin_->setValue(value - 1);
        lowerThresholdSpin_->blockSignals(false);
    }

    //update doubleSlider to new maxValue
    float sliderMax = (value - currentRange_.x) / static_cast<float>(currentRange_.y - currentRange_.x);
    doubleSlider_->blockSignals(true);
    doubleSlider_->setMaxValue(sliderMax);
    doubleSlider_->blockSignals(false);

    //apply threshold to both transfer functions
    applyThreshold();
    updateTransferFunction();
}

void TransFuncEditorIntensityPet::expandGradient() {
    //save old range
    ranges_.push_back(currentRange_);

    //new range of spinboxes
    currentRange_ = tgt::ivec2(lowerThresholdSpin_->value(), upperThresholdSpin_->value());

    //set minimum and maximum of thresholdspins
    lowerThresholdSpin_->setMinimum(lowerThresholdSpin_->value());
    lowerThresholdSpin_->setMaximum(upperThresholdSpin_->value() - 1);
    upperThresholdSpin_->setMinimum(lowerThresholdSpin_->value() + 1);
    upperThresholdSpin_->setMaximum(upperThresholdSpin_->value());

    //reset doubleslider
    doubleSlider_->blockSignals(true);
    doubleSlider_->setValues(0.f, 1.f);
    doubleSlider_->setMinimalAllowedSliderDistance(1.f / static_cast<float>(currentRange_.y - currentRange_.x));
    doubleSlider_->blockSignals(false);

    applyThreshold();
    updateTransferFunction();
}

void TransFuncEditorIntensityPet::collapseGradient() {
    if (ranges_.empty())
        return;

    //restore old range
    currentRange_ = ranges_[ranges_.size() - 1];
    ranges_.pop_back();

    //reset minimum and maximum values of spinboxes to values before last expand
    lowerThresholdSpin_->setMinimum(currentRange_.x);
    lowerThresholdSpin_->setMaximum(currentRange_.y - 1);
    upperThresholdSpin_->setMinimum(currentRange_.x + 1);
    upperThresholdSpin_->setMaximum(currentRange_.y);

    //reset double slider to range before last expand
    doubleSlider_->blockSignals(true);
    doubleSlider_->setMinimalAllowedSliderDistance(1.f / static_cast<float>(currentRange_.y - currentRange_.x));
    doubleSlider_->setValues((lowerThresholdSpin_->value() - currentRange_.x) / static_cast<float>(currentRange_.y - currentRange_.x),
                             (upperThresholdSpin_->value() - currentRange_.x) / static_cast<float>(currentRange_.y - currentRange_.x));
    doubleSlider_->blockSignals(false);

    applyThreshold();
    updateTransferFunction();
}

void TransFuncEditorIntensityPet::updateTransferFunction() {
    transferFuncIntensity_->textureUpdateNeeded();
    transferFuncGradient_->textureUpdateNeeded();

    if (!property_->getManualRepaint())
        causeVolumeRenderingRepaint();
}

void TransFuncEditorIntensityPet::update() {
    // check whether the volume associated with the TransFuncProperty has changed
    Volume* newVol = property_->getVolume();
    if (newVol != volume_) {
        volume_ = newVol;
        volumeChanged();
    }

    if (transferFuncGradient_) {
        transferFuncGradient_->setTextureDimension(maximumIntensity_ + 1, 1);

        bool emitRepaint = false;
        // adjust tf when not all keys have maximum alpha
        if (!transferFunctionCorrect()) {
            LINFO("The transfer function contains keys that don't have maximum alpha. "
                   << "The alpha value of these keys is set to 255.");
            rectifyTransferFunction();

            emitRepaint = true;
        }

        // update threshold control elements when necessary
        if (transferFuncIntensity_->getThresholds() != tgt::vec2(0.f, 1.f)) {
            restoreThresholds();
            emitRepaint = true;
        }

        if (emitRepaint) {
            property_->notifyChange();
            emit transferFunctionChanged();
        }

        // repaint control elements
        repaintAll();
    }
    else
        resetEditor();
}

void TransFuncEditorIntensityPet::volumeChanged() {
    if (volume_) {
        int bits = volume_->getBitsStored() / volume_->getNumChannels();
        int maxNew = static_cast<int>(pow(2.f, static_cast<float>(bits))) - 1;
        if (maxNew != maximumIntensity_) {
            maximumIntensity_ = maxNew;
            currentRange_ = tgt::ivec2(0, maximumIntensity_);

            lowerThresholdSpin_->blockSignals(true);
            upperThresholdSpin_->blockSignals(true);

            lowerThresholdSpin_->setRange(0, maximumIntensity_ - 1);
            lowerThresholdSpin_->setValue(tgt::iround(oldThreshold_.x * maximumIntensity_));

            upperThresholdSpin_->setRange(1, maximumIntensity_);
            upperThresholdSpin_->setValue(tgt::iround(oldThreshold_.y * maximumIntensity_));

            lowerThresholdSpin_->updateGeometry();
            upperThresholdSpin_->updateGeometry();

            lowerThresholdSpin_->blockSignals(false);
            upperThresholdSpin_->blockSignals(false);

            //set minimal distance of sliders in doubleSlider widget
            doubleSlider_->setMinimalAllowedSliderDistance(1.f / static_cast<float>(maximumIntensity_));
            doubleSlider_->blockSignals(true);
            doubleSlider_->setValues(oldThreshold_.x, oldThreshold_.y);
            doubleSlider_->blockSignals(false);

            applyThreshold();
        }
    }

    // propagate new volume to histogrampainter
    histogramPainter_->setHistogram(new HistogramIntensity(volume_, maximumIntensity_+1));
    // resize histogram painter
    histogramPainter_->setMinimumSize(200, 150);

    // clear old expand values
    ranges_.clear();
}

void TransFuncEditorIntensityPet::resetTransferFunction() {
    transferFuncIntensity_->createStdFunc();
    //set left key to maximum alpha
    TransFuncMappingKey* key = transferFuncIntensity_->getKey(0);
    key->setAlphaL(1.f);

    transferFuncGradient_->createStdFunc();
    //set left key to maximum alpha
    key = transferFuncGradient_->getKey(0);
    key->setAlphaL(1.f);
}

void TransFuncEditorIntensityPet::applyThreshold() {
    float min = static_cast<float>(lowerThresholdSpin_->value()) / static_cast<float>(maximumIntensity_);
    float max = static_cast<float>(upperThresholdSpin_->value()) / static_cast<float>(maximumIntensity_);

    for (int i=0; i < transferFuncIntensity_->getNumKeys(); ++i){
        // get intensity from original transfer function and renormalize it
        float intensity = (transferFuncIntensity_->getKey(i)->getIntensity() - oldThreshold_.x) /
                          (oldThreshold_.y - oldThreshold_.x);

        // calculate intensity value for complete gradient
        transferFuncIntensity_->getKey(i)->setIntensity((max - min) * intensity + min);

        // calculate intensity value for expanded gradient
        float start = static_cast<float>(currentRange_.x) / static_cast<float>(maximumIntensity_);
        float end = static_cast<float>(currentRange_.y) / static_cast<float>(maximumIntensity_);
        float temp = (max - min) * intensity + min;
        transferFuncGradient_->getKey(i)->setIntensity((temp - start) / (end - start));

    }
    oldThreshold_ = tgt::vec2(min, max);
}

void TransFuncEditorIntensityPet::restoreThresholds() {
    tgt::vec2 thresh = transferFuncIntensity_->getThresholds();
    resetThresholds();

    // update spinboxes
    int min = tgt::iround(thresh.x * (currentRange_.y - currentRange_.x) + currentRange_.x);
    int max = tgt::iround(thresh.y * (currentRange_.y - currentRange_.x) + currentRange_.x);

    lowerThresholdSpin_->blockSignals(true);
    lowerThresholdSpin_->setValue(min);
    lowerThresholdSpin_->blockSignals(false);
    upperThresholdSpin_->blockSignals(true);
    upperThresholdSpin_->setValue(max);
    upperThresholdSpin_->blockSignals(false);
    if (min + 1 < currentRange_.y) {
        //increment maximum of lower spin when maximum was reached and we are below upper range
        if (min == lowerThresholdSpin_->maximum())
            lowerThresholdSpin_->setMaximum(min + 1);

        //update minimum of upper spin
        upperThresholdSpin_->blockSignals(true);
        upperThresholdSpin_->setMinimum(min);
        upperThresholdSpin_->blockSignals(false);
    }

    if (max - 1 > currentRange_.x) {
        //increment minimum of upper spin when minimum was reached and we are above lower range
        if (max == upperThresholdSpin_->minimum())
            upperThresholdSpin_->setMinimum(max - 1);

        //update maximum of lower spin
        lowerThresholdSpin_->blockSignals(true);
        lowerThresholdSpin_->setMaximum(max);
        lowerThresholdSpin_->blockSignals(false);
    }

    transferFuncIntensity_->setThresholds(0.f, 1.f);
    transferFuncGradient_->setThresholds(0.f, 1.f);

    applyThreshold();

    // update doubleSlider
    doubleSlider_->blockSignals(true);
    doubleSlider_->setValues(oldThreshold_.x, oldThreshold_.y);
    doubleSlider_->setMinimalAllowedSliderDistance(1.f / static_cast<float>(currentRange_.y - currentRange_.x));
    doubleSlider_->blockSignals(false);
}

void TransFuncEditorIntensityPet::getThresholds(int& l, int& u) {
    l = lowerThresholdSpin_->value();
    u = upperThresholdSpin_->value();
}

void TransFuncEditorIntensityPet::resetEditor() {
    if (property_->get() != transferFuncIntensity_) {
        LDEBUG("The pointers of property and transfer function do not match."
                << "Creating new transfer function object.....");
        transferFuncIntensity_ = new TransFuncIntensity(maximumIntensity_ + 1);
        property_->set(transferFuncIntensity_);
        delete transferFuncGradient_;
        transferFuncGradient_ = new TransFuncIntensity(*transferFuncIntensity_);

        // propagate transfer functions to the gradients
        completeTexturePainter_->setTransFunc(transferFuncIntensity_);
        expandedTexturePainter_->setTransFunc(transferFuncGradient_);
    }

    resetTransferFunction();
    resetThresholds();

    causeVolumeRenderingRepaint();
}

void TransFuncEditorIntensityPet::repaintAll() {
    histogramPainter_->repaint();
    doubleSlider_->repaint();
    completeTextureCanvas_->update();
    expandedTextureCanvas_->update();
}

} // namespace voreen
