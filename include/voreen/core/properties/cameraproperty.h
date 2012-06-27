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

#ifndef VRN_CAMERAPROPERTY_H
#define VRN_CAMERAPROPERTY_H

#include "voreen/core/properties/templateproperty.h"
#include "voreen/core/properties/condition.h"

namespace tgt {
    class Camera;
}

namespace voreen {

/**
 * Property encapsulating a tgt::Camera object.
 */
class CameraProperty : public TemplateProperty<tgt::Camera*> {

public:

    /**
     * Constructor.
     *
     * @param adjustProjectionToViewport when set to true, the camera's projection matrix
     *        is adjusted automatically to viewport changes. This is especially necessary to
     *        reflect the viewport's aspect ratio.
     */
    CameraProperty(const std::string& id, const std::string& guiText, tgt::Camera* const value,
               bool adjustProjectionToViewport = true,
               Processor::InvalidationLevel invalidationLevel=Processor::INVALID_RESULT);

    virtual ~CameraProperty();

    /**
     * Assigns the passed camera object to the property.
     */
    void set(const tgt::Camera& camera);

    /**
     * When set to true, the camera's projection matrix is adjusted automatically
     * to viewport changes. This is especially necessary to
     * reflect the viewport's aspect ratio.
     */
    void setAdjustProjectionToViewport(bool adjust);

    /// \sa setAdjustProjectionToViewport
    bool getAdjustProjectionToViewport() const;

    /**
     * Executes all member actions that belong to the property. Generally the owner of the
     * property is invalidated.
     */
    void notifyChange();

    /**
     * Notifies the CameraProperty that the viewport of the canvas it is (indirectly) associated with,
     * has changed. This causes the camera's projection matrix to be updated,
     * if the according constructor parameter has been set.
     */
    void viewportChanged(const tgt::ivec2& viewport);

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);

    /**
     * @see Property::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Property::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

private:
    bool adjustProjectionToViewport_;
};

} // namespace voreen

#endif // VRN_CAMERAPROPERTY_H
