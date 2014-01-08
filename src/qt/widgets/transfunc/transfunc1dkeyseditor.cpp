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

#include "voreen/qt/widgets/transfunc/transfunc1dkeyseditor.h"

#include "voreen/qt/widgets/transfunc/colorpicker.h"
#include "voreen/qt/widgets/transfunc/colorluminancepicker.h"
#include "voreen/qt/widgets/transfunc/doubleslider.h"
#include "voreen/qt/widgets/transfunc/transfunc1dkeyspainter.h"
#include "voreen/qt/widgets/transfunc/transfuncmappingcanvas.h"
#include "voreen/qt/widgets/transfunc/transfunciohelperqt.h"

#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/volume/histogram.h"
#include "voreen/core/datastructures/volume/volume.h"

#include "tgt/logmanager.h"
#include "tgt/qt/qtcanvas.h"

#include <QApplication>
#include <QPushButton>
#include <QCheckBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSplitter>
#include <QToolButton>
#include <QSizePolicy>
#include <QComboBox>

namespace voreen {

const std::string TransFunc1DKeysEditor::loggerCat_("voreen.qt.TransFunc1DKeysEditor");


//----------------------------------------------------------------------------------------------
//      Constructor and Qt Stuff
//----------------------------------------------------------------------------------------------
TransFunc1DKeysEditor::TransFunc1DKeysEditor(TransFuncProperty* prop, QWidget* parent)
    : TransFuncEditor(prop, parent)
    , transCanvas_(0)
    , transferFuncIntensity_(0)
    , textureCanvas_(0)
    , texturePainter_(0)
    , thresholdSlider_(0)
    , maxDigits_(7)
    , histogram_(0)
    , domainMinValue_(0.f), domainMaxValue_(1.f)
{
    title_ = QString("Intensity");
    transferFuncIntensity_ = dynamic_cast<TransFunc1DKeys*>(property_->get());
}

TransFunc1DKeysEditor::~TransFunc1DKeysEditor() {
}

QLayout* TransFunc1DKeysEditor::createMappingLayout() {
    transCanvas_ = new TransFuncMappingCanvas(0, transferFuncIntensity_);
    transCanvas_->setMinimumWidth(200);

    QWidget* additionalSpace = new QWidget();
    additionalSpace->setMinimumHeight(2);

    // threshold slider
    QHBoxLayout* hboxSlider = new QHBoxLayout();
    thresholdSlider_ = new DoubleSlider();
    thresholdSlider_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    thresholdSlider_->setOffsets(12, 27);
    hboxSlider->addWidget(thresholdSlider_);

    //add gradient that displays the transferfunction as image
    textureCanvas_ = new tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, 0, true);
    texturePainter_ = new TransFunc1DKeysPainter(textureCanvas_);
    texturePainter_->initialize();
    texturePainter_->setTransFunc(transferFuncIntensity_);
    textureCanvas_->setPainter(texturePainter_, false);
    textureCanvas_->setFixedHeight(12);
    textureCanvas_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    QHBoxLayout* hboxTexture = new QHBoxLayout();
    hboxTexture->addSpacing(12);
    hboxTexture->addWidget(textureCanvas_);
    hboxTexture->addSpacing(25);

    // put widgets in layout
    QVBoxLayout* vBox = new QVBoxLayout();
    vBox->setMargin(0);
    vBox->setSpacing(1);
    vBox->addStretch();
    vBox->addWidget(transCanvas_, 1);
    vBox->addLayout(hboxTexture);//Widget(textureCanvas_);
    vBox->addWidget(additionalSpace);
    vBox->addLayout(hboxSlider);
    vBox->addSpacing(1);

    return vBox;
}

QLayout* TransFunc1DKeysEditor::createBaseButtonLayout() {
    QBoxLayout* buttonLayout = new QVBoxLayout();

    clearButton_ = new QToolButton();
    clearButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    clearButton_->setText("Reset");
    clearButton_->setFixedWidth(65);
    clearButton_->setIcon(QIcon(":/qt/icons/clear.png"));

    loadButton_ = new QToolButton();
    loadButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    loadButton_->setText("Load");
    loadButton_->setFixedWidth(65);
    loadButton_->setIcon(QIcon(":/qt/icons/open.png"));

    saveButton_ = new QToolButton();
    saveButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    saveButton_->setText("Save");
    saveButton_->setFixedWidth(65);
    saveButton_->setIcon(QIcon(":/qt/icons/save.png"));

    buttonLayout->addWidget(clearButton_);
    buttonLayout->addWidget(loadButton_);
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addStretch();

    return buttonLayout;
}

