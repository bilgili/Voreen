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

    if (BoolMetaData* hemis = dynamic_cast<BoolMetaData*>(property_->getMetaDataContainer().getMetaData("UseBackHemisphere")))
        followCam_->setChecked(hemis->getValue());
    else
        followCam_->setChecked(false);

    layout_->setDirection(QBoxLayout::TopToBottom);
    layout_->addWidget(light_);
    layout_->addWidget(followCam_);
    connect(light_, SIGNAL(lightWidgetChanged(tgt::vec4)), this, SLOT(changeWidgetLight(tgt::vec4)));
    PropertyOwner* propOwner = prop->getOwner();
    VolumeRenderer* vol = dynamic_cast<VolumeRenderer*>(propOwner);
    if (vol) {
        std::vector<Property*> props = vol->getProperties();
        std::vector<Property*>::iterator it = props.begin();
        while (it != props.end()) {
            if (dynamic_cast<CameraProperty*>(*it))
                dynamic_cast<CameraProperty*>(*it)->onChange(CallMemberAction<LightPropertyWidget>(this, &LightPropertyWidget::cameraUpdate));

            ++it;
        }
    }

    light_->setLightPosition(getCamera().getViewMatrix().getRotationalPart() * property_->get());

    addVisibilityControls();
    // we have to add this widget to the property to enable automatic metadata serialization
    property_->addWidget(this);
}

tgt::Camera LightPropertyWidget::getCamera() {
    PropertyOwner* propOwner = property_->getOwner();  // this should not be done on any change but for some reason the roation matrix is invalid when the camera is moved!
    tgt::Camera camera;
    if (dynamic_cast<VolumeRenderer*>(propOwner)) {
        VolumeRenderer* vol = dynamic_cast<VolumeRenderer*>(propOwner);
        std::vector<Property*> props = vol->getProperties();
        std::vector<Property*>::iterator it = props.begin();
        while (it != props.end()){
            if(dynamic_cast<CameraProperty*>(*it)) {
                camera = (dynamic_cast<CameraProperty*>(*it)->get());
            }
            ++it;
        }
    }
    return camera;
}

void LightPropertyWidget::changeWidgetLight(tgt::vec4 lightPos) {
    const tgt::Camera& camera = getCamera();

    //hackish, but we don't want an additional updateFromProperty of the lightwidget which caused the call of this function in the first place and which is already updated FL
    noUpdateFromProp_ = true;
    property_->set(camera.getViewMatrixInverse().getRotationalPart() * lightPos);
    noUpdateFromProp_ = false;
}

void LightPropertyWidget::cameraUpdate() {
    const tgt::Camera& camera = getCamera();
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
    BoolMetaData* hemis = new BoolMetaData();
    hemis->setValue(light_->getHemisphereStatus());

    property_->getMetaDataContainer().addMetaData("FollowCamera", follow);
    property_->getMetaDataContainer().addMetaData("UseBackHemisphere", hemis);
}

} // namespace voreen
