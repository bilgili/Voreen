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

#include "voreen/qt/widgets/property/lightpropertywidget.h"

#include "voreen/core/processors/volumeraycaster.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties//property.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/qt/widgets/lightwidget.h"
#include "voreen/core/datastructures/meta/primitivemetadata.h"

#include <QCheckBox>

namespace voreen {

LightPropertyWidget::LightPropertyWidget(FloatVec4Property* prop, QWidget* parent)
    : QPropertyWidget(prop, parent)
    , property_(prop)
    , noUpdateFromProp_(false)
{
    light_ = new LightWidget(this);
    followCam_ = new QCheckBox(tr("Follow Camera"), this);
    if (BoolMetaData* follow = dynamic_cast<BoolMetaData*>(property_->getMetaDataContainer().getMetaData("FollowCamera")))
        followCam_->setChecked(follow->getValue());
    else
        followCam_->setChecked(false);

    layout_->setDirection(QBoxLayout::TopToBottom);
    layout_->addWidget(light_);
    layout_->addWidget(followCam_);
    connect(light_, SIGNAL(lightWidgetChanged(tgt::vec4)), this, SLOT(changeWidgetLight(tgt::vec4)));

    CameraProperty* camProp = getCamera();
    if(camProp) {
        camProp->onChange(CallMemberAction<LightPropertyWidget>(this, &LightPropertyWidget::cameraUpdate));
        light_->setLightPosition(camProp->get().getViewMatrix().getRotationalPart() * property_->get());
        cameraUpdate();
    }

    addVisibilityControls();
    // we have to add this widget to the property to enable automatic metadata serialization
    property_->addWidget(this);
}

CameraProperty* LightPropertyWidget::getCamera() {
    PropertyOwner* propOwner = property_->getOwner();  // this should not be done on any change but for some reason the roation matrix is invalid when the camera is moved!

    if (dynamic_cast<VolumeRenderer*>(propOwner)) {
        VolumeRenderer* vol = dynamic_cast<VolumeRenderer*>(propOwner);
        std::vector<Property*> props = vol->getProperties();
        std::vector<Property*>::iterator it = props.begin();
        while (it != props.end()) {
            if(CameraProperty* camProp = dynamic_cast<CameraProperty*>(*it))
                return camProp;
            ++it;
        }
    }
    return 0;
}

void LightPropertyWidget::changeWidgetLight(tgt::vec4 lightPos) {
    const CameraProperty* camProp = getCamera();
    if(!camProp)
        return;

    noUpdateFromProp_ = true;

    const tgt::Camera& camera = camProp->get();

    //hackish, but we don't want an additional updateFromProperty of the lightwidget which caused the call of this function in the first place and which is already updated FL
    property_->set(camera.getViewMatrixInverse().getRotationalPart() * lightPos);
    noUpdateFromProp_ = false;
}

void LightPropertyWidget::cameraUpdate() {
    const CameraProperty* camProp = getCamera();
    if(!camProp)
        return;

    float maxVal = camProp->getMaxValue() / 50.f;
    if(maxVal != property_->getMaxValue().x) {
        float oldRelDist = length(property_->get().xyz()) / property_->getMaxValue().x;
        light_->setMinDist(maxVal * 0.1f);
        light_->setMaxDist(maxVal);
        property_->setMinValue(tgt::vec4(-maxVal));
        property_->setMaxValue(tgt::vec4(maxVal));
        property_->set(property_->get() * oldRelDist);
    }

    const tgt::Camera& camera = camProp->get();
    if (followCam_->isChecked()) {
        noUpdateFromProp_ = true;
        property_->set(camera.getViewMatrixInverse().getRotationalPart() * light_->getLightPosition());
        noUpdateFromProp_ = false;
    }
    else
        light_->setLightPosition(camera.getViewMatrix().getRotationalPart() * property_->get());
}

void LightPropertyWidget::updateFromProperty() {
    if (noUpdateFromProp_)
        return;
    else
        light_->setLightPosition(property_->get());
}

void LightPropertyWidget::updateMetaData() const {
    BoolMetaData* follow = new BoolMetaData();
    follow->setValue(followCam_->isChecked());
    property_->getMetaDataContainer().addMetaData("FollowCamera", follow);
}

} // namespace voreen