QLayout* TransFunc1DKeysEditor::createColorLayout() {
    // ColorPicker
    colorPicker_ = new ColorPicker();
    colorPicker_->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    colorPicker_->setFixedWidth(100);
    colorPicker_->setFixedHeight(100);

    // ColorLuminacePicker
    colorLumPicker_ = new ColorLuminancePicker();
    colorLumPicker_->setFixedWidth(20);
    colorLumPicker_->setFixedHeight(100);

    QHBoxLayout* hBoxColor = new QHBoxLayout();
    hBoxColor->addWidget(colorPicker_);
    hBoxColor->addWidget(colorLumPicker_);

    return hBoxColor;
}

QLayout* TransFunc1DKeysEditor::createDomainLayout() {
    //data set domains
    QHBoxLayout* dataSetDomainLayout = new QHBoxLayout();
    lowerData_ = new QLabel();
    lowerData_->setAlignment(Qt::AlignCenter);
    lowerData_->setFixedWidth(6*maxDigits_+25);
    upperData_ = new QLabel();
    upperData_->setAlignment(Qt::AlignCenter);
    upperData_->setFixedWidth(6*maxDigits_+25);
    QLabel* dataLabel = new QLabel("Data Set Bounds");
    dataSetDomainLayout->addWidget(lowerData_);
    dataSetDomainLayout->addStretch(1);
    dataSetDomainLayout->addWidget(dataLabel);
    dataSetDomainLayout->addStretch(1);
    dataSetDomainLayout->addWidget(upperData_);
    //domains
    QHBoxLayout* domainLayout = new QHBoxLayout();
    lowerDomainSpin_ = new QDoubleSpinBox();
    upperDomainSpin_ = new QDoubleSpinBox();
    upperDomainSpin_->setRange(-9999999.0, 9999999.0);
    lowerDomainSpin_->setRange(-9999999.0, 9999999.0);
    upperDomainSpin_->setValue(1.0);
    lowerDomainSpin_->setValue(0.0);
    upperDomainSpin_->setKeyboardTracking(false);
    lowerDomainSpin_->setKeyboardTracking(false);
    upperDomainSpin_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lowerDomainSpin_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    upperDomainSpin_->setDecimals(maxDigits_-1);
    lowerDomainSpin_->setDecimals(maxDigits_-1);
    upperDomainSpin_->setFixedWidth(6*maxDigits_+25);
    lowerDomainSpin_->setFixedWidth(6*maxDigits_+25);
    QLabel* domainLabel = new QLabel();
    domainLabel->setText("Color Map Domain");
    domainLayout->addWidget(lowerDomainSpin_);
    domainLayout->addStretch(1);
    domainLayout->addWidget(domainLabel);
    domainLayout->addStretch(1);
    domainLayout->addWidget(upperDomainSpin_);
    //autofit
    QHBoxLayout* autoFitLayout = new QHBoxLayout();
    fitDomainToData_ = new QPushButton();
    fitDomainToData_->setText(" Fit Domain To Data ");
    domainFittingStrategy_ = new QComboBox();
    domainFittingStrategy_->addItem("Never Fit Domain",     TransFuncProperty::FIT_DOMAIN_NEVER);
    domainFittingStrategy_->addItem("Fit Initial Domain",   TransFuncProperty::FIT_DOMAIN_INITIAL);
    domainFittingStrategy_->addItem("Always Fit Domain",    TransFuncProperty::FIT_DOMAIN_ALWAYS);
    autoFitLayout->addWidget(fitDomainToData_);
    autoFitLayout->addStretch();
    autoFitLayout->addWidget(domainFittingStrategy_);
    //threshold
    QHBoxLayout* thresholdLayout = new QHBoxLayout();
    lowerThresholdSpin_ = new QDoubleSpinBox();
    upperThresholdSpin_ = new QDoubleSpinBox();
    upperThresholdSpin_->setRange(-9999999.0, 9999999.0);
    lowerThresholdSpin_->setRange(-9999999.0, 9999999.0);
    upperThresholdSpin_->setValue(1.0);
    lowerThresholdSpin_->setValue(0.0);
    upperThresholdSpin_->setKeyboardTracking(false);
    lowerThresholdSpin_->setKeyboardTracking(false);
    upperThresholdSpin_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lowerThresholdSpin_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    upperThresholdSpin_->setDecimals(maxDigits_-1);
    lowerThresholdSpin_->setDecimals(maxDigits_-1);
    upperThresholdSpin_->setFixedWidth(6*maxDigits_+25);
    lowerThresholdSpin_->setFixedWidth(6*maxDigits_+25);
    QLabel* thresholdLabel = new QLabel();
    thresholdLabel->setText("Threshold Bounds");
    thresholdLayout->addWidget(lowerThresholdSpin_);
    thresholdLayout->addStretch(1);
    thresholdLayout->addWidget(thresholdLabel);
    thresholdLayout->addStretch(1);
    thresholdLayout->addWidget(upperThresholdSpin_);
    //line
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QVBoxLayout* vBox = new QVBoxLayout();
    vBox->addLayout(dataSetDomainLayout);
    vBox->addLayout(domainLayout);
    vBox->addLayout(autoFitLayout);
    vBox->addWidget(line);
    vBox->addLayout(thresholdLayout);
    return vBox;
}

