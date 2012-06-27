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

#include "voreen/core/properties/cameraproperty.h"

#include "voreen/core/properties/propertywidgetfactory.h"
#include "tgt/camera.h"

namespace voreen {

using tgt::Camera;

CameraProperty::CameraProperty(const std::string& id, const std::string& guiText,
                       tgt::Camera* const value, bool adjustProjectionToViewport,
                       Processor::InvalidationLevel invalidationLevel) :
        TemplateProperty<Camera*>(id, guiText, value, invalidationLevel)
{
    tgtAssert(value, "No camera");

    value->setNearDist(0.01f);
    value->setFovy(45.f);

    adjustProjectionToViewport_ = adjustProjectionToViewport;
}

CameraProperty::~CameraProperty() {
    delete value_;
    value_ = 0;
}

void CameraProperty::set(const tgt::Camera& camera) {
    if (!value_)
        return;

    if (value_->getPosition()   !=  camera.getPosition()   ||
        value_->getFocus()      !=  camera.getFocus()      ||
        value_->getUpVector()   !=  camera.getUpVector())
    {
            value_->positionCamera(camera.getPosition(), camera.getFocus(), camera.getUpVector());
            invalidate();
    }
}

void CameraProperty::setAdjustProjectionToViewport(bool adjust) {
    adjustProjectionToViewport_ = adjust;
}

bool CameraProperty::getAdjustProjectionToViewport() const {
    return adjustProjectionToViewport_;
}

void CameraProperty::notifyChange() {
    Camera* cam = value_;

    // execute links
    if (cam) {
        executeLinks(cam, cam);
    }

    // check if conditions are met and exec actions
    for (size_t j = 0; j < conditions_.size(); ++j)
        conditions_[j]->exec();

    updateWidgets();

    // invalidate owner:
    invalidateOwner();
}

void CameraProperty::viewportChanged(const tgt::ivec2& viewport) {

    if (adjustProjectionToViewport_) {
        value_->setRatio(static_cast<float>(viewport.x) / viewport.y);
        //value_->updateFrustum();
        invalidateOwner(invalidationLevel_);
    }

}

PropertyWidget* CameraProperty::createWidget(PropertyWidgetFactory* f)     {
    return f->createWidget(this);
}

void CameraProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    s.serialize("adjustProjectionToViewport", adjustProjectionToViewport_);

    s.serialize("position", value_->getPosition());
    s.serialize("focus", value_->getFocus());
    s.serialize("upVector", value_->getUpVector());
}

void CameraProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    s.deserialize("adjustProjectionToViewport", adjustProjectionToViewport_);

    tgt::vec3 vector;

    s.deserialize("position", vector);
    value_->setPosition(vector);
    s.deserialize("focus", vector);
    value_->setFocus(vector);
    s.deserialize("upVector", vector);
    value_->setUpVector(vector);
}

std::string CameraProperty::getTypeString() const {
    return "Camera";
}

} // namespace voreen
