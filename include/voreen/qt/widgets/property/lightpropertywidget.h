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

#ifndef VRN_LIGHTPROPTERYWIDGET_H
#define VRN_LIGHTPROPERTYWIDGET_H

#include <QWidget>
#include <math.h>

#include <tgt/vector.h>
#include <tgt/camera.h>

#include "voreen/qt/widgets/property/floatvec4propertywidget.h"
#include "voreen/qt/widgets/lightwidget.h"

class FloatVec4Property;

namespace voreen {
    class LightPropertyWidget : public QPropertyWidget {
Q_OBJECT
public:
    LightPropertyWidget(FloatVec4Property*, QWidget* parent = 0);

protected:
    tgt::mat4 viewMatrix_;
    FloatVec4Property* property_;
    tgt::Vector4f vector_;
    tgt::Camera camera_;
    int hemisphere_;
    LightWidget* light_;
    tgt::Camera getCamera();
    void cameraUpdate();    // on property change of the cameraproperty this is invoked by calling
                            // a member action
    void updateFromProperty();

protected slots:
    void changeWidgetLight(tgt::vec4);

signals:
    void changeLightWidget(tgt::vec4);

private:
    FloatVec4PropertyWidget* floatVec4_;
};

} // namespace voreen

#endif // VRN_LIGHTPROPTERYWIDGET_H
