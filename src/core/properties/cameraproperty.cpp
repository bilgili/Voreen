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

#include "voreen/core/interaction/voreentrackball.h"
#include "voreen/core/properties/cameraproperty.h"

#include "tgt/camera.h"

namespace voreen {

using tgt::Camera;

CameraProperty::CameraProperty(const std::string& id, const std::string& guiText,
                       tgt::Camera const value, bool adjustProjectionToViewport,
                       float maxValue,
                       int invalidationLevel)
        : TemplateProperty<Camera>(id, guiText, value, invalidationLevel)
        , track_(0)
        , maxValue_(maxValue)
{}

CameraProperty::CameraProperty() : track_(0) {
}

CameraProperty::~CameraProperty() {
    delete track_;
}

Property* CameraProperty::create() const {
    return new CameraProperty();
}

void CameraProperty::set(const tgt::Camera& camera) {
    value_ = Camera(camera);
    invalidate();
}

void CameraProperty::setPosition(const tgt::vec3& pos) {
    value_.setPosition(pos);
    notifyChange();
}

void CameraProperty::setFocus(const tgt::vec3& focus) {
    value_.setFocus(focus);
    notifyChange();
}

void CameraProperty::setUpVector(const tgt::vec3& up) {
    value_.setUpVector(up);
    notifyChange();
}

void CameraProperty::setFrustum(const tgt::Frustum& frust) {
    value_.setFrustum(frust);
    notifyChange();
}

void CameraProperty::setFarDist(float dist) {
    value_.setFarDist(dist);
    notifyChange();
}

void CameraProperty::setNearDist(float dist) {
    value_.setNearDist(dist);
    notifyChange();
}

bool CameraProperty::setStereoEyeSeparation(float separation) {
    bool result = value_.setStereoEyeSeparation(separation);
    if(result)
        notifyChange();
    return result;
}

bool CameraProperty::setStereoEyeMode(tgt::Camera::StereoEyeMode mode) {
    bool result = value_.setStereoEyeMode(mode);
    if(result)
        notifyChange();
    return result;
}

//bool CameraProperty::stereoShift(tgt::vec3 shift) {
    //bool result = value_.stereoShift(shift);
    //if(result)
        //notifyChange();
    //return result;
//}

void CameraProperty::setStereoAxisMode(tgt::Camera::StereoAxisMode mode) {
    value_.setStereoAxisMode(mode);
}

void CameraProperty::setMaxValue(float val) {
    maxValue_ = val;
    notifyChange();
}

float CameraProperty::getMaxValue() const {
    return maxValue_;
}

void CameraProperty::notifyChange() {
    // execute links
    executeLinks();

    // check if conditions are met and exec actions
    for (size_t j = 0; j < conditions_.size(); ++j)
        conditions_[j]->exec();

    updateWidgets();

    // invalidate owner:
    invalidateOwner();
}

void CameraProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("projectionMode", (int)value_.getProjectionMode());

    s.serialize("position", value_.getPosition());
    s.serialize("focus", value_.getFocus());
    s.serialize("upVector", value_.getUpVector());

    s.serialize("frustLeft", value_.getFrustLeft());
    s.serialize("frustRight", value_.getFrustRight());
    s.serialize("frustBottom", value_.getFrustBottom());
    s.serialize("frustTop", value_.getFrustTop());
    s.serialize("frustNear", value_.getNearDist());
    s.serialize("frustFar", value_.getFarDist());

    s.serialize("fovy", value_.getFovy());

    s.serialize("eyeMode", (int)value_.getStereoEyeMode());
    s.serialize("eyeSeparation", value_.getStereoEyeSeparation());
    s.serialize("axisMode", (int)value_.getStereoAxisMode());
}

void CameraProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    try {
        float left, right, bottom, top, nearP, farP;
        s.deserialize("frustLeft", left);
        s.deserialize("frustRight", right);
        s.deserialize("frustBottom", bottom);
        s.deserialize("frustTop", top);
        s.deserialize("frustNear", nearP);
        s.deserialize("frustFar", farP);
        value_.setFrustum(tgt::Frustum(left, right, bottom, top, nearP, farP));
    } catch(SerializationException&) {
        s.removeLastError();
    }

    int projMode;
    try {
        s.deserialize("projectionMode", projMode);
    } catch(SerializationException&) {
        s.removeLastError();
        projMode = tgt::Camera::PERSPECTIVE;
    }

    value_.setProjectionMode((tgt::Camera::ProjectionMode)projMode);

    tgt::vec3 vector;
    s.deserialize("position", vector);
    value_.setPosition(vector);
    s.deserialize("focus", vector);
    value_.setFocus(vector);
    s.deserialize("upVector", vector);
    value_.setUpVector(vector);

    try {
        float fovy;
        s.deserialize("fovy", fovy);
        value_.setFovy(fovy);
    }
    catch(SerializationException&) {
        s.removeLastError();
    }

    try {
        int eyeMode, axisMode;
        float eyeSep;
        s.deserialize("eyeMode", eyeMode);
        s.deserialize("eyeSeparation", eyeSep);
        s.deserialize("axisMode", axisMode);
        value_.setStereoEyeMode((tgt::Camera::StereoEyeMode)eyeMode, false);
        value_.setStereoEyeSeparation(eyeSep, false);
        value_.setStereoAxisMode((tgt::Camera::StereoAxisMode)axisMode);
    }
    catch(SerializationException&) {
        s.removeLastError();
    }
}

void CameraProperty::look(tgt::ivec2 viewportSize) {
    value_.look(viewportSize);
}

VoreenTrackball* CameraProperty::getTrackball() {
    if(!track_)
        track_ = new VoreenTrackball(this);
    return track_;
}

} // namespace voreen
