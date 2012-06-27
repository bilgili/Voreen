/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#ifndef VRN_COMPACTPROPERTYWIDGETFACTORY_H
#define VRN_COMPACTPROPERTYWIDGETFACTORY_H

#include "voreen/core/vis/properties/property.h"
#include "voreen/core/vis/propertywidget.h"
#include "voreen/core/vis/properties/allproperties.h"

namespace voreen {

/**
 * Creates compact PropertyWidgets (used in VoreenVE)
 */
class CompactPropertyWidgetFactory : public PropertyWidgetFactory {
public:
    PropertyWidget* createWidget(FloatProp* p);
    PropertyWidget* createWidget(IntProp* p);
    PropertyWidget* createWidget(BoolProp* p);
    PropertyWidget* createWidget(StringProp* p);
    PropertyWidget* createWidget(StringVectorProp* p);
    PropertyWidget* createWidget(StringSelectionProp* p);
    PropertyWidget* createWidget(ColorProp* p);
    PropertyWidget* createWidget(EnumProp* p);
    PropertyWidget* createWidget(FileDialogProp* p);
    PropertyWidget* createWidget(TransFuncProp* p);
    PropertyWidget* createWidget(FloatVec2Prop* p);
    PropertyWidget* createWidget(FloatVec3Prop* p);
    PropertyWidget* createWidget(FloatVec4Prop* p);
    PropertyWidget* createWidget(IntVec2Prop* p);
    PropertyWidget* createWidget(IntVec3Prop* p);
    PropertyWidget* createWidget(IntVec4Prop* p);
    PropertyWidget* createWidget(OptionPropertyBase* p);
};

} // namespace

#endif // VRN_COMPACTPROPERTYWIDGETFACTORY_H
