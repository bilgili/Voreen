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

#ifndef VRN_PROPERTYWIDGETFACTORY_H
#define VRN_PROPERTYWIDGETFACTORY_H

#include "voreen/core/vis/properties/properties_decl.h"

namespace voreen {

/**
 * A PropertyWidgetFactory creates Widgets for every Property.
 * See Property::createWidget(PropertyWidgetFactory* f) for how it is used.
 */
class PropertyWidgetFactory {
public:
    virtual ~PropertyWidgetFactory() {}
    virtual PropertyWidget* createWidget(FloatProp* p) = 0;
    virtual PropertyWidget* createWidget(IntProp* p) = 0;
    virtual PropertyWidget* createWidget(BoolProp* p) = 0;
    virtual PropertyWidget* createWidget(StringProp* p) = 0;
    virtual PropertyWidget* createWidget(StringSelectionProp* p) = 0;
    virtual PropertyWidget* createWidget(StringVectorProp* p) = 0;
    virtual PropertyWidget* createWidget(ColorProp* p) = 0;
    virtual PropertyWidget* createWidget(EnumProp* p) = 0;
    virtual PropertyWidget* createWidget(FileDialogProp* p) = 0;
    virtual PropertyWidget* createWidget(TransFuncProp* p) = 0;
    virtual PropertyWidget* createWidget(FloatVec2Prop* p) = 0;
    virtual PropertyWidget* createWidget(FloatVec3Prop* p) = 0;
    virtual PropertyWidget* createWidget(FloatVec4Prop* p) = 0;
    virtual PropertyWidget* createWidget(IntVec2Prop* p) = 0;
    virtual PropertyWidget* createWidget(IntVec3Prop* p) = 0;
    virtual PropertyWidget* createWidget(IntVec4Prop* p) = 0;
    virtual PropertyWidget* createWidget(OptionPropertyBase* p) = 0;
};

} // namespace

#endif // VRN_PROPERTYWIDGETFACTORY_H