QLayout* TransFunc1DKeysEditor::createTFLayout() {
    //make ramp
    makeRampButton_ = new QToolButton();
    makeRampButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    makeRampButton_->setIcon(QIcon(":/qt/icons/ramp.png"));
    makeRampButton_->setText("Make Ramp");
    //invert
    invertButton_ = new QToolButton();
    invertButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    invertButton_->setIcon(QIcon(":/qt/icons/arrow-leftright.png"));
    invertButton_->setText("Invert Map");
    //alpha
    QLabel* alphaLabel = new QLabel("     Transparency:");
    alphaButton_ = new QToolButton();
    alphaButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    alphaButton_->setIcon(QIcon(":/qt/icons/alpha.png"));
    alphaButton_->setText("Alpha ");
    alphaButton_->setPopupMode(QToolButton::InstantPopup);
    alphaMenu_ = new QMenu();
    alphaMenu_->addAction(QIcon(":/qt/icons/alpha_trans.png"),"Transparent");
    alphaMenu_->addAction(QIcon(":/qt/icons/alpha_use.png"),"Use Alpha");
    alphaMenu_->addAction(QIcon(":/qt/icons/alpha_opaque.png"),"Opaque");
    alphaButton_->setMenu(alphaMenu_);
    //gamma
    gammaSpin_ = new QDoubleSpinBox();
    gammaSpin_->setRange(0.1, 5.0);
    gammaSpin_->setValue(1.0);
    gammaSpin_->setSingleStep(0.1);
    gammaSpin_->setKeyboardTracking(false);
    gammaSpin_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    gammaSpin_->setDecimals(2);
    gammaSpin_->setFixedWidth(6*3+25);
    QLabel* gammaLabel = new QLabel("     Gamma Value:");

    QGridLayout* layout = new QGridLayout();
    layout->addWidget(makeRampButton_,0,0);
    layout->addWidget(invertButton_,1,0);
    layout->addWidget(alphaLabel,0,1);
    layout->addWidget(alphaButton_,0,2);
    layout->addWidget(gammaLabel,1,1);
    layout->addWidget(gammaSpin_,1,2);

    return layout;
}

void TransFunc1DKeysEditor::createWidgets() {
    //main left and right widget
    QWidget* mainLeft = new QWidget();
    QWidget* mainRight = new QWidget();
    //create different base layouts
    QLayout* mappingLayout = createMappingLayout();
    QLayout* colorLayout = createColorLayout();
    QLayout* buttonLayout = createBaseButtonLayout();
    QLayout* domainLayout = createDomainLayout();
    QLayout* tfLayout = createTFLayout();
    //set left layout
    mainLeft->setLayout(mappingLayout);
    //set right layout
    QGridLayout* gridLayout = new QGridLayout();

    QGroupBox* colorBox = new QGroupBox("Color Picking");
    colorBox->setLayout(colorLayout);
    colorBox->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    QGroupBox* baseBox = new QGroupBox("Load and Save");
    baseBox->setLayout(buttonLayout);
    baseBox->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    QGroupBox* tfBox = new QGroupBox("Color Map Settings");
    tfBox->setLayout(tfLayout);
    tfBox->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);

    QGroupBox* domainBox = new QGroupBox("Domain Settings");
    domainBox->setLayout(domainLayout);
    domainBox->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    gridLayout->addWidget(colorBox,0,0);
    gridLayout->addWidget(baseBox,0,1);
    gridLayout->addWidget(tfBox,1,0,2,0);
    gridLayout->addWidget(domainBox,3,0,2,0);
    gridLayout->setSizeConstraint(QLayout::SetFixedSize); //fixes the left widget to

    mainRight->setLayout(gridLayout);

    //create splitter for main layout
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->setStretchFactor(0, QSizePolicy::Expanding); // mapping should be stretched
    splitter->setStretchFactor(1, QSizePolicy::Fixed); // color should not be stretched
    splitter->setChildrenCollapsible(true);
    splitter->addWidget(mainLeft);
    splitter->addWidget(mainRight);
    //create main layout
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->setMargin(4);
    mainLayout->addWidget(splitter);
    //set main layout
    setLayout(mainLayout);
    updateThresholdFromProperty();
}

