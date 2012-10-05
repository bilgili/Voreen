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

#ifndef VRN_LIGHTPROPTERYWIDGET_H
#define VRN_LIGHTPROPERTYWIDGET_H

#include "voreen/qt/widgets/property/qpropertywidget.h"
#include "voreen/core/properties/vectorproperty.h"
#include "tgt/camera.h"

class QCheckBox;
class QWidget;

namespace voreen {

class FloatVec4Property;
class FloatVec4PropertyWidget;
class LightWidget;
class CameraProperty;

class LightPropertyWidget : public QPropertyWidget {
Q_OBJECT
public:
    LightPropertyWidget(FloatVec4Property* prop, QWidget* parent = 0);

protected:
    FloatVec4Property* property_;
    LightWidget* light_;
    QCheckBox* followCam_;
    bool noUpdateFromProp_;

    CameraProperty* getCamera();
    void cameraUpdate();    // on property change of the cameraproperty this is invoked by calling
                            // a member action
    void updateFromProperty();
    //MetaDataBase* getWidgetMetaData() const;
    virtual void updateMetaData() const;

protected slots:
    void changeWidgetLight(tgt::vec4);

signals:
    void changeLightWidget(tgt::vec4);

private:
    FloatVec4PropertyWidget* floatVec4_;
};

} // namespace voreen

#endif // VRN_LIGHTPROPTERYWIDGET_H
