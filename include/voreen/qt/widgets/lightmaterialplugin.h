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

#ifndef LIGHTMATERIALPLUGIN_H
#define LIGHTMATERIALPLUGIN_H

#include "widgetplugin.h"

#include <QLabel>
#include <QComboBox>
#include <QBasicTimer>
#include <QPushButton>
#include <QMenu>
#include <QCheckBox>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QIcon>

namespace voreen {

class LightMaterialPlugin : public WidgetPlugin {
    Q_OBJECT

    public:
        LightMaterialPlugin(QWidget* parent = 0, MessageReceiver* msgReceiver = 0);
        virtual ~LightMaterialPlugin() {
        }

        /**
         * This enumeration is used to define which light features are presented to the user for
         * modification. By default all features are enabled.
         */
        enum Features {
            LIGHTING_MODEL      = 1,        ///< Should the lighting model box be shown?
            LIGHT               = 2,        ///< Should the light box be shown?
            MATERIAL            = 4,        ///< Should the material box be shown?
            ATTENUATION         = 8,        ///< Should the attenuation box be shown?
            LIGHT_AMBIENT       = 16,       ///< Show ambient light color?
            LIGHT_DIFFUSE       = 32,       ///< Show diffuse light color?
            LIGHT_SPECULAR      = 64,       ///< Show specular light color?
            MATERIAL_USE_OPENGL_MATERIAL = 128,    ///< Show checkbox "Use OpenGL material"?
            MATERIAL_AMBIENT    = 256,      ///< Show ambient material color?
            MATERIAL_DIFFUSE    = 512,      ///< Show diffuse material color?
            MATERIAL_SPECULAR   = 1024,     ///< Show specular material color and shininess?
            MATERIAL_EMISSION   = 2048,     ///< Show emission material color?
            ALL_FEATURES        = 4095      ///< Show everything
        };

        void initGLState();
        /// Enable features by passing a bitstring. \see Features
        void enableFeatures(int features);
        /// Disable features by passing a bitstring. \see Features
        void disableFeatures(int features);

    protected:
        virtual void createWidgets();
        virtual void createConnections();
        /// Returns wether a certain feature is enabled.
        bool isFeatureEnabled(Features feature);

        /// Determines which features are presented to the user for modification.
        int features_;

    public slots:

        // lighting model settings
        void setAddAmbient(bool checked);
        void setAddSpecular(bool checked);
        void setApplyAttenuation(bool checked);

        // light settings
        void updateLightSpecularColor(int value);
        void updateLightAmbientColor(int value);
        void updateLightDiffuseColor(int value);
        void showLightAmbientColor();
        void showLightDiffuseColor();
        void showLightSpecularColor();
        void updateConstantAttenuationSlider();
        void updateLinearAttenuationSlider();
        void updateQuadraticAttenuationSlider();
        void updateConstantAttenuationSpin(double value, bool updateGLState = true);
        void updateLinearAttenuationSpin(double value, bool updateGLState = true);
        void updateQuadraticAttenuationSpin(double value, bool updateGLState = true);

        // material settings
        void updateMaterialSpecularColor();
        void updateMaterialAmbientColor();
        void updateMaterialDiffuseColor();
        void updateMaterialEmissionColor();
        void setUseOGLMaterial(bool b);
        void showMaterialAmbientColor();
        void showMaterialDiffuseColor();
        void showMaterialSpecularColor();
        void showMaterialEmissionColor();
        void updateShininessSlider(int value);
        void updateShininessSpin(double value);

  private:

        // lighting model settings
        QGroupBox *groupLightingModel_;
        QCheckBox *checkAddAmbient_;
        QCheckBox *checkAddSpecular_;
        QCheckBox *checkApplyAttenuation_;

        bool addAmbient_;
        bool addSpecular_;
        bool applyAttenuation_;

        // light settings
        QGroupBox *groupLightColors_;
        QGroupBox *groupAttenuation_;
        QLabel *laOrientation_;
        QCheckBox* checkUseOpenGLMaterial_;
        QLabel *laShowLightSpecular_;
        QLabel *laShowLightAmbient_;
        QLabel *laShowLightDiffuse_;
        QLabel *laLightSpecular_;
        QLabel *laLightAmbient_;
        QLabel *laLightDiffuse_;
        QSlider *sliderLightSpecular_;
        QSlider *sliderLightDiffuse_;
        QSlider *sliderLightAmbient_;

        QSlider *sliderConstantAttenuation_;
        QSlider *sliderLinearAttenuation_;
        QSlider *sliderQuadraticAttenuation_;
        QDoubleSpinBox *spinConstantAttenuation_;
        QDoubleSpinBox *spinLinearAttenuation_;
        QDoubleSpinBox *spinQuadraticAttenuation_;

        QIcon *specIcon_;
        QIcon *ambIcon_;
        QIcon *diffIcon_;

        bool useOpenGLMaterial_;
        float currentLightAmbient_;
        float currentLightSpecular_;
        float currentLightDiffuse_;

        tgt::vec2 constantAttenuationRange_;
        tgt::vec2 linearAttenuationRange_;
        tgt::vec2 quadraticAttenuationRange_;
        float currentConstantAttenuation_;
        float currentLinearAttenuation_;
        float currentQuadraticAttenuation_;
        int sliderSteps_;


        // material settings
        QGroupBox *groupMaterialParams_;
        QPushButton *getMaterialSpecular_;
        QPushButton *getMaterialAmbient_;
        QPushButton *getMaterialDiffuse_;
        QPushButton *getMaterialEmission_;
        QLabel *laShowMaterialSpecular_;
        QLabel *laShowMaterialAmbient_;
        QLabel *laShowMaterialDiffuse_;
        QLabel *laShowMaterialEmission_;
        QLabel *laShininess_;

        QSlider *sliderShininess_;
        QDoubleSpinBox *spinShininess_;

        tgt::Color currentMaterialAmbient_;
        tgt::Color currentMaterialSpecular_;
        tgt::Color currentMaterialDiffuse_;
        tgt::Color currentMaterialEmission_;
        float currentShininess_;

};

} // namespace voreen

#endif