void TransFunc1DKeysEditor::createConnections() {
    // signals from transferMappingCanvas
    connect(transCanvas_, SIGNAL(changed()), this, SLOT(updateTransferFunction()));
    connect(transCanvas_, SIGNAL(loadTransferFunction()), this, SLOT(loadTransferFunction()));
    connect(transCanvas_, SIGNAL(saveTransferFunction()), this, SLOT(saveTransferFunction()));
    connect(transCanvas_, SIGNAL(resetTransferFunction()), this, SLOT(clearButtonClicked()));
    connect(transCanvas_, SIGNAL(toggleInteractionMode(bool)), this, SLOT(toggleInteractionMode(bool)));
    // thresholdslider
    connect(thresholdSlider_, SIGNAL(valuesChanged(float, float)), this, SLOT(applySliderThreshold()));
    connect(thresholdSlider_, SIGNAL(toggleInteractionMode(bool)), this, SLOT(toggleInteractionMode(bool)));
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
    // Base Buttons
    connect(clearButton_, SIGNAL(clicked()), this, SLOT(clearButtonClicked()));
    connect(loadButton_, SIGNAL(clicked()), this, SLOT(loadTransferFunction()));
    connect(saveButton_, SIGNAL(clicked()), this, SLOT(saveTransferFunction()));
    //color map settings
    connect(makeRampButton_, SIGNAL(clicked()), this, SLOT(makeRamp()));
    connect(invertButton_, SIGNAL(clicked()), this, SLOT(invertMap()));
    connect(gammaSpin_, SIGNAL(valueChanged(double)), this, SLOT(gammaChanged(double)));
    connect(alphaMenu_, SIGNAL(triggered(QAction*)), this, SLOT(setAlpha(QAction*)));

    //domain settings
    connect(lowerDomainSpin_, SIGNAL(valueChanged(double)), this, SLOT(lowerMappingChanged(double)));
    connect(upperDomainSpin_, SIGNAL(valueChanged(double)), this, SLOT(upperMappingChanged(double)));
    connect(fitDomainToData_, SIGNAL(clicked()), this, SLOT(fitDomainToData()));
    connect(domainFittingStrategy_, SIGNAL(currentIndexChanged(int)), this, SLOT(domainFittingStrategyChanged(int)));
    connect(lowerThresholdSpin_, SIGNAL(valueChanged(double)), this, SLOT(applySpinThreshold()));
    connect(upperThresholdSpin_, SIGNAL(valueChanged(double)), this, SLOT(applySpinThreshold()));
}

//----------------------------------------------------------------------------------------------
//      small functions
//----------------------------------------------------------------------------------------------
void TransFunc1DKeysEditor::causeVolumeRenderingRepaint() {
    // this informs the owner about change in transfer function texture
    property_->notifyChange();
    repaintAll();
    emit transferFunctionChanged();
}

void TransFunc1DKeysEditor::repaintAll() {
    transCanvas_->update();
    thresholdSlider_->update();
    textureCanvas_->update();
}

void TransFunc1DKeysEditor::clearButtonClicked() {
    resetTransferFunction();
    causeVolumeRenderingRepaint();
}

void TransFunc1DKeysEditor::resetTransferFunction() {
    if (!transferFuncIntensity_) {
        LWARNING("No valid transfer function assigned");
        return;
    }

    transferFuncIntensity_->setToStandardFunc();
    fitDomainToData();
}

void TransFunc1DKeysEditor::fitDomainToData() {
    property_->fitDomainToData();
    updateMappingSpin(true);
    updateThresholdFromProperty();
    updateTransferFunction();
}

