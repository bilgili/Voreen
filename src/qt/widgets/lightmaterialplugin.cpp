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

#include "voreen/qt/widgets/lightmaterialplugin.h"

#include <QColorDialog>
#include <QPixmap>
#include <QVBoxLayout>
#include <QApplication>
#include <cmath>

#include "voreen/core/vis/lightmaterial.h"

using tgt::vec2;

namespace voreen {

LightMaterialPlugin::LightMaterialPlugin(QWidget* parent, MessageReceiver* msgReceiver) :
    WidgetPlugin(parent, msgReceiver),
    features_(ALL_FEATURES)
{
    setObjectName(tr("Lighting Parameters"));
    icon_ = QIcon(":/icons/bulb.png");

    // lighting model settings
    addAmbient_ = true;
    addSpecular_ = true;
    applyAttenuation_ = false;

    // light settings
    useOpenGLMaterial_ = false;
    currentLightAmbient_  = 0.5f;
    currentLightDiffuse_  = 0.8f;
    currentLightSpecular_ = 0.6f;

    constantAttenuationRange_ = vec2(0.1f, 5.f);
    linearAttenuationRange_ = vec2(0.f, 1.f);
    quadraticAttenuationRange_ = vec2(0.f, 0.5f);
    currentConstantAttenuation_ = 1.f;
    currentLinearAttenuation_ = 0.f;
    currentQuadraticAttenuation_ = 0.f;
    sliderSteps_ = 50;

    // material settings
    currentMaterialAmbient_  = tgt::Color(1.f, 1.f, 1.f, 1.f);
    currentMaterialSpecular_ = tgt::Color(1.f, 1.f, 1.f, 1.f);
    currentMaterialDiffuse_  = tgt::Color(1.f, 1.f, 1.f, 1.f);
    currentMaterialEmission_ = tgt::Color(0.f, 0.f, 0.f, 1.f);
    currentShininess_ = 60;
}

void LightMaterialPlugin::initGLState() {
    postMessage(new ColorMsg( LightMaterial::setLightAmbient_, tgt::Color(currentLightAmbient_)));
    postMessage(new ColorMsg( LightMaterial::setLightDiffuse_, tgt::Color(currentLightDiffuse_)));
    postMessage(new ColorMsg( LightMaterial::setLightSpecular_, tgt::Color(currentLightSpecular_)));
    postMessage(new Vec3Msg( LightMaterial::setLightAttenuation_,
        tgt::vec3(currentConstantAttenuation_, currentLinearAttenuation_, currentQuadraticAttenuation_)));

    postMessage(new ColorMsg(LightMaterial::setMaterialAmbient_, currentMaterialAmbient_));
    postMessage(new ColorMsg(LightMaterial::setMaterialDiffuse_, currentMaterialDiffuse_));
    postMessage(new ColorMsg(LightMaterial::setMaterialSpecular_, currentMaterialSpecular_));
    postMessage(new ColorMsg(LightMaterial::setMaterialEmission_, currentMaterialEmission_));
    postMessage(new FloatMsg(LightMaterial::setMaterialShininess_, currentShininess_));

    // do this last, as sending this message seems to change GL focus (which is a bug of course)
    postMessage(new BoolMsg(LightMaterial::switchUseOpenGLMaterial_, useOpenGLMaterial_));
    postMessage(new BoolMsg(LightMaterial::switchPhongApplyAttenuation_, applyAttenuation_ ));
    postMessage(new BoolMsg(LightMaterial::switchPhongAddAmbient_, addAmbient_));
    postMessage(new BoolMsg(LightMaterial::switchPhongAddSpecular_, addSpecular_));
}

void LightMaterialPlugin::enableFeatures(int features) {
     features_ |= features;
}

void LightMaterialPlugin::disableFeatures(int features) {
     features_ &= ~features;
}

bool LightMaterialPlugin::isFeatureEnabled(LightMaterialPlugin::Features feature) {
    return (features_ & feature);
}

void LightMaterialPlugin::createWidgets() {
    resize(300,300);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    QVBoxLayout* vboxLayout;
    QGridLayout* gridLayout;

    //
    // lighting model settings
    //
    groupLightingModel_ = new QGroupBox(tr("Phong Lighting Model"));
    vboxLayout = new QVBoxLayout();

    checkAddAmbient_ = new QCheckBox(tr("Add ambient reflection"), groupLightingModel_);
    checkAddAmbient_->setChecked(addAmbient_);
    vboxLayout->addWidget(checkAddAmbient_);

    checkAddSpecular_ = new QCheckBox(tr("Add specular reflection"), groupLightingModel_);
    checkAddSpecular_->setChecked(addSpecular_);
    vboxLayout->addWidget(checkAddSpecular_);

    checkApplyAttenuation_ = new QCheckBox(tr("Apply attenuation"), groupLightingModel_);
    checkApplyAttenuation_->setChecked(applyAttenuation_);
    vboxLayout->addWidget(checkApplyAttenuation_);

    groupLightingModel_->setLayout(vboxLayout);
    if (!isFeatureEnabled(LIGHTING_MODEL))
        groupLightingModel_->setVisible( false );
    mainLayout->addWidget(groupLightingModel_);


    //
    // light colors
    //

    QGroupBox* groupLightColors_ = new QGroupBox(tr("Light Source Colors"));

    vboxLayout = new QVBoxLayout();

    gridLayout = new QGridLayout();
    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnStretch(2, 0);

    QFrame* spacer = new QFrame();
    spacer->setFrameStyle(QFrame::NoFrame);
    gridLayout->addWidget(spacer, 0, 1, 3, 1);

    laShowLightAmbient_ = new QLabel("");
    laShowLightAmbient_->setMinimumWidth(40);
    laShowLightAmbient_->setAutoFillBackground(true);
    laShowLightAmbient_->setEnabled(addAmbient_);
    laLightAmbient_ = new QLabel(tr("Ambient"));
    laLightAmbient_->setEnabled(addAmbient_);
    sliderLightAmbient_ = new QSlider(Qt::Horizontal, this);
    sliderLightAmbient_->setRange(0,100);
    sliderLightAmbient_->setSliderPosition(int(currentLightAmbient_*100.f));
    sliderLightAmbient_->setEnabled(addAmbient_);
    showLightAmbientColor();
    //gridLayout->addWidget(getLightAmbient_, 0, 0);
    gridLayout->addWidget(laShowLightAmbient_, 0, 2);
    if (!isFeatureEnabled(LIGHT_AMBIENT)) {
        //getLightAmbient_->setVisible( false );
        laLightAmbient_->setVisible(false);
        sliderLightAmbient_->setVisible(false);
        laShowLightAmbient_->setVisible( false );
    }

    gridLayout->addWidget(laLightAmbient_, 0, 0);
    gridLayout->addWidget(sliderLightAmbient_, 0, 1);

    laShowLightDiffuse_ = new QLabel("");
    laShowLightDiffuse_->setAutoFillBackground(true);
    laShowLightDiffuse_->setMinimumWidth(40);
    showLightDiffuseColor();
    laLightDiffuse_ = new QLabel(tr("Diffuse"));
    sliderLightDiffuse_ = new QSlider(Qt::Horizontal, this);
    sliderLightDiffuse_->setRange(0,100);
    sliderLightDiffuse_->setSliderPosition(int(currentLightDiffuse_*100.f));
    gridLayout->addWidget(laLightDiffuse_, 1, 0);
    gridLayout->addWidget(sliderLightDiffuse_, 1, 1);
    gridLayout->addWidget(laShowLightDiffuse_, 1, 2);
    if (!isFeatureEnabled(LIGHT_DIFFUSE)) {
        //getLightDiffuse_->setVisible( false );
        laLightDiffuse_->setVisible( false );
        sliderLightDiffuse_->setVisible( false );
        laShowLightDiffuse_->setVisible( false );
    }


    laShowLightSpecular_ = new QLabel("");
    laShowLightSpecular_->setAutoFillBackground(true);
    laShowLightSpecular_->setMinimumWidth(40);
    laShowLightSpecular_->setEnabled(addSpecular_);
    showLightSpecularColor();
    laLightSpecular_ = new QLabel(tr("Specular"));
    laLightSpecular_->setEnabled(addSpecular_);
    sliderLightSpecular_ = new QSlider(Qt::Horizontal, this);
    sliderLightSpecular_->setRange(0,100);
    sliderLightSpecular_->setSliderPosition(int(currentLightSpecular_*100.f));
    sliderLightSpecular_->setEnabled(addSpecular_);
    gridLayout->addWidget(laLightSpecular_, 2, 0);
    gridLayout->addWidget(sliderLightSpecular_, 2, 1);
    gridLayout->addWidget(laShowLightSpecular_, 2, 2);
    if (!isFeatureEnabled(LIGHT_SPECULAR)) {
        //getLightSpecular_->setVisible( false );
        laLightSpecular_->setVisible( false );
        sliderLightSpecular_->setValue( false );
        laShowLightSpecular_->setVisible( false );
    }

    vboxLayout->addLayout(gridLayout);

    groupLightColors_->setLayout(vboxLayout);
    if (!isFeatureEnabled(LIGHT))
        groupLightColors_->setVisible( false );

    mainLayout->addWidget(groupLightColors_);

    //
    // material settings
    //

    groupMaterialParams_ = new QGroupBox(tr("Material Parameters"));
    gridLayout = new QGridLayout();
    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnStretch(2, 0);
    vboxLayout = new QVBoxLayout();

    /*spacer = new QFrame();
    spacer->setFrameStyle(QFrame::NoFrame);
    gridLayout->addWidget(spacer, 0, 1, 4, 1); */

    checkUseOpenGLMaterial_ =
        new QCheckBox(tr("Use ambient / diffuse material color"));
    checkUseOpenGLMaterial_->setChecked(useOpenGLMaterial_);
    //vboxLayout->addWidget(checkUseOpenGLMaterial_);
    gridLayout->addWidget(checkUseOpenGLMaterial_, 0, 0, 1, 3);
    if (!isFeatureEnabled(MATERIAL_USE_OPENGL_MATERIAL))
        checkUseOpenGLMaterial_->setVisible(false);

    getMaterialAmbient_  = new QPushButton(tr("Ambient color ..."), this);
    getMaterialAmbient_->adjustSize();
    getMaterialAmbient_->setMinimumWidth(getMaterialAmbient_->width() + 20);
    getMaterialAmbient_->setEnabled(addAmbient_);
    laShowMaterialAmbient_ = new QLabel("");
    laShowMaterialAmbient_->setMinimumWidth(40);
    laShowMaterialAmbient_->setAutoFillBackground(true);
    laShowMaterialAmbient_->setEnabled(addAmbient_);
    showMaterialAmbientColor();
    gridLayout->addWidget(getMaterialAmbient_, 1, 0);
    gridLayout->addWidget(laShowMaterialAmbient_, 1, 2);
    if ( !useOpenGLMaterial_ || !isFeatureEnabled(MATERIAL_AMBIENT) ) {
        getMaterialAmbient_->setVisible( false );
        laShowMaterialAmbient_->setVisible( false );
    }

    getMaterialDiffuse_  = new QPushButton(tr("Diffuse color ..."), this);
    getMaterialDiffuse_->adjustSize();
    getMaterialDiffuse_->setMinimumWidth(getMaterialDiffuse_->width() + 20);
    laShowMaterialDiffuse_ = new QLabel("");
    laShowMaterialDiffuse_->setMinimumWidth(40);
    laShowMaterialDiffuse_->setAutoFillBackground(true);
    showMaterialDiffuseColor();
    gridLayout->addWidget(getMaterialDiffuse_, 2, 0);
    gridLayout->addWidget(laShowMaterialDiffuse_, 2, 2);
    if ( !useOpenGLMaterial_ || !isFeatureEnabled(MATERIAL_DIFFUSE) ) {
        getMaterialDiffuse_->setVisible( false );
        laShowMaterialDiffuse_->setVisible( false );
    }

    getMaterialSpecular_ = new QPushButton(tr("Specular color ..."), this);
    getMaterialSpecular_->adjustSize();
    getMaterialSpecular_->setMinimumWidth(getMaterialSpecular_->width() + 20);
    getMaterialSpecular_->setEnabled(addSpecular_);
    laShowMaterialSpecular_ = new QLabel("");
    laShowMaterialSpecular_->setMinimumWidth(40);
    laShowMaterialSpecular_->setAutoFillBackground(true);
    laShowMaterialSpecular_->setEnabled(addSpecular_);
    showMaterialSpecularColor();
    gridLayout->addWidget(getMaterialSpecular_, 3, 0);
    gridLayout->addWidget(laShowMaterialSpecular_, 3, 2);
    //if ( !isFeatureEnabled(MATERIAL_SPECULAR) ) {
        getMaterialSpecular_->setVisible( false );
        laShowMaterialSpecular_->setVisible( false );
    //}

    getMaterialEmission_ = new QPushButton(tr("Emission color ..."), this);
    getMaterialEmission_->adjustSize();
    getMaterialEmission_->setMinimumWidth(getMaterialEmission_->width() + 20);
    laShowMaterialEmission_ = new QLabel("");
    laShowMaterialEmission_->setMinimumWidth(40);
    laShowMaterialEmission_->setAutoFillBackground(true);
    showMaterialEmissionColor();
    gridLayout->addWidget(getMaterialEmission_, 4, 0);
    gridLayout->addWidget(laShowMaterialEmission_, 4, 2);
    if (!isFeatureEnabled(MATERIAL_EMISSION)) {
        getMaterialEmission_->setVisible( false );
        laShowMaterialEmission_->setVisible( false );
    }

    vboxLayout->addLayout(gridLayout);

    gridLayout = new QGridLayout();
    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnStretch(2, 0);

    sliderShininess_ = new QSlider();
    spinShininess_ = new QDoubleSpinBox();
    laShininess_ = new QLabel(tr("Shininess"));
    sliderShininess_->setEnabled(addSpecular_);
    spinShininess_->setEnabled(addSpecular_);
    laShininess_->setEnabled(addSpecular_);
    gridLayout->addWidget(laShininess_, 5, 0);
    gridLayout->addWidget(sliderShininess_, 5, 1);
    gridLayout->addWidget(spinShininess_, 5, 2);
    if (!isFeatureEnabled(MATERIAL_SPECULAR)) {
        laShininess_->setVisible( false );
        sliderShininess_->setVisible( false );
        spinShininess_->setVisible( false );
    }

    sliderShininess_->setRange(1, 128);
    sliderShininess_->setValue(int(currentShininess_+0.5f));
    sliderShininess_->setOrientation(Qt::Horizontal);
    spinShininess_->setDecimals(1);
    spinShininess_->setRange(1.0, 128.0);
    spinShininess_->setValue(currentShininess_);

    vboxLayout->addLayout(gridLayout);

    groupMaterialParams_->setLayout(vboxLayout);
    if (!isFeatureEnabled(MATERIAL))
        groupMaterialParams_->setVisible( false );

    mainLayout->addWidget(groupMaterialParams_);

    setLayout(mainLayout);

    //
    // attenuation factors
    //

    groupAttenuation_ = new QGroupBox(tr("Attenuation Factors"));
    gridLayout = new QGridLayout();

    gridLayout->addWidget(new QLabel(tr("Constant")), 0, 0);
    sliderConstantAttenuation_ = new QSlider(Qt::Horizontal, 0);
    spinConstantAttenuation_ = new QDoubleSpinBox();
    gridLayout->addWidget(sliderConstantAttenuation_, 0, 1);
    gridLayout->addWidget(spinConstantAttenuation_, 0, 2);

    gridLayout->addWidget(new QLabel(tr("Linear")), 1, 0);
    sliderLinearAttenuation_ = new QSlider(Qt::Horizontal, 0);
    spinLinearAttenuation_ = new QDoubleSpinBox();
    gridLayout->addWidget(sliderLinearAttenuation_, 1, 1);
    gridLayout->addWidget(spinLinearAttenuation_, 1, 2);

    gridLayout->addWidget(new QLabel(tr("Quadratic")), 2, 0);
    sliderQuadraticAttenuation_ = new QSlider(Qt::Horizontal, 0);
    spinQuadraticAttenuation_ = new QDoubleSpinBox();
    gridLayout->addWidget(sliderQuadraticAttenuation_, 2, 1);
    gridLayout->addWidget(spinQuadraticAttenuation_, 2, 2);

    sliderConstantAttenuation_->setRange(0, sliderSteps_-1);
    spinConstantAttenuation_->setDecimals(2);
    spinConstantAttenuation_->setRange(constantAttenuationRange_.x, constantAttenuationRange_.y);
    spinConstantAttenuation_->setValue(currentConstantAttenuation_);
    float step = std::floor( (constantAttenuationRange_.y - constantAttenuationRange_.x) /
        (sliderSteps_-1)*100.f + 0.5f) / 100.f;
    spinConstantAttenuation_->setSingleStep( step );
    updateConstantAttenuationSpin(currentConstantAttenuation_, false);

    sliderLinearAttenuation_->setRange(0, sliderSteps_);
    spinLinearAttenuation_->setDecimals(3);
    spinLinearAttenuation_->setRange(linearAttenuationRange_.x, linearAttenuationRange_.y);
    spinLinearAttenuation_->setValue(currentLinearAttenuation_);
    step = std::floor( (linearAttenuationRange_.y - linearAttenuationRange_.x) /
        (sliderSteps_)*100.f + 0.5f) / 100.f;
    spinLinearAttenuation_->setSingleStep( step );
    updateLinearAttenuationSpin(currentLinearAttenuation_, false);

    sliderQuadraticAttenuation_->setRange(0, sliderSteps_);
    spinQuadraticAttenuation_->setDecimals(3);
    spinQuadraticAttenuation_->setRange(quadraticAttenuationRange_.x, quadraticAttenuationRange_.y);
    spinQuadraticAttenuation_->setValue(currentQuadraticAttenuation_);
    step = std::floor( (quadraticAttenuationRange_.y - quadraticAttenuationRange_.x) /
        (sliderSteps_-1)*100.f + 0.5f) / 100.f;
    spinQuadraticAttenuation_->setSingleStep( step );
    updateQuadraticAttenuationSpin(currentQuadraticAttenuation_, false);

    groupAttenuation_->setLayout(gridLayout);
    if ( !isFeatureEnabled(ATTENUATION) ) {
        groupAttenuation_->setVisible( false );
    }

    groupAttenuation_->setEnabled(applyAttenuation_);
    mainLayout->addWidget(groupAttenuation_);

    mainLayout->addStretch();

    // synchronize some sizes for visual appeal
    spinQuadraticAttenuation_->adjustSize();
    int minWidth = spinQuadraticAttenuation_->width();
    laShowLightAmbient_->setMinimumWidth(minWidth);
    laShowLightDiffuse_->setMinimumWidth(minWidth);
    laShowLightSpecular_->setMinimumWidth(minWidth);
    laShowMaterialAmbient_->setMinimumWidth(minWidth);
    laShowMaterialDiffuse_->setMinimumWidth(minWidth);
    laShowMaterialSpecular_->setMinimumWidth(minWidth);
    laShowMaterialEmission_->setMinimumWidth(minWidth);

    laShininess_->adjustSize();
    minWidth = laShininess_->width();
    laLightAmbient_->setMinimumWidth(minWidth);
    laLightDiffuse_->setMinimumWidth(minWidth);
    laLightSpecular_->setMinimumWidth(minWidth);
}

void LightMaterialPlugin::createConnections() {
    connect(checkAddAmbient_, SIGNAL(toggled(bool)),     this,
        SLOT(setAddAmbient(bool)) );
    connect(checkAddSpecular_, SIGNAL(toggled(bool)),     this,
        SLOT(setAddSpecular(bool)) );
    connect(checkApplyAttenuation_, SIGNAL(toggled(bool)),     this,
        SLOT(setApplyAttenuation(bool)) );

    connect(sliderConstantAttenuation_,  SIGNAL(valueChanged(int)), this,
        SLOT(updateConstantAttenuationSlider()));
    connect(sliderLinearAttenuation_,    SIGNAL(valueChanged(int)), this,
        SLOT(updateLinearAttenuationSlider()));
    connect(sliderQuadraticAttenuation_, SIGNAL(valueChanged(int)), this,
        SLOT(updateQuadraticAttenuationSlider()));
    connect(sliderShininess_, SIGNAL(valueChanged(int)), this,
        SLOT(updateShininessSlider(int)));

    connect(spinConstantAttenuation_,  SIGNAL(valueChanged(double)), this,
        SLOT(updateConstantAttenuationSpin(double)));
    connect(spinLinearAttenuation_,    SIGNAL(valueChanged(double)), this,
        SLOT(updateLinearAttenuationSpin(double)));
    connect(spinQuadraticAttenuation_, SIGNAL(valueChanged(double)), this,
        SLOT(updateQuadraticAttenuationSpin(double)));
    connect(spinShininess_, SIGNAL(valueChanged(double)), this,
        SLOT(updateShininessSpin(double)));

    connect(sliderConstantAttenuation_,  SIGNAL(sliderPressed()), this,
        SLOT(startTracking()));
    connect(sliderLinearAttenuation_,    SIGNAL(sliderPressed()), this,
        SLOT(startTracking()));
    connect(sliderQuadraticAttenuation_, SIGNAL(sliderPressed()), this,
        SLOT(startTracking()));
    connect(sliderShininess_,       SIGNAL(sliderPressed()), this,
        SLOT(startTracking()));
    connect(sliderLightAmbient_,    SIGNAL(sliderPressed()), this,
        SLOT(startTracking()));
    connect(sliderLightDiffuse_,    SIGNAL(sliderPressed()), this,
        SLOT(startTracking()));
    connect(sliderLightSpecular_,   SIGNAL(sliderPressed()), this,
        SLOT(startTracking()));

    connect(sliderConstantAttenuation_,  SIGNAL(sliderReleased()), this,
        SLOT(stopTracking()));
    connect(sliderLinearAttenuation_,    SIGNAL(sliderReleased()), this,
        SLOT(stopTracking()));
    connect(sliderQuadraticAttenuation_, SIGNAL(sliderReleased()), this,
        SLOT(stopTracking()));
    connect(sliderLightAmbient_,        SIGNAL(sliderReleased()), this,
        SLOT(stopTracking()));
    connect(sliderLightDiffuse_,        SIGNAL(sliderReleased()), this,
        SLOT(stopTracking()));
    connect(sliderLightSpecular_,       SIGNAL(sliderReleased()), this,
        SLOT(stopTracking()));
    connect(sliderShininess_,            SIGNAL(sliderReleased()), this,
        SLOT(stopTracking()));

    connect(checkUseOpenGLMaterial_,     SIGNAL(toggled(bool)), this,
        SLOT(setUseOGLMaterial(bool)));

    connect(sliderLightSpecular_,            SIGNAL(valueChanged(int)), this,
        SLOT(updateLightSpecularColor(int)));
    connect(sliderLightAmbient_,             SIGNAL(valueChanged(int)), this,
        SLOT(updateLightAmbientColor(int)));
    connect(sliderLightDiffuse_,             SIGNAL(valueChanged(int)), this,
        SLOT(updateLightDiffuseColor(int)));

    connect(getMaterialSpecular_,         SIGNAL(clicked()), this,
        SLOT(updateMaterialSpecularColor()));
    connect(getMaterialAmbient_,          SIGNAL(clicked()), this,
        SLOT(updateMaterialAmbientColor()));
    connect(getMaterialDiffuse_,          SIGNAL(clicked()), this,
        SLOT(updateMaterialDiffuseColor()));
    connect(getMaterialEmission_,         SIGNAL(clicked()), this,
        SLOT(updateMaterialEmissionColor()));
}

void LightMaterialPlugin::setAddAmbient(bool checked) {
    addAmbient_ = checked;

    laLightAmbient_->setEnabled(addAmbient_);
    sliderLightAmbient_->setEnabled(addAmbient_);
    laShowLightAmbient_->setEnabled(addAmbient_);
    getMaterialAmbient_->setEnabled(addAmbient_);
    laShowMaterialAmbient_->setEnabled(addAmbient_);

    checkAddAmbient_->blockSignals(true);
    checkAddAmbient_->setChecked(addAmbient_);
    checkAddAmbient_->blockSignals(false);

    postMessage(new BoolMsg(LightMaterial::switchPhongAddAmbient_, addAmbient_));
    repaintCanvases();
}

void LightMaterialPlugin::setAddSpecular(bool checked) {
    addSpecular_ = checked;

    laShowLightSpecular_->setEnabled(addSpecular_);
    laLightSpecular_->setEnabled(addSpecular_);
    sliderLightSpecular_->setEnabled(addSpecular_);
    getMaterialSpecular_->setEnabled(addSpecular_);
    laShowMaterialSpecular_->setEnabled(addSpecular_);
    laShininess_->setEnabled(addSpecular_);
    sliderShininess_->setEnabled(addSpecular_);
    spinShininess_->setEnabled(addSpecular_);

    checkAddSpecular_->blockSignals(true);
    checkAddSpecular_->setChecked(addSpecular_);
    checkAddSpecular_->blockSignals(false);

    postMessage(new BoolMsg(LightMaterial::switchPhongAddSpecular_, addSpecular_));
    repaintCanvases();
}

void LightMaterialPlugin::setApplyAttenuation(bool checked) {
    applyAttenuation_ = checked;

    groupAttenuation_->setEnabled(applyAttenuation_);

    checkApplyAttenuation_->blockSignals(true);
    checkApplyAttenuation_->setChecked(applyAttenuation_);
    checkApplyAttenuation_->blockSignals(false);

    postMessage(new BoolMsg(LightMaterial::switchPhongApplyAttenuation_, applyAttenuation_));
    repaintCanvases();
}

void LightMaterialPlugin::setUseOGLMaterial(bool b) {
    bool ambientVisible = b && isFeatureEnabled(MATERIAL_AMBIENT);
    bool diffuseVisible = b && isFeatureEnabled(MATERIAL_DIFFUSE);

    getMaterialAmbient_->setVisible(ambientVisible);
    getMaterialDiffuse_->setVisible(diffuseVisible);
    laShowMaterialAmbient_->setVisible(ambientVisible);
    laShowMaterialDiffuse_->setVisible(diffuseVisible);

    useOpenGLMaterial_ = b;

    checkUseOpenGLMaterial_->blockSignals(true);
    checkUseOpenGLMaterial_->setChecked(useOpenGLMaterial_);
    checkUseOpenGLMaterial_->blockSignals(false);

    postMessage(new BoolMsg(LightMaterial::switchUseOpenGLMaterial_, b));
    repaintCanvases();
}

void LightMaterialPlugin::updateLightSpecularColor(int value) {
    currentLightSpecular_ = value / 100.f;
    showLightSpecularColor();
    QApplication::processEvents();

    postMessage(new ColorMsg(LightMaterial::setLightSpecular_,
        tgt::Color(currentLightSpecular_, currentLightSpecular_, currentLightSpecular_, 1.f))  );

    repaintCanvases();
}

void LightMaterialPlugin::updateLightAmbientColor(int value) {
    currentLightAmbient_ = value / 100.f;
    showLightAmbientColor();
    QApplication::processEvents();

    postMessage(new ColorMsg(LightMaterial::setLightAmbient_,
        tgt::Color(currentLightAmbient_, currentLightAmbient_, currentLightAmbient_, 1.f))  );

    repaintCanvases();
}

void LightMaterialPlugin::updateLightDiffuseColor(int value) {
    currentLightDiffuse_ = value / 100.f;
    showLightDiffuseColor();
    QApplication::processEvents();

    postMessage(new ColorMsg(LightMaterial::setLightDiffuse_,
        tgt::Color(currentLightDiffuse_, currentLightDiffuse_, currentLightDiffuse_, 1.f))  );

    repaintCanvases();
}

void LightMaterialPlugin::updateMaterialSpecularColor() {
    QColor oldDiff =
        QColor::fromRgbF(currentMaterialSpecular_.r, currentMaterialSpecular_.g,
        currentMaterialSpecular_.b, currentMaterialSpecular_.a);
    QColor newDiff = QColorDialog::getColor(oldDiff, 0);

    if (newDiff.isValid()){
        currentMaterialSpecular_.a = static_cast<float>(newDiff.alphaF());
        currentMaterialSpecular_.r = static_cast<float>(newDiff.redF());
        currentMaterialSpecular_.g = static_cast<float>(newDiff.greenF());
        currentMaterialSpecular_.b = static_cast<float>(newDiff.blueF());

        postMessage(new ColorMsg(LightMaterial::setMaterialSpecular_, currentMaterialSpecular_));

        repaintCanvases();
        showMaterialSpecularColor();
    }
}

void LightMaterialPlugin::updateMaterialAmbientColor() {
    QColor oldDiff =
        QColor::fromRgbF(currentMaterialAmbient_.r, currentMaterialAmbient_.g,
        currentMaterialAmbient_.b, currentMaterialAmbient_.a);
    QColor newDiff = QColorDialog::getColor(oldDiff, 0);

    if (newDiff.isValid()){
        currentMaterialAmbient_.a = static_cast<float>(newDiff.alphaF());
        currentMaterialAmbient_.r = static_cast<float>(newDiff.redF());
        currentMaterialAmbient_.g = static_cast<float>(newDiff.greenF());
        currentMaterialAmbient_.b = static_cast<float>(newDiff.blueF());
        showMaterialAmbientColor();

        postMessage(new ColorMsg(LightMaterial::setMaterialAmbient_, currentMaterialAmbient_));

        repaintCanvases();
    }
}

void LightMaterialPlugin::updateMaterialDiffuseColor() {
    QColor oldDiff =
        QColor::fromRgbF(currentMaterialDiffuse_.r, currentMaterialDiffuse_.g,
        currentMaterialDiffuse_.b, currentMaterialDiffuse_.a);
    QColor newDiff = QColorDialog::getColor(oldDiff, 0);

    if (newDiff.isValid()){
        currentMaterialDiffuse_.a = static_cast<float>(newDiff.alphaF());
        currentMaterialDiffuse_.r = static_cast<float>(newDiff.redF());
        currentMaterialDiffuse_.g = static_cast<float>(newDiff.greenF());
        currentMaterialDiffuse_.b = static_cast<float>(newDiff.blueF());
        showMaterialDiffuseColor();

        postMessage(new ColorMsg(LightMaterial::setMaterialDiffuse_, currentMaterialDiffuse_));

        repaintCanvases();
    }
}

void LightMaterialPlugin::updateMaterialEmissionColor() {
    QColor oldDiff =
        QColor::fromRgbF(currentMaterialEmission_.r, currentMaterialEmission_.g,
        currentMaterialEmission_.b, currentMaterialEmission_.a);
    QColor newDiff = QColorDialog::getColor(oldDiff, 0);

    if (newDiff.isValid()){
        currentMaterialEmission_.a = static_cast<float>(newDiff.alphaF());
        currentMaterialEmission_.r = static_cast<float>(newDiff.redF());
        currentMaterialEmission_.g = static_cast<float>(newDiff.greenF());
        currentMaterialEmission_.b = static_cast<float>(newDiff.blueF());
        showMaterialEmissionColor();

        postMessage(new ColorMsg(LightMaterial::setMaterialEmission_, currentMaterialEmission_));
        repaintCanvases();
    }
}


void LightMaterialPlugin::showLightAmbientColor() {
    QPalette palette = laShowLightAmbient_->palette();
    palette.setColor(QPalette::Window, QColor(
        static_cast<int>(currentLightAmbient_*255),
        static_cast<int>(currentLightAmbient_*255),
        static_cast<int>(currentLightAmbient_*255),
        255                                   )
    );
    laShowLightAmbient_->setPalette(palette);
    laShowLightAmbient_->setBackgroundRole(palette.Window);
}

void LightMaterialPlugin::showLightDiffuseColor() {
    QPalette palette = laShowLightDiffuse_->palette();
    palette.setColor(QPalette::Window, QColor(
        static_cast<int>(currentLightDiffuse_*255),
        static_cast<int>(currentLightDiffuse_*255),
        static_cast<int>(currentLightDiffuse_*255),
        255                                   )
    );
    laShowLightDiffuse_->setPalette(palette);
    laShowLightDiffuse_->setBackgroundRole(palette.Window);
}

void LightMaterialPlugin::showLightSpecularColor() {
    QPalette palette = laShowLightSpecular_->palette();
    palette.setColor(QPalette::Window, QColor(
        static_cast<int>(currentLightSpecular_*255),
        static_cast<int>(currentLightSpecular_*255),
        static_cast<int>(currentLightSpecular_*255),
        255                                   )
    );
    laShowLightSpecular_->setPalette(palette);
    laShowLightSpecular_->setBackgroundRole(palette.Window);
}

void LightMaterialPlugin::showMaterialAmbientColor() {
    QPalette palette = laShowMaterialAmbient_->palette();
    palette.setColor(QPalette::Window, QColor(
        static_cast<int>(currentMaterialAmbient_.r*255),
        static_cast<int>(currentMaterialAmbient_.g*255),
        static_cast<int>(currentMaterialAmbient_.b*255),
        255                                   )
    );
    laShowMaterialAmbient_->setPalette(palette);
    laShowMaterialAmbient_->setBackgroundRole(palette.Window);
}

void LightMaterialPlugin::showMaterialDiffuseColor() {
    QPalette palette = laShowMaterialDiffuse_->palette();
    palette.setColor(QPalette::Window, QColor(
        static_cast<int>(currentMaterialDiffuse_.r*255),
        static_cast<int>(currentMaterialDiffuse_.g*255),
        static_cast<int>(currentMaterialDiffuse_.b*255),
        255                                   )
    );
    laShowMaterialDiffuse_->setPalette(palette);
    laShowMaterialDiffuse_->setBackgroundRole(palette.Window);
}

void LightMaterialPlugin::showMaterialSpecularColor() {
    QPalette palette = laShowMaterialSpecular_->palette();
    palette.setColor(QPalette::Window, QColor(
        static_cast<int>(currentMaterialSpecular_.r*255),
        static_cast<int>(currentMaterialSpecular_.g*255),
        static_cast<int>(currentMaterialSpecular_.b*255),
        255                                   )
    );
    laShowMaterialSpecular_->setPalette(palette);
    laShowMaterialSpecular_->setBackgroundRole(palette.Window);
}

void LightMaterialPlugin::showMaterialEmissionColor() {
    QPalette palette = laShowMaterialEmission_->palette();
    palette.setColor(QPalette::Window, QColor(
        static_cast<int>(currentMaterialEmission_.r*255),
        static_cast<int>(currentMaterialEmission_.g*255),
        static_cast<int>(currentMaterialEmission_.b*255),
        255                                   )
    );
    laShowMaterialEmission_->setPalette(palette);
    laShowMaterialEmission_->setBackgroundRole(palette.Window);
}

void LightMaterialPlugin::updateConstantAttenuationSlider() {
    float att =
        constantAttenuationRange_.x + spinConstantAttenuation_->singleStep()*sliderConstantAttenuation_->value();
    currentConstantAttenuation_ = att;

    spinConstantAttenuation_->blockSignals(true);
    spinConstantAttenuation_->setValue(att);
    spinConstantAttenuation_->blockSignals(false);

    postMessage(new Vec3Msg(LightMaterial::setLightAttenuation_,
        tgt::vec3(currentConstantAttenuation_, currentLinearAttenuation_, currentQuadraticAttenuation_)));
    repaintCanvases();
}

void LightMaterialPlugin::updateLinearAttenuationSlider() {
    float att =
        linearAttenuationRange_.x + spinLinearAttenuation_->singleStep()*sliderLinearAttenuation_->value();
    currentLinearAttenuation_ = att;

    spinLinearAttenuation_->blockSignals(true);
    spinLinearAttenuation_->setValue(att);
    spinLinearAttenuation_->blockSignals(false);

    postMessage(new Vec3Msg(LightMaterial::setLightAttenuation_,
        tgt::vec3(currentConstantAttenuation_, currentLinearAttenuation_, currentQuadraticAttenuation_)));
    repaintCanvases();
}

void LightMaterialPlugin::updateQuadraticAttenuationSlider() {
    float att =
        quadraticAttenuationRange_.x + spinQuadraticAttenuation_->singleStep()*sliderQuadraticAttenuation_->value();
    currentQuadraticAttenuation_ = att;

    spinQuadraticAttenuation_->blockSignals(true);
    spinQuadraticAttenuation_->setValue(att);
    spinQuadraticAttenuation_->blockSignals(false);

    postMessage(new Vec3Msg(LightMaterial::setLightAttenuation_,
        tgt::vec3(currentConstantAttenuation_, currentLinearAttenuation_, currentQuadraticAttenuation_)));
    repaintCanvases();
}

void LightMaterialPlugin::updateConstantAttenuationSpin(double value, bool updateGLState) {
    currentConstantAttenuation_ = static_cast<float>(value);

    int position = static_cast<int>( std::floor( (value - constantAttenuationRange_.x)
                                    / (constantAttenuationRange_.y - constantAttenuationRange_.x)
                                    * sliderConstantAttenuation_->maximum() + 0.5f ) );

    sliderConstantAttenuation_->blockSignals(true);
    sliderConstantAttenuation_->setSliderPosition(position);
    sliderConstantAttenuation_->blockSignals(false);

    if (updateGLState) {
        postMessage(new Vec3Msg(LightMaterial::setLightAttenuation_,
            tgt::vec3(currentConstantAttenuation_, currentLinearAttenuation_, currentQuadraticAttenuation_)));
        repaintCanvases();
    }
}

void LightMaterialPlugin::updateLinearAttenuationSpin(double value, bool updateGLState) {
    currentLinearAttenuation_ = static_cast<float>(value);

    int position = static_cast<int>( std::floor( (value - linearAttenuationRange_.x)
                                    / (linearAttenuationRange_.y - linearAttenuationRange_.x)
                                    * sliderLinearAttenuation_->maximum() + 0.5f ) );

    sliderLinearAttenuation_->blockSignals(true);
    sliderLinearAttenuation_->setSliderPosition(position);
    sliderLinearAttenuation_->blockSignals(false);

    if (updateGLState) {
        postMessage(new Vec3Msg(LightMaterial::setLightAttenuation_,
            tgt::vec3(currentConstantAttenuation_, currentLinearAttenuation_, currentQuadraticAttenuation_)));
        repaintCanvases();
    }
}

void LightMaterialPlugin::updateQuadraticAttenuationSpin(double value, bool updateGLState) {
    currentQuadraticAttenuation_ = static_cast<float>(value);

    int position = static_cast<int>( std::floor( (value - quadraticAttenuationRange_.x)
                                    / (quadraticAttenuationRange_.y - quadraticAttenuationRange_.x)
                                    * sliderQuadraticAttenuation_->maximum() + 0.5f ) );

    sliderQuadraticAttenuation_->blockSignals(true);
    sliderQuadraticAttenuation_->setSliderPosition(position);
    sliderQuadraticAttenuation_->blockSignals(false);

    if (updateGLState) {
        postMessage(new Vec3Msg(LightMaterial::setLightAttenuation_,
            tgt::vec3(currentConstantAttenuation_, currentLinearAttenuation_, currentQuadraticAttenuation_)));
        repaintCanvases();
    }
}

void LightMaterialPlugin::updateShininessSlider(int value) {
    currentShininess_ = static_cast<float>(value);

    spinShininess_->blockSignals(true);
    spinShininess_->setValue(currentShininess_);
    spinShininess_->blockSignals(false);

    postMessage(new FloatMsg(LightMaterial::setMaterialShininess_, currentShininess_));

    repaintCanvases();
}

void LightMaterialPlugin::updateShininessSpin(double value) {
    currentShininess_ = static_cast<float>(value);

    sliderShininess_->blockSignals(true);
    sliderShininess_->setSliderPosition(int(value+0.5f));
    sliderShininess_->blockSignals(false);

    postMessage(new FloatMsg(LightMaterial::setMaterialShininess_, currentShininess_));
    repaintCanvases();
}

} // namespace voreen
