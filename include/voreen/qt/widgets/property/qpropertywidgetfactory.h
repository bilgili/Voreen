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

#ifndef VRN_QPROPERTYWIDGETFACTORY_H
#define VRN_QPROPERTYWIDGETFACTORY_H

#include "voreen/core/properties/property.h"
#include "voreen/core/properties/propertywidget.h"
#include "voreen/core/properties/allproperties.h"
#include "voreen/qt/widgets/property/qpropertywidget.h"

namespace voreen {

/**
 * Creates Qt PropertyWidgets
 */
class QPropertyWidgetFactory : public PropertyWidgetFactory {
public:
    QPropertyWidget* createWidget(BoolProperty* p);
    QPropertyWidget* createWidget(ButtonProperty* p);
    QPropertyWidget* createWidget(CameraProperty* p);
    QPropertyWidget* createWidget(ColorMapProperty* p);
    QPropertyWidget* createWidget(FileDialogProperty* p);
    QPropertyWidget* createWidget(FloatProperty* p);
    QPropertyWidget* createWidget(FloatVec2Property* p);
    QPropertyWidget* createWidget(FloatVec3Property* p);
    QPropertyWidget* createWidget(FloatVec4Property* p);
    QPropertyWidget* createWidget(FontProperty* p);
    QPropertyWidget* createWidget(IntProperty* p);
    QPropertyWidget* createWidget(IntVec2Property* p);
    QPropertyWidget* createWidget(IntVec3Property* p);
    QPropertyWidget* createWidget(IntVec4Property* p);
    QPropertyWidget* createWidget(FloatMat2Property* p);
    QPropertyWidget* createWidget(FloatMat3Property* p);
    QPropertyWidget* createWidget(FloatMat4Property* p);
    QPropertyWidget* createWidget(PlotEntitiesProperty* p);
    QPropertyWidget* createWidget(OptionPropertyBase* p);
    QPropertyWidget* createWidget(PlotPredicateProperty* p);
    QPropertyWidget* createWidget(PlotDataProperty* p);
    QPropertyWidget* createWidget(PlotSelectionProperty* p);
    QPropertyWidget* createWidget(PropertyVector* p);
    QPropertyWidget* createWidget(ShaderProperty* p);
    QPropertyWidget* createWidget(StringProperty* p);
    QPropertyWidget* createWidget(TransFuncProperty* p);
    QPropertyWidget* createWidget(VolumeCollectionProperty* p);
    QPropertyWidget* createWidget(VolumeHandleProperty* p);

    // deprecated
    QPropertyWidget* createWidget(ColorProperty* p);
    QPropertyWidget* createWidget(LightProperty* p);
};

} // namespace

#endif // VRN_QPROPERTYWIDGETFACTORY_H