void TransFunc1DKeysEditor::domainFittingStrategyChanged(int index)  {
    if (index < 0 || index >= domainFittingStrategy_->count()) {
        LERROR("domainFittingStrategyChanged(): invalid index " << index);
        return;
    }

    TransFuncProperty::DomainAutoFittingStrategy strategy =
        (TransFuncProperty::DomainAutoFittingStrategy)(domainFittingStrategy_->itemData(index).toUInt());
    property_->setDomainFittingStrategy(strategy);
}

void TransFunc1DKeysEditor::setTransFuncProp(TransFuncProperty* prop) {

    TransFuncEditor::setTransFuncProp(prop);

    // update widgets
    transferFuncIntensity_ = dynamic_cast<TransFunc1DKeys*>(prop->get());
    texturePainter_->setTransFunc(transferFuncIntensity_);
    transCanvas_->setTransFunc(transferFuncIntensity_);
    updateFromProperty();
}

const TransFuncProperty* TransFunc1DKeysEditor::getTransFuncProp() const {
    return property_;
}

void TransFunc1DKeysEditor::updateTransferFunction() {
    if (!transferFuncIntensity_)
        return;

    transferFuncIntensity_->invalidateTexture();
    property_->notifyChange();
    emit transferFunctionChanged();
}

void TransFunc1DKeysEditor::markerColorChanged(int h, int s, int v) {
    transCanvas_->changeCurrentColor(QColor::fromHsv(h, s, v));
}

//----------------------------------------------------------------------------------------------
//      load and save
//----------------------------------------------------------------------------------------------
void TransFunc1DKeysEditor::saveTransferFunction() {

    if (!transferFuncIntensity_) {
        LWARNING("No valid transfer function assigned");
        return;
    }

    TransFuncIOHelperQt::saveTransferFunction(transferFuncIntensity_);
}

void TransFunc1DKeysEditor::loadTransferFunction() {

    if (!transferFuncIntensity_) {
        LWARNING("No valid transfer function assigned");
        return;
    }

    if(TransFuncIOHelperQt::loadTransferFunction(transferFuncIntensity_)) {
        updateMappingSpin(true);
        updateThresholdFromProperty();
        updateTransferFunction();
    }
}

void TransFunc1DKeysEditor::makeRamp() {
    if (transferFuncIntensity_) {
        transferFuncIntensity_->makeRamp();
        updateTransferFunction();
    }
}

void TransFunc1DKeysEditor::invertMap() {
    if (transferFuncIntensity_) {
        transferFuncIntensity_->invertKeys();
        updateTransferFunction();
    }
}

void TransFunc1DKeysEditor::gammaChanged(double gamma) {
    if (transferFuncIntensity_) {
        transferFuncIntensity_->setGammaValue((float)gamma);
        updateTransferFunction();
    }
}

void TransFunc1DKeysEditor::setAlpha(QAction* action) {
    if (transferFuncIntensity_) {
        TransFunc::AlphaMode mode;
        if(action->text() == "Transparent") {
            mode = TransFunc::TF_ZERO_ALPHA;
        } else if(action->text() == "Opaque") {
            mode = TransFunc::TF_ONE_ALPHA;
        } else {
            mode = TransFunc::TF_USE_ALPHA;
        }
        transferFuncIntensity_->setAlphaMode(mode);
        updateAlphaButton(mode);
        updateTransferFunction();
    }
}

void TransFunc1DKeysEditor::updateAlphaButton(TransFunc::AlphaMode mode){
    switch(mode) {
    case TransFunc::TF_ZERO_ALPHA:
        alphaButton_->setIcon(QIcon(":/qt/icons/alpha_trans.png"));
        alphaButton_->setText("   ");
    break;
    case TransFunc::TF_USE_ALPHA:
        alphaButton_->setIcon(QIcon(":/qt/icons/alpha_use.png"));
        alphaButton_->setText("   ");
    break;
    case TransFunc::TF_ONE_ALPHA:
        alphaButton_->setIcon(QIcon(":/qt/icons/alpha_opaque.png"));
        alphaButton_->setText("   ");
    break;
    }
}

void TransFunc1DKeysEditor::updateDataBounds() {
    lowerData_->setText(QString::number(domainMinValue_));
    upperData_->setText(QString::number(domainMaxValue_));
}

