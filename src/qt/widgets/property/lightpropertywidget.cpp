/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/core/processors/volumeraycaster.h"

#include "voreen/qt/widgets/property/floatvec4propertywidget.h"
#include "voreen/qt/widgets/property/lightpropertywidget.h"
#include "voreen/qt/widgets/sliderspinboxwidget.h"

#include "voreen/qt/widgets/property/floatmat4propertywidget.h"
#include "voreen/core/properties/matrixproperty.h"
#include "voreen/core/properties/cameraproperty.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QSlider>
#include <QStyle>
#include <QTabWidget>
#include <QWidget>

namespace voreen {

LightPropertyWidget::LightPropertyWidget(FloatVec4Property* prop, QWidget* parent)
    : QPropertyWidget(prop, parent)
    , property_(prop)
    , hemisphere_(1)
{
    light_ = new LightWidget(this);
    vector_ = tgt::Vector4f::zero;
    layout_->addWidget(light_);
    connect(light_, SIGNAL(lightWidgetChanged(tgt::vec4)), this, SLOT(changeWidgetLight(tgt::vec4)));
    PropertyOwner* propOwner = prop->getOwner();
    if(dynamic_cast<VolumeRaycaster*>(propOwner)) {
        VolumeRaycaster* vol = dynamic_cast<VolumeRaycaster*>(propOwner);
        std::vector<Property*> props = vol->getProperties();
        std::vector<Property*>::iterator it = props.begin();
        while(it != props.end()){
            if(dynamic_cast<CameraProperty*>(*it)) {
                camera_ = (dynamic_cast<CameraProperty*>(*it)->get());
                dynamic_cast<CameraProperty*>(*it)->onChange(CallMemberAction<LightPropertyWidget>(this, &LightPropertyWidget::cameraUpdate));
            }
            ++it;
        }
    }
}
tgt::Camera LightPropertyWidget::getCamera() {
    PropertyOwner* propOwner = property_->getOwner();       // this should not be done on any change but for some reason the roation matrix is invalid when the camera is moved!
    tgt::Camera camera;
    if(dynamic_cast<VolumeRenderer*>(propOwner)) {
        VolumeRenderer* vol = dynamic_cast<VolumeRenderer*>(propOwner);
        std::vector<Property*> props = vol->getProperties();
        std::vector<Property*>::iterator it = props.begin();
        while(it != props.end()){
            if(dynamic_cast<CameraProperty*>(*it)) {
                camera = (dynamic_cast<CameraProperty*>(*it)->get());
            }
            ++it;
        }
    }
    return camera;
}

void LightPropertyWidget::changeWidgetLight(tgt::vec4 lightPos) {
    tgt::Camera camera = getCamera();

    viewMatrix_ = camera.getViewMatrix();
    tgt::mat4 viewMatrix = camera.getViewMatrixInverse();

    float x = viewMatrix.t00 * lightPos.x - viewMatrix.t01 * lightPos.y + viewMatrix.t02 * lightPos.z;
    float y = viewMatrix.t10 * lightPos.x - viewMatrix.t11 * lightPos.y + viewMatrix.t12 * lightPos.z;
    float z = viewMatrix.t20 * lightPos.x - viewMatrix.t21 * lightPos.y + viewMatrix.t22 * lightPos.z;
    vector_[0] = x;
    vector_[1] = y;
    vector_[2] = z;
    property_->set(vector_);
}

void LightPropertyWidget::cameraUpdate() {
    /*tgt::vec4 position;
    tgt::Camera camera = getCamera();
    if (!camera_)
        return;
    tgt::mat4 mat = camera->getViewMatrix() - viewMatrix_;
    tgt::vec4 pos;*/
    //pos.x = widgets_[0]->getValue() * 6;
    //pos.y = widgets_[1]->getValue() * 6;
    //pos.z = widgets_[2]->getValue();
    //pos.w = 0;

    //light_->setLightPosition(pos);
}

void LightPropertyWidget::updateFromProperty() {
    //FloatVec4PropertyWidget::updateFromProperty();
}

} // namespace voreen
