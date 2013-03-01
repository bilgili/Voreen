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

#ifndef VRN_SHCLASS_H
#define VRN_SHCLASS_H

#include "shcoefftrans.h"
#include "shlightfunc.h"

#include "tgt/navigation/trackball.h"
#include "tgt/event/mouseevent.h"

#include "voreen/core/processors/volumeraycaster.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/filedialogproperty.h"

#include "voreen/core/interaction/trackballnavigation.h"
#include "voreen/core/interaction/interactionhandler.h"

namespace voreen {

class LightInteractionHandler;

class SHClass {

public:

    SHClass(VolumeRaycaster* rc, TransFuncProperty* tf);

    virtual ~SHClass();

    void initialize() throw (tgt::Exception);

    std::string getShaderDefines() const;
    void setRCShaderUniforms(tgt::Shader* rcProg);

    LightInteractionHandler* getLightHandler() const {
        return lightHandler_;
    }

    const std::vector<Property*>& getProps() const {
        return props_;
    }

    void initAndCalcCoeffs(const VolumeBase* volumeHandle);
    void onTransFuncChange();
    void onTransCoeffChange();
    void onLightFuncChange();
    void onNavigationChange();
    void updateLight(const tgt::mat4& diff);
    void setSHInteraction();

private:

    VolumeRaycaster* rc_;
    TransFuncProperty* tf_;

    const VolumeBase* currentVolume_;

    std::vector<Property*> props_;
    StringOptionProperty* bleedModeProp_;
    FloatProperty* coeffSizeFactor_;
    IntProperty* numSampleProp_;
    ButtonProperty* recalcProp_;
    FileDialogProperty* lpFilename_;
    BoolProperty* lightRotProp_;
    BoolProperty* interactionProp_;
    BoolProperty* considerNormalsProp_;
    BoolProperty* eriProp_;
    IntProperty* scaleProp_;

    SHCoeffTrans* transCalc_;

    tgt::Trackball* trackball_;
    LightInteractionHandler* lightHandler_;
    CameraProperty* cameraDummy_;

    bool interactiveMode_;
};

/**
 * Interaction handler for lightfunction-rotation.
 */
class LightInteractionHandler: public InteractionHandler {

public:
    LightInteractionHandler(SHClass* sh, CameraProperty* cam)
    : InteractionHandler("light-interaction", "Light Interaction")
    ,sh_(sh)
    {
        lightNavi_ = new TrackballNavigation(cam, TrackballNavigation::ROTATE_MODE, 0.05f);

        // event properties trackball
        rotateEvent_ = new EventProperty<LightInteractionHandler>("light-interaction.rotate", "Light Interaction Rotate", this,
            &LightInteractionHandler::rotateEvent,
            tgt::MouseEvent::MOUSE_BUTTON_LEFT,
            tgt::MouseEvent::ACTION_ALL,
            tgt::Event::CTRL, false, true);
        addEventProperty(rotateEvent_);
    }

    virtual ~LightInteractionHandler() {
        delete lightNavi_;
    }

    virtual std::string getClassName() const   { return "LightInteractionHandler"; }
    virtual InteractionHandler* create() const { return 0; }

    void rotateEvent(tgt::MouseEvent* e) {
        // propagate event to trackball navigation
        if (e->action() == tgt::MouseEvent::PRESSED)
            lightNavi_->mousePressEvent(e);
        else if (e->action() == tgt::MouseEvent::RELEASED)
            lightNavi_->mouseReleaseEvent(e);
        else if (e->action() == tgt::MouseEvent::MOTION) {
            lightNavi_->mouseMoveEvent(e);
            sh_->updateLight(generateMatrixFromQuat(lightNavi_->getTrackball()->getLastOrientationChange()).getRotationalPart());
        }
        e->accept();
    }

    // we don't need this, but it has to be declared, otherwise this class is abstract
    virtual void onEvent(tgt::Event* /*eve*/) {}

private:

    SHClass* sh_;
    TrackballNavigation* lightNavi_;
    EventProperty<LightInteractionHandler>* rotateEvent_;
};

} // namespace voreen
#endif // VRN_SHCLASS_H