//----------------------------------------------------------------------------------------------
//      update mapping
//----------------------------------------------------------------------------------------------
void TransFunc1DKeysEditor::updateMappingSpin(bool fromTF){
   if (!transferFuncIntensity_)
       return;

   lowerDomainSpin_->blockSignals(true);
   upperDomainSpin_->blockSignals(true);
   lowerThresholdSpin_->blockSignals(true);
   upperThresholdSpin_->blockSignals(true);

   if(fromTF){
        tgt::vec2 domain = transferFuncIntensity_->getDomain();
        lowerDomainSpin_->setValue(domain.x);
        upperDomainSpin_->setValue(domain.y);
   } else
        transferFuncIntensity_->setDomain(static_cast<float>(lowerDomainSpin_->value()),static_cast<float>(upperDomainSpin_->value()),0);

   transCanvas_->domainChanged();

   double min = lowerDomainSpin_->value();
   double max = upperDomainSpin_->value();
   double diff = max - min;

   lowerThresholdSpin_->setRange(min,max);
   upperThresholdSpin_->setRange(min,max);
   lowerThresholdSpin_->setValue(min+diff*transferFuncIntensity_->getThresholds().x);
   upperThresholdSpin_->setValue(min+diff*transferFuncIntensity_->getThresholds().y);

   //set decimals
   if(abs(min) < 1.0){
       lowerDomainSpin_->setDecimals(maxDigits_-1);
       lowerThresholdSpin_->setDecimals(maxDigits_-1);
   } else {
       lowerDomainSpin_->setDecimals(maxDigits_-(static_cast<int>( log10( abs( min ) ) ) + 1));
       lowerThresholdSpin_->setDecimals(maxDigits_-(static_cast<int>( log10( abs( min ) ) ) + 1));
   }
   if(abs(max) < 1.0) {
       upperDomainSpin_->setDecimals(maxDigits_-1);
       upperThresholdSpin_->setDecimals(maxDigits_-1);
   }
   else {
       upperDomainSpin_->setDecimals(maxDigits_-(static_cast<int>( log10( abs( max ) ) ) + 1));
       upperThresholdSpin_->setDecimals(maxDigits_-(static_cast<int>( log10( abs( max ) ) ) + 1));
   }

   //set stepsize
   lowerDomainSpin_->setSingleStep(diff/1000.0);
   upperDomainSpin_->setSingleStep(diff/1000.0);
   lowerThresholdSpin_->setSingleStep(diff/1000.0);
   upperThresholdSpin_->setSingleStep(diff/1000.0);

   lowerDomainSpin_->blockSignals(false);
   upperDomainSpin_->blockSignals(false);
   lowerThresholdSpin_->blockSignals(false);
   upperThresholdSpin_->blockSignals(false);
}

void TransFunc1DKeysEditor::mappingChanged() {
    if (!transferFuncIntensity_)
        return;

    updateMappingSpin(false);
    updateThresholdFromProperty();

    updateTransferFunction();
    checkDomainVersusData();
}

void TransFunc1DKeysEditor::lowerMappingChanged(double value) {
    if (!transferFuncIntensity_)
        return;

    //increment value of lower mapping spin when it equals value of upper mapping spin
    if (value >= upperDomainSpin_->value()) {
        upperDomainSpin_->blockSignals(true);
        upperDomainSpin_->setValue(value+0.01);
        upperDomainSpin_->blockSignals(false);
    }
    mappingChanged();
}

void TransFunc1DKeysEditor::upperMappingChanged(double value) {
    if (!transferFuncIntensity_)
        return;

    //increment value of upper mapping spin when it equals value of lower mapping spin
    if (value <= lowerDomainSpin_->value()) {
        lowerDomainSpin_->blockSignals(true);
        lowerDomainSpin_->setValue(value-0.01);
        lowerDomainSpin_->blockSignals(false);
    }
    mappingChanged();
}

void TransFunc1DKeysEditor::updateThresholdFromProperty() {
    if (!transferFuncIntensity_)
        return;

   double min = transferFuncIntensity_->getThresholds().x;
   double max = transferFuncIntensity_->getThresholds().y;

   transCanvas_->setThreshold(min, max);

   thresholdSlider_->blockSignals(true);
   thresholdSlider_->setValues(min ,max);
   thresholdSlider_->blockSignals(false);
   lowerThresholdSpin_->blockSignals(true);
   lowerThresholdSpin_->setValue(lowerDomainSpin_->value() + (upperDomainSpin_->value()-lowerDomainSpin_->value())*min);
   lowerThresholdSpin_->blockSignals(false);
   upperThresholdSpin_->blockSignals(true);
   upperThresholdSpin_->setValue(lowerDomainSpin_->value() + (upperDomainSpin_->value()-lowerDomainSpin_->value())*max);
   upperThresholdSpin_->blockSignals(false);
}

void TransFunc1DKeysEditor::applyThreshold(bool fromSlider) {
    if (!transferFuncIntensity_)
        return;

    float min,max;
    if(fromSlider) {
        min = thresholdSlider_->getMinValue();
        max = thresholdSlider_->getMaxValue();
        lowerThresholdSpin_->blockSignals(true);
        lowerThresholdSpin_->setValue(lowerDomainSpin_->value() + (upperDomainSpin_->value()-lowerDomainSpin_->value())*min);
        lowerThresholdSpin_->blockSignals(false);
        upperThresholdSpin_->blockSignals(true);
        upperThresholdSpin_->setValue(lowerDomainSpin_->value() + (upperDomainSpin_->value()-lowerDomainSpin_->value())*max);
        upperThresholdSpin_->blockSignals(false);
    } else {
        min = (lowerThresholdSpin_->value()-lowerDomainSpin_->value())/(upperDomainSpin_->value()-lowerDomainSpin_->value());
        max = (upperThresholdSpin_->value()-lowerDomainSpin_->value())/(upperDomainSpin_->value()-lowerDomainSpin_->value());
        thresholdSlider_->blockSignals(true);
        thresholdSlider_->setValues(min,max);
        thresholdSlider_->blockSignals(false);
    }

    transCanvas_->setThreshold(min, max);
    transferFuncIntensity_->setThresholds(min, max);
    updateTransferFunction();
}

//----------------------------------------------------------------------------------------------
//      other functions
//----------------------------------------------------------------------------------------------
void TransFunc1DKeysEditor::checkDomainVersusData() {
    bool warnLower = false;
    bool warnUpper = false;
    if (transferFuncIntensity_) {
        float min = domainMinValue_;
        float max = domainMaxValue_;

        tgt::vec2 domain = transferFuncIntensity_->getDomain();
        float avg = (domain.x + domain.y) / 2.0f;

        if(domain.x > min)
             warnLower = true;
        if(domain.y < max)
             warnUpper = true;

        if(min > avg)
            warnLower = true;
        if(max < avg)
            warnUpper = true;
    }

    QPalette lowerPal(lowerDomainSpin_->palette());
    if(warnLower)
        lowerPal.setColor(QPalette::Base, Qt::yellow);
    else
        lowerPal.setColor(QPalette::Base, QApplication::palette().color(QPalette::Base));
    lowerDomainSpin_->setPalette(lowerPal);


    QPalette upperPal(upperDomainSpin_->palette());
    if(warnUpper)
        upperPal.setColor(QPalette::Base, Qt::yellow);
    else
        upperPal.setColor(QPalette::Base, QApplication::palette().color(QPalette::Base));
    upperDomainSpin_->setPalette(upperPal);
}


void TransFunc1DKeysEditor::updateFromProperty() {
    tgtAssert(property_, "No property");

    // check whether new transfer function object has been assigned
    if (property_->get() != transferFuncIntensity_) {
        transferFuncIntensity_ = dynamic_cast<TransFunc1DKeys*>(property_->get());
        // propagate transfer function to mapping canvas and texture painter
        texturePainter_->setTransFunc(transferFuncIntensity_);
        transCanvas_->setTransFunc(transferFuncIntensity_);

        updateDataBounds();
        updateMappingSpin(true);
        updateThresholdFromProperty();

        if(transferFuncIntensity_) {
            gammaSpin_->blockSignals(true);
            gammaSpin_->setValue(transferFuncIntensity_->getGammaValue());
            gammaSpin_->blockSignals(false);

            updateAlphaButton(transferFuncIntensity_->getAlphaMode());
        }

        if (property_->get() && !transferFuncIntensity_) {
            if (isEnabled()) {
                LWARNING("Current transfer function not supported by this editor. Disabling.");
                setEnabled(false);
            }
        }
    }

    // check whether the volume associated with the TransFuncProperty has changed
    const VolumeBase* newHandle = property_->getVolumeHandle();
    volume_ = newHandle;
    if (newHandle) {
        volume_->addObserver(this);
        //get histogram
        if(newHandle->hasDerivedData<VolumeHistogramIntensity>()) {
            if(&(newHandle->getDerivedData<VolumeHistogramIntensity>()->getHistogram(property_->getVolumeChannel())) != histogram_){
                histogram_ = &newHandle->getDerivedData<VolumeHistogramIntensity>()->getHistogram(property_->getVolumeChannel());
                if (histogram_) {
                    domainMinValue_ = histogram_->getMinValue();
                    domainMaxValue_ = histogram_->getMaxValue();
                    updateDataBounds();

                    // propagate new volume to transfuncMappingCanvas
                    transCanvas_->setHistogram(histogram_);
                }
                else {
                    transCanvas_->setHistogram(0);
                }
                checkDomainVersusData();
            }
        }
        else {
            newHandle->getDerivedDataThreaded<VolumeHistogramIntensity>();
            histogram_ = 0;
            transCanvas_->setHistogram(0);
        }
        //get minmax
        if(newHandle->hasDerivedData<VolumeMinMax>()) {
            domainMinValue_ = newHandle->getDerivedData<VolumeMinMax>()->getMinNormalized(property_->getVolumeChannel());
            domainMaxValue_ = newHandle->getDerivedData<VolumeMinMax>()->getMaxNormalized(property_->getVolumeChannel());

            domainMinValue_ = newHandle->getRealWorldMapping().normalizedToRealWorld(domainMinValue_);
            domainMaxValue_ = newHandle->getRealWorldMapping().normalizedToRealWorld(domainMaxValue_);

            updateDataBounds();
        }
        else {
            newHandle->getDerivedDataThreaded<VolumeMinMax>();
            domainMinValue_ = 0.f; domainMaxValue_ = 1.f;
            updateDataBounds();
        }
    }


    tgtAssert(property_->getDomainFittingStrategy() < domainFittingStrategy_->count(), "invalid domain fitting strategy");
    if (property_->getDomainFittingStrategy() < domainFittingStrategy_->count()) {
        domainFittingStrategy_->blockSignals(true);
        domainFittingStrategy_->setCurrentIndex(property_->getDomainFittingStrategy());
        domainFittingStrategy_->blockSignals(false);
    }
    else {
        LERROR("Invalid domain fitting strategy: " << property_->getDomainFittingStrategy());
    }

    if (transferFuncIntensity_) {
        setEnabled(true);

        // update treshold widgets from tf
        updateDataBounds();
        updateMappingSpin(true);
        updateThresholdFromProperty();
        gammaSpin_->blockSignals(true);
        gammaSpin_->setValue(transferFuncIntensity_->getGammaValue());
        gammaSpin_->blockSignals(false);

        updateAlphaButton(transferFuncIntensity_->getAlphaMode());

        // repaint control elements
        repaintAll();
    }
    else {
        setEnabled(false);
    }
}

/*void TransFunc1DKeysEditor::derivedDataThreadFinished(const VolumeBase* source, const VolumeDerivedData* derivedData) {
    if(dynamic_cast<const VolumeHistogramIntensity*>(derivedData)) {
        histogram_ = &(dynamic_cast<const VolumeHistogramIntensity*>(derivedData)->getHistogram(property_->getVolumeChannel()));
        domainMinValue_ = histogram_->getMinValue();
        domainMaxValue_ = histogram_->getMaxValue();
        updateDataBounds();
        transCanvas_->setHistogram(histogram_);
        repaintAll();
    } else
    if(dynamic_cast<const VolumeMinMax*>(derivedData)) {
        domainMinValue_ = source->getDerivedData<VolumeMinMax>()->getMinNormalized(property_->getVolumeChannel());
        domainMaxValue_ = source->getDerivedData<VolumeMinMax>()->getMaxNormalized(property_->getVolumeChannel());
        domainMinValue_ = source->getRealWorldMapping().normalizedToRealWorld(domainMinValue_);
        domainMaxValue_ = source->getRealWorldMapping().normalizedToRealWorld(domainMaxValue_);
        updateDataBounds();
        repaintAll();
    }
}*/

void TransFunc1DKeysEditor::resetEditor() {
    if (property_->get() != transferFuncIntensity_) {
        LDEBUG("The pointers of property and transfer function do not match."
                << "Creating new transfer function object.....");
        transferFuncIntensity_ = new TransFunc1DKeys(1024);//TODO
        property_->set(transferFuncIntensity_);

        // propagate transfer function to mapping canvas and texture painter
        texturePainter_->setTransFunc(transferFuncIntensity_);
        transCanvas_->setTransFunc(transferFuncIntensity_);
    }

    // reset transfer function and thresholds
    resetTransferFunction();

    causeVolumeRenderingRepaint();
}

} // namespace voreen
